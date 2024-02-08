/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "bridge/declarative_frontend/engine/jsi/components/arkts_native_text_input_modifier.h"

#include "base/geometry/dimension.h"
#include "core/components/common/layout/constants.h"
#include "core/components/text_field/textfield_theme.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/text_field/text_field_model_ng.h"
#include "core/pipeline/base/element_register.h"
#include "bridge/common/utils/utils.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components/common/properties/alignment.h"
#include "core/components/text_field/textfield_theme.h"
#include "bridge/declarative_frontend/engine/jsi/nativeModule/arkts_utils.h"

namespace OHOS::Ace::NG {
const uint32_t MAX_LINES = 3;
constexpr uint32_t DEFAULT_CARET_COLOR = 0xFF007DFF;
constexpr uint32_t DEFAULT_CARE_POSITION = 0;
constexpr CopyOptions DEFAULT_TEXT_INPUT_COPY_OPTION = CopyOptions::Local;
constexpr bool DEFAULT_SHOW_PASSWORD_ICON_VALUE = true;
constexpr TextAlign DAFAULT_TEXT_ALIGN_VALUE = TextAlign::START;
constexpr InputStyle DEFAULT_INPUT_STYLE = InputStyle::DEFAULT;
constexpr bool DEFAULT_SELECTION_MENU_HIDDEN = false;
constexpr bool DEFAULT_SHOW_UNDER_LINE = false;
constexpr bool DEFAULT_REQUEST_KEYBOARD_ON_FOCUS = true;
constexpr DisplayMode DEFAULT_BAR_STATE = DisplayMode::AUTO;
constexpr FontWeight DEFAULT_FONT_WEIGHT = FontWeight::NORMAL;
constexpr Ace::FontStyle DEFAULT_FONT_STYLE = Ace::FontStyle::NORMAL;
constexpr int16_t DEFAULT_APLHA = 255;
constexpr double DEFAULT_OPACITY = 0.2;
const std::vector<std::string> DEFAULT_FONT_FAMILY = { "HarmonyOS Sans" };
const std::vector<TextAlign> TEXT_ALIGNS = { TextAlign::START, TextAlign::CENTER, TextAlign::END, TextAlign::JUSTIFY };

void SetTextInputCaretColor(NodeHandle node, uint32_t color)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCaretColor(frameNode, Color(color));
}

void ResetTextInputCaretColor(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCaretColor(frameNode, Color(DEFAULT_CARET_COLOR));
}

void SetTextInputType(NodeHandle node, int32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetType(frameNode, static_cast<TextInputType>(value));
}

void ResetTextInputType(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetType(frameNode, TextInputType::UNSPECIFIED);
    return;
}

void SetTextInputMaxLines(NodeHandle node, int32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    if (value <= 0) {
        TextFieldModelNG::SetMaxViewLines(frameNode, MAX_LINES);
        return;
    }

    TextFieldModelNG::SetMaxViewLines(frameNode, value);
}

void ResetTextInputMaxLines(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetMaxViewLines(frameNode, MAX_LINES);
}

void SetTextInputPlaceholderColor(NodeHandle node, uint32_t color)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetPlaceholderColor(frameNode, Color(color));
}

void ResetTextInputPlaceholderColor(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto theme = ArkTSUtils::GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);
    TextFieldModelNG::SetPlaceholderColor(frameNode, theme->GetPlaceholderColor());
}

void SetTextInputCaretPosition(NodeHandle node, int32_t caretPosition)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCaretPosition(frameNode, caretPosition);
}

void ResetTextInputCaretPosition(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCaretPosition(frameNode, DEFAULT_CARE_POSITION);
}

void SetTextInputCopyOption(NodeHandle node, int32_t copyOption)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCopyOption(frameNode, static_cast<CopyOptions>(copyOption));
}

void ResetTextInputCopyOption(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetCopyOption(frameNode, DEFAULT_TEXT_INPUT_COPY_OPTION);
}

void SetTextInputShowPasswordIcon(NodeHandle node, uint32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetShowPasswordIcon(frameNode, static_cast<bool>(value));
}

void ResetTextInputShowPasswordIcon(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetShowPasswordIcon(frameNode, DEFAULT_SHOW_PASSWORD_ICON_VALUE);
}

void SetTextInputPasswordIcon(NodeHandle node, const struct ArkUIPasswordIconType* value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    PasswordIcon passwordIcon;
    if (value->showResult != nullptr && std::string(value->showResult) != "") {
        passwordIcon.showResult = value->showResult;
    } else {
        if (value->showBundleName != nullptr && std::string(value->showBundleName) != "") {
            passwordIcon.showBundleName = value->showBundleName;
        }
        if (value->showModuleName != nullptr && std::string(value->showModuleName) != "") {
            passwordIcon.showModuleName = value->showModuleName;
        }
    }
    if (value->hideResult != nullptr && std::string(value->hideResult) != "") {
        passwordIcon.hideResult = value->hideResult;
    } else {
        if (value->hideBundleName != nullptr && std::string(value->hideBundleName) != "") {
            passwordIcon.hideBundleName = value->hideBundleName;
        }
        if (value->hideModuleName != nullptr && std::string(value->hideModuleName) != "") {
            passwordIcon.hideModuleName = value->hideModuleName;
        }
    }
    TextFieldModelNG::SetPasswordIcon(frameNode, passwordIcon);
}

void ResetTextInputPasswordIcon(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    PasswordIcon passwordIcon;
    passwordIcon.showResult = "";
    passwordIcon.hideResult = "";
    TextFieldModelNG::SetPasswordIcon(frameNode, passwordIcon);
}

void SetTextInputTextAlign(NodeHandle node, int32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    if (value >= 0 && value < static_cast<int32_t>(TEXT_ALIGNS.size())) {
        TextFieldModelNG::SetTextAlign(frameNode, TEXT_ALIGNS[value]);
    } else {
        TextFieldModelNG::SetTextAlign(frameNode, DAFAULT_TEXT_ALIGN_VALUE);
    }
}

void ResetTextInputTextAlign(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetTextAlign(frameNode, DAFAULT_TEXT_ALIGN_VALUE);
}

void SetTextInputStyle(NodeHandle node, int32_t style)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetInputStyle(frameNode, static_cast<InputStyle>(style));
}

void ResetTextInputStyle(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetInputStyle(frameNode, DEFAULT_INPUT_STYLE);
}

void SetTextInputSelectionMenuHidden(NodeHandle node, uint32_t menuHiddenValue)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetSelectionMenuHidden(frameNode, static_cast<bool>(menuHiddenValue));
}

void ResetTextInputSelectionMenuHidden(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetSelectionMenuHidden(frameNode, DEFAULT_SELECTION_MENU_HIDDEN);
}

void SetTextInputShowUnderline(NodeHandle node, uint32_t showUnderLine)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetShowUnderline(frameNode, static_cast<bool>(showUnderLine));
}

void ResetTextInputShowUnderline(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetShowUnderline(frameNode, DEFAULT_SHOW_UNDER_LINE);
}

void SetTextInputCaretStyle(NodeHandle node, const ArkUILengthType* value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension width;
    if (value->string != nullptr) {
        width.SetCalcValue(value->string);
    } else {
        width.SetValue(value->number);
    }
    width.SetUnit(static_cast<DimensionUnit>(value->unit));

    CaretStyle caretStyle;
    caretStyle.caretWidth = width;
    TextFieldModelNG::SetCaretStyle(frameNode, caretStyle);
}

void ResetTextInputCaretStyle(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);

    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetThemeManager()->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);

    CaretStyle caretStyle;
    caretStyle.caretWidth = theme->GetCursorWidth();
    TextFieldModelNG::SetCaretStyle(frameNode, caretStyle);
}

void SetTextInputEnableKeyboardOnFocus(NodeHandle node, uint32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::RequestKeyboardOnFocus(frameNode, static_cast<bool>(value));
}

void ResetTextInputEnableKeyboardOnFocus(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::RequestKeyboardOnFocus(frameNode, DEFAULT_REQUEST_KEYBOARD_ON_FOCUS);
}

void SetTextInputBarState(NodeHandle node, int32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetBarState(frameNode, static_cast<DisplayMode>(value));
}

void ResetTextInputBarState(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetBarState(frameNode, DEFAULT_BAR_STATE);
}

void SetTextInputEnterKeyType(NodeHandle node, int32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetEnterKeyType(frameNode, static_cast<TextInputAction>(value));
}

void ResetTextInputEnterKeyType(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetEnterKeyType(frameNode, TextInputAction::DONE);
}

void SetTextInputFontWeight(NodeHandle node, const char *fontWeight)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontWeight(frameNode, Framework::ConvertStrToFontWeight(fontWeight));
}

void ResetTextInputFontWeight(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontWeight(frameNode, FontWeight::NORMAL);
}

void SetTextInputFontSize(NodeHandle node, const struct ArkUILengthType *value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension fontSize;
    if (value->string != nullptr) {
        fontSize.SetCalcValue(value->string);
    } else {
        fontSize.SetValue(value->number);
    }
    fontSize.SetUnit(static_cast<DimensionUnit>(value->unit));
    TextFieldModelNG::SetFontSize(frameNode, fontSize);
}

void ResetTextInputFontSize(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto theme = ArkTSUtils::GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);
    TextFieldModelNG::SetFontSize(frameNode, theme->GetFontSize());
}

void SetTextInputMaxLength(NodeHandle node, uint32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetMaxLength(frameNode, value);
}

void ResetTextInputMaxLength(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::ResetMaxLength(frameNode);
}

void SetTextInputSelectedBackgroundColor(NodeHandle node, uint32_t color)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetSelectedBackgroundColor(frameNode, Color(color));
}

void ResetTextInputSelectedBackgroundColor(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    Color selectedColor;
    auto pipeline = PipelineBase::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto theme = pipeline->GetThemeManager()->GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);
    selectedColor = theme->GetSelectedColor();
    if (selectedColor.GetAlpha() == DEFAULT_APLHA) {
        // Default setting of 20% opacity
        selectedColor = selectedColor.ChangeOpacity(DEFAULT_OPACITY);
    }
    TextFieldModelNG::SetSelectedBackgroundColor(frameNode, selectedColor);
}

void SetTextInputShowError(NodeHandle node, const char *error, uint32_t visible)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetShowError(frameNode, std::string(error), static_cast<bool>(visible));
}

void ResetTextInputShowError(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetShowError(frameNode, std::string(""), false);
}

void SetTextInputPlaceholderFont(NodeHandle node, const struct ArkUIPlaceholderFontType* placeholderFont)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    Font font;
    CalcDimension fontSize;
    if (placeholderFont->size != nullptr) {
        if (placeholderFont->size->string != nullptr) {
            fontSize.SetCalcValue(placeholderFont->size->string);
        } else {
            fontSize.SetValue(placeholderFont->size->number);
        }
    }
    fontSize.SetUnit(static_cast<DimensionUnit>(placeholderFont->size->unit));
    font.fontSize = fontSize;
    if (placeholderFont->weight != nullptr && std::string(placeholderFont->weight) != "") {
        font.fontWeight = Framework::ConvertStrToFontWeight(placeholderFont->weight);
    }
    if (placeholderFont->fontFamilies != nullptr && placeholderFont->length > 0) {
        for (uint32_t i = 0; i < placeholderFont->length; i++) {
            const char* family = *(placeholderFont->fontFamilies + i);
            if (family != nullptr) {
                font.fontFamilies.emplace_back(std::string(family));
            }
        }
    }
    if (placeholderFont->style >= 0) {
        font.fontStyle = static_cast<Ace::FontStyle>(placeholderFont->style);
    }
    TextFieldModelNG::SetPlaceholderFont(frameNode, font);
}

void ResetTextInputPlaceholderFont(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    Font font;
    auto theme = ArkTSUtils::GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);
    font.fontSize = theme->GetFontSize();
    font.fontWeight = DEFAULT_FONT_WEIGHT;
    font.fontStyle = DEFAULT_FONT_STYLE;
    TextFieldModelNG::SetPlaceholderFont(frameNode, font);
}

void SetTextInputFontColor(NodeHandle node, uint32_t color)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetTextColor(frameNode, Color(color));
}

void ResetTextInputFontColor(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    auto theme = ArkTSUtils::GetTheme<TextFieldTheme>();
    CHECK_NULL_VOID(theme);
    TextFieldModelNG::SetTextColor(frameNode, theme->GetTextColor());
}

void SetTextInputFontStyle(NodeHandle node, uint32_t value)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontStyle(frameNode, static_cast<Ace::FontStyle>(value));
}

void ResetTextInputFontStyle(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontStyle(frameNode, DEFAULT_FONT_STYLE);
}

void SetTextInputFontFamily(NodeHandle node, const char** fontFamilies, uint32_t length)
{
    CHECK_NULL_VOID(fontFamilies);
    if (length <= 0) {
        return;
    }
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    std::vector<std::string> families;
    for (uint32_t i = 0; i < length; i++) {
        const char* family = *(fontFamilies + i);
        if (family != nullptr) {
            families.emplace_back(std::string(family));
        }
    }
    TextFieldModelNG::SetFontFamily(frameNode, families);
}

void ResetTextInputFontFamily(NodeHandle node)
{
    auto *frameNode = reinterpret_cast<FrameNode *>(node);
    CHECK_NULL_VOID(frameNode);
    TextFieldModelNG::SetFontFamily(frameNode, DEFAULT_FONT_FAMILY);
}
ArkUITextInputModifierAPI GetTextInputModifier()
{
    static const ArkUITextInputModifierAPI modifier = { SetTextInputCaretColor,
                                                        ResetTextInputCaretColor,
                                                        SetTextInputType,
                                                        ResetTextInputType,
                                                        SetTextInputMaxLines,
                                                        ResetTextInputMaxLines,
                                                        SetTextInputPlaceholderColor,
                                                        ResetTextInputPlaceholderColor,
                                                        SetTextInputCaretPosition,
                                                        ResetTextInputCaretPosition,
                                                        SetTextInputCopyOption,
                                                        ResetTextInputCopyOption,
                                                        SetTextInputShowPasswordIcon,
                                                        ResetTextInputShowPasswordIcon,
                                                        SetTextInputPasswordIcon,
                                                        ResetTextInputPasswordIcon,
                                                        SetTextInputTextAlign,
                                                        ResetTextInputTextAlign,
                                                        SetTextInputStyle,
                                                        ResetTextInputStyle,
                                                        SetTextInputSelectionMenuHidden,
                                                        ResetTextInputSelectionMenuHidden,
                                                        SetTextInputShowUnderline,
                                                        ResetTextInputShowUnderline,
                                                        SetTextInputCaretStyle,
                                                        ResetTextInputCaretStyle,
                                                        SetTextInputEnableKeyboardOnFocus,
                                                        ResetTextInputEnableKeyboardOnFocus,
                                                        SetTextInputBarState,
                                                        ResetTextInputBarState,
                                                        SetTextInputEnterKeyType,
                                                        ResetTextInputEnterKeyType,
                                                        SetTextInputFontWeight,
                                                        ResetTextInputFontWeight,
                                                        SetTextInputFontSize,
                                                        ResetTextInputFontSize,
                                                        SetTextInputMaxLength,
                                                        ResetTextInputMaxLength,
                                                        SetTextInputSelectedBackgroundColor,
                                                        ResetTextInputSelectedBackgroundColor,
                                                        SetTextInputShowError,
                                                        ResetTextInputShowError,
                                                        SetTextInputPlaceholderFont,
                                                        ResetTextInputPlaceholderFont,
                                                        SetTextInputFontColor,
                                                        ResetTextInputFontColor,
                                                        SetTextInputFontStyle,
                                                        ResetTextInputFontStyle,
                                                        SetTextInputFontFamily,
                                                        ResetTextInputFontFamily };

    return modifier;
}
}