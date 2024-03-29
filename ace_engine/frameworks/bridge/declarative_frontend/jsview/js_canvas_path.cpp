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

#include "bridge/declarative_frontend/jsview/js_canvas_path.h"
#include "bridge/declarative_frontend/jsview/js_rendering_context.h"
#include "bridge/declarative_frontend/jsview/js_view_common_def.h"

namespace OHOS::Ace::Framework {

JSCanvasPath::JSCanvasPath() = default;

void JSCanvasPath::JsPath2DSetTransform(const JSCallbackInfo& info)
{
    if (info.Length() == 6) {
        if (info[0]->IsNumber()) {
            double scaleX = 0.0;
            double skewX = 0.0;
            double skewY = 0.0;
            double scaleY = 0.0;
            double translateX = 0.0;
            double translateY = 0.0;
            JSViewAbstract::ParseJsDouble(info[0], scaleX);
            JSViewAbstract::ParseJsDouble(info[1], skewX);
            JSViewAbstract::ParseJsDouble(info[2], skewY);
            JSViewAbstract::ParseJsDouble(info[3], scaleY);
            JSViewAbstract::ParseJsDouble(info[4], translateX);
            JSViewAbstract::ParseJsDouble(info[5], translateY);
            translateX = PipelineBase::Vp2PxWithCurrentDensity(translateX);
            translateY = PipelineBase::Vp2PxWithCurrentDensity(translateY);
            path2d_->SetTransform(scaleX, skewX, skewY, scaleY, translateX, translateY);
        }
    }
}

void JSCanvasPath::JsPath2DMoveTo(const JSCallbackInfo& info)
{
    if (info.Length() == 2) {
        if (info[0]->IsNumber()) {
            double X = 0.0;
            double Y = 0.0;
            JSViewAbstract::ParseJsDouble(info[0], X);
            JSViewAbstract::ParseJsDouble(info[1], Y);
            X = PipelineBase::Vp2PxWithCurrentDensity(X);
            Y = PipelineBase::Vp2PxWithCurrentDensity(Y);
            path2d_->MoveTo(X, Y);
        }
    }
}

void JSCanvasPath::JsPath2DLineTo(const JSCallbackInfo& info)
{
    if (info.Length() == 2) {
        if (info[0]->IsNumber()) {
            double X = 0.0;
            double Y = 0.0;
            JSViewAbstract::ParseJsDouble(info[0], X);
            JSViewAbstract::ParseJsDouble(info[1], Y);
            X = PipelineBase::Vp2PxWithCurrentDensity(X);
            Y = PipelineBase::Vp2PxWithCurrentDensity(Y);
            path2d_->LineTo(X, Y);
        }
    }
}

void JSCanvasPath::JsPath2DArc(const JSCallbackInfo& info)
{
    if (info.Length() == 5 || info.Length() == 6) {
        if (info[0]->IsNumber()) {
            double X = 0.0;
            double Y = 0.0;
            double radius = 0.0;
            double startAngle = 0.0;
            double endAngle = 0.0;
            bool anticlockwise = false;
            JSViewAbstract::ParseJsDouble(info[0], X);
            JSViewAbstract::ParseJsDouble(info[1], Y);
            JSViewAbstract::ParseJsDouble(info[2], radius);
            JSViewAbstract::ParseJsDouble(info[3], startAngle);
            JSViewAbstract::ParseJsDouble(info[4], endAngle);
            X = PipelineBase::Vp2PxWithCurrentDensity(X);
            Y = PipelineBase::Vp2PxWithCurrentDensity(Y);
            radius = PipelineBase::Vp2PxWithCurrentDensity(radius);
            if (info.Length() == 6) {
                JSViewAbstract::ParseJsBool(info[5], anticlockwise);
            }
            path2d_->Arc(X, Y, radius, startAngle, endAngle, anticlockwise);
        }
    }
}

void JSCanvasPath::JsPath2DArcTo(const JSCallbackInfo& info)
{
    if (info.Length() == 5) {
        if (info[0]->IsNumber()) {
            double x1 = 0.0;
            double y1 = 0.0;
            double x2 = 0.0;
            double y2 = 0.0;
            double radius = 0.0;
            JSViewAbstract::ParseJsDouble(info[0], x1);
            JSViewAbstract::ParseJsDouble(info[1], y1);
            JSViewAbstract::ParseJsDouble(info[2], x2);
            JSViewAbstract::ParseJsDouble(info[3], y2);
            JSViewAbstract::ParseJsDouble(info[4], radius);
            x1 = PipelineBase::Vp2PxWithCurrentDensity(x1);
            y1 = PipelineBase::Vp2PxWithCurrentDensity(y1);
            x2 = PipelineBase::Vp2PxWithCurrentDensity(x2);
            y2 = PipelineBase::Vp2PxWithCurrentDensity(y2);
            radius = PipelineBase::Vp2PxWithCurrentDensity(radius);
            path2d_->ArcTo(x1, y1, x2, y2, radius);
        }
    }
}

void JSCanvasPath::JsPath2DQuadraticCurveTo(const JSCallbackInfo& info)
{
    if (info[0]->IsNumber()) {
        double cpx = 0.0;
        double cpy = 0.0;
        double x = 0.0;
        double y = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], cpx);
        JSViewAbstract::ParseJsDouble(info[1], cpy);
        JSViewAbstract::ParseJsDouble(info[2], x);
        JSViewAbstract::ParseJsDouble(info[3], y);
        cpx = PipelineBase::Vp2PxWithCurrentDensity(cpx);
        cpy = PipelineBase::Vp2PxWithCurrentDensity(cpy);
        x = PipelineBase::Vp2PxWithCurrentDensity(x);
        y = PipelineBase::Vp2PxWithCurrentDensity(y);
        path2d_->QuadraticCurveTo(cpx, cpy, x, y);
    }
}

void JSCanvasPath::JsPath2DBezierCurveTo(const JSCallbackInfo& info)
{
    if (info[0]->IsNumber()) {
        double cp1x = 0.0;
        double cp1y = 0.0;
        double cp2x = 0.0;
        double cp2y = 0.0;
        double x = 0.0;
        double y = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], cp1x);
        JSViewAbstract::ParseJsDouble(info[1], cp1y);
        JSViewAbstract::ParseJsDouble(info[2], cp2x);
        JSViewAbstract::ParseJsDouble(info[3], cp2y);
        JSViewAbstract::ParseJsDouble(info[4], x);
        JSViewAbstract::ParseJsDouble(info[5], y);
        cp1x = PipelineBase::Vp2PxWithCurrentDensity(cp1x);
        cp1y = PipelineBase::Vp2PxWithCurrentDensity(cp1y);
        cp2x = PipelineBase::Vp2PxWithCurrentDensity(cp2x);
        cp2y = PipelineBase::Vp2PxWithCurrentDensity(cp2y);
        x = PipelineBase::Vp2PxWithCurrentDensity(x);
        y = PipelineBase::Vp2PxWithCurrentDensity(y);
        path2d_->BezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y);
    }
}

void JSCanvasPath::JsPath2DEllipse(const JSCallbackInfo& info)
{
    if (info[0]->IsNumber()) {
        double X = 0.0;
        double Y = 0.0;
        double radiusX = 0.0;
        double radiusY = 0.0;
        double rotation = 0.0;
        double startAngle = 0.0;
        double endAngle = 0.0;

        JSViewAbstract::ParseJsDouble(info[0], X);
        JSViewAbstract::ParseJsDouble(info[1], Y);
        JSViewAbstract::ParseJsDouble(info[2], radiusX);
        JSViewAbstract::ParseJsDouble(info[3], radiusY);
        JSViewAbstract::ParseJsDouble(info[4], rotation);
        JSViewAbstract::ParseJsDouble(info[5], startAngle);
        JSViewAbstract::ParseJsDouble(info[6], endAngle);
        X = PipelineBase::Vp2PxWithCurrentDensity(X);
        Y = PipelineBase::Vp2PxWithCurrentDensity(Y);
        radiusX = PipelineBase::Vp2PxWithCurrentDensity(radiusX);
        radiusY = PipelineBase::Vp2PxWithCurrentDensity(radiusY);

        bool anticlockwise = false;

        if (info.Length() == 8) {
            JSViewAbstract::ParseJsBool(info[7], anticlockwise);
        }

        path2d_->Ellipse(X, Y, radiusX, radiusY, rotation, startAngle, endAngle, anticlockwise);
    }
}

void JSCanvasPath::JsPath2DRect(const JSCallbackInfo& info)
{
    if (info[0]->IsNumber()) {
        double x = 0.0;
        double y = 0.0;
        double width = 0.0;
        double height = 0.0;
        JSViewAbstract::ParseJsDouble(info[0], x);
        JSViewAbstract::ParseJsDouble(info[1], y);
        JSViewAbstract::ParseJsDouble(info[2], width);
        JSViewAbstract::ParseJsDouble(info[3], height);
        x = PipelineBase::Vp2PxWithCurrentDensity(x);
        y = PipelineBase::Vp2PxWithCurrentDensity(y);
        width = PipelineBase::Vp2PxWithCurrentDensity(width);
        height = PipelineBase::Vp2PxWithCurrentDensity(height);

        path2d_->Rect(x, y, width, height);
    }
}

void JSCanvasPath::JsPath2DClosePath(const JSCallbackInfo& info)
{
    path2d_->ClosePath();
}

} // namespace OHOS::Ace::Framework
