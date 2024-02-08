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

#include "core/components_ng/pattern/grid/irregular/grid_irregular_filler.h"

#include "base/geometry/ng/size_t.h"
#include "core/components_ng/pattern/grid/grid_layout_options.h"
#include "core/components_ng/pattern/grid/grid_layout_property.h"

namespace OHOS::Ace::NG {
GridIrregularFiller::GridIrregularFiller(GridLayoutInfo* info, LayoutWrapper* wrapper) : info_(info), wrapper_(wrapper)
{
    // init starting pos
    InitPos();
}

void GridIrregularFiller::InitPos()
{
    const auto& row = info_->gridMatrix_.find(info_->startMainLineIndex_);
    if (row == info_->gridMatrix_.end()) {
        // implies empty matrix
        return;
    }
    for (auto& [col, idx] : row->second) {
        if (idx == info_->startIndex_) {
            posY_ = info_->startMainLineIndex_;
            // to land on the first item after advancing once
            posX_ = col - 1;
            return;
        }
    }
}

bool GridIrregularFiller::IsFull(float targetLen)
{
    return length_ > targetLen || info_->endIndex_ == wrapper_->GetTotalChildCount() - 1;
}

float GridIrregularFiller::Fill(const FillParameters& params)
{
    while (!IsFull(params.targetLen)) {
        int32_t prevRow = posY_;
        if (!FindNextItem(++info_->endIndex_)) {
            FillOne();
        }

        if (posY_ > prevRow) {
            // previous row has been filled
            UpdateLength(prevRow, params.mainGap);
        }

        MeasureNewItem(params, posX_);
    }
    info_->endMainLineIndex_ = posY_;
    return length_;
}

int32_t GridIrregularFiller::FitItem(const decltype(GridLayoutInfo::gridMatrix_)::iterator& it, int32_t itemWidth)
{
    if (it == info_->gridMatrix_.end()) {
        // empty row, can fit
        return 0;
    }

    if (it->second.size() + itemWidth > info_->crossCount_) {
        // not enough space
        return -1;
    }

    for (int i = 0; i <= info_->crossCount_ - itemWidth; ++i) {
        bool flag = true;
        for (int j = 0; j < itemWidth; ++j) {
            if (it->second.find(i + j) != it->second.end()) {
                // spot already filled
                flag = false;
                break;
            }
        }

        if (flag) {
            return i;
        }
    }
    return -1;
}

GridItemSize GridIrregularFiller::GetItemSize(int32_t idx)
{
    GridItemSize size { 1, 1 };
    auto props = AceType::DynamicCast<GridLayoutProperty>(wrapper_->GetLayoutProperty());
    const auto& opts = *props->GetLayoutOptions();
    if (opts.irregularIndexes.find(idx) != opts.irregularIndexes.end()) {
        if (!opts.getSizeByIndex) {
            // default irregular size = [1, full cross length]
            return { 1, info_->crossCount_ };
        }

        size = opts.getSizeByIndex(idx);

        // assume [row] represents crossLength and [column] represents mainLength in this class, so flip sides if
        // horizontal
        if (info_->axis_ == Axis::HORIZONTAL) {
            std::swap(size.rows, size.columns);
        }
    }

    // handle illegal size
    if (size.columns > info_->crossCount_) {
        size.columns = info_->crossCount_;
    }
    return size;
}

void GridIrregularFiller::FillOne()
{
    /* alias */
    const int32_t& idx = info_->endIndex_;
    int32_t row = posY_;

    auto size = GetItemSize(idx);

    auto it = info_->gridMatrix_.find(row);
    int32_t col = FitItem(it, size.columns);
    while (col == -1) {
        // can't fill at end, find the next available line
        it = info_->gridMatrix_.find(++row);
        col = FitItem(it, size.columns);
    }

    if (it == info_->gridMatrix_.end()) {
        // create a new line
        info_->gridMatrix_[row] = {};
    }

    // top left square should be set to [idx], the rest to -1
    for (int32_t r = 0; r < size.rows; ++r) {
        for (int32_t c = 0; c < size.columns; ++c) {
            info_->gridMatrix_[row + r][col + c] = -1;
        }
    }

    info_->gridMatrix_[row][col] = idx;

    posY_ = row;
    posX_ = col;
}

bool GridIrregularFiller::FindNextItem(int32_t target)
{
    const auto& mat = info_->gridMatrix_;
    while (AdvancePos()) {
        if (mat.at(posY_).at(posX_) == target) {
            return true;
        }
    }
    return false;
}

bool GridIrregularFiller::AdvancePos()
{
    ++posX_;
    if (posX_ == info_->crossCount_) {
        // go to next row
        ++posY_;
        posX_ = 0;
    }

    const auto& mat = info_->gridMatrix_;
    if (mat.find(posY_) == mat.end()) {
        return false;
    }

    const auto& row = mat.at(posY_);
    return row.find(posX_) != row.end();
}

void GridIrregularFiller::UpdateLength(int32_t prevRow, float mainGap)
{
    for (int32_t row = prevRow; row < posY_; ++row) {
        length_ += info_->lineHeightMap_[row] + mainGap;
    }
    if (prevRow == info_->startMainLineIndex_) {
        // no gap on first row
        length_ -= mainGap;
    }
}

void GridIrregularFiller::MeasureNewItem(const FillParameters& params, int32_t col)
{
    auto child = wrapper_->GetOrCreateChildByIndex(info_->endIndex_);
    auto props = AceType::DynamicCast<GridLayoutProperty>(wrapper_->GetLayoutProperty());
    auto constraint = props->CreateChildConstraint();

    auto itemSize = GetItemSize(info_->endIndex_);
    float crossLen = 0.0f;
    for (int32_t i = 0; i < itemSize.columns; ++i) {
        crossLen += params.crossLens[i + col];
    }
    crossLen += params.crossGap * (itemSize.columns - 1);

    constraint.percentReference.SetCrossSize(crossLen, info_->axis_);
    if (info_->axis_ == Axis::VERTICAL) {
        constraint.maxSize = SizeF { crossLen, Infinity<float>() };
        constraint.selfIdealSize = OptionalSizeF(crossLen, std::nullopt);
    } else {
        constraint.maxSize = SizeF { Infinity<float>(), crossLen };
        constraint.selfIdealSize = OptionalSizeF(std::nullopt, crossLen);
    }

    child->Measure(constraint);

    float childHeight = child->GetGeometryNode()->GetMarginFrameSize().MainSize(info_->axis_);
    // spread height to each row. May be buggy?
    float heightPerRow = (childHeight - (params.mainGap * (itemSize.rows - 1))) / itemSize.rows;
    for (int32_t i = 0; i < itemSize.rows; ++i) {
        info_->lineHeightMap_[posY_] = std::max(info_->lineHeightMap_[posY_], heightPerRow);
    }
}
} // namespace OHOS::Ace::NG
