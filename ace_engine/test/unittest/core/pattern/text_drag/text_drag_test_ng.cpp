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

#include "text_drag_test_ng.h"

#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/text/text_pattern.h"
#include "core/components_ng/pattern/text_drag/text_drag_base.h"
#include "core/components_ng/pattern/text_drag/text_drag_overlay_modifier.h"
#include "core/components_ng/pattern/text_drag/text_drag_pattern.h"
#include "core/components_ng/render/canvas_image.h"

#include "test/mock/core/common/mock_theme_manager.h"
#include "test/mock/core/pipeline/mock_pipeline_context.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::Ace::NG {
namespace {
const std::string TEXT_CONTENT1 = "hello";
const std::string TEXT_CONTENT2 = "Winter is a beautiful season\n, especial when it snow.";
constexpr float TEXT_NODE_WIDTH = 200.f;
constexpr float TEXT_NODE_HEIGHT = 100.f;
constexpr Color FRONT_COLOR = Color(0xff0000ff);
constexpr Color BG_COLOR = Color(0xffc0c0c0);
const Dimension LINE_HEIGHT_VALUE = Dimension(30.0, DimensionUnit::PX);
const Dimension FONT_SIZE_VALUE = Dimension(30.0, DimensionUnit::PX);
const Dimension BORDER_WIDTH_VALUE = Dimension(1.0, DimensionUnit::PX);
constexpr float EXPECT_MOCK_HEIGHT = 50.f;
constexpr float EXPECT_MOCK_LONGEST_LINE = 460.f;
constexpr float EXPECT_MOCK_MAX_TEXT_WIDTH = 460.f;
constexpr float EXPECT_MOCK_LINE_COUNT = 1;
constexpr float EXPECT_MOCK_TEXT_WIDTH  = 120.f;
const RectF DEFAULT_TEXT_CONTENT_RECTF(80.f, 80.f, 100.f, 200.f);
} // namespace

void TextDragTestNg::SetUpTestSuite()
{
    MockPipelineContext::SetUp();
    auto themeManager = AceType::MakeRefPtr<MockThemeManager>();
    MockPipelineContext::GetCurrent()->SetThemeManager(themeManager);
    auto textTheme = AceType::MakeRefPtr<TextTheme>();
    EXPECT_CALL(*themeManager, GetTheme(_)).WillRepeatedly(Return(textTheme));
}

void TextDragTestNg::TearDownTestSuite()
{
    MockPipelineContext::TearDown();
    MockParagraph::TearDown();
}

void TextDragTestNg::SetUp()
{
    paragraph_  = MockParagraph::GetOrCreateMockParagraph();
}

void TextDragTestNg::TearDown()
{
    frameNode_ = nullptr;
    pattern_ = nullptr;
    paragraph_ = nullptr;
    dragNode_ = nullptr;
    dragPattern_ = nullptr;
}

void TextDragTestNg::CreateTextWithDragNode(const std::string& textContent, const TextProperty& textProperty)
{
    ViewAbstract::SetWidth(CalcLength(TEXT_NODE_WIDTH));
    ViewAbstract::SetHeight(CalcLength(TEXT_NODE_HEIGHT));
    ViewAbstract::SetForegroundColor(FRONT_COLOR);
    ViewAbstract::SetBackgroundColor(BG_COLOR);
    ViewAbstract::SetBorderWidth(BORDER_WIDTH_VALUE);
    TextModelNG textModel;
    textModel.Create(textContent);
    SetTextProperty(textModel, textProperty);
    GetInstance();
}

void TextDragTestNg::SetTextProperty(TextModelNG& textModel, const TextProperty& textProperty)
{
    if (textProperty.fontSizeValue.has_value()) {
        textModel.SetFontSize(textProperty.fontSizeValue.value());
    }
    if (textProperty.textColorValue.has_value()) {
        textModel.SetTextColor(textProperty.textColorValue.value());
    }
    if (textProperty.italicFontStyleValue.has_value()) {
        textModel.SetItalicFontStyle(textProperty.italicFontStyleValue.value());
    }
    if (textProperty.fontWeightValue.has_value()) {
        textModel.SetFontWeight(textProperty.fontWeightValue.value());
    }
    if (textProperty.fontFamilyValue.has_value()) {
        textModel.SetFontFamily(textProperty.fontFamilyValue.value());
    }
    if (textProperty.textAlignValue.has_value()) {
        textModel.SetTextAlign(textProperty.textAlignValue.value());
    }
    if (textProperty.textOverflowValue.has_value()) {
        textModel.SetTextOverflow(textProperty.textOverflowValue.value());
    }
    if (textProperty.maxLinesValue.has_value()) {
        textModel.SetMaxLines(textProperty.maxLinesValue.value());
    }
    if (textProperty.lineHeightValue.has_value()) {
        textModel.SetLineHeight(textProperty.lineHeightValue.value());
    }
    if (textProperty.textDecorationValue.has_value()) {
        textModel.SetTextDecoration(textProperty.textDecorationValue.value());
    }
    if (textProperty.textDecorationColorValue.has_value()) {
        textModel.SetTextDecorationColor(textProperty.textDecorationColorValue.value());
    }
    if (textProperty.baselineOffsetValue.has_value()) {
        textModel.SetBaselineOffset(textProperty.baselineOffsetValue.value());
    }
    if (textProperty.textIndent.has_value()) {
        textModel.SetTextIndent(textProperty.textIndent.value());
    }
    if (textProperty.wordBreak.has_value()) {
        textModel.SetWordBreak(textProperty.wordBreak.value());
    }
    if (textProperty.textSelectionValue.has_value()) {
        textModel.SetTextSelection(textProperty.textSelectionValue.value().first,
            textProperty.textSelectionValue.value().second);
    }
}

void TextDragTestNg::GetInstance()
{
    frameNode_ = AceType::DynamicCast<FrameNode>(ViewStackProcessor::GetInstance()->Finish());
    pattern_ = frameNode_->GetPattern<TextPattern>();
    pattern_->paragraph_ = paragraph_;
    // set textPattern content rect default value.
    pattern_->contentRect_ = DEFAULT_TEXT_CONTENT_RECTF;
    dragNode_ = TextDragPattern::CreateDragNode(frameNode_);
    pattern_->dragNode_ = dragNode_;
    dragPattern_ = dragNode_->GetPattern<TextDragPattern>();
    dragContext_ = dragNode_->GetRenderContext();
    FlushLayoutTask(frameNode_);
}

void TextDragTestNg::SetMockParagraphExpectCallParas(MockParagraphExpectCallParas params)
{
    EXPECT_CALL(*paragraph_, PushStyle(_)).Times(AnyNumber());
    EXPECT_CALL(*paragraph_, AddText(_)).Times(AnyNumber());
    EXPECT_CALL(*paragraph_, PopStyle()).Times(AnyNumber());
    EXPECT_CALL(*paragraph_, Build()).Times(AnyNumber());
    EXPECT_CALL(*paragraph_, Layout(_)).Times(AnyNumber());
    EXPECT_CALL(*paragraph_, GetAlphabeticBaseline()).WillRepeatedly(Return(0.f));
    if (params.heightValue.has_value()) {
        EXPECT_CALL(*paragraph_, GetHeight()).WillRepeatedly(Return(params.heightValue.value()));
    }
    if (params.longestLineValue.has_value()) {
        EXPECT_CALL(*paragraph_, GetLongestLine()).WillRepeatedly(Return(params.longestLineValue.value()));
    }
    if (params.maxWidthValue.has_value()) {
        EXPECT_CALL(*paragraph_, GetMaxWidth()).WillRepeatedly(Return(params.maxWidthValue.value()));
    }
    if (params.lineCountValue.has_value()) {
        EXPECT_CALL(*paragraph_, GetLineCount()).WillRepeatedly(Return(params.lineCountValue.value()));
    }
    if (params.textWidthValue.has_value()) {
        EXPECT_CALL(*paragraph_, GetTextWidth()).WillRepeatedly(Return(params.textWidthValue.value()));
    }
}

/**
 * @tc.name: TextDragCreateNodeTestNg001
 * @tc.desc: Test TextDragPattern::CreateDragNode.
 * @tc.type: FUNC
 */
HWTEST_F(TextDragTestNg, TextDragCreateNodeTestNg001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init Text with dragNode, EXPECT_CALL mockparagraph GetRectsForRange
     *     return empty selected text range vector.
     * @tc.expected: call CreatDragNode and test dragNode property.
     */
    MockParagraphExpectCallParas params = {
        .heightValue = std::make_optional(EXPECT_MOCK_HEIGHT),
        .longestLineValue = std::make_optional(EXPECT_MOCK_LONGEST_LINE),
        .maxWidthValue = std::make_optional(EXPECT_MOCK_MAX_TEXT_WIDTH),
        .lineCountValue = std::make_optional(EXPECT_MOCK_LINE_COUNT),
        .textWidthValue = std::make_optional(EXPECT_MOCK_TEXT_WIDTH),
    };
    SetMockParagraphExpectCallParas(params);
    std::vector<RectF> rects; // return null select range
    EXPECT_CALL(*paragraph_, GetRectsForRange(_, _, _)).WillRepeatedly(SetArgReferee<2>(rects));
    const TextProperty textProperty = {
        .fontSizeValue = std::make_optional(FONT_SIZE_VALUE),
        .textAlignValue = std::make_optional(TextAlign::START),
        .maxLinesValue = std::make_optional(1),
        .lineHeightValue = std::make_optional(LINE_HEIGHT_VALUE),
    };

    /**
     * @tc.steps: step2. construct context and call onDraw().
     * @tc.expected: call C and result is expected.
     */
    CreateTextWithDragNode(TEXT_CONTENT1, textProperty);
    EXPECT_EQ(dragPattern_->GetContentOffset(),  OffsetF(0.f, 0.f));
    EXPECT_EQ(dragPattern_->GetTextRect(),  RectF(0.f, 0.f, 0.f, 0.f));
}

/**
 * @tc.name: TextDragCreateNodeTestNg002
 * @tc.desc: Test CreateDragNode properties with one-line-selected.
 * @tc.type: FUNC
 */
HWTEST_F(TextDragTestNg, TextDragCreateNodeTestNg002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init Text with dragNode, EXPECT_CALL mockparagraph GetRectsForRange
     *     return one-line-seleced rect.
     * @tc.expected: call CreatDragNode success.
     */
    MockParagraphExpectCallParas params = {
        .heightValue = std::make_optional(EXPECT_MOCK_HEIGHT),
        .longestLineValue = std::make_optional(EXPECT_MOCK_LONGEST_LINE),
        .maxWidthValue = std::make_optional(EXPECT_MOCK_MAX_TEXT_WIDTH),
        .lineCountValue = std::make_optional(EXPECT_MOCK_LINE_COUNT),
        .textWidthValue = std::make_optional(EXPECT_MOCK_TEXT_WIDTH),
    };
    SetMockParagraphExpectCallParas(params);
    std::vector<RectF> rects { RectF(40, 40, 60, 80) };
    EXPECT_CALL(*paragraph_, GetRectsForRange(_, _, _)).WillRepeatedly(SetArgReferee<2>(rects));
    const TextProperty textProperty = {
        .fontSizeValue = std::make_optional(FONT_SIZE_VALUE),
        .textAlignValue = std::make_optional(TextAlign::START),
        .maxLinesValue = std::make_optional(1),
        .lineHeightValue = std::make_optional(LINE_HEIGHT_VALUE),
    };
    CreateTextWithDragNode(TEXT_CONTENT1, textProperty);

    /**
     * @tc.steps: step2. construct drageNode.
     * @tc.expected: dragNode lastLineHeight, textRect as expected
     */
    EXPECT_EQ(dragPattern_->GetContentOffset(),  OffsetF(32.f, 32.f));
    EXPECT_EQ(dragPattern_->GetTextRect(),  RectF(-32.f, -32.f, 60.f, 80.f));
}

/**
 * @tc.name: TextDragCreateNodeTestNg003
 * @tc.desc: Test CreateDragNode with multi-lines-selected.
 * @tc.type: FUNC
 */
HWTEST_F(TextDragTestNg, TextDragCreateNodeTestNg003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init Text with dragNode.
     * @tc.expected: call CreatDragNode, EXPECT_CALL mockparagraph GetRectsForRange
     *     return multi-lines-selected rect.
     */
    MockParagraphExpectCallParas params = {
        .heightValue = std::make_optional(EXPECT_MOCK_HEIGHT),
        .longestLineValue = std::make_optional(EXPECT_MOCK_LONGEST_LINE),
        .maxWidthValue = std::make_optional(EXPECT_MOCK_MAX_TEXT_WIDTH),
        .lineCountValue = std::make_optional(EXPECT_MOCK_LINE_COUNT + 2), // 2: multi-lines
        .textWidthValue = std::make_optional(EXPECT_MOCK_TEXT_WIDTH),
    };
    SetMockParagraphExpectCallParas(params);
    std::vector<RectF> rects { RectF(40, 40, 60, 60), RectF(100, 100, 60, 100)};
    EXPECT_CALL(*paragraph_, GetRectsForRange(_, _, _)).WillRepeatedly(SetArgReferee<2>(rects));
    const TextProperty textProperty = {
        .fontSizeValue = std::make_optional(FONT_SIZE_VALUE),
        .textAlignValue = std::make_optional(TextAlign::START),
        .maxLinesValue = std::make_optional(10),
        .lineHeightValue = std::make_optional(LINE_HEIGHT_VALUE),
        .textSelectionValue = std::make_optional(std::make_pair(0, TEXT_CONTENT2.size())),
    };
    CreateTextWithDragNode(TEXT_CONTENT2, textProperty);

    /**
     * @tc.steps: step1. construct dragNode.
     * @tc.expected: lastLineHeight/textRect/contentoffset as expected.
     */
    EXPECT_EQ(dragPattern_->GetContentOffset(),  OffsetF(-8.f, 32.f));
    EXPECT_EQ(dragPattern_->GetTextRect(),  RectF(8.f, -32.f, 100.f, 160.f));
}

/**
 * @tc.name: TextDragTestNg004
 * @tc.desc: Test TextDragPattern::CreateDragNode.
 * @tc.type: FUNC
 */
HWTEST_F(TextDragTestNg, TextDragTestNg004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init Text with dragNode.
     * @tc.expected: call CreatDragNode and flush.
     */
    MockParagraphExpectCallParas params = {
        .heightValue = std::make_optional(EXPECT_MOCK_HEIGHT),
        .longestLineValue = std::make_optional(EXPECT_MOCK_LONGEST_LINE),
        .maxWidthValue = std::make_optional(EXPECT_MOCK_MAX_TEXT_WIDTH),
        .lineCountValue = std::make_optional(EXPECT_MOCK_LINE_COUNT), // 2: multi-lines
        .textWidthValue = std::make_optional(EXPECT_MOCK_TEXT_WIDTH),
    };
    SetMockParagraphExpectCallParas(params);
    std::vector<RectF> rects { RectF(80, 40, 20, 60) };
    EXPECT_CALL(*paragraph_, GetRectsForRange(_, _, _)).WillRepeatedly(SetArgReferee<2>(rects));
    const TextProperty textProperty = {
        .fontSizeValue = std::make_optional(FONT_SIZE_VALUE),
        .textAlignValue = std::make_optional(TextAlign::START),
        .maxLinesValue = std::make_optional(10),
        .lineHeightValue = std::make_optional(LINE_HEIGHT_VALUE),
        .textSelectionValue = std::make_optional(std::make_pair(0, TEXT_CONTENT2.size())),
    };
    CreateTextWithDragNode(TEXT_CONTENT2, textProperty);

    /**
     * @tc.steps: step2. construct context and call onDraw().
     * @tc.expected: expect frame width & height
     */
    EXPECT_EQ(dragPattern_->GetFrameWidth(), 64.f);
    EXPECT_EQ(dragPattern_->GetFrameHeight(), 76.f);
}
} // namespace OHOS::Ace::NG
