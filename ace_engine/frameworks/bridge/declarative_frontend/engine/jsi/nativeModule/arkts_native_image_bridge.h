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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_JSI_NATIVEMODULE_ARKTS_NATIVE_IMAGE_BRIDGE_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_JSI_NATIVEMODULE_ARKTS_NATIVE_IMAGE_BRIDGE_H

#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_native_api_bridge.h"

namespace OHOS::Ace::NG {
class ImageBridge {
public:
    static ArkUINativeModuleValue SetCopyOption(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetCopyOption(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetAutoResize(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetAutoResize(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetObjectRepeat(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetObjectRepeat(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetRenderMode(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetRenderMode(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetSyncLoad(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetSyncLoad(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetObjectFit(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetObjectFit(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetFitOriginalSize(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetFitOriginalSize(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetSourceSize(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetSourceSize(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetMatchTextDirection(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetMatchTextDirection(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetFillColor(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetFillColor(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetAlt(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetAlt(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetImageInterpolation(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetImageInterpolation(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetColorFilter(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetColorFilter(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetDraggable(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue ResetDraggable(ArkUIRuntimeCallInfo *runtimeCallInfo);
    static ArkUINativeModuleValue SetBorderRadius(ArkUIRuntimeCallInfo* runtimeCallInfo);
    static ArkUINativeModuleValue ResetBorderRadius(ArkUIRuntimeCallInfo* runtimeCallInfo);
};
}

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_ENGINE_JSI_NATIVEMODULE_ARKTS_NATIVE_IMAGE_BRIDGE_H