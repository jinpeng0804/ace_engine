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
#include "core/components_ng/pattern/bubble/bubble_paint_method.h"

#include <vector>

#include "base/geometry/dimension.h"
#include "base/geometry/ng/offset_t.h"
#include "base/geometry/ng/rect_t.h"
#include "base/geometry/rect.h"
#include "base/geometry/rrect.h"
#include "base/utils/utils.h"
#include "core/components/common/properties/alignment.h"
#include "core/components/common/properties/border.h"
#include "core/components/common/properties/color.h"
#include "core/components/common/properties/decoration.h"
#include "core/components/common/properties/placement.h"
#include "core/components/common/properties/shadow_config.h"
#include "core/components/popup/popup_theme.h"
#include "core/components/theme/theme_manager.h"
#include "core/components_ng/pattern/bubble/bubble_pattern.h"
#include "core/components_ng/pattern/bubble/bubble_render_property.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/common/container.h"

namespace OHOS::Ace::NG {
namespace {
constexpr Dimension BEZIER_WIDTH_HALF = 16.0_vp;
constexpr Dimension BEZIER_HORIZON_OFFSET_FIRST = 1.3_vp;
constexpr Dimension BEZIER_HORIZON_OFFSET_SECOND = 3.2_vp;
constexpr Dimension BEZIER_HORIZON_OFFSET_THIRD = 6.6_vp;
constexpr Dimension BEZIER_HORIZON_OFFSET_FOURTH = 16.0_vp;
constexpr Dimension BEZIER_VERTICAL_OFFSET_FIRST = 0.1_vp;
constexpr Dimension BEZIER_VERTICAL_OFFSET_SECOND = 3.0_vp;
constexpr Dimension BEZIER_VERTICAL_OFFSET_THIRD = 8.0_vp;
constexpr Dimension ARROW_WIDTH = 32.0_vp;
constexpr Dimension ARROW_ZERO_PERCENT_VALUE = Dimension(0.0, DimensionUnit::PERCENT);
constexpr Dimension ARROW_HALF_PERCENT_VALUE = Dimension(0.5, DimensionUnit::PERCENT);
constexpr Dimension ARROW_ONE_HUNDRED_PERCENT_VALUE = Dimension(1.0, DimensionUnit::PERCENT);
constexpr float BLUR_MASK_FILTER = 0.55f;

constexpr double HALF = 2.0;
constexpr Dimension ARROW_RADIUS = 2.0_vp;
constexpr Dimension ARROW_WIDTH_SMALL = 16.0_vp;
constexpr Dimension ARROW_POINT_OFFSET_X = 0.8_vp;
constexpr Dimension ARROW_POINT_OFFSET_Y = 1.25_vp;
constexpr Dimension TOP_BORDER_P1_X = 1.8_vp;
constexpr Dimension TOP_BORDER_P2_X = 1.0_vp;
constexpr Dimension BUTTOM_BORDER_P2_X = 0.4_vp;
constexpr Dimension BUTTOM_BORDER_P3_X = 1.8_vp;
constexpr Dimension ARROW_HORIZON_P2_OFFSET_X = 7.32_vp;
constexpr Dimension ARROW_VERTICAL_P4_OFFSET_X = 1.5_vp;
constexpr Dimension ARROW_VERTICAL_P4_OFFSET_Y = 7.32_vp;
} // namespace

float ModifyBorderRadius(float borderRadius, float halfChildHeight)
{
    return GreatOrEqual(borderRadius, halfChildHeight) ? halfChildHeight : borderRadius;
}

void BubblePaintMethod::PaintMask(RSCanvas& canvas, PaintWrapper* paintWrapper)
{
    CHECK_NULL_VOID(paintWrapper);
    auto paintProperty = DynamicCast<BubbleRenderProperty>(paintWrapper->GetPaintProperty());
    CHECK_NULL_VOID(paintProperty);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto popupTheme = pipelineContext->GetTheme<PopupTheme>();
    CHECK_NULL_VOID(popupTheme);
    auto maskColor = paintProperty->GetMaskColor().value_or(popupTheme->GetMaskColor());
    auto layoutSize = paintWrapper->GetContentSize();
    canvas.Save();
    RSBrush brush;
    brush.SetColor(maskColor.GetValue());
    brush.SetAntiAlias(true);
    canvas.AttachBrush(brush);
    canvas.DrawRect(RSRect(0.0, 0.0, layoutSize.Width(), layoutSize.Height()));
    canvas.DetachBrush();
    canvas.Restore();
}

void BubblePaintMethod::PaintBorder(RSCanvas& canvas, PaintWrapper* paintWrapper)
{
    BorderEdge edge = border_.Left();
    if (!border_.IsAllEqual()) {
        edge = border_.GetValidEdge();
        border_ = Border(edge);
    }
    if (!border_.HasValue()) {
        PaintDoubleBorder(canvas, paintWrapper);
        return;
    }
    float borderWidth = edge.GetWidth().ConvertToPx();
    RSPen paint;
    paint.SetWidth(borderWidth);
    paint.SetColor(edge.GetColor().GetValue());
    paint.SetAntiAlias(true);
    if (edge.GetBorderStyle() == BorderStyle::DOTTED) {
#ifndef USE_ROSEN_DRAWING
        RSPath dotPath;
        dotPath.AddCircle(0.0f, 0.0f, borderWidth / 2.0);
        paint.SetPathEffect(
            RSPathEffect::CreatePathDashEffect(dotPath, borderWidth * 2.0, 0.0, RSPathDashStyle::ROTATE));
#else
        RSRecordingPath dotPath;
        dotPath.AddCircle(0.0f, 0.0f, borderWidth / 2.0);
        paint.SetPathEffect(
            RSRecordingPathEffect::CreatePathDashEffect(dotPath, borderWidth * 2.0, 0.0, RSPathDashStyle::ROTATE));
#endif
    } else if (edge.GetBorderStyle() == BorderStyle::DASHED) {
#ifndef USE_ROSEN_DRAWING
        const float intervals[] = { borderWidth, borderWidth };
        paint.SetPathEffect(RSPathEffect::CreateDashPathEffect(intervals, 2, 0.0));
#else
        const RSScalar intervals[] = { borderWidth, borderWidth };
        paint.SetPathEffect(RSRecordingPathEffect::CreateDashPathEffect(intervals, 2, 0.0));
#endif
        canvas.AttachPen(paint);
        canvas.DrawPath(path_);
        canvas.DetachPen();
    } else {
        paint.SetPathEffect(nullptr);
    }

    canvas.Save();
    canvas.Translate(childOffset_.GetX() + childSize_.Width() / 2.0, childOffset_.GetY() + childSize_.Height() / 2.0);
    canvas.Scale(1.0 - (borderWidth / childSize_.Width()), 1.0 - (borderWidth / childSize_.Height()));
    canvas.Translate(
        -(childOffset_.GetX() + childSize_.Width() / 2.0), -(childOffset_.GetY() + childSize_.Height() / 2.0));
    canvas.AttachPen(paint);
    auto rect = MakeRRect();
    canvas.DrawRoundRect(rect);
    canvas.DetachPen();
    canvas.Restore();
}

void BubblePaintMethod::PaintBubble(RSCanvas& canvas, PaintWrapper* paintWrapper)
{
    isPaintBubble_ = false;
    CHECK_NULL_VOID(paintWrapper);
    auto paintProperty = DynamicCast<BubbleRenderProperty>(paintWrapper->GetPaintProperty());
    CHECK_NULL_VOID(paintProperty);
    useCustom_ = paintProperty->GetUseCustom().value_or(false);
    enableArrow_ = paintProperty->GetEnableArrow().value_or(true);
    arrowPlacement_ = paintProperty->GetPlacement().value_or(Placement::BOTTOM);
    UpdateArrowOffset(paintProperty->GetArrowOffset(), arrowPlacement_);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto popupTheme = pipelineContext->GetTheme<PopupTheme>();
    CHECK_NULL_VOID(popupTheme);
    backgroundColor_ = paintProperty->GetBackgroundColor().value_or(popupTheme->GetBackgroundColor());
    border_.SetBorderRadius(popupTheme->GetRadius());
    padding_ = popupTheme->GetPadding();
    RSPen paint;
    paint.SetColor(backgroundColor_.GetValue());
    paint.SetAntiAlias(true);
    RSFilter filter;
    filter.SetMaskFilter(RSMaskFilter::CreateBlurMaskFilter(RSBlurType::SOLID, BLUR_MASK_FILTER));
    paint.SetFilter(filter);
    // TODO: color is not correct
    RSBrush brush;
    brush.SetColor(static_cast<int>(backgroundColor_.GetValue()));
    brush.SetAntiAlias(true);
    brush.SetFilter(filter);
    canvas.AttachPen(paint);
    canvas.AttachBrush(brush);
    if (enableArrow_ && showArrow_) {
        if (popupTheme->GetPopupDoubleBorderEnable()) {
            canvas.DetachPen();
            paint.SetWidth(outerBorderWidth_);
            paint.SetColor(popupTheme->GetPopupOuterBorderColor().GetValue());
            canvas.AttachPen(paint);
        }
        PaintBubbleWithArrow(canvas, paintWrapper);
    } else {
        PaintDefaultBubble(canvas);
    }
    canvas.DetachBrush();
    canvas.DetachPen();
    if (enableArrow_ && showArrow_ && popupTheme->GetPopupDoubleBorderEnable()) {
        paint.SetWidth(innerBorderWidth_);
        paint.SetColor(popupTheme->GetPopupInnerBorderColor().GetValue());
        canvas.AttachPen(paint);
        needPaintInnerBorder_ = true;
        PaintBubbleWithArrow(canvas, paintWrapper);
        needPaintInnerBorder_ = false;
        canvas.DetachPen();
    }
}

void BubblePaintMethod::PaintDoubleBorder(RSCanvas& canvas, PaintWrapper* paintWrapper)
{
    if (!Container::GreatOrEqualAPIVersion(PlatformVersion::VERSION_ELEVEN) || isPaintBubble_) {
        return;
    }
    CHECK_NULL_VOID(paintWrapper);
    auto paintProperty = DynamicCast<BubbleRenderProperty>(paintWrapper->GetPaintProperty());
    CHECK_NULL_VOID(paintProperty);
    enableArrow_ = paintProperty->GetEnableArrow().value_or(true);
    arrowPlacement_ = paintProperty->GetPlacement().value_or(Placement::BOTTOM);
    UpdateArrowOffset(paintProperty->GetArrowOffset(), arrowPlacement_);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto popupTheme = pipelineContext->GetTheme<PopupTheme>();
    CHECK_NULL_VOID(popupTheme);
    border_.SetBorderRadius(popupTheme->GetRadius());
    padding_ = popupTheme->GetPadding();
    RSPen paint;
    if (enableArrow_ && showArrow_ && popupTheme->GetPopupDoubleBorderEnable()) {
        // Fill the gap
        isFillTheGap_ = true;
        paint.SetAntiAlias(true);
        paint.SetWidth(innerBorderWidth_ + outerBorderWidth_);
        paint.SetColor(popupTheme->GetPopupInnerBorderColor().GetValue());
        canvas.AttachPen(paint);
        PaintDoubleBorderWithArrow(canvas, paintWrapper);
        isFillTheGap_ = false;
        paint.SetAntiAlias(true);
        paint.SetWidth(innerBorderWidth_);
        paint.SetColor(popupTheme->GetPopupInnerBorderColor().GetValue());
        canvas.AttachPen(paint);
        PaintDoubleBorderWithArrow(canvas, paintWrapper);
        canvas.DetachPen();
    }
    if (enableArrow_ && showArrow_ && popupTheme->GetPopupDoubleBorderEnable()) {
        paint.SetAntiAlias(true);
        paint.SetWidth(outerBorderWidth_);
        paint.SetColor(popupTheme->GetPopupOuterBorderColor().GetValue());
        canvas.AttachPen(paint);
        needPaintInnerBorder_ = true;
        PaintDoubleBorderWithArrow(canvas, paintWrapper);
        needPaintInnerBorder_ = false;
        canvas.DetachPen();
    }
}

void BubblePaintMethod::ClipBubble(PaintWrapper* paintWrapper)
{
    CHECK_NULL_VOID(paintWrapper);
    auto paintProperty = DynamicCast<BubbleRenderProperty>(paintWrapper->GetPaintProperty());
    CHECK_NULL_VOID(paintProperty);
    enableArrow_ = paintProperty->GetEnableArrow().value_or(true);
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto popupTheme = pipelineContext->GetTheme<PopupTheme>();
    CHECK_NULL_VOID(popupTheme);
    border_.SetBorderRadius(popupTheme->GetRadius());
    if (clipFrameNode_) {
        ClipBubbleWithPath(clipFrameNode_);
    }
}

void BubblePaintMethod::UpdateArrowOffset(const std::optional<Dimension>& offset, const Placement& placement)
{
    if (offset.has_value()) {
        arrowOffset_ = offset.value();
        if (arrowOffset_.Unit() == DimensionUnit::PERCENT) {
            arrowOffset_.SetValue(std::clamp(arrowOffset_.Value(), 0.0, 1.0));
        }
        return;
    }
    switch (placement) {
        case Placement::LEFT:
        case Placement::RIGHT:
        case Placement::TOP:
        case Placement::BOTTOM:
            arrowOffset_ = ARROW_HALF_PERCENT_VALUE;
            break;
        case Placement::TOP_LEFT:
        case Placement::BOTTOM_LEFT:
        case Placement::LEFT_TOP:
        case Placement::RIGHT_TOP:
            arrowOffset_ = ARROW_ZERO_PERCENT_VALUE;
            break;
        case Placement::TOP_RIGHT:
        case Placement::BOTTOM_RIGHT:
        case Placement::LEFT_BOTTOM:
        case Placement::RIGHT_BOTTOM:
            arrowOffset_ = ARROW_ONE_HUNDRED_PERCENT_VALUE;
            break;
        default:
            break;
    }
}

void BubblePaintMethod::PaintShadow(const RSPath& path, const Shadow& shadow, RSCanvas& canvas)
{
    canvas.Save();
#ifndef USE_ROSEN_DRAWING
    RSPath rsPath = path;
#else
    RSRecordingPath rsPath;
    rsPath.AddPath(path);
#endif
    rsPath.Offset(shadow.GetOffset().GetX(), shadow.GetOffset().GetY());
    RSColor spotColor = ToRSColor(shadow.GetColor());
    RSPoint3 planeParams = { 0.0f, 0.0f, shadow.GetElevation() };
#ifndef USE_ROSEN_DRAWING
    RSPoint3 lightPos = { rsPath.GetBounds().GetLeft() / 2.0 + rsPath.GetBounds().GetRight() / 2.0,
        rsPath.GetBounds().GetTop() / 2.0 + rsPath.GetBounds().GetBottom() / 2.0, shadow.GetLightHeight() };
#else
    auto bounds = rsPath.GetBounds();
    RSPoint3 lightPos = { bounds.GetLeft() / 2.0 + bounds.GetRight() / 2.0,
        bounds.GetTop() / 2.0 + bounds.GetBottom() / 2.0, shadow.GetLightHeight() };
#endif
    RSColor ambientColor = RSColor(0, 0, 0, 0);
    canvas.DrawShadow(rsPath, planeParams, lightPos, shadow.GetLightRadius(), ambientColor, spotColor,
        RSShadowFlags::TRANSPARENT_OCCLUDER);
    canvas.Restore();
}

void BubblePaintMethod::PaintDefaultBubble(RSCanvas& canvas)
{
    auto rect = MakeRRect();
    path_.AddRoundRect(
        rect.GetRect(), border_.TopLeftRadius().GetX().ConvertToPx(), border_.TopRightRadius().GetX().ConvertToPx());
    canvas.Save();
    canvas.ClipPath(path_, RSClipOp::DIFFERENCE, true);
    PaintShadow(path_, ShadowConfig::DefaultShadowM, canvas);
    canvas.Restore();
    canvas.DrawRoundRect(rect);
}

RSRoundRect BubblePaintMethod::MakeRRect()
{
    auto rect = RSRect(childOffset_.GetX(), childOffset_.GetY(), childOffset_.GetX() + childSize_.Width(),
        childOffset_.GetY() + childSize_.Height());
    std::vector<RSPoint> rectRadii;
    rectRadii.resize(4);
    rectRadii[RSRoundRect::TOP_LEFT_POS] =
        RSPoint(border_.TopLeftRadius().GetX().ConvertToPx(), border_.TopLeftRadius().GetY().ConvertToPx());
    rectRadii[RSRoundRect::TOP_RIGHT_POS] =
        RSPoint(border_.TopRightRadius().GetX().ConvertToPx(), border_.TopRightRadius().GetY().ConvertToPx());
    rectRadii[RSRoundRect::BOTTOM_RIGHT_POS] =
        RSPoint(border_.BottomRightRadius().GetX().ConvertToPx(), border_.BottomRightRadius().GetY().ConvertToPx());
    rectRadii[RSRoundRect::BOTTOM_LEFT_POS] =
        RSPoint(border_.BottomLeftRadius().GetX().ConvertToPx(), border_.BottomLeftRadius().GetY().ConvertToPx());
    return RSRoundRect(rect, rectRadii);
}

void BubblePaintMethod::PaintDoubleBorderWithArrow(RSCanvas& canvas, PaintWrapper* paintWrapper)
{
    BuildDoubleBorderPath(path_);
    canvas.Save();
    if (needPaintInnerBorder_) {
        PaintShadow(path_, ShadowConfig::DefaultShadowM, canvas);
    }
    canvas.Restore();
    canvas.DrawPath(path_);
}

void BubblePaintMethod::PaintBubbleWithArrow(RSCanvas& canvas, PaintWrapper* paintWrapper)
{
    BuildCompletePath(path_);
    canvas.Save();
    canvas.ClipPath(path_, RSClipOp::DIFFERENCE, true);
    if (!needPaintInnerBorder_) {
        PaintShadow(path_, ShadowConfig::DefaultShadowM, canvas);
    }
    canvas.Restore();
    canvas.DrawPath(path_);
}

float BubblePaintMethod::GetInnerBorderOffset()
{
    float borderOffset = 0;
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, 0);
    auto popupTheme = pipeline->GetTheme<PopupTheme>();
    CHECK_NULL_RETURN(popupTheme, 0);
    if (popupTheme->GetPopupDoubleBorderEnable() && needPaintInnerBorder_) {
        borderOffset = outerBorderWidth_;
    }
    return borderOffset;
}

float BubblePaintMethod::GetBorderOffset()
{
    if (isFillTheGap_) {
        return 0.0f;
    }
    float borderOffset = 0.0f;
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, 0);
    auto popupTheme = pipeline->GetTheme<PopupTheme>();
    CHECK_NULL_RETURN(popupTheme, 0);
    if (popupTheme->GetPopupDoubleBorderEnable()) {
        borderOffset = -innerBorderWidth_;
        if (needPaintInnerBorder_) {
            borderOffset = -innerBorderWidth_;
        }
    }
    return borderOffset;
}

void BubblePaintMethod::BuildDoubleBorderPath(RSPath& path)
{
    float borderOffset = GetBorderOffset();
    auto borderRadius = ModifyBorderRadius(border_.BottomLeftRadius().GetY().ConvertToPx(), childSize_.Height() / 2);
    float radiusPx = borderRadius - borderOffset;
    path.Reset();
    path.MoveTo(childOffset_.GetX() + radiusPx, childOffset_.GetY() + borderOffset);
    BuildTopDoubleBorderPath(path, arrowOffsetsFromClip_[0], radiusPx);
    BuildCornerPath(path, Placement::TOP_RIGHT, radiusPx);
    BuildRightDoubleBorderPath(path, arrowOffsetsFromClip_[1], radiusPx);
    BuildCornerPath(path, Placement::BOTTOM_RIGHT, radiusPx);
    BuildBottomDoubleBorderPath(path, arrowOffsetsFromClip_[2], radiusPx);
    BuildCornerPath(path, Placement::BOTTOM_LEFT, radiusPx);
    BuildLeftDoubleBorderPath(path, arrowOffsetsFromClip_[3], radiusPx);
    BuildCornerPath(path, Placement::TOP_LEFT, radiusPx);
    path.Close();
}

void BubblePaintMethod::BuildCompletePath(RSPath& path)
{
    float borderOffset = GetBorderOffset();
    float arrowOffset = GetArrowOffset(arrowPlacement_);
    auto borderRadius = ModifyBorderRadius(border_.BottomLeftRadius().GetY().ConvertToPx(), childSize_.Height() / 2);
    float radiusPx = borderRadius - borderOffset;
    path.Reset();
    path.MoveTo(childOffset_.GetX() + radiusPx, childOffset_.GetY() + borderOffset);
    BuildTopLinePath(path, arrowOffset, radiusPx);
    BuildCornerPath(path, Placement::TOP_RIGHT, radiusPx);
    BuildRightLinePath(path, arrowOffset, radiusPx);
    BuildCornerPath(path, Placement::BOTTOM_RIGHT, radiusPx);
    BuildBottomLinePath(path, arrowOffset, radiusPx);
    BuildCornerPath(path, Placement::BOTTOM_LEFT, radiusPx);
    BuildLeftLinePath(path, arrowOffset, radiusPx);
    BuildCornerPath(path, Placement::TOP_LEFT, radiusPx);
    path.Close();
}

void BubblePaintMethod::BuildTopLinePath(RSPath& path, float arrowOffset, float radius)
{
    float borderOffset = GetBorderOffset();
    float childOffsetY = childOffset_.GetY();
    float arrowPositionY = arrowPosition_.GetY();
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto popupTheme = pipeline->GetTheme<PopupTheme>();
    CHECK_NULL_VOID(popupTheme);
    auto leftOffset =
        childOffset_.GetX() + popupTheme->GetRadius().GetX().ConvertToPx() + ARROW_WIDTH.ConvertToPx() / 2;
    auto rightOffset = childOffset_.GetX() + childSize_.Width() - popupTheme->GetRadius().GetX().ConvertToPx() -
                       ARROW_WIDTH.ConvertToPx() / 2;
    auto arrowTopOffset = std::clamp(
        arrowPosition_.GetX() + arrowOffset, static_cast<float>(leftOffset), static_cast<float>(rightOffset));
    switch (arrowPlacement_) {
        case Placement::BOTTOM:
        case Placement::BOTTOM_LEFT:
        case Placement::BOTTOM_RIGHT:
            path.LineTo(arrowTopOffset - BEZIER_WIDTH_HALF.ConvertToPx() + borderOffset,
                childOffsetY + borderOffset);
            path.QuadTo(arrowTopOffset - BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPositionY + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx() + borderOffset,
                arrowTopOffset - BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx(),
                arrowPositionY + BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx() + borderOffset);
            path.QuadTo(arrowTopOffset - BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx() + borderOffset,
                arrowTopOffset, arrowPositionY + borderOffset);
            path.QuadTo(arrowTopOffset + BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx() + borderOffset,
                arrowTopOffset + BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx(),
                arrowPositionY + BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx() + borderOffset);
            path.QuadTo(arrowTopOffset + BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPositionY + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx() + borderOffset,
                arrowTopOffset + BEZIER_HORIZON_OFFSET_FOURTH.ConvertToPx(),
                arrowPositionY + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx() + borderOffset);
            break;
        default:
            break;
    }
    path.LineTo(childOffset_.GetX() + childSize_.Width() - radius, childOffsetY + borderOffset);
}

void BubblePaintMethod::BuildCornerPath(RSPath& path, const Placement& placement, float radius)
{
    float borderOffset = GetBorderOffset();
    float childOffsetY = childOffset_.GetY();
    switch (placement) {
        case Placement::TOP_LEFT:
            path.ArcTo(radius, radius, 0.0f, RSPathDirection::CW_DIRECTION,
                childOffset_.GetX() + radius, childOffsetY + borderOffset);
            break;
        case Placement::TOP_RIGHT:
            path.ArcTo(radius, radius, 0.0f, RSPathDirection::CW_DIRECTION,
                childOffset_.GetX() + childSize_.Width() - borderOffset, childOffsetY + radius);
            break;
        case Placement::BOTTOM_RIGHT:
            path.ArcTo(radius, radius, 0.0f, RSPathDirection::CW_DIRECTION,
                childOffset_.GetX() + childSize_.Width() - radius - borderOffset,
                childOffsetY + childSize_.Height() - borderOffset);
            break;
        case Placement::BOTTOM_LEFT:
            path.ArcTo(radius, radius, 0.0f, RSPathDirection::CW_DIRECTION,
                childOffset_.GetX() + borderOffset,
                childOffsetY + childSize_.Height() - radius + borderOffset);
            break;
        default:
            break;
    }
}

void BubblePaintMethod::BuildRightLinePath(RSPath& path, float arrowOffset, float radius)
{
    float borderOffset = GetBorderOffset();
    float childOffsetY = childOffset_.GetY();
    float arrowPositionY = arrowPosition_.GetY();
    switch (arrowPlacement_) {
        case Placement::LEFT:
        case Placement::LEFT_TOP:
        case Placement::LEFT_BOTTOM:
            path.LineTo(childOffset_.GetX() + childSize_.Width() - borderOffset,
                arrowPositionY + arrowOffset - BEZIER_WIDTH_HALF.ConvertToPx() + borderOffset);
            path.QuadTo(arrowPosition_.GetX() - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx() - borderOffset,
                arrowPositionY + arrowOffset - BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPosition_.GetX() - BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx() - borderOffset,
                arrowPositionY + arrowOffset - BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx());
            path.QuadTo(arrowPosition_.GetX() + BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx() - borderOffset,
                arrowPositionY + arrowOffset - BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx(),
                arrowPosition_.GetX() - borderOffset, arrowPositionY + arrowOffset);
            path.QuadTo(arrowPosition_.GetX() + BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx() - borderOffset,
                arrowPositionY + arrowOffset + BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx(),
                arrowPosition_.GetX() - BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx() - borderOffset,
                arrowPositionY + arrowOffset + BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx());
            path.QuadTo(arrowPosition_.GetX() - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx() - borderOffset,
                arrowPositionY + arrowOffset + BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPosition_.GetX() - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx() - borderOffset,
                arrowPositionY + arrowOffset + BEZIER_HORIZON_OFFSET_FOURTH.ConvertToPx());
            break;
        default:
            break;
    }
    path.LineTo(childOffset_.GetX() + childSize_.Width() - borderOffset,
        childOffsetY + childSize_.Height() - radius - borderOffset);
}

void BubblePaintMethod::BuildRightDoubleBorderPath(RSPath& path, float arrowOffset, float radius)
{
    float borderOffset = GetBorderOffset();
    float childOffsetY = childOffset_.GetY();
    float arrowPositionX = arrowPosition_.GetX();
    float arrowRightOffset = childOffset_.GetY() + arrowOffset;
    switch (arrowPlacement_) {
        case Placement::LEFT:
        case Placement::LEFT_TOP:
        case Placement::LEFT_BOTTOM:
            path.LineTo(childOffset_.GetX() + childSize_.Width() - borderOffset,
                arrowRightOffset + ARROW_POINT_OFFSET_Y.ConvertToPx() + borderOffset);
            path.LineTo(arrowPositionX + ARROW_POINT_OFFSET_X.ConvertToPx() - borderOffset,
                arrowRightOffset + ARROW_POINT_OFFSET_Y.ConvertToPx() + ARROW_WIDTH_SMALL.ConvertToPx() / HALF
                + borderOffset);
            path.ArcTo(ARROW_RADIUS.ConvertToPx(), ARROW_RADIUS.ConvertToPx(), 0.0f, 0,
                childOffset_.GetX() + childSize_.Width() - ARROW_VERTICAL_P4_OFFSET_X.ConvertToPx() + borderOffset,
                arrowRightOffset + ARROW_POINT_OFFSET_Y.ConvertToPx() + ARROW_WIDTH_SMALL.ConvertToPx() / HALF
                + ARROW_VERTICAL_P4_OFFSET_Y.ConvertToPx() - borderOffset);
            path.LineTo(childOffset_.GetX() + childSize_.Width() - borderOffset,
                arrowRightOffset + ARROW_WIDTH_SMALL.ConvertToPx() + borderOffset);
            break;
        default:
            break;
    }
    path.LineTo(childOffset_.GetX() + childSize_.Width() - borderOffset,
        childOffsetY + childSize_.Height() - radius - borderOffset);
}

void BubblePaintMethod::BuildBottomLinePath(RSPath& path, float arrowOffset, float radius)
{
    float borderOffset = GetBorderOffset();
    float childOffsetY = childOffset_.GetY();
    float arrowPositionY = arrowPosition_.GetY();
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto popupTheme = pipeline->GetTheme<PopupTheme>();
    CHECK_NULL_VOID(popupTheme);
    auto leftOffset =
        childOffset_.GetX() + popupTheme->GetRadius().GetX().ConvertToPx() + ARROW_WIDTH.ConvertToPx() / HALF;
    auto rightOffset = childOffset_.GetX() + childSize_.Width() - popupTheme->GetRadius().GetX().ConvertToPx() -
                       ARROW_WIDTH.ConvertToPx() / HALF;
    auto arrowBottomOffset = std::clamp(
        arrowPosition_.GetX() + arrowOffset, static_cast<float>(leftOffset), static_cast<float>(rightOffset));
    switch (arrowPlacement_) {
        case Placement::TOP:
        case Placement::TOP_LEFT:
        case Placement::TOP_RIGHT:
            path.LineTo(arrowBottomOffset + BEZIER_WIDTH_HALF.ConvertToPx() - borderOffset,
                childOffsetY + childSize_.Height() - borderOffset);
            path.QuadTo(arrowBottomOffset + BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx() - borderOffset,
                arrowBottomOffset + BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx() - borderOffset);
            path.QuadTo(arrowBottomOffset + BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx() - borderOffset,
                arrowBottomOffset, arrowPositionY - borderOffset);
            path.QuadTo(arrowBottomOffset - BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx() - borderOffset,
                arrowBottomOffset - BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx() - borderOffset);
            path.QuadTo(arrowBottomOffset - BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx() - borderOffset,
                arrowBottomOffset - BEZIER_HORIZON_OFFSET_FOURTH.ConvertToPx(),
                arrowPositionY - BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx() - borderOffset);
            break;
        default:
            break;
    }
    path.LineTo(childOffset_.GetX() + radius, childOffsetY + childSize_.Height() - borderOffset);
}

void BubblePaintMethod::BuildBottomDoubleBorderPath(RSPath& path, float arrowOffset, float radius)
{
    float borderOffset = GetBorderOffset();
    float childOffsetY = childOffset_.GetY();
    float arrowPositionY = arrowPosition_.GetY();
    float arrowBottomOffset = childOffset_.GetX() + arrowOffset;
    switch (arrowPlacement_) {
        case Placement::TOP:
        case Placement::TOP_LEFT:
        case Placement::TOP_RIGHT:
            path.LineTo(arrowBottomOffset - borderOffset,
                childOffsetY + childSize_.Height() - borderOffset);
            path.LineTo(arrowBottomOffset - ARROW_WIDTH_SMALL.ConvertToPx() / HALF - ARROW_POINT_OFFSET_X.ConvertToPx()
                - borderOffset + BUTTOM_BORDER_P2_X.ConvertToPx(),
                arrowPositionY + ARROW_POINT_OFFSET_Y.ConvertToPx() - borderOffset);
            path.ArcTo(ARROW_RADIUS.ConvertToPx() - borderOffset, ARROW_RADIUS.ConvertToPx() - borderOffset, 0.0f, 0,
                arrowBottomOffset - ARROW_WIDTH_SMALL.ConvertToPx() / HALF - ARROW_VERTICAL_P4_OFFSET_X.ConvertToPx()
                - BUTTOM_BORDER_P3_X.ConvertToPx() + borderOffset,
                arrowPositionY + ARROW_POINT_OFFSET_Y.ConvertToPx() - borderOffset);
            path.LineTo(arrowBottomOffset - ARROW_WIDTH_SMALL.ConvertToPx() + borderOffset,
                childOffsetY + childSize_.Height() - borderOffset);
            break;
        default:
            break;
    }
    path.LineTo(childOffset_.GetX() + radius, childOffsetY + childSize_.Height() - borderOffset);
}

void BubblePaintMethod::BuildTopDoubleBorderPath(RSPath& path, float arrowOffset, float radius)
{
    float borderOffset = GetBorderOffset();
    float childOffsetY = childOffset_.GetY();
    float arrowPositionY = arrowPosition_.GetY();
    float arrowTopOffset = childOffset_.GetX() + arrowOffset;
    switch (arrowPlacement_) {
        case Placement::BOTTOM:
        case Placement::BOTTOM_LEFT:
        case Placement::BOTTOM_RIGHT:
            path.LineTo(arrowTopOffset - TOP_BORDER_P1_X.ConvertToPx(),
                childOffsetY + borderOffset);
            path.LineTo(arrowTopOffset + ARROW_WIDTH_SMALL.ConvertToPx() / HALF - TOP_BORDER_P2_X.ConvertToPx()
                - borderOffset,
                arrowPositionY - ARROW_POINT_OFFSET_Y.ConvertToPx() + borderOffset);
            path.ArcTo(ARROW_RADIUS.ConvertToPx() - borderOffset, ARROW_RADIUS.ConvertToPx() - borderOffset, 0.0f, 0,
                arrowTopOffset - borderOffset + ARROW_VERTICAL_P4_OFFSET_X.ConvertToPx(),
                arrowPositionY - ARROW_POINT_OFFSET_Y.ConvertToPx() + borderOffset);
            path.LineTo(arrowTopOffset - borderOffset + ARROW_WIDTH_SMALL.ConvertToPx(),
                childOffsetY + borderOffset);
            break;
        default:
            break;
    }
    path.LineTo(childOffset_.GetX() + childSize_.Width() - radius, childOffsetY + borderOffset);
}

void BubblePaintMethod::BuildLeftLinePath(RSPath& path, float arrowOffset, float radius)
{
    float borderOffset = GetBorderOffset();
    float childOffsetY = childOffset_.GetY();
    float arrowPositionY = arrowPosition_.GetY();
    switch (arrowPlacement_) {
        case Placement::RIGHT:
        case Placement::RIGHT_TOP:
        case Placement::RIGHT_BOTTOM:
            path.LineTo(childOffset_.GetX() + borderOffset,
                arrowPositionY + arrowOffset + BEZIER_WIDTH_HALF.ConvertToPx() - borderOffset);
            path.QuadTo(arrowPosition_.GetX() + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx() + borderOffset,
                arrowPositionY + arrowOffset + BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPosition_.GetX() + BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx() + borderOffset,
                arrowPositionY + arrowOffset + BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx());
            path.QuadTo(arrowPosition_.GetX() - BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx() + borderOffset,
                arrowPositionY + arrowOffset + BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx() + borderOffset,
                arrowPosition_.GetX(), arrowPositionY + arrowOffset);
            path.QuadTo(arrowPosition_.GetX() - BEZIER_VERTICAL_OFFSET_FIRST.ConvertToPx() + borderOffset,
                arrowPositionY + arrowOffset - BEZIER_HORIZON_OFFSET_FIRST.ConvertToPx(),
                arrowPosition_.GetX() + BEZIER_VERTICAL_OFFSET_SECOND.ConvertToPx() + borderOffset,
                arrowPositionY + arrowOffset - BEZIER_HORIZON_OFFSET_SECOND.ConvertToPx());
            path.QuadTo(arrowPosition_.GetX() + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx() + borderOffset,
                arrowPositionY + arrowOffset - BEZIER_HORIZON_OFFSET_THIRD.ConvertToPx(),
                arrowPosition_.GetX() + BEZIER_VERTICAL_OFFSET_THIRD.ConvertToPx() + borderOffset,
                arrowPositionY + arrowOffset - BEZIER_HORIZON_OFFSET_FOURTH.ConvertToPx());
            break;
        default:
            break;
    }
    path.LineTo(childOffset_.GetX() + borderOffset, childOffsetY + radius + borderOffset);
}

void BubblePaintMethod::BuildLeftDoubleBorderPath(RSPath& path, float arrowOffset, float radius)
{
    float borderOffset = GetBorderOffset();
    float childOffsetY = childOffset_.GetY();
    float arrowLeftOffset = childOffset_.GetY() + arrowOffset - ARROW_WIDTH_SMALL.ConvertToPx();
    switch (arrowPlacement_) {
        case Placement::RIGHT:
        case Placement::RIGHT_TOP:
        case Placement::RIGHT_BOTTOM:
            path.LineTo(childOffset_.GetX() + borderOffset,
                arrowLeftOffset + ARROW_WIDTH_SMALL.ConvertToPx() - borderOffset);
            path.LineTo(childOffset_.GetX() + borderOffset - ARROW_HORIZON_P2_OFFSET_X.ConvertToPx()
                - ARROW_POINT_OFFSET_X.ConvertToPx(),
                arrowLeftOffset + ARROW_WIDTH_SMALL.ConvertToPx() / HALF - borderOffset);
            path.ArcTo(ARROW_RADIUS.ConvertToPx(), ARROW_RADIUS.ConvertToPx(), 0.0f, 0,
                childOffset_.GetX() - ARROW_VERTICAL_P4_OFFSET_X.ConvertToPx() + borderOffset,
                arrowLeftOffset + ARROW_WIDTH_SMALL.ConvertToPx() / HALF + ARROW_VERTICAL_P4_OFFSET_Y.ConvertToPx()
                + borderOffset);
            path.LineTo(childOffset_.GetX() + borderOffset, arrowLeftOffset + borderOffset);
            break;
        default:
            break;
    }
    path.LineTo(childOffset_.GetX() + borderOffset, childOffsetY + radius + borderOffset);
}

float BubblePaintMethod::GetArrowOffset(const Placement& placement)
{
    double motionRange = 0.0;
    Edge edge;
    InitEdgeSize(edge);
    switch (placement) {
        case Placement::TOP_LEFT:
        case Placement::TOP_RIGHT:
            motionRange = childSize_.Width() - edge.Top().Value() - ARROW_WIDTH.ConvertToPx();
            break;
        case Placement::TOP:
            motionRange = childSize_.Width() - edge.Top().Value() - ARROW_WIDTH.ConvertToPx();
            break;
        case Placement::BOTTOM:
            motionRange = childSize_.Width() - edge.Bottom().Value() - ARROW_WIDTH.ConvertToPx();
            break;
        case Placement::LEFT:
        case Placement::LEFT_TOP:
        case Placement::LEFT_BOTTOM:
            motionRange = childSize_.Height() - edge.Left().Value() - ARROW_WIDTH.ConvertToPx();
            break;
        case Placement::RIGHT:
        case Placement::RIGHT_TOP:
        case Placement::RIGHT_BOTTOM:
            motionRange = childSize_.Height() - edge.Right().Value() - ARROW_WIDTH.ConvertToPx();
            break;
        case Placement::BOTTOM_LEFT:
        case Placement::BOTTOM_RIGHT:
            motionRange = childSize_.Width() - edge.Bottom().Value() - ARROW_WIDTH.ConvertToPx();
            break;
        default:
            break;
    }
    return std::clamp(
        arrowOffset_.Unit() == DimensionUnit::PERCENT ? arrowOffset_.Value() * motionRange : arrowOffset_.ConvertToPx(),
        0.0, motionRange);
}

void BubblePaintMethod::InitEdgeSize(Edge& edge)
{
    edge.SetTop(Dimension(std::max(padding_.Left().ConvertToPx(), border_.TopLeftRadius().GetX().ConvertToPx()) +
                          std::max(padding_.Right().ConvertToPx(), border_.TopRightRadius().GetX().ConvertToPx())));
    edge.SetBottom(
        Dimension(std::max(padding_.Left().ConvertToPx(), border_.BottomLeftRadius().GetX().ConvertToPx()) +
                  std::max(padding_.Right().ConvertToPx(), border_.BottomRightRadius().GetX().ConvertToPx())));
    edge.SetLeft(
        Dimension(std::max(padding_.Top().ConvertToPx(), border_.TopRightRadius().GetY().ConvertToPx()) +
                  std::max(padding_.Bottom().ConvertToPx(), border_.BottomRightRadius().GetY().ConvertToPx())));
    edge.SetRight(
        Dimension(std::max(padding_.Top().ConvertToPx(), border_.TopLeftRadius().GetY().ConvertToPx()) +
                  std::max(padding_.Bottom().ConvertToPx(), border_.BottomLeftRadius().GetY().ConvertToPx())));
}

void BubblePaintMethod::ClipBubbleWithPath(const RefPtr<FrameNode>& frameNode)
{
    auto path = AceType::MakeRefPtr<Path>();
    path->SetValue(clipPath_);
    path->SetBasicShapeType(BasicShapeType::PATH);
    auto renderContext = frameNode->GetRenderContext();
    renderContext->UpdateClipShape(path);
}

} // namespace OHOS::Ace::NG
