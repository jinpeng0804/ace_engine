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

#include "core/components_ng/pattern/navigation/navigation_content_layout_algorithm.h"

#include "core/components_ng/layout/box_layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/navrouter/navdestination_layout_algorithm.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

void NavigationContentLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    auto layoutConstraint = layoutWrapper->GetLayoutProperty()->CreateChildConstraint();
    auto childSize = layoutWrapper->GetTotalChildCount();
    std::list<RefPtr<LayoutWrapper>> children;
    for (auto index = 0; index < childSize; index++) {
        auto child = layoutWrapper->GetOrCreateChildByIndex(index, false);
        if (child->IsActive()) {
            child->Measure(layoutConstraint);
            children.emplace_back(child);
        }
    }
    PerformMeasureSelfWithChildList(layoutWrapper, { children });
}

float GetSafeAreaHeight(LayoutWrapper* LayoutWrapper)
{
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, 0.0f);
    auto safeArea = pipeline->GetSafeArea();
    auto safeAreaHeight = safeArea.top_.Length();
    auto hostNode = LayoutWrapper->GetHostNode();
    CHECK_NULL_RETURN(hostNode, 0.0f);
    auto geometryNode = hostNode->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, 0.0f);
    auto parentGlobalOffset = hostNode->GetParentGlobalOffsetDuringLayout();
    auto frame = geometryNode->GetFrameRect() + parentGlobalOffset;
    if (!safeArea.top_.IsOverlapped(frame.Top())) {
        safeAreaHeight = 0.0f;
    }
    return safeAreaHeight;
}

void NavigationContentLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    // update child position.
    auto size = layoutWrapper->GetGeometryNode()->GetFrameSize();
    const auto& padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    auto left = padding.left.value_or(0);
    auto top = padding.top.value_or(0);
    auto paddingOffset = OffsetF(left, top);
    auto align = Alignment::CENTER;
    if (layoutWrapper->GetLayoutProperty()->GetPositionProperty()) {
        align = layoutWrapper->GetLayoutProperty()->GetPositionProperty()->GetAlignment().value_or(align);
    }
    // Update child position.
    auto safeAreaHeight = GetSafeAreaHeight(layoutWrapper);
    for (const auto& child : layoutWrapper->GetAllChildrenWithBuild(false)) {
        if (child->IsActive()) {
            SizeF childSize = child->GetGeometryNode()->GetMarginFrameSize();
            auto translate = Alignment::GetAlignPosition(size, childSize, align) + paddingOffset;
            child->GetGeometryNode()->SetMarginFrameOffset(translate);
            child->Layout();

            auto childNode = child->GetHostNode();
            CHECK_NULL_VOID(childNode);
            auto childGeo = child->GetGeometryNode();
            CHECK_NULL_VOID(childGeo);
            auto offset = childGeo->GetFrameOffset();
            offset.SetY(offset.GetY() + safeAreaHeight);
            childGeo->SetFrameOffset(offset);
            childNode->ForceSyncGeometryNode();
        }
    }
    // Update content position.
    const auto& content = layoutWrapper->GetGeometryNode()->GetContent();
    if (content) {
        auto translate = Alignment::GetAlignPosition(size, content->GetRect().GetSize(), align) + paddingOffset;
        content->SetOffset(translate);
    }
}

} // namespace OHOS::Ace::NG
