/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/search/search_text_field.h"

#include "core/components_ng/pattern/search/search_event_hub.h"

namespace OHOS::Ace::NG {
bool SearchTextFieldPattern::IsSearchParentNode() const
{
    auto parentFrameNode = AceType::DynamicCast<FrameNode>(GetHost()->GetParent());
    return parentFrameNode && parentFrameNode->GetTag() == V2::SEARCH_ETS_TAG;
}

RefPtr<FocusHub> SearchTextFieldPattern::GetFocusHub() const
{
    if (!IsSearchParentNode()) {
        return TextFieldPattern::GetFocusHub();
    }

    auto parentFrameNode = AceType::DynamicCast<FrameNode>(GetHost()->GetParent());
    return parentFrameNode->GetOrCreateFocusHub();
}

void SearchTextFieldPattern::PerformAction(TextInputAction action, bool forceCloseKeyboard)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto parentFrameNode = AceType::DynamicCast<FrameNode>(host->GetParent());
    auto eventHub = parentFrameNode->GetEventHub<SearchEventHub>();
    CHECK_NULL_VOID(eventHub);
    eventHub->UpdateSubmitEvent(GetTextValue());
    CloseKeyboard(forceCloseKeyboard);
}

TextInputAction SearchTextFieldPattern::GetDefaultTextInputAction() const
{
    return TextInputAction::SEARCH;
}

#ifdef ENABLE_DRAG_FRAMEWORK
void SearchTextFieldPattern::InitDragEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->SetDraggable(true);
    TextFieldPattern::InitDragEvent();
    auto gestureHub = host->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    gestureHub->SetTextDraggable(true);
}
#endif
} // namespace OHOS::Ace::NG