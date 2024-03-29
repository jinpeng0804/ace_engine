/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_BASE_LAYOUT_POSITION_PARAM_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_BASE_LAYOUT_POSITION_PARAM_H

#include "base/geometry/animatable_dimension.h"
#include "core/components/common/layout/constants.h"

namespace OHOS::Ace {

constexpr int32_t HORIZONTAL_DIRECTION_RANGE = 3;
constexpr int32_t VERTICAL_DIRECTION_RANGE = 6;

struct PositionParam {
    std::pair<AnimatableDimension, bool> left = { AnimatableDimension(0.0, DimensionUnit::PX), false };
    std::pair<AnimatableDimension, bool> right = { AnimatableDimension(0.0, DimensionUnit::PX), false };
    std::pair<AnimatableDimension, bool> top = { AnimatableDimension(0.0, DimensionUnit::PX), false };
    std::pair<AnimatableDimension, bool> bottom = { AnimatableDimension(0.0, DimensionUnit::PX), false };
    std::pair<Dimension, Dimension> anchor = {0.0_px, 0.0_px};
    PositionType type = PositionType::PTRELATIVE;
};

enum class AlignDirection {
    LEFT,
    MIDDLE,
    RIGHT,
    TOP,
    CENTER,
    BOTTOM,
};
struct AlignRule {
    std::string anchor;
    union {
        HorizontalAlign horizontal;
        VerticalAlign vertical;
    };
    
    bool operator==(const AlignRule& right) const
    {
        return ((this->anchor == right.anchor) && (this->vertical == right.vertical) &&
                (this->horizontal == right.horizontal));
    }
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_BASE_LAYOUT_POSITION_PARAM_H
