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

#include "core/components_ng/pattern/folder_stack/folder_stack_layout_algorithm.h"

#include "base/memory/ace_type.h"
#include "base/subwindow/subwindow_manager.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/common/display_info.h"
#include "core/components/common/layout/constants.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/folder_stack/control_parts_stack_node.h"
#include "core/components_ng/pattern/folder_stack/folder_stack_group_node.h"
#include "core/components_ng/pattern/folder_stack/folder_stack_layout_property.h"
#include "core/components_ng/pattern/folder_stack/folder_stack_pattern.h"
#include "core/components_ng/pattern/folder_stack/hover_stack_node.h"
#include "core/components_ng/pattern/stack/stack_layout_algorithm.h"
#include "core/components_ng/pattern/stack/stack_layout_property.h"
#include "core/components_ng/syntax/if_else_model.h"
#include "core/pipeline/pipeline_base.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
namespace {
constexpr float OFFSET_VALUE = 1.0f;
constexpr float OFFSET_DIVISOR = 2.0f;
} // namespace

FolderStackLayoutAlgorithm::FolderStackLayoutAlgorithm() = default;

void FolderStackLayoutAlgorithm::Layout(LayoutWrapper* layoutWrapper)
{
    auto folderStackLayoutProperty =
        AceType::DynamicCast<FolderStackLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(folderStackLayoutProperty);
    auto hostNode = AceType::DynamicCast<FolderStackGroupNode>(layoutWrapper->GetHostNode());
    CHECK_NULL_VOID(hostNode);
    auto align = Alignment::CENTER;
    if (folderStackLayoutProperty->GetPositionProperty()) {
        align = folderStackLayoutProperty->GetPositionProperty()->GetAlignment().value_or(Alignment::CENTER);
    }
    PerformLayout(hostNode, align);
    if (!isIntoFolderStack_) {
        auto childLayoutProperty = AceType::DynamicCast<StackLayoutProperty>(layoutWrapper->GetLayoutProperty());
        if (childLayoutProperty->GetPositionProperty()) {
            childLayoutProperty->GetPositionProperty()->UpdateAlignment(align);
        }
        StackLayoutAlgorithm::Layout(layoutWrapper);
        return;
    }
    LayoutHoverStack(layoutWrapper, hostNode, folderStackLayoutProperty);
    LayoutControlPartsStack(layoutWrapper, hostNode, folderStackLayoutProperty);
}

void FolderStackLayoutAlgorithm::LayoutHoverStack(LayoutWrapper* layoutWrapper,
    const RefPtr<FolderStackGroupNode>& hostNode, const RefPtr<FolderStackLayoutProperty>& folderStackLayoutProperty)
{
    auto folderStackGeometryNode = layoutWrapper->GetGeometryNode();
    auto size = folderStackGeometryNode->GetFrameSize();
    const auto& padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    auto layoutDirection = layoutWrapper->GetLayoutProperty()->GetLayoutDirection();
    if (layoutDirection == TextDirection::AUTO) {
        layoutDirection = AceApplicationInfo::GetInstance().IsRightToLeft() ? TextDirection::RTL : TextDirection::LTR;
    }
    MinusPaddingToSize(padding, size);
    auto left = padding.left.value_or(0);
    auto top = padding.top.value_or(0);
    auto paddingOffset = OffsetF(left, top);
    auto align = Alignment::CENTER;
    if (folderStackLayoutProperty->GetPositionProperty()) {
        align = folderStackLayoutProperty->GetPositionProperty()->GetAlignment().value_or(Alignment::CENTER);
    }
    auto hoverNode = hostNode->GetHoverNode();
    auto index = hostNode->GetChildIndexById(hoverNode->GetId());
    auto hoverStackWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    auto geometryNode = hoverStackWrapper->GetGeometryNode();
    auto hoverStackOffset = OffsetT<float>(0.0f, 0.0f);
    geometryNode->SetMarginFrameOffset(hoverStackOffset);
    auto hoverSize = geometryNode->GetFrameSize();
    for (auto&& child : hoverStackWrapper->GetAllChildrenWithBuild()) {
        auto translate =
            CalculateStackAlignment(hoverSize, child->GetGeometryNode()->GetMarginFrameSize(), align) + paddingOffset;
        if (layoutDirection == TextDirection::RTL) {
            translate.SetX(
                hoverSize.Width() - translate.GetX() - child->GetGeometryNode()->GetMarginFrameSize().Width());
        }
        child->GetGeometryNode()->SetMarginFrameOffset(translate);
        child->Layout();
    }
    hoverStackWrapper->Layout();
}

void FolderStackLayoutAlgorithm::LayoutControlPartsStack(LayoutWrapper* layoutWrapper,
    const RefPtr<FolderStackGroupNode>& hostNode, const RefPtr<FolderStackLayoutProperty>& folderStackLayoutProperty)
{
    auto folderStackGeometryNode = layoutWrapper->GetGeometryNode();
    auto size = folderStackGeometryNode->GetFrameSize();
    const auto& padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    auto layoutDirection = layoutWrapper->GetLayoutProperty()->GetLayoutDirection();
    if (layoutDirection == TextDirection::AUTO) {
        layoutDirection = AceApplicationInfo::GetInstance().IsRightToLeft() ? TextDirection::RTL : TextDirection::LTR;
    }
    MinusPaddingToSize(padding, size);
    auto left = padding.left.value_or(0);
    auto top = padding.top.value_or(0);
    auto paddingOffset = OffsetF(left, top);
    auto align = Alignment::CENTER;
    if (folderStackLayoutProperty->GetPositionProperty()) {
        align = folderStackLayoutProperty->GetPositionProperty()->GetAlignment().value_or(Alignment::CENTER);
    }
    auto controlPartsStackNode = hostNode->GetControlPartsStackNode();
    auto index = hostNode->GetChildIndexById(controlPartsStackNode->GetId());
    auto controlPartsStackWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    auto geometryNode = controlPartsStackWrapper->GetGeometryNode();
    auto controlPartsStackSize = geometryNode->GetFrameSize();
    auto controlPartsStackRect = GetControlPartsStackRect();
    geometryNode->SetMarginFrameOffset(controlPartsStackRect);
    for (auto&& child : controlPartsStackWrapper->GetAllChildrenWithBuild()) {
        auto translate =
            CalculateStackAlignment(controlPartsStackSize, child->GetGeometryNode()->GetMarginFrameSize(), align) +
            paddingOffset;
        if (layoutDirection == TextDirection::RTL) {
            translate.SetX(controlPartsStackSize.Width() - translate.GetX() -
                           child->GetGeometryNode()->GetMarginFrameSize().Width());
        }
        child->GetGeometryNode()->SetMarginFrameOffset(translate);
        child->Layout();
    }
    controlPartsStackWrapper->Layout();
}

void FolderStackLayoutAlgorithm::Measure(LayoutWrapper* layoutWrapper)
{
    CHECK_NULL_VOID(layoutWrapper);
    auto hostNode = AceType::DynamicCast<FolderStackGroupNode>(layoutWrapper->GetHostNode());
    CHECK_NULL_VOID(hostNode);
    const auto& layoutProperty = DynamicCast<FolderStackLayoutProperty>(layoutWrapper->GetLayoutProperty());
    CHECK_NULL_VOID(layoutProperty);
    const auto& layoutConstraint = layoutProperty->GetLayoutConstraint();
    CHECK_NULL_VOID(layoutConstraint);
    auto geometryNode = layoutWrapper->GetGeometryNode();
    auto size = CreateIdealSizeByPercentRef(layoutConstraint.value(), Axis::HORIZONTAL, MeasureType::MATCH_PARENT)
                    .ConvertToSizeT();
    layoutWrapper->GetGeometryNode()->SetFrameSize(size);
    isIntoFolderStack_ = IsIntoFolderStack(size, layoutProperty, layoutWrapper);
    AdjustNodeTree(hostNode);
    if (!isIntoFolderStack_) {
        StackLayoutAlgorithm::Measure(layoutWrapper);
        return;
    }
    const auto& padding = layoutWrapper->GetLayoutProperty()->CreatePaddingAndBorder();
    MinusPaddingToSize(padding, size);
    RangeCalculation(hostNode, layoutProperty, size);
    MeasureHoverStack(layoutWrapper, hostNode, layoutProperty, size);
    MeasureControlPartsStack(layoutWrapper, hostNode, layoutProperty, size);
}

void FolderStackLayoutAlgorithm::MeasureHoverStack(LayoutWrapper* layoutWrapper,
    const RefPtr<FolderStackGroupNode>& hostNode, const RefPtr<FolderStackLayoutProperty>& foldStackLayoutProperty,
    const SizeF& size)
{
    auto hoverNode = hostNode->GetHoverNode();
    CHECK_NULL_VOID(hoverNode);
    auto index = hostNode->GetChildIndexById(hoverNode->GetId());
    auto hoverWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_VOID(hoverWrapper);
    auto constraint = foldStackLayoutProperty->CreateChildConstraint();
    constraint.selfIdealSize = OptionalSizeF(size.Width(), preHoverStackHeight_);
    hoverWrapper->Measure(constraint);
}

void FolderStackLayoutAlgorithm::MeasureControlPartsStack(LayoutWrapper* layoutWrapper,
    const RefPtr<FolderStackGroupNode>& hostNode, const RefPtr<FolderStackLayoutProperty>& foldStackLayoutProperty,
    const SizeF& size)
{
    auto controlPartsStackNode = hostNode->GetControlPartsStackNode();
    CHECK_NULL_VOID(controlPartsStackNode);
    auto index = hostNode->GetChildIndexById(controlPartsStackNode->GetId());
    auto controlPartsWrapper = layoutWrapper->GetOrCreateChildByIndex(index);
    CHECK_NULL_VOID(controlPartsWrapper);
    auto constraint = foldStackLayoutProperty->CreateChildConstraint();
    constraint.selfIdealSize = OptionalSizeF(size.Width(), preControlPartsStackHeight_);
    controlPartsWrapper->Measure(constraint);
}

void FolderStackLayoutAlgorithm::RangeCalculation(const RefPtr<FolderStackGroupNode>& hostNode,
    const RefPtr<FolderStackLayoutProperty>& folderStackLayoutProperty, const SizeF& size)
{
    int32_t creaseY = 0;
    int32_t creaseHeight = 0;
    const auto& constraint = folderStackLayoutProperty->GetLayoutConstraint();
    CHECK_NULL_VOID(constraint);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto safeArea = pipeline->GetSafeArea();
    auto length = safeArea.top_.Length();
    auto rootHeight = pipeline->GetRootHeight();
    auto container = Container::Current();
    CHECK_NULL_VOID(container);
    auto displayInfo = container->GetDisplayInfo();
    CHECK_NULL_VOID(displayInfo);
    auto foldCreaseRects = displayInfo->GetCurrentFoldCreaseRegion();
    if (!foldCreaseRects.empty()) {
        auto foldCrease = foldCreaseRects.front();
        creaseY = static_cast<int32_t>(foldCrease.Bottom() - foldCrease.Height());
        creaseHeight = static_cast<int32_t>(foldCrease.Height());
    }
    auto parentRect = hostNode->GetParentGlobalOffsetDuringLayout();
    preHoverStackHeight_ = static_cast<float>(creaseY - length);
    preControlPartsStackHeight_ = static_cast<float>(rootHeight - creaseY - creaseHeight);
    controlPartsStackRect_ = OffsetF(parentRect.GetX(), creaseY - length + creaseHeight);
}

bool FolderStackLayoutAlgorithm::IsFullWindow(
    SizeF& frameSize, const RefPtr<FolderStackLayoutProperty>& foldStackLayoutProperty)
{
    auto padding = foldStackLayoutProperty->CreatePaddingAndBorder();
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(pipeline, false);
    auto windowManager = pipeline->GetWindowManager();
    auto safeArea = pipeline->GetSafeArea();
    CHECK_NULL_RETURN(windowManager, false);
    auto windowMode = windowManager->GetWindowMode();
    auto realWidth = frameSize.Width() + padding.Width();
    auto realHeight = frameSize.Height() + padding.Height();
    if (!NearEqual(realWidth, pipeline->GetRootWidth() - safeArea.left_.Length() - safeArea.right_.Length()) ||
        !NearEqual(realHeight, pipeline->GetRootHeight() - safeArea.top_.Length() - safeArea.bottom_.Length()) ||
        windowMode != WindowMode::WINDOW_MODE_FULLSCREEN) {
        return false;
    }
    return true;
}

void FolderStackLayoutAlgorithm::AdjustNodeTree(const RefPtr<FolderStackGroupNode>& hostNode)
{
    auto hoverNode = hostNode->GetHoverNode();
    auto controlPartsStackNode = hostNode->GetControlPartsStackNode();
    for (auto& hover : hoverNode->GetChildren()) {
        hoverNode->RemoveChild(hover);
    }
    for (auto& controlPart : controlPartsStackNode->GetChildren()) {
        controlPartsStackNode->RemoveChild(controlPart);
    }
    if (!isIntoFolderStack_) {
        for (auto& childNode : hostNode->GetChildNode()) {
            controlPartsStackNode->AddChild(childNode);
        }
    } else {
        auto itemId = hostNode->GetItemId();
        for (auto& childNode : hostNode->GetChildNode()) {
            if (std::count(itemId.begin(), itemId.end(), childNode->GetInspectorId())) {
                hoverNode->AddChild(childNode);
            } else {
                controlPartsStackNode->AddChild(childNode);
            }
        }
    }
}

NG::OffsetF FolderStackLayoutAlgorithm::CalculateStackAlignment(
    const NG::SizeF& parentSize, const NG::SizeF& childSize, const Alignment& alignment)
{
    NG::OffsetF offset;
    offset.SetX((OFFSET_VALUE + alignment.GetHorizontal()) * (parentSize.Width() - childSize.Width()) / OFFSET_DIVISOR);
    offset.SetY((OFFSET_VALUE + alignment.GetVertical()) * (parentSize.Height() - childSize.Height()) / OFFSET_DIVISOR);
    return offset;
}

void FolderStackLayoutAlgorithm::PerformLayout(const RefPtr<FolderStackGroupNode>& hostNode, const Alignment align)
{
    auto controlPartsStackNode = AceType::DynamicCast<ControlPartsStackNode>(hostNode->GetControlPartsStackNode());
    if (controlPartsStackNode) {
        auto controlPartsLayoutProperty =
            AceType::DynamicCast<LayoutProperty>(controlPartsStackNode->GetLayoutProperty());
        controlPartsLayoutProperty->UpdateAlignment(align);
    }
    auto hoverStackNode = AceType::DynamicCast<HoverStackNode>(hostNode->GetHoverNode());
    if (hoverStackNode) {
        auto hoverLayoutProperty = AceType::DynamicCast<LayoutProperty>(hoverStackNode->GetLayoutProperty());
        hoverLayoutProperty->UpdateAlignment(align);
    }
}

bool FolderStackLayoutAlgorithm::IsIntoFolderStack(
    SizeF& frameSize, const RefPtr<FolderStackLayoutProperty>& foldStackLayoutProperty, LayoutWrapper* layoutWrapper)
{
    auto pattern = layoutWrapper->GetHostNode()->GetPattern<FolderStackPattern>();
    CHECK_NULL_RETURN(pattern, false);
    auto displayInfo = pattern->GetDisplayInfo();
    if (!displayInfo) {
        auto container = Container::Current();
        CHECK_NULL_RETURN(container, false);
        displayInfo = container->GetDisplayInfo();
    }
    CHECK_NULL_RETURN(displayInfo, false);
    bool isFullWindow = IsFullWindow(frameSize, foldStackLayoutProperty);
    bool isFoldable = displayInfo->GetIsFoldable();
    auto foldStatus = displayInfo->GetFoldStatus();
    auto rotation = displayInfo->GetRotation();
    auto isLandscape = rotation == Rotation::ROTATION_90 || rotation == Rotation::ROTATION_270;
    return isFullWindow && isFoldable && foldStatus == FoldStatus::HALF_FOLD && isLandscape;
}
} // namespace OHOS::Ace::NG
