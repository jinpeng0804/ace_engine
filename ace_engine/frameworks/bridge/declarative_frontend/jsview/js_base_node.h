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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_BASE_NODE_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_BASE_NODE_H

#include <functional>
#include "base/geometry/size.h"
#include "base/memory/ace_type.h"
#include "bridge/declarative_frontend/engine/functions/js_function.h"
#include "bridge/declarative_frontend/engine/js_types.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/ui_node.h"
#include "frameworks/bridge/declarative_frontend/engine/bindings_defines.h"

namespace OHOS::Ace::Framework {
class JSBaseNode : public AceType {
    DECLARE_ACE_TYPE(JSBaseNode, AceType)
public:
    JSBaseNode() = default;
    ~JSBaseNode() override = default;

    static void JSBind(BindingTarget globalObj);
    static void ConstructorCallback(const JSCallbackInfo& info);
    static void DestructorCallback(JSBaseNode* node);
    void FinishUpdateFunc(const JSCallbackInfo& info);
    void BuildNode(const JSCallbackInfo& info);
    void PostTouchEvent(const JSCallbackInfo& info);
    void CreateRenderNode(const JSCallbackInfo& info);

    RefPtr<NG::UINode> GetViewNode() const
    {
        return viewNode_;
    }

protected:
    RefPtr<NG::UINode> viewNode_;
};
} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_BASE_NODE_H
