/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "core/components_ng/pattern/ui_extension/session_wrapper_impl.h"

#include <memory>

#include "accessibility_event_info.h"
#include "session_manager/include/extension_session_manager.h"
#include "ui/rs_surface_node.h"

#include "adapter/ohos/entrance/ace_container.h"
#include "adapter/ohos/osal/want_wrap_ohos.h"
#include "base/utils/utils.h"

namespace OHOS::Ace::NG {

class UIExtensionLifecycleListener : public Rosen::ILifecycleListener {
public:
    UIExtensionLifecycleListener(int32_t instanceId, const WeakPtr<UIExtensionPattern>& hostPattern)
        : instanceId_(instanceId), hostPattern_(hostPattern)
    {}
    virtual ~UIExtensionLifecycleListener() = default;

    void OnActivation() override {}
    void OnForeground() override {}
    void OnBackground() override {}

    void OnConnect() override
    {
        ContainerScope scope(instanceId_);
        auto pipeline = PipelineBase::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto taskExecutor = pipeline->GetTaskExecutor();
        CHECK_NULL_VOID(taskExecutor);
        taskExecutor->PostTask(
            [weak = hostPattern_]() {
                auto pattern = weak.Upgrade();
                CHECK_NULL_VOID(pattern);
                pattern->OnConnect();
            },
            TaskExecutor::TaskType::UI);
    }

    void OnDisconnect() override
    {
        ContainerScope scope(instanceId_);
        auto pipeline = PipelineBase::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto taskExecutor = pipeline->GetTaskExecutor();
        CHECK_NULL_VOID(taskExecutor);
        taskExecutor->PostTask(
            [weak = hostPattern_]() {
                auto pattern = weak.Upgrade();
                CHECK_NULL_VOID(pattern);
                pattern->OnDisconnect();
            },
            TaskExecutor::TaskType::UI);
    }

    void OnExtensionDied() override
    {
        ContainerScope scope(instanceId_);
        auto pipeline = PipelineBase::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto taskExecutor = pipeline->GetTaskExecutor();
        CHECK_NULL_VOID(taskExecutor);
        taskExecutor->PostTask(
            [weak = hostPattern_]() {
                auto pattern = weak.Upgrade();
                CHECK_NULL_VOID(pattern);
                pattern->OnExtensionDied();
            },
            TaskExecutor::TaskType::UI);
    }

    void OnAccessibilityEvent(
        const Accessibility::AccessibilityEventInfo& info, int32_t uiExtensionOffset) override
    {
        ContainerScope scope(instanceId_);
        auto pipeline = PipelineBase::GetCurrentContext();
        CHECK_NULL_VOID(pipeline);
        auto taskExecutor = pipeline->GetTaskExecutor();
        CHECK_NULL_VOID(taskExecutor);
        taskExecutor->PostTask(
            [weak = hostPattern_, info, uiExtensionOffset]() {
                auto pattern = weak.Upgrade();
                CHECK_NULL_VOID(pattern);
                pattern->OnAccessibilityEvent(info, uiExtensionOffset);
            },
            TaskExecutor::TaskType::UI);
    }

private:
    int32_t instanceId_;
    WeakPtr<UIExtensionPattern> hostPattern_;
};

/************************************************ Begin: Initialization ***********************************************/
SessionWrapperImpl::SessionWrapperImpl(
    const WeakPtr<UIExtensionPattern>& hostPattern, int32_t instanceId, bool isTransferringCaller)
    : hostPattern_(hostPattern), instanceId_(instanceId), isTransferringCaller_(isTransferringCaller)
{}

SessionWrapperImpl::~SessionWrapperImpl() {}

void SessionWrapperImpl::InitAllCallback()
{
    CHECK_NULL_VOID(session_);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto taskExecutor = pipeline->GetTaskExecutor();
    CHECK_NULL_VOID(taskExecutor);
    auto sessionCallbacks = session_->GetExtensionSessionEventCallback();

    foregroundCallback_ = [weak = hostPattern_, taskExecutor](OHOS::Rosen::WSError errcode) {
        if (errcode != OHOS::Rosen::WSError::WS_OK) {
            taskExecutor->PostTask(
                [weak, errcode] {
                    auto pattern = weak.Upgrade();
                    CHECK_NULL_VOID(pattern);
                    std::string name = "start_ability_fail";
                    std::string message =
                        "Start ui extension ability failed, please check the want of UIextensionAbility.";
                    pattern->FireOnErrorCallback(static_cast<int32_t>(errcode), name, message);
                },
                TaskExecutor::TaskType::UI);
        }
    };
    backgroundCallback_ = [weak = hostPattern_, taskExecutor](OHOS::Rosen::WSError errcode) {
        if (errcode != OHOS::Rosen::WSError::WS_OK) {
            taskExecutor->PostTask(
                [weak, errcode] {
                    auto pattern = weak.Upgrade();
                    CHECK_NULL_VOID(pattern);
                    std::string name = "background_fail";
                    std::string message = "background ui extension ability failed, please check AMS log.";
                    pattern->FireOnErrorCallback(static_cast<int32_t>(errcode), name, message);
                },
                TaskExecutor::TaskType::UI);
        }
    };
    destructionCallback_ = [weak = hostPattern_, taskExecutor](OHOS::Rosen::WSError errcode) {
        if (errcode != OHOS::Rosen::WSError::WS_OK) {
            taskExecutor->PostTask(
                [weak, errcode] {
                    auto pattern = weak.Upgrade();
                    CHECK_NULL_VOID(pattern);
                    std::string name = "terminate_fail";
                    std::string message = "terminate ui extension ability failed, please check AMS log.";
                    pattern->FireOnErrorCallback(static_cast<int32_t>(errcode), name, message);
                },
                TaskExecutor::TaskType::UI);
        }
    };
    sessionCallbacks->transferAbilityResultFunc_ = [weak = hostPattern_, taskExecutor](
                                                       int32_t code, const AAFwk::Want& want) {
        taskExecutor->PostTask(
            [weak, code, want]() {
                auto pattern = weak.Upgrade();
                CHECK_NULL_VOID(pattern);
                pattern->FireOnResultCallback(code, want);
            },
            TaskExecutor::TaskType::UI);
    };
    sessionCallbacks->transferExtensionDataFunc_ = [weak = hostPattern_, taskExecutor](
                                                       const AAFwk::WantParams& params) {
        taskExecutor->PostTask(
            [weak, params]() {
                auto pattern = weak.Upgrade();
                CHECK_NULL_VOID(pattern);
                pattern->FireOnReceiveCallback(params);
            },
            TaskExecutor::TaskType::UI);
    };
    sessionCallbacks->notifyRemoteReadyFunc_ = [weak = hostPattern_, taskExecutor]() {
        taskExecutor->PostTask(
            [weak]() {
                auto pattern = weak.Upgrade();
                CHECK_NULL_VOID(pattern);
                pattern->FireOnRemoteReadyCallback();
            },
            TaskExecutor::TaskType::UI);
    };
    sessionCallbacks->notifySyncOnFunc_ = [weak = hostPattern_, taskExecutor]() {
        taskExecutor->PostTask(
            [weak]() {
                auto pattern = weak.Upgrade();
                CHECK_NULL_VOID(pattern);
                pattern->FireSyncCallbacks();
            },
            TaskExecutor::TaskType::UI);
    };
    sessionCallbacks->notifyAsyncOnFunc_ = [weak = hostPattern_, taskExecutor]() {
        taskExecutor->PostTask(
            [weak]() {
                auto pattern = weak.Upgrade();
                CHECK_NULL_VOID(pattern);
                pattern->FireAsyncCallbacks();
            },
            TaskExecutor::TaskType::UI);
    };
}
/************************************************ End: Initialization *************************************************/

/************************************************ Begin: About session ************************************************/
void SessionWrapperImpl::CreateSession(const AAFwk::Want& want)
{
    TAG_LOGI(AceLogTag::ACE_UIEXTENSIONCOMPONENT, "Create session: %{private}s", want.ToString().c_str());
    auto container = AceType::DynamicCast<Platform::AceContainer>(Container::Current());
    CHECK_NULL_VOID(container);
    auto callerToken = container->GetToken();
    auto parentToken = container->GetParentToken();
    Rosen::SessionInfo extensionSessionInfo = {
        .bundleName_ = want.GetElement().GetBundleName(),
        .abilityName_ = want.GetElement().GetAbilityName(),
        .callerToken_ = callerToken,
        .rootToken_ = (isTransferringCaller_ && parentToken) ? parentToken : callerToken,
        .want = std::make_shared<Want>(want),
    };
    session_ = Rosen::ExtensionSessionManager::GetInstance().RequestExtensionSession(extensionSessionInfo);
    CHECK_NULL_VOID(session_);
    lifecycleListener_ = std::make_shared<UIExtensionLifecycleListener>(instanceId_, hostPattern_);
    session_->RegisterLifecycleListener(lifecycleListener_);
    InitAllCallback();
}

void SessionWrapperImpl::DestroySession()
{
    CHECK_NULL_VOID(session_);
    session_->UnregisterLifecycleListener(lifecycleListener_);
    session_ = nullptr;
}

bool SessionWrapperImpl::IsSessionValid()
{
    return session_ != nullptr;
}

int32_t SessionWrapperImpl::GetSessionId()
{
    return session_ ? session_->GetPersistentId() : 0;
}

const std::shared_ptr<AAFwk::Want> SessionWrapperImpl::GetWant()
{
    return session_ ? session_->GetSessionInfo().want : nullptr;
}
/************************************************ End: About session **************************************************/

/************************************************ Begin: Synchronous interface for event notify ***********************/
bool SessionWrapperImpl::NotifyFocusEventSync(bool isFocus)
{
    return false;
}
bool SessionWrapperImpl::NotifyFocusStateSync(bool focusState)
{
    return false;
}

bool SessionWrapperImpl::NotifyBackPressedSync()
{
    CHECK_NULL_RETURN(session_, false);
    bool isConsumed = false;
    session_->TransferBackPressedEventForConsumed(isConsumed);
    return isConsumed;
}

bool SessionWrapperImpl::NotifyPointerEventSync(const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent)
{
    return false;
}

bool SessionWrapperImpl::NotifyKeyEventSync(const std::shared_ptr<OHOS::MMI::KeyEvent>& keyEvent)
{
    CHECK_NULL_RETURN(session_, false);
    bool isConsumed = false;
    session_->TransferKeyEventForConsumed(keyEvent, isConsumed);
    return isConsumed;
}

bool SessionWrapperImpl::NotifyAxisEventSync(const std::shared_ptr<OHOS::MMI::AxisEvent>& axisEvent)
{
    return false;
}
/************************************************ End: Synchronous interface for event notify *************************/

/************************************************ Begin: Asynchronous interface for event notify **********************/
bool SessionWrapperImpl::NotifyFocusEventAsync(bool isFocus)
{
    CHECK_NULL_RETURN(session_, false);
    session_->TransferFocusActiveEvent(isFocus);
    return true;
}

bool SessionWrapperImpl::NotifyFocusStateAsync(bool focusState)
{
    CHECK_NULL_RETURN(session_, false);
    session_->TransferFocusStateEvent(focusState);
    return true;
}

bool SessionWrapperImpl::NotifyBackPressedAsync()
{
    return false;
}
bool SessionWrapperImpl::NotifyPointerEventAsync(const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent)
{
    if (session_ && pointerEvent) {
        session_->TransferPointerEvent(pointerEvent);
    }
    return false;
}
bool SessionWrapperImpl::NotifyKeyEventAsync(const std::shared_ptr<OHOS::MMI::KeyEvent>& keyEvent)
{
    if (session_ && keyEvent) {
        session_->TransferKeyEvent(keyEvent);
    }
    return false;
}
bool SessionWrapperImpl::NotifyAxisEventAsync(const std::shared_ptr<OHOS::MMI::AxisEvent>& axisEvent)
{
    return false;
}
/************************************************ End: Asynchronous interface for event notify ************************/

/************************************************ Begin: The lifecycle interface **************************************/
void SessionWrapperImpl::NotifyCreate() {}

void SessionWrapperImpl::NotifyForeground()
{
    TAG_LOGI(AceLogTag::ACE_UIEXTENSIONCOMPONENT, "Foreground: session = %{public}s", session_ ? "non-null" : "null");
    CHECK_NULL_VOID(session_);
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto hostWindowId = pipeline->GetFocusWindowId();
    Rosen::ExtensionSessionManager::GetInstance().RequestExtensionSessionActivation(
        session_, hostWindowId, std::move(foregroundCallback_));
}

void SessionWrapperImpl::NotifyBackground()
{
    TAG_LOGI(AceLogTag::ACE_UIEXTENSIONCOMPONENT, "Background: session = %{public}s", session_ ? "non-null" : "null");
    CHECK_NULL_VOID(session_);
    Rosen::ExtensionSessionManager::GetInstance().RequestExtensionSessionBackground(
        session_, std::move(backgroundCallback_));
}
void SessionWrapperImpl::NotifyDestroy()
{
    TAG_LOGI(AceLogTag::ACE_UIEXTENSIONCOMPONENT, "Destroy: session = %{public}s", session_ ? "non-null" : "null");
    CHECK_NULL_VOID(session_);
    Rosen::ExtensionSessionManager::GetInstance().RequestExtensionSessionDestruction(
        session_, std::move(destructionCallback_));
}

void SessionWrapperImpl::NotifyConfigurationUpdate() {}
/************************************************ End: The lifecycle interface ****************************************/

/************************************************ Begin: The interface about the accessibility ************************/
bool SessionWrapperImpl::TransferExecuteAction(
    int32_t elementId, const std::map<std::string, std::string>& actionArguments, int32_t action, int32_t offset)
{
    CHECK_NULL_RETURN(session_, false);
    return OHOS::Rosen::WSError::WS_OK == session_->TransferExecuteAction(elementId, actionArguments, action, offset);
}

void SessionWrapperImpl::SearchExtensionElementInfoByAccessibilityId(
    int32_t elementId, int32_t mode, int32_t baseParent, std::list<Accessibility::AccessibilityElementInfo>& output)
{
    CHECK_NULL_VOID(session_);
    session_->TransferSearchElementInfo(elementId, mode, baseParent, output);
}

void SessionWrapperImpl::SearchElementInfosByText(int32_t elementId, const std::string& text, int32_t baseParent,
    std::list<Accessibility::AccessibilityElementInfo>& output)
{
    CHECK_NULL_VOID(session_);
    session_->TransferSearchElementInfosByText(elementId, text, baseParent, output);
}

void SessionWrapperImpl::FindFocusedElementInfo(
    int32_t elementId, int32_t focusType, int32_t baseParent, Accessibility::AccessibilityElementInfo& output)
{
    CHECK_NULL_VOID(session_);
    session_->TransferFindFocusedElementInfo(elementId, focusType, baseParent, output);
}

void SessionWrapperImpl::FocusMoveSearch(
    int32_t elementId, int32_t direction, int32_t baseParent, Accessibility::AccessibilityElementInfo& output)
{
    CHECK_NULL_VOID(session_);
    session_->TransferFocusMoveSearch(elementId, direction, baseParent, output);
}
/************************************************ Begin: The interface about the accessibility ************************/

/************************************************ Begin: The interface to control the display area ********************/
std::shared_ptr<Rosen::RSSurfaceNode> SessionWrapperImpl::GetSurfaceNode() const
{
    return session_ ? session_->GetSurfaceNode() : nullptr;
}

void SessionWrapperImpl::RefreshDisplayArea(float left, float top, float width, float height)
{
    CHECK_NULL_VOID(session_);
    Rosen::WSRect windowRect {
        .posX_ = std::round(left), .posY_ = std::round(top), .width_ = std::round(width), .height_ = std::round(height)
    };
    session_->UpdateRect(windowRect, Rosen::SizeChangeReason::UNDEFINED);
}
/************************************************ End: The interface to control the display area **********************/

/************************************************ Begin: The interface to send the data for ArkTS *********************/
void SessionWrapperImpl::SendDataAsync(const AAFwk::WantParams& params) const
{
    TAG_LOGI(AceLogTag::ACE_UIEXTENSIONCOMPONENT, "Sync: session = %{public}s", session_ ? "non-null" : "null");
    CHECK_NULL_VOID(session_);
    session_->TransferComponentData(params);
}

int32_t SessionWrapperImpl::SendDataSync(const AAFwk::WantParams& wantParams, AAFwk::WantParams& reWantParams) const
{
    TAG_LOGI(AceLogTag::ACE_UIEXTENSIONCOMPONENT, "Async: session = %{public}s", session_ ? "non-null" : "null");
    Rosen::WSErrorCode transferCode = Rosen::WSErrorCode::WS_ERROR_TRANSFER_DATA_FAILED;
    if (session_) {
        transferCode = session_->TransferComponentDataSync(wantParams, reWantParams);
    }
    return static_cast<int32_t>(transferCode);
}
/************************************************ End: The interface to send the data for ArkTS ***********************/

} // namespace OHOS::Ace::NG
