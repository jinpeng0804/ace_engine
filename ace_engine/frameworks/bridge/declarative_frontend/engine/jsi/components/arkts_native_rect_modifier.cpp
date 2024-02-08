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
#include "bridge/declarative_frontend/engine/jsi/components/arkts_native_rect_modifier.h"

#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/pattern/shape/rect_model_ng.h"

namespace OHOS::Ace::NG {
namespace {
constexpr uint32_t VALID_RADIUS_PAIR_FLAG = 1;
} // namespace
void SetRectRadiusWidth(NodeHandle node, double radiusWidthValue, int32_t radiusWidthUnit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    RectModelNG::SetRadiusWidth(frameNode, CalcDimension(radiusWidthValue, (DimensionUnit)radiusWidthUnit));
}

void ResetRectRadiusWidth(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension defaultDimension;
    defaultDimension.Reset();
    RectModelNG::SetRadiusWidth(frameNode, defaultDimension);
    return;
}

void SetRectRadiusHeight(NodeHandle node, double radiusHeightValue, int32_t radiusHeightUnit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    RectModelNG::SetRadiusHeight(frameNode, CalcDimension(radiusHeightValue, (DimensionUnit)radiusHeightUnit));
}

void ResetRectRadiusHeight(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CalcDimension defaultDimension;
    defaultDimension.Reset();
    RectModelNG::SetRadiusHeight(frameNode, defaultDimension);
}

void SetRectRadiusWithArray(NodeHandle node, double* radiusValues, int32_t* radiusUnits, uint32_t* radiusValidPairs,
    size_t radiusValidPairsSize)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    CHECK_NULL_VOID(radiusValues);
    CHECK_NULL_VOID(radiusUnits);
    CHECK_NULL_VOID(radiusValidPairs);
    for (size_t index = 0; index < radiusValidPairsSize; index++) {
        if (radiusValidPairs[index] == VALID_RADIUS_PAIR_FLAG) {
            int xIndex = index * 2;
            int yIndex = xIndex + 1;
            auto radiusX = CalcDimension(radiusValues[xIndex], (DimensionUnit)radiusUnits[xIndex]);
            auto radiusY = CalcDimension(radiusValues[yIndex], (DimensionUnit)radiusUnits[yIndex]);
            RectModelNG::SetRadiusValue(frameNode, radiusX, radiusY, index);
        }
    }
}

void SetRectRadiusWithValue(NodeHandle node, double radiusValue, int32_t radiusUnit)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    RectModelNG::SetRadiusWidth(frameNode, CalcDimension(radiusValue, (DimensionUnit)radiusUnit));
    RectModelNG::SetRadiusHeight(frameNode, CalcDimension(radiusValue, (DimensionUnit)radiusUnit));
}

void ResetRectRadius(NodeHandle node)
{
    auto* frameNode = reinterpret_cast<FrameNode*>(node);
    CHECK_NULL_VOID(frameNode);
    NG::ResetRectRadiusHeight(frameNode);
    NG::ResetRectRadiusWidth(frameNode);
}

ArkUIRectModifierAPI GetRectModifier()
{
    static const ArkUIRectModifierAPI modifier = { SetRectRadiusWidth, ResetRectRadiusWidth, SetRectRadiusHeight,
        ResetRectRadiusHeight, SetRectRadiusWithArray, SetRectRadiusWithValue, ResetRectRadius };

    return modifier;
}
} // namespace OHOS::Ace::NG
