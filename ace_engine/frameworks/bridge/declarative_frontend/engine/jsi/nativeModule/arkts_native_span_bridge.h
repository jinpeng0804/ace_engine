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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_JSI_NATIVEMODULE_ARKTS_NATIVE_SPAN_BRIDGE_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_JSI_NATIVEMODULE_ARKTS_NATIVE_SPAN_BRIDGE_H

#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_api_bridge.h"

namespace OHOS::Ace::NG {
class SpanBridge {
public:
    static ArkUINativeModuleValue SetTextCase(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetTextCase(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetFontWeight(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetFontWeight(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetLineHeight(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ReSetLineHeight(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetFontStyle(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ReSetFontStyle(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetFontSize(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetFontSize(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetFontFamily(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetFontFamily(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetDecoration(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetDecoration(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetFontColor(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetFontColor(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetLetterSpacing(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetLetterSpacing(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetFont(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetFont(ArkUIRuntimeCallInfo *runtimeCallInfo);
};
}

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_JSI_NATIVEMODULE_ARKTS_NATIVE_SPAN_BRIDGE_H
