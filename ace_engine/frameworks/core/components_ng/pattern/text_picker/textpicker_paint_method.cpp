/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/text_picker/textpicker_paint_method.h"

#include "core/components/common/properties/color.h"
#include "core/components/picker/picker_theme.h"
#include "core/components_ng/pattern/text_picker/textpicker_pattern.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

namespace {
constexpr uint8_t ENABLED_ALPHA = 255;
constexpr uint8_t DISABLED_ALPHA = 102;
constexpr uint8_t DOUBLE = 2;
const Dimension PICKER_DIALOG_DIVIDER_MARGIN = 24.0_vp;
} // namespace

CanvasDrawFunction TextPickerPaintMethod::GetForegroundDrawFunction(PaintWrapper* paintWrapper)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, nullptr);
    auto theme = pipeline->GetTheme<PickerTheme>();
    auto dividerColor = theme->GetDividerColor();
    auto dividerLineWidth = theme->GetDividerThickness().ConvertToPx();
    auto dividerSpacing = pipeline->NormalizeToPx(theme->GetDividerSpacing());
    auto pressColor = theme->GetPressColor();
    const auto& geometryNode = paintWrapper->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, nullptr);
    auto frameRect = geometryNode->GetFrameRect();
    return [weak = WeakClaim(this), dividerLineWidth, frameRect, dividerColor, dividerSpacing, pressColor,
               enabled = enabled_, pattern = pattern_](RSCanvas& canvas) {
        auto picker = weak.Upgrade();
        CHECK_NULL_VOID(picker);
        auto textPickerPattern = DynamicCast<TextPickerPattern>(pattern.Upgrade());
        CHECK_NULL_VOID(textPickerPattern);
        auto dividerLength = frameRect.Width();
        auto dividerMargin = 0.0;
        if (textPickerPattern->GetIsShowInDialog()) {
            dividerLength = frameRect.Width() - PICKER_DIALOG_DIVIDER_MARGIN.ConvertToPx() * DOUBLE;
            dividerMargin = PICKER_DIALOG_DIVIDER_MARGIN.ConvertToPx();
        }
        DividerPainter dividerPainter(dividerLineWidth, dividerLength, false, dividerColor, LineCap::SQUARE);
        auto height = picker->defaultPickerItemHeight_;
        if (textPickerPattern->GetResizeFlag()) {
            height = textPickerPattern->GetResizePickerItemHeight();
        }
        double upperLine = (frameRect.Height() - height) / 2.0;
        double downLine = (frameRect.Height() + height) / 2.0;
        OffsetF offset = OffsetF(dividerMargin, upperLine);
        dividerPainter.DrawLine(canvas, offset);
        OffsetF offsetY = OffsetF(dividerMargin, downLine);
        dividerPainter.DrawLine(canvas, offsetY);
        if (!enabled) {
            picker->PaintDisable(canvas, frameRect.Width(), frameRect.Height());
        }
    };
}

void TextPickerPaintMethod::PaintDisable(RSCanvas& canvas, double X, double Y)
{
    double centerY = Y;
    double centerX = X;
    RSRect rRect(0, 0, centerX, centerY);
    RSPath path;
    path.AddRoundRect(rRect, 0, 0, RSPathDirection::CW_DIRECTION);
    RSPen pen;
    RSBrush brush;
    brush.SetColor(float(DISABLED_ALPHA) / ENABLED_ALPHA);
    pen.SetColor(float(DISABLED_ALPHA) / ENABLED_ALPHA);
    canvas.AttachBrush(brush);
    canvas.AttachPen(pen);
    canvas.DrawPath(path);
    canvas.DetachPen();
    canvas.DetachBrush();
}
} // namespace OHOS::Ace::NG
