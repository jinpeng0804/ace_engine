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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_UI_EXTENSION_MODEL_NG_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_UI_EXTENSION_MODEL_NG_H

#include <memory>
#include <string>

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_model.h"

namespace OHOS::Ace {
struct ModalUIExtensionCallbacks;
} // namespace OHOS::Ace

namespace OHOS::Ace::NG {
class ACE_EXPORT UIExtensionModelNG : public UIExtensionModel {
public:
    static RefPtr<FrameNode> Create(const std::string& bundleName, const std::string& abilityName,
        const std::map<std::string, std::string>& params, std::function<void(int32_t)>&& onRelease,
        std::function<void(int32_t, const std::string&, const std::string&)>&& onError);
    static RefPtr<FrameNode> Create(const AAFwk::Want& want, const ModalUIExtensionCallbacks& callbacks);

    void Create(const RefPtr<OHOS::Ace::WantWrap>& wantWrap, bool transferringCaller = false) override;
    // for dynamic component
    void Create() override;
    void InitializeDynamicComponent(const RefPtr<FrameNode>& frameNode, const std::string& hapPath,
        const std::string& abcPath, const RefPtr<OHOS::Ace::WantWrap>& wantWrap, void* runtime) override;

    void SetOnRemoteReady(std::function<void(const RefPtr<UIExtensionProxy>&)>&& onRemoteReady) override;
    void SetOnRelease(std::function<void(int32_t)>&& onRelease) override;
    void SetOnResult(std::function<void(int32_t, const AAFwk::Want&)>&& onResult) override;
    void SetOnReceive(std::function<void(const AAFwk::WantParams&)>&& onReceive) override;
    void SetOnError(
        std::function<void(int32_t code, const std::string& name, const std::string& message)>&& onError) override;
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_UI_EXTENSION_UI_EXTENSION_MODEL_NG_H
