/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_OVERLAY_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_OVERLAY_MANAGER_H

#include <cstdint>
#include <functional>
#include <stack>
#include <unordered_map>
#include <utility>

#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/utils/noncopyable.h"
#include "core/components/common/properties/placement.h"
#include "core/components/dialog/dialog_properties.h"
#include "core/components/picker/picker_data.h"
#include "core/components_ng/animation/geometry_transition.h"
#include "core/components_ng/base/ui_node.h"
#include "core/components_ng/pattern/calendar_picker/calendar_type_define.h"
#include "core/components_ng/pattern/overlay/modal_style.h"
#include "core/components_ng/pattern/overlay/sheet_style.h"
#include "core/components_ng/pattern/picker/datepicker_event_hub.h"
#include "core/components_ng/pattern/picker/picker_type_define.h"
#include "core/components_ng/pattern/text_picker/textpicker_event_hub.h"
#include "core/components_ng/pattern/toast/toast_layout_property.h"
#include "core/components_ng/pattern/toast/toast_view.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace {
struct ModalUIExtensionCallbacks;
} // namespace OHOS::Ace

namespace OHOS::AAFwk {
class Want;
} // namespace OHOS::AAFwk

namespace OHOS::Ace::NG {

struct PopupInfo {
    int32_t popupId = -1;
    WeakPtr<FrameNode> target;
    RefPtr<FrameNode> popupNode;
    bool markNeedUpdate = false;
    bool isCurrentOnShow = false;
    bool isBlockEvent = true;
    SizeF targetSize;
    OffsetF targetOffset;
};

// StageManager is the base class for root render node to perform page switch.
class ACE_EXPORT OverlayManager : public virtual AceType {
    DECLARE_ACE_TYPE(OverlayManager, AceType);

public:
    explicit OverlayManager(const RefPtr<FrameNode>& rootNode) : rootNodeWeak_(rootNode) {}
    ~OverlayManager() override
    {
        popupMap_.clear();
    }
    void ShowIndexerPopup(int32_t targetId, RefPtr<FrameNode>& customNode);
    void RemoveIndexerPopupById(int32_t targetId);
    void RemoveIndexerPopup();
    void HidePopup(int32_t targetId, const PopupInfo& popupInfo);
    RefPtr<FrameNode> HidePopupWithoutAnimation(int32_t targetId, const PopupInfo& popupInfo);
    void ShowPopup(int32_t targetId, const PopupInfo& popupInfo);
    void ErasePopup(int32_t targetId);
    void HideAllPopups();
    void HideCustomPopups();

    PopupInfo GetPopupInfo(int32_t targetId) const
    {
        auto it = popupMap_.find(targetId);
        if (it == popupMap_.end()) {
            return {};
        }
        return it->second;
    }

    bool HasPopupInfo(int32_t targetId) const
    {
        return popupMap_.find(targetId) != popupMap_.end();
    }

    void ErasePopupInfo(int32_t targetId)
    {
        if (popupMap_.find(targetId) != popupMap_.end()) {
            popupMap_.erase(targetId);
        }
    }

    void ShowMenu(int32_t targetId, const NG::OffsetF& offset, RefPtr<FrameNode> menu = nullptr);
    void HideMenu(const RefPtr<FrameNode>& menu, int32_t targetId, bool isMenuOnTouch = false);
    void DeleteMenu(int32_t targetId);
    void ShowMenuInSubWindow(int32_t targetId, const NG::OffsetF& offset, RefPtr<FrameNode> menu = nullptr);
    void HideMenuInSubWindow(const RefPtr<FrameNode>& menu, int32_t targetId);
    RefPtr<FrameNode> GetMenuNode(int32_t targetId);
    void HideMenuInSubWindow(bool showPreviewAnimation = true, bool startDrag = false);
    void CleanMenuInSubWindow();
    void CleanPreviewInSubWindow();
    void CleanPopupInSubWindow();
    void CleanMenuInSubWindowWithAnimation();
    void HideAllMenus();

    void ClearToastInSubwindow();
    void ClearToast();
    void ShowToast(const std::string& message, int32_t duration, const std::string& bottom, bool isRightToLeft,
        const ToastShowMode& showMode = ToastShowMode::DEFAULT);

    std::unordered_map<int32_t, RefPtr<FrameNode>> GetDialogMap()
    {
        return dialogMap_;
    };
    RefPtr<FrameNode> GetDialog(int32_t dialogId);
    // customNode only used by customDialog, pass in nullptr if not customDialog
    RefPtr<FrameNode> ShowDialog(
        const DialogProperties& dialogProps, std::function<void()>&& buildFunc, bool isRightToLeft = false);
    void ShowCustomDialog(const RefPtr<FrameNode>& customNode);
    void ShowDateDialog(const DialogProperties& dialogProps, const DatePickerSettingData& settingData,
        std::map<std::string, NG::DialogEvent> dialogEvent,
        std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent);
    void ShowTimeDialog(const DialogProperties& dialogProps, const TimePickerSettingData& settingData,
        std::map<std::string, PickerTime> timePickerProperty, std::map<std::string, NG::DialogEvent> dialogEvent,
        std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent);
    void ShowTextDialog(const DialogProperties& dialogProps, const TextPickerSettingData& settingData,
        std::map<std::string, NG::DialogTextEvent> dialogEvent,
        std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent);
    void ShowCalendarDialog(const DialogProperties& dialogProps, const CalendarSettingData& settingData,
        std::map<std::string, NG::DialogEvent> dialogEvent,
        std::map<std::string, NG::DialogGestureEvent> dialogCancelEvent);
    void PopModalDialog(int32_t maskId);

    void CloseDialog(const RefPtr<FrameNode>& dialogNode);

    void OpenCustomDialog(const DialogProperties& dialogProps, std::function<void(int32_t)> &&callback);
    void CloseCustomDialog(const int32_t dialogId);

    void SetSubWindowId(int32_t subWindowId)
    {
        subWindowId_ = subWindowId;
    }
    int32_t GetSubwindowId()
    {
        return subWindowId_;
    }
    void SetMaskNodeId(int32_t dialogId, int32_t maskId)
    {
        maskNodeIdMap_[dialogId] = maskId;
    }
    bool isMaskNode(int32_t maskId)
    {
        for (auto it = maskNodeIdMap_.begin(); it != maskNodeIdMap_.end(); it++) {
            if (it->second == maskId) {
                return true;
            }
        }
        return false;
    }
    int32_t GetMaskNodeIdWithDialogId(int32_t dialogId)
    {
        int32_t maskNodeId = -1;
        for (auto it = maskNodeIdMap_.begin(); it != maskNodeIdMap_.end(); it++) {
            if (it->first == dialogId) {
                maskNodeId = it->second;
                break;
            }
        }
        return maskNodeId;
    }
    /**  pop overlays (if any) on back press
     *
     *   @return    true if popup was removed, false if no overlay exists
     */
    bool RemoveOverlay(bool isBackPressed, bool isPageRouter = false);
    bool RemoveDialog(const RefPtr<FrameNode>& overlay, bool isBackPressed, bool isPageRouter);
    bool RemoveBubble(const RefPtr<FrameNode>& overlay);
    bool RemoveMenu(const RefPtr<FrameNode>& overlay);
    bool RemoveModalInOverlay();
    bool RemoveAllModalInOverlay();
    bool RemoveOverlayInSubwindow();

    void RegisterOnHideDialog(std::function<void()> callback)
    {
        onHideDialogCallback_ = callback;
    }

    void CallOnHideDialogCallback()
    {
        if (onHideDialogCallback_) {
            onHideDialogCallback_();
        }
    }

    void SetBackPressEvent(std::function<bool()> event)
    {
        backPressEvent_ = event;
    }

    bool FireBackPressEvent() const
    {
        if (backPressEvent_) {
            return backPressEvent_();
        }
        return false;
    }

#ifdef ENABLE_DRAG_FRAMEWORK
    bool GetHasPixelMap()
    {
        return hasPixelMap_;
    }

    void SetHasPixelMap(bool hasPixelMap)
    {
        hasPixelMap_ = hasPixelMap;
    }

    RefPtr<FrameNode> GetPixelMapNode()
    {
        return pixmapColumnNodeWeak_.Upgrade();
    }

    RefPtr<FrameNode> GetPixelMapContentNode() const
    {
        auto column = pixmapColumnNodeWeak_.Upgrade();
        if (!column) {
            return nullptr;
        }
        auto imageNode = AceType::DynamicCast<FrameNode>(column->GetFirstChild());
        return imageNode;
    }

    bool GetHasFilter()
    {
        return hasFilter_;
    }

    void SetHasFilter(bool hasFilter)
    {
        hasFilter_ = hasFilter;
    }

    bool GetHasEvent()
    {
        return hasEvent_;
    }

    void SetHasEvent(bool hasEvent)
    {
        hasEvent_ = hasEvent;
    }

    bool GetIsOnAnimation()
    {
        return isOnAnimation_;
    }

    void SetIsOnAnimation(bool isOnAnimation)
    {
        isOnAnimation_ = isOnAnimation;
    }

    void SetFilterColumnNode(const RefPtr<FrameNode>& columnNode)
    {
        filterColumnNodeWeak_ = columnNode;
    }
    void MountFilterToWindowScene(const RefPtr<FrameNode>& columnNode, const RefPtr<UINode>& windowScene);
    void MountPixelMapToWindowScene(const RefPtr<FrameNode>& columnNode, const RefPtr<UINode>& windowScene);
    void MountEventToWindowScene(const RefPtr<FrameNode>& columnNode, const RefPtr<UINode>& windowScene);
    void MountPixelMapToRootNode(const RefPtr<FrameNode>& columnNode);
    void MountEventToRootNode(const RefPtr<FrameNode>& columnNode);
    void RemovePixelMap();
    void RemovePixelMapAnimation(bool startDrag, double x, double y);
    void UpdatePixelMapScale(float& scale);
    void RemoveFilter();
    void RemoveFilterAnimation();
    void RemoveEventColumn();
#endif // ENABLE_DRAG_FRAMEWORK
    void UpdateContextMenuDisappearPosition(const NG::OffsetF& offset);

    void ResetContextMenuDragHideFinished()
    {
        isContextMenuDragHideFinished_ = false;
        dragMoveVector_ = OffsetF(0.0f, 0.0f);
        lastDragMoveVector_ = OffsetF(0.0f, 0.0f);
    }

    void SetContextMenuDragHideFinished(bool isContextMenuDragHideFinished)
    {
        isContextMenuDragHideFinished_ = isContextMenuDragHideFinished;
    }

    bool IsContextMenuDragHideFinished() const
    {
        return isContextMenuDragHideFinished_ == true;
    }

    bool IsOriginDragMoveVector() const
    {
        return dragMoveVector_.NonOffset() && lastDragMoveVector_.NonOffset();
    }

    bool IsUpdateDragMoveVector() const
    {
        return !GetUpdateDragMoveVector().NonOffset() && !lastDragMoveVector_.NonOffset();
    }

    void UpdateDragMoveVector(const NG::OffsetF& offset)
    {
        lastDragMoveVector_ = dragMoveVector_;
        dragMoveVector_ = offset;
    }

    OffsetF GetUpdateDragMoveVector() const
    {
        return dragMoveVector_ - lastDragMoveVector_;
    }

    void BindContentCover(bool isShow, std::function<void(const std::string&)>&& callback,
        std::function<RefPtr<UINode>()>&& buildNodeFunc, NG::ModalStyle& modalStyle, std::function<void()>&& onAppear,
        std::function<void()>&& onDisappear, const RefPtr<FrameNode>& targetNode, int32_t sessionId = 0);

    void BindSheet(bool isShow, std::function<void(const std::string&)>&& callback,
        std::function<RefPtr<UINode>()>&& buildNodeFunc, std::function<RefPtr<UINode>()>&& buildTitleNodeFunc,
        NG::SheetStyle& sheetStyle, std::function<void()>&& onAppear, std::function<void()>&& onDisappear,
        std::function<void()>&& shouldDismiss, const RefPtr<FrameNode>& targetNode);
    void OnBindSheet(bool isShow, std::function<void(const std::string&)>&& callback,
        std::function<RefPtr<UINode>()>&& buildNodeFunc, std::function<RefPtr<UINode>()>&& buildtitleNodeFunc,
        NG::SheetStyle& sheetStyle, std::function<void()>&& onAppear, std::function<void()>&& onDisappear,
        std::function<void()>&& shouldDismiss, const RefPtr<FrameNode>& targetNode);
    void CloseSheet(int32_t targetId);

    void DismissSheet();

    void DestroySheet(const RefPtr<FrameNode>& sheetNode, int32_t targetId);

    RefPtr<FrameNode> GetSheetMask(const RefPtr<FrameNode>& sheetNode);

    void DeleteModal(int32_t targetId);

    void BindKeyboard(const std::function<void()>& keybordBuilder, int32_t targetId);
    void CloseKeyboard(int32_t targetId);
    void DestroyKeyboard();

    RefPtr<UINode> FindWindowScene(RefPtr<FrameNode> targetNode);

    // ui extension
    int32_t CreateModalUIExtension(
        const AAFwk::Want& want, const ModalUIExtensionCallbacks& callbacks, bool isProhibitBack);
    void CloseModalUIExtension(int32_t sessionId);

    RefPtr<FrameNode> BindUIExtensionToMenu(const RefPtr<FrameNode>& uiExtNode,
        const RefPtr<NG::FrameNode>& targetNode,  std::string longestContent, int32_t menuSize);
    SizeF CaculateMenuSize(const RefPtr<FrameNode>& menuNode,  std::string longestContent, int32_t menuSize);
    bool ShowUIExtensionMenu(const RefPtr<NG::FrameNode>& uiExtNode, NG::RectF aiRect, std::string longestContent,
        int32_t menuSize, const RefPtr<NG::FrameNode>& targetNode);

    void MarkDirty(PropertyChangeFlag flag);
    float GetRootHeight() const;

    void PlaySheetMaskTransition(RefPtr<FrameNode> maskNode, bool isTransitionIn);

    void PlaySheetTransition(RefPtr<FrameNode> sheetNode, bool isTransitionIn, bool isFirstTransition = true,
        bool isModeChangeToAuto = false);

    void ComputeSheetOffset(NG::SheetStyle& sheetStyle, RefPtr<FrameNode> sheetNode);

    void ComputeSingleGearSheetOffset(NG::SheetStyle& sheetStyle, RefPtr<FrameNode> sheetNode);

    void ComputeDetentsSheetOffset(NG::SheetStyle& sheetStyle, RefPtr<FrameNode> sheetNode);

    void SetSheetHeight(float height)
    {
        sheetHeight_ = height;
    }

    const WeakPtr<UINode>& GetRootNode() const
    {
        return rootNodeWeak_;
    }

private:
    void PopToast(int32_t targetId);

    // toast should contain id to avoid multiple delete.
    std::unordered_map<int32_t, WeakPtr<FrameNode>> toastMap_;

    /**  find/register menu node and update menu's display position
     *
     *   @return     true if process is successful
     */
    bool ShowMenuHelper(RefPtr<FrameNode>& menu, int32_t targetId, const NG::OffsetF& offset);

    // The focus logic of overlay node (menu and dialog):
    // 1. before start show animation: lower level node set unfocusabel and lost focus;
    // 2. end show animation: overlay node get focus;
    // 3. before start hide animation: lower level node set focusable;
    // 4. end hide animation: overlay node lost focus, lower level node get focus.
    void FocusOverlayNode(const RefPtr<FrameNode>& overlayNode, bool isInSubWindow = false);
    void BlurOverlayNode(const RefPtr<FrameNode>& currentOverlay, bool isInSubWindow = false);
    void BlurLowerNode(const RefPtr<FrameNode>& currentOverlay);
    void ResetLowerNodeFocusable(const RefPtr<FrameNode>& currentOverlay);
    void PostDialogFinishEvent(const WeakPtr<FrameNode>& nodeWk);
    void OnDialogCloseEvent(const RefPtr<FrameNode>& node);

    void CloseDialogInner(const RefPtr<FrameNode>& dialogNode);

    void SetShowMenuAnimation(const RefPtr<FrameNode>& menu, bool isInSubWindow = false);
    void PopMenuAnimation(const RefPtr<FrameNode>& menu, bool showPreviewAnimation = true, bool startDrag = false);
    void ClearMenuAnimation(const RefPtr<FrameNode>& menu, bool showPreviewAnimation = true, bool startDrag = false);
    void ShowMenuClearAnimation(const RefPtr<FrameNode>& menu, AnimationOption& option,
        bool showPreviewAnimation, bool startDrag);

    void OpenDialogAnimation(const RefPtr<FrameNode>& node);
    void CloseDialogAnimation(const RefPtr<FrameNode>& node);
    void SetContainerButtonEnable(bool isEnabled);

    void SaveLastModalNode();
    void PlayDefaultModalTransition(const RefPtr<FrameNode>& modalNode, bool isTransitionIn);
    void DefaultModalTransition(bool isTransitionIn);
    void PlayAlphaModalTransition(const RefPtr<FrameNode>& modalNode, bool isTransitionIn);
    void FireModalPageShow();
    void FireModalPageHide();
    void ModalPageLostFocus(const RefPtr<FrameNode>& node);

    void SetSheetBackgroundBlurStyle(const RefPtr<FrameNode>& sheetNode, const BlurStyleOption& bgBlurStyle);

    bool ModalExitProcess(const RefPtr<FrameNode>& topModalNode);

    void BeforeShowDialog(const RefPtr<FrameNode>& dialogNode);
    void RemoveDialogFromMap(const RefPtr<FrameNode>& node);
    bool DialogInMapHoldingFocus();
    void PlayKeyboardTransition(RefPtr<FrameNode> customKeyboard, bool isTransitionIn);
    void FireNavigationStateChange(const RefPtr<UINode>& root, bool show, const RefPtr<UINode>& node = nullptr);
    RefPtr<FrameNode> GetModalNodeInStack(std::stack<WeakPtr<FrameNode>>& stack);
    void PlayBubbleStyleSheetTransition(RefPtr<FrameNode> sheetNode, bool isTransitionIn);

    // Key: target Id, Value: PopupInfo
    std::unordered_map<int32_t, NG::PopupInfo> popupMap_;
    // K: target frameNode ID, V: menuNode
    std::unordered_map<int32_t, RefPtr<FrameNode>> menuMap_;
    std::unordered_map<int32_t, RefPtr<FrameNode>> dialogMap_;
    std::unordered_map<int32_t, RefPtr<FrameNode>> customPopupMap_;
    std::unordered_map<int32_t, RefPtr<FrameNode>> customKeyboardMap_;
    std::stack<WeakPtr<FrameNode>> modalStack_;
    std::list<WeakPtr<FrameNode>> modalList_;
    std::unordered_map<int32_t, WeakPtr<FrameNode>> sheetMap_;
    WeakPtr<FrameNode> lastModalNode_; // Previous Modal Node
    float sheetHeight_ { 0.0 };
    WeakPtr<UINode> rootNodeWeak_;
    int32_t dialogCount_ = 0;
    std::unordered_map<int32_t, int32_t> maskNodeIdMap_;
    int32_t subWindowId_;
#ifdef ENABLE_DRAG_FRAMEWORK
    bool hasPixelMap_ { false };
    bool hasFilter_ { false };
    bool hasEvent_ { false };
    bool isOnAnimation_ { false };
    WeakPtr<FrameNode> pixmapColumnNodeWeak_;
    WeakPtr<FrameNode> filterColumnNodeWeak_;
    WeakPtr<FrameNode> eventColumnNodeWeak_;
#endif // ENABLE_DRAG_FRAMEWORK
    bool isContextMenuDragHideFinished_ = false;
    OffsetF dragMoveVector_ = OffsetF(0.0f, 0.0f);
    OffsetF lastDragMoveVector_ = OffsetF(0.0f, 0.0f);

    std::function<void()> onHideDialogCallback_ = nullptr;
    CancelableCallback<void()> continuousTask_;
    std::function<bool()> backPressEvent_ = nullptr;

    std::set<WeakPtr<UINode>> windowSceneSet_;

    RefPtr<NG::ClickEvent> sheetMaskClickEvent_;

    // native modal ui extension
    bool isProhibitBack_ = false;

    ACE_DISALLOW_COPY_AND_MOVE(OverlayManager);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_OVERLAY_OVERLAY_MANAGER_H
