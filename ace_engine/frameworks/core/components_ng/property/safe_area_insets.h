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
#ifndef FOUNDATION_ACE_FRAMEWORKS_COMPONENTS_NG_PROPERTIES_SAFE_AREA_INSETS_H
#define FOUNDATION_ACE_FRAMEWORKS_COMPONENTS_NG_PROPERTIES_SAFE_AREA_INSETS_H

#include <cstdint>
#include <string>

namespace OHOS::Ace::NG {
/**
 * @brief Represents the safe area insets in all four directions of the screen.
 *
 * The SafeAreaInsets class represents the insets for the safe area of a device's screen.
 * It provides information about the areas of the screen that are not covered by app's UI elements,
 * such as the status bar, navigation bar, or notch.
 */
struct SafeAreaInsets {
    /**
     * @brief Represents an Inset in one direction of the screen with start/end values in pixel index. For example, a
     * status bar of height 120px would be Inset(start = 0, end = 119) in Top direction.
     */
    struct Inset {
        uint32_t start = 0;
        uint32_t end = 0;

        uint32_t Length() const
        {
            return end - start;
        }
        bool IsValid() const
        {
            return start < end;
        }

        /**
         * @brief Checks if the given position is overlapped by the safe area insets.
         *
         * @param pos The position to check.
         * @return True if the position is overlapped by the safe area insets, false otherwise.
         */
        bool IsOverlapped(float pos) const;

        /**
         * @brief Combines the current inset with another inset.
         *
         * This function combines the current inset with another inset and returns the result.
         *
         * @param other The other inset to combine with.
         * @return The combined inset.
         */
        Inset Combine(const Inset& other) const;

        bool operator==(const Inset& other) const
        {
            return start == other.start && end == other.end;
        }
        std::string ToString() const;
    };

    Inset left_;
    Inset top_;
    Inset right_;
    Inset bottom_;

    SafeAreaInsets() = default;

    SafeAreaInsets(Inset left, Inset top, Inset right, Inset bottom)
        : left_(left), top_(top), right_(right), bottom_(bottom)
    {}

    bool IsValid() const;

    std::string ToString() const;

    bool operator==(const SafeAreaInsets& other) const
    {
        return left_ == other.left_ && top_ == other.top_ && right_ == other.right_ && bottom_ == other.bottom_;
    }

    bool operator!=(const SafeAreaInsets& other) const
    {
        return !(*this == other);
    }

    /**
     * @brief Combines the current SafeAreaInsets with another SafeAreaInsets.
     *
     * This function takes the union of the current SafeAreaInsets with another SafeAreaInsets.
     *
     * @param other The SafeAreaInsets to combine with.
     * @return The combined SafeAreaInsets.
     */
    SafeAreaInsets Combine(const SafeAreaInsets& other) const;
};

inline constexpr uint32_t SAFE_AREA_TYPE_NONE = 0;
inline constexpr uint32_t SAFE_AREA_TYPE_SYSTEM = 1;
inline constexpr uint32_t SAFE_AREA_TYPE_CUTOUT = 1 << 1;
inline constexpr uint32_t SAFE_AREA_TYPE_KEYBOARD = 1 << 2;
inline constexpr uint32_t SAFE_AREA_TYPE_ALL = 0b111;

inline constexpr uint32_t SAFE_AREA_EDGE_NONE = 0;
inline constexpr uint32_t SAFE_AREA_EDGE_TOP = 1;
inline constexpr uint32_t SAFE_AREA_EDGE_BOTTOM = 1 << 1;
inline constexpr uint32_t SAFE_AREA_EDGE_START = 1 << 2;
inline constexpr uint32_t SAFE_AREA_EDGE_END = 1 << 3;
inline constexpr uint32_t SAFE_AREA_EDGE_ALL = 0b1111;

struct SafeAreaExpandOpts {
    uint32_t type = SAFE_AREA_TYPE_NONE;
    uint32_t edges = SAFE_AREA_EDGE_NONE;

    bool operator==(const SafeAreaExpandOpts& other) const
    {
        return type == other.type && edges == other.edges;
    }

    bool operator!=(const SafeAreaExpandOpts& other) const
    {
        return !(*this == other);
    }

    bool Expansive() const
    {
        return type != SAFE_AREA_TYPE_NONE && edges != SAFE_AREA_EDGE_NONE;
    }
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_COMPONENTS_NG_PROPERTIES_SAFE_AREA_INSETS_H
