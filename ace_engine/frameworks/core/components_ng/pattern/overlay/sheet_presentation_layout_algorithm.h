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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_OVERLAY_SHEET_PRESENTATION_LAYOUT_ALGORITHM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_OVERLAY_SHEET_PRESENTATION_LAYOUT_ALGORITHM_H

#include <cstdint>
#include <optional>

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "base/geometry/ng/size_t.h"
#include "base/memory/referenced.h"
#include "core/components_ng/layout/layout_algorithm.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/components_ng/pattern/linear_layout/linear_layout_algorithm.h"
#include "core/components_ng/pattern/overlay/sheet_presentation_property.h"
#include "core/components_ng/pattern/overlay/sheet_style.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT SheetPresentationLayoutAlgorithm : public LinearLayoutAlgorithm {
    DECLARE_ACE_TYPE(SheetPresentationLayoutAlgorithm, LayoutAlgorithm);

public:
    SheetPresentationLayoutAlgorithm() = default;
    SheetPresentationLayoutAlgorithm(int32_t id, const std::string& tag, SheetType sheetType)
        : targetNodeId_(id), targetTag_(tag), sheetType_(sheetType)
    {}
    ~SheetPresentationLayoutAlgorithm() override = default;

    void OnReset() override {}
    void Measure(LayoutWrapper* layoutWrapper) override;
    void Layout(LayoutWrapper* layoutWrapper) override;
    float GetSheetMaxHeight()
    {
        return sheetMaxHeight_;
    }

    float GetSheetMaxWidth()
    {
        return sheetMaxWidth_;
    }

    float GetCenterHeight()
    {
        return sheetHeight_;
    }

    float GetSheetOffsetX()
    {
        return sheetOffsetX_;
    }

    float GetSheetOffsetY()
    {
        return sheetOffsetY_;
    }

private:
    int32_t targetNodeId_ = -1;
    std::string targetTag_;
    OffsetF GetPopupStyleSheetOffset();
    float GetWidthByScreenSizeType(const SizeF& maxSize) const;
    float GetHeightByScreenSizeType(const SizeF& maxSize) const;
    float GetHeightBySheetStyle() const;
    LayoutConstraintF CreateSheetChildConstraint(RefPtr<SheetPresentationProperty> layoutprop);
    float sheetHeight_ = 0.0f;
    float sheetWidth_ = 0.0f;
    float sheetMaxHeight_ = 0.0f;
    float sheetMaxWidth_ = 0.0f;
    float sheetOffsetX_ = 0.0f;
    float sheetOffsetY_ = 0.0f;
    SheetType sheetType_ = SheetType::SHEET_BOTTOM;
    SheetStyle sheetStyle_;
    ACE_DISALLOW_COPY_AND_MOVE(SheetPresentationLayoutAlgorithm);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_OVERLAY_SHEET_PRESENTATION_LAYOUT_ALGORITHM_H