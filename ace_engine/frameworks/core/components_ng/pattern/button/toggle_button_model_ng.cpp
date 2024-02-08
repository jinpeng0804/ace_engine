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
#include "core/components_ng/pattern/button/toggle_button_model_ng.h"

#include "base/memory/ace_type.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/view_abstract.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/button/toggle_button_paint_property.h"
#include "core/components_ng/pattern/button/toggle_button_pattern.h"

namespace OHOS::Ace::NG {
void ToggleButtonModelNG::Create(const std::string& tagName)
{
    auto* stack = ViewStackProcessor::GetInstance();
    auto nodeId = stack->ClaimNodeId();
    ACE_LAYOUT_SCOPED_TRACE("Create[%s][self:%d]", tagName.c_str(), nodeId);
    auto frameNode =
        FrameNode::GetOrCreateFrameNode(tagName, nodeId, []() { return AceType::MakeRefPtr<ToggleButtonPattern>(); });
    stack->Push(frameNode);
}

void ToggleButtonModelNG::SetSelectedColor(const Color& selectedColor)
{
    ACE_UPDATE_PAINT_PROPERTY(ToggleButtonPaintProperty, SelectedColor, selectedColor);
}

void ToggleButtonModelNG::SetIsOn(bool isOn)
{
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto eventHub = frameNode->GetEventHub<ToggleButtonEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->SetCurrentUIState(UI_STATE_SELECTED, isOn);

    ACE_UPDATE_PAINT_PROPERTY(ToggleButtonPaintProperty, IsOn, isOn);
}

void ToggleButtonModelNG::SetBackgroundColor(const Color& backgroundColor)
{
    auto* stack = ViewStackProcessor::GetInstance();
    CHECK_NULL_VOID(stack);
    auto frameNode = ViewStackProcessor::GetInstance()->GetMainFrameNode();
    CHECK_NULL_VOID(frameNode);
    auto buttonPattern = stack->GetMainFrameNodePattern<ToggleButtonPattern>();
    if (buttonPattern) {
        ACE_UPDATE_PAINT_PROPERTY(ToggleButtonPaintProperty, BackgroundColor, backgroundColor);
        return;
    }
    NG::ViewAbstract::SetBackgroundColor(backgroundColor);
}

void ToggleButtonModelNG::SetSelectedColor(FrameNode* frameNode, const Color& selectedColor)
{
    ACE_UPDATE_NODE_PAINT_PROPERTY(ToggleButtonPaintProperty, SelectedColor, selectedColor, frameNode);
}

void ToggleButtonModelNG::SetBackgroundColor(FrameNode* frameNode, const Color& backgroundColor)
{
    CHECK_NULL_VOID(frameNode);
    auto buttonPattern = AceType::DynamicCast<ToggleButtonPattern>(frameNode->GetPattern());
    if (buttonPattern) {
        ACE_UPDATE_NODE_PAINT_PROPERTY(ToggleButtonPaintProperty, BackgroundColor, backgroundColor, frameNode);
        return;
    }
    NG::ViewAbstract::SetBackgroundColor(frameNode, backgroundColor);
}
} // namespace OHOS::Ace::NG