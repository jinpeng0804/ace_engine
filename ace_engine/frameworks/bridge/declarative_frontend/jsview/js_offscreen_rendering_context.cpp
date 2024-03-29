/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "bridge/declarative_frontend/jsview/js_offscreen_rendering_context.h"

#include "bridge/declarative_frontend/jsview/models/offscreen_context_model_impl.h"
#include "core/components_ng/pattern/canvas_context/offscreen_context_model_ng.h"
#include "core/components_ng/pattern/custom_paint/offscreen_canvas_pattern.h"

namespace OHOS::Ace {
std::unique_ptr<OffscreenContextModel> OffscreenContextModel::instance_ = nullptr;
std::mutex OffscreenContextModel::mutex_;
OffscreenContextModel* OffscreenContextModel::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!instance_) {
#ifdef NG_BUILD
            instance_.reset(new NG::OffscreenContextModelNG());
#else
            if (Container::IsCurrentUseNewPipeline()) {
                instance_.reset(new NG::OffscreenContextModelNG());
            } else {
                instance_.reset(new Framework::OffscreenContextModelImpl());
            }
#endif
        }
    }
    return instance_.get();
}
} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {
std::mutex JSOffscreenRenderingContext::mutex_;
std::unordered_map<uint32_t, RefPtr<AceType>> JSOffscreenRenderingContext::offscreenPatternMap_;
uint32_t JSOffscreenRenderingContext::offscreenPatternCount_ = 0;

JSOffscreenRenderingContext::JSOffscreenRenderingContext()
{
    id = offscreenPatternCount_;
}

void JSOffscreenRenderingContext::JSBind(BindingTarget globalObj)
{
    JSClass<JSOffscreenRenderingContext>::Declare("OffscreenCanvasRenderingContext2D");

    JSClass<JSOffscreenRenderingContext>::CustomMethod("transferToImageBitmap",
        &JSOffscreenRenderingContext::JsTransferToImageBitmap);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("toDataURL", &JSCanvasRenderer::JsToDataUrl);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("createRadialGradient",
        &JSCanvasRenderer::JsCreateRadialGradient);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("fillRect", &JSCanvasRenderer::JsFillRect);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("strokeRect", &JSCanvasRenderer::JsStrokeRect);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("clearRect", &JSCanvasRenderer::JsClearRect);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("createLinearGradient",
        &JSCanvasRenderer::JsCreateLinearGradient);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("fillText", &JSCanvasRenderer::JsFillText);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("strokeText", &JSCanvasRenderer::JsStrokeText);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("measureText", &JSCanvasRenderer::JsMeasureText);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("moveTo", &JSCanvasRenderer::JsMoveTo);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("lineTo", &JSCanvasRenderer::JsLineTo);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("bezierCurveTo", &JSCanvasRenderer::JsBezierCurveTo);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("quadraticCurveTo", &JSCanvasRenderer::JsQuadraticCurveTo);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("arcTo", &JSCanvasRenderer::JsArcTo);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("arc", &JSCanvasRenderer::JsArc);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("ellipse", &JSCanvasRenderer::JsEllipse);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("fill", &JSCanvasRenderer::JsFill);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("stroke", &JSCanvasRenderer::JsStroke);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("clip", &JSCanvasRenderer::JsClip);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("rect", &JSCanvasRenderer::JsRect);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("beginPath", &JSCanvasRenderer::JsBeginPath);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("closePath", &JSCanvasRenderer::JsClosePath);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("restore", &JSCanvasRenderer::JsRestore);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("save", &JSCanvasRenderer::JsSave);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("rotate", &JSCanvasRenderer::JsRotate);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("scale", &JSCanvasRenderer::JsScale);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("getTransform", &JSCanvasRenderer::JsGetTransform);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("setTransform", &JSCanvasRenderer::JsSetTransform);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("resetTransform", &JSCanvasRenderer::JsResetTransform);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("transform", &JSCanvasRenderer::JsTransform);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("translate", &JSCanvasRenderer::JsTranslate);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("setLineDash", &JSCanvasRenderer::JsSetLineDash);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("getLineDash", &JSCanvasRenderer::JsGetLineDash);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("drawImage", &JSCanvasRenderer::JsDrawImage);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("createPattern", &JSCanvasRenderer::JsCreatePattern);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("createImageData", &JSCanvasRenderer::JsCreateImageData);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("putImageData", &JSCanvasRenderer::JsPutImageData);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("getImageData", &JSCanvasRenderer::JsGetImageData);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("getJsonData", &JSCanvasRenderer::JsGetJsonData);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("getPixelMap", &JSCanvasRenderer::JsGetPixelMap);
    JSClass<JSOffscreenRenderingContext>::CustomMethod("setPixelMap", &JSCanvasRenderer::JsSetPixelMap);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("filter", &JSCanvasRenderer::JsGetFilter,
        &JSCanvasRenderer::JsSetFilter);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("direction", &JSCanvasRenderer::JsGetDirection,
        &JSCanvasRenderer::JsSetDirection);

    JSClass<JSOffscreenRenderingContext>::CustomProperty("fillStyle", &JSCanvasRenderer::JsGetFillStyle,
        &JSCanvasRenderer::JsSetFillStyle);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("strokeStyle", &JSCanvasRenderer::JsGetStrokeStyle,
        &JSCanvasRenderer::JsSetStrokeStyle);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("lineCap", &JSCanvasRenderer::JsGetLineCap,
        &JSCanvasRenderer::JsSetLineCap);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("lineJoin", &JSCanvasRenderer::JsGetLineJoin,
        &JSCanvasRenderer::JsSetLineJoin);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("miterLimit", &JSCanvasRenderer::JsGetMiterLimit,
        &JSCanvasRenderer::JsSetMiterLimit);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("lineWidth", &JSCanvasRenderer::JsGetLineWidth,
        &JSCanvasRenderer::JsSetLineWidth);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("font", &JSCanvasRenderer::JsGetFont,
        &JSCanvasRenderer::JsSetFont);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("textAlign", &JSCanvasRenderer::JsGetTextAlign,
        &JSCanvasRenderer::JsSetTextAlign);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("textBaseline", &JSCanvasRenderer::JsGetTextBaseline,
        &JSCanvasRenderer::JsSetTextBaseline);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("globalAlpha", &JSCanvasRenderer::JsGetGlobalAlpha,
        &JSCanvasRenderer::JsSetGlobalAlpha);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("globalCompositeOperation",
        &JSCanvasRenderer::JsGetGlobalCompositeOperation, &JSCanvasRenderer::JsSetGlobalCompositeOperation);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("lineDashOffset", &JSCanvasRenderer::JsGetLineDashOffset,
        &JSCanvasRenderer::JsSetLineDashOffset);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("shadowBlur", &JSCanvasRenderer::JsGetShadowBlur,
        &JSCanvasRenderer::JsSetShadowBlur);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("shadowColor", &JSCanvasRenderer::JsGetShadowColor,
        &JSCanvasRenderer::JsSetShadowColor);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("shadowOffsetX", &JSCanvasRenderer::JsGetShadowOffsetX,
        &JSCanvasRenderer::JsSetShadowOffsetX);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("shadowOffsetY", &JSCanvasRenderer::JsGetShadowOffsetY,
        &JSCanvasRenderer::JsSetShadowOffsetY);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("imageSmoothingEnabled",
        &JSCanvasRenderer::JsGetImageSmoothingEnabled, &JSCanvasRenderer::JsSetImageSmoothingEnabled);
    JSClass<JSOffscreenRenderingContext>::CustomProperty("imageSmoothingQuality",
        &JSCanvasRenderer::JsGetImageSmoothingQuality, &JSCanvasRenderer::JsSetImageSmoothingQuality);

    JSClass<JSOffscreenRenderingContext>::CustomMethod("createConicGradient",
        &JSCanvasRenderer::JsCreateConicGradient);

    JSClass<JSOffscreenRenderingContext>::Bind(globalObj, JSOffscreenRenderingContext::Constructor,
        JSOffscreenRenderingContext::Destructor);
}

void JSOffscreenRenderingContext::Constructor(const JSCallbackInfo& args)
{
    auto jsRenderContext = Referenced::MakeRefPtr<JSOffscreenRenderingContext>();
    jsRenderContext->IncRefCount();
    args.SetReturnValue(Referenced::RawPtr(jsRenderContext));

    if (args.Length() < 3) {
        return;
    }
    if (args[0]->IsNumber() && args[1]->IsNumber()) {
        double fWidth = 0.0;
        double fHeight = 0.0;
        int width = 0;
        int height = 0;
        JSViewAbstract::ParseJsDouble(args[0], fWidth);
        JSViewAbstract::ParseJsDouble(args[1], fHeight);

        fWidth = PipelineBase::Vp2PxWithCurrentDensity(fWidth);
        fHeight = PipelineBase::Vp2PxWithCurrentDensity(fHeight);
        width = round(fWidth);
        height = round(fHeight);

        auto offscreenPattern = OffscreenContextModel::GetInstance()->CreateOffscreenPattern(width, height);
        CHECK_NULL_VOID(offscreenPattern);
        size_t bitmapSize = OffscreenContextModel::GetInstance()->GetBitmapSize(offscreenPattern);
        args.SetSize(bitmapSize);
        jsRenderContext->SetOffscreenPattern(offscreenPattern);
        std::lock_guard<std::mutex> lock(mutex_);
        offscreenPatternMap_[offscreenPatternCount_++] = offscreenPattern;

        if (!OffscreenContextModel::GetInstance()->IsSucceed(offscreenPattern)) {
            return;
        }
    }
    if (args[2]->IsObject()) {
        JSRenderingContextSettings* jsContextSetting
            = JSRef<JSObject>::Cast(args[2])->Unwrap<JSRenderingContextSettings>();
        if (jsContextSetting == nullptr) {
            return;
        }
        bool anti = jsContextSetting->GetAntialias();
        jsRenderContext->SetAnti(anti);
        jsRenderContext->SetAntiAlias();
    }
}

void JSOffscreenRenderingContext::Destructor(JSOffscreenRenderingContext* context)
{
    uint32_t contextId;
    if (context != nullptr) {
        contextId = context->GetId();
        context->DecRefCount();
    } else {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    offscreenPatternMap_.erase(contextId);
}

void JSOffscreenRenderingContext::JsTransferToImageBitmap(const JSCallbackInfo& info)
{
    auto retObj = JSRef<JSObject>::New();
    retObj->SetProperty("__id", id);
    info.SetReturnValue(retObj);
}

} // namespace OHOS::Ace::Framework
