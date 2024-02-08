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
#include "core/components_ng/pattern/text_field/text_field_paint_method.h"

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
#include "core/components/popup/popup_theme.h"
#include "core/components/theme/theme_manager.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/search/search_event_hub.h"
#include "core/components_ng/pattern/search/search_pattern.h"
#include "core/components_ng/pattern/search/search_text_field.h"
#include "core/components_ng/pattern/text_field/text_field_pattern.h"
#include "core/components_ng/property/measure_utils.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/components_ng/render/image_painter.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
TextFieldPaintMethod::TextFieldPaintMethod(const WeakPtr<Pattern>& pattern,
    const RefPtr<TextFieldOverlayModifier>& textFieldOverlayModifier,
    const RefPtr<TextFieldContentModifier>& textFieldContentModifier)
    : pattern_(pattern), textFieldOverlayModifier_(textFieldOverlayModifier),
      textFieldContentModifier_(textFieldContentModifier)
{}

RefPtr<Modifier> TextFieldPaintMethod::GetContentModifier(PaintWrapper* paintWrapper)
{
    return textFieldContentModifier_;
}

void TextFieldPaintMethod::UpdateContentModifier(PaintWrapper* paintWrapper)
{
    CHECK_NULL_VOID(paintWrapper);
    CHECK_NULL_VOID(textFieldContentModifier_);
    auto paintProperty = DynamicCast<TextFieldPaintProperty>(paintWrapper->GetPaintProperty());
    CHECK_NULL_VOID(paintProperty);
    OffsetF contentOffset = paintWrapper->GetContentOffset();
    SizeF contentSize = paintWrapper->GetContentSize();
    auto textFieldPattern = DynamicCast<TextFieldPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(textFieldPattern);
    if (textFieldPattern->GetContChange()) {
        textFieldContentModifier_->ChangeDragStatus();
        textFieldPattern->ResetContChange();
    }

    auto textFieldLayoutProperty = textFieldPattern->GetLayoutProperty<TextFieldLayoutProperty>();
    CHECK_NULL_VOID(textFieldLayoutProperty);
    auto textValue = textFieldPattern->GetTextValue();
    auto isPasswordType = textFieldPattern->IsInPasswordMode();
    auto showPlaceHolder = textFieldLayoutProperty->GetValueValue("").empty();
    auto needObscureText = isPasswordType && textFieldPattern->GetTextObscured() && !showPlaceHolder;
    auto text =
        TextFieldPattern::CreateDisplayText(textValue, textFieldPattern->GetNakedCharPosition(), needObscureText);
    auto displayText = StringUtils::Str16ToStr8(text);
    textFieldContentModifier_->SetTextValue(displayText);
    textFieldContentModifier_->SetPlaceholderValue(textFieldPattern->GetPlaceHolder());

    auto frameNode = textFieldPattern->GetHost();
    CHECK_NULL_VOID(frameNode);
    auto currentTextRectOffsetX = textFieldPattern->GetTextRect().GetX();
    auto currentTextRectOffsetY =
        textFieldPattern->IsTextArea() ? textFieldPattern->GetTextRect().GetY() : contentOffset.GetY();
    if (textFieldContentModifier_->GetTextRectX() != currentTextRectOffsetX ||
        (textFieldPattern->IsTextArea() ? textFieldContentModifier_->GetTextRectY()
                                        : textFieldContentModifier_->GetContentOffsetY()) != currentTextRectOffsetY) {
        auto searchField = DynamicCast<SearchTextFieldPattern>(textFieldPattern);
        if (searchField) {
            auto parentFrameNode = AceType::DynamicCast<FrameNode>(frameNode->GetParent());
            CHECK_NULL_VOID(parentFrameNode);
            auto searchPattern = parentFrameNode->GetPattern<SearchPattern>();
            CHECK_NULL_VOID(searchPattern);
            auto textFieldOffset = searchPattern->GetTextFieldOffset();
            currentTextRectOffsetX += textFieldOffset.GetX();
            currentTextRectOffsetY += textFieldOffset.GetY();
        }
        auto eventHub = frameNode->GetEventHub<TextFieldEventHub>();
        eventHub->FireOnScrollChangeEvent(currentTextRectOffsetX, currentTextRectOffsetY);
    }
    textFieldContentModifier_->SetContentOffset(contentOffset);
    textFieldContentModifier_->SetContentSize(contentSize);
    textFieldContentModifier_->SetTextRectY(textFieldPattern->GetTextRect().GetY());
    textFieldContentModifier_->SetTextRectX(textFieldPattern->GetTextRect().GetX());
    textFieldContentModifier_->SetTextAlign(textFieldPattern->GetTextAlign());
    auto layoutProperty = frameNode->GetLayoutProperty<TextFieldLayoutProperty>();
    CHECK_NULL_VOID(layoutProperty);
    textFieldContentModifier_->SetTextObscured(textFieldPattern->GetTextObscured());
    textFieldContentModifier_->SetShowErrorState(
        layoutProperty->GetShowErrorTextValue(false) &&
        !textFieldPattern->IsNormalInlineState());
    textFieldContentModifier_->SetErrorTextValue(layoutProperty->GetErrorTextValue(""));
    textFieldContentModifier_->SetShowUnderlineState(layoutProperty->GetShowUnderlineValue(false));
    auto pipeline = frameNode->GetContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetTheme<TextFieldTheme>();
    PropertyChangeFlag flag = 0;
    if (textFieldContentModifier_->NeedMeasureUpdate(flag)) {
        frameNode->MarkDirtyNode(flag);
    }
}

RefPtr<Modifier> TextFieldPaintMethod::GetOverlayModifier(PaintWrapper* paintWrapper)
{
    return textFieldOverlayModifier_;
}

void TextFieldPaintMethod::UpdateOverlayModifier(PaintWrapper* paintWrapper)
{
    CHECK_NULL_VOID(paintWrapper);
    CHECK_NULL_VOID(textFieldOverlayModifier_);

    auto textFieldPattern = DynamicCast<TextFieldPattern>(pattern_.Upgrade());
    CHECK_NULL_VOID(textFieldPattern);
    auto paintProperty = DynamicCast<TextFieldPaintProperty>(paintWrapper->GetPaintProperty());
    CHECK_NULL_VOID(paintProperty);

    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    auto themeManager = pipelineContext->GetThemeManager();
    CHECK_NULL_VOID(themeManager);
    auto theme = themeManager->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);

    OffsetF contentOffset = paintWrapper->GetContentOffset();
    textFieldOverlayModifier_->SetContentOffset(contentOffset);
    SizeF contentSize = paintWrapper->GetContentSize();
    textFieldOverlayModifier_->SetContentSize(contentSize);
    auto frameSize = paintWrapper->GetGeometryNode()->GetFrameSize();
    textFieldOverlayModifier_->SetFrameSize(frameSize);
    auto currentOffset = textFieldPattern->GetCurrentOffset();
    textFieldOverlayModifier_->SetCurrentOffset(currentOffset);

    auto cursorVisible = textFieldPattern->GetCursorVisible();
    textFieldOverlayModifier_->SetCursorVisible(cursorVisible);
    auto cursorRect = textFieldPattern->GetCaretRect();
    textFieldOverlayModifier_->SetCursorOffset(cursorRect.GetOffset());
    textFieldOverlayModifier_->SetCursorWidth(cursorRect.Width());
    auto cursorColor = paintProperty->GetCursorColorValue(theme->GetCursorColor());
    textFieldOverlayModifier_->SetCursorColor(cursorColor);

    InputStyle inputStyle = paintProperty->GetInputStyleValue(InputStyle::DEFAULT);
    textFieldOverlayModifier_->SetInputStyle(inputStyle);

    auto selectedColor = paintProperty->GetSelectedBackgroundColorValue(theme->GetSelectedColor());
    textFieldOverlayModifier_->SetSelectedBackGroundColor(selectedColor);
    textFieldOverlayModifier_->SetChangeSelectedRects(textFieldPattern->NeedPaintSelect());

    textFieldOverlayModifier_->SetUnderlineWidth(textFieldPattern->GetUnderlineWidth());
    textFieldOverlayModifier_->SetUnderlineColor(textFieldPattern->GetUnderlineColor());

    textFieldOverlayModifier_->SetShowSelect(textFieldPattern->GetShowSelect());
    textFieldOverlayModifier_->SetChangeSelectedRects(textFieldPattern->NeedPaintSelect());

    textFieldOverlayModifier_->SetTextRect(textFieldPattern->GetTextRect());
    UpdateScrollBar();
}

void TextFieldPaintMethod::UpdateScrollBar()
{
    auto scrollBar = scrollBar_.Upgrade();
    if (!scrollBar || !scrollBar->NeedPaint()) {
        return;
    }
    if (scrollBar->GetPositionModeUpdate()) {
        textFieldOverlayModifier_->SetPositionMode(scrollBar->GetPositionMode());
    }
    OffsetF fgOffset(scrollBar->GetActiveRect().Left(), scrollBar->GetActiveRect().Top());
    textFieldOverlayModifier_->StartBarAnimation(scrollBar->GetHoverAnimationType(),
        scrollBar->GetOpacityAnimationType(), scrollBar->GetNeedAdaptAnimation(), scrollBar->GetActiveRect());
    scrollBar->SetHoverAnimationType(HoverAnimationType::NONE);
    textFieldOverlayModifier_->SetBarColor(scrollBar->GetForegroundColor());
    scrollBar->SetOpacityAnimationType(OpacityAnimationType::NONE);
}
} // namespace OHOS::Ace::NG
