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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SAFE_AREA_SAFE_AREA_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SAFE_AREA_SAFE_AREA_MANAGER_H

#include "base/memory/ace_type.h"
#include "base/utils/noncopyable.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/property/safe_area_insets.h"
#include "core/components_ng/property/transition_property.h"

namespace OHOS::Ace::NG {
// SafeAreaManager stores layout information to apply SafeArea correctly.
class SafeAreaManager : public virtual AceType {
    DECLARE_ACE_TYPE(SafeAreaManager, AceType);

public:
    SafeAreaManager() = default;
    ~SafeAreaManager() override = default;

    /**
     * @brief Updates the system safe area.
     *
     * @param safeArea The new system safe area.
     * @return True if the system safe area was modified, false otherwise.
     */
    bool UpdateSystemSafeArea(const SafeAreaInsets& safeArea);

    /**
     * @brief Updates the navigation indictor safe area.
     *
     * @param safeArea The new navigation indictor safe area.
     * @return True if the system safe area was modified, false otherwise.
     */
    bool UpdateNavArea(const SafeAreaInsets& safeArea);

    /**
     * @brief Retrieves the system safe area insets.
     *
     * This function returns the safe area insets of the system, which represents the portion of the screen that is
     * covered by system UI elements such as the status bar or navigation bar.
     *
     * @return The system safe area insets.
     */
    SafeAreaInsets GetSystemSafeArea() const;

    /**
     * @brief Updates the cutout safe area.
     *
     * This function is responsible for updating the cutout safe area based on the provided SafeAreaInsets.
     *
     * @param safeArea The SafeAreaInsets representing the new cutout safe area.
     * @return true if the cutout safe area was successfully updated, false otherwise.
     */
    bool UpdateCutoutSafeArea(const SafeAreaInsets& safeArea);

    /**
     * @brief Retrieves the safe area insets that account for any cutout areas on the screen.
     *
     * @return The safe area insets that account for any cutout areas on the screen.
     */
    SafeAreaInsets GetCutoutSafeArea() const;

    /**
     * @brief Retrieves the safe area insets combining System and Cutout.
     *
     * @return The System & Cutout safe area insets.
     */
    SafeAreaInsets GetSafeArea() const;

    /**
     * @brief Updates the safe area to accommodate the keyboard.
     *
     * This function is called to update the safe area when the keyboard is shown or hidden.
     *
     * @param keyboardHeight The height of the keyboard in pixels.
     * @return true if the safe area was modified, false otherwise.
     */
    bool UpdateKeyboardSafeArea(float keyboardHeight);

    /**
     * @brief Retrieves the inset of the safe area caused by the keyboard.
     *
     * @return The inset of the safe area caused by the keyboard.
     */
    SafeAreaInsets::Inset GetKeyboardInset() const
    {
        return keyboardInset_;
    }

    void UpdateKeyboardOffset(float offset)
    {
        keyboardOffset_ = offset;
    }
    float GetKeyboardOffset() const;

    bool KeyboardSafeAreaEnabled() const
    {
        return keyboardSafeAreaEnabled_;
    }

    SafeAreaInsets GetCombinedSafeArea(const SafeAreaExpandOpts& opts) const;

    const std::set<WeakPtr<FrameNode>>& GetGeoRestoreNodes() const
    {
        return geoRestoreNodes_;
    }

    void AddGeoRestoreNode(const WeakPtr<FrameNode>& node)
    {
        geoRestoreNodes_.insert(node);
    }

    void RemoveRestoreNode(const WeakPtr<FrameNode>& node)
    {
        geoRestoreNodes_.erase(node);
    }

    RefPtr<InterpolatingSpring> GetSafeAreaCurve() const
    {
        return safeAreaCurve_;
    }

    OffsetF GetWindowWrapperOffset();

    bool SetIsFullScreen(bool value);
    bool SetIsNeedAvoidWindow(bool value);
    bool SetIgnoreSafeArea(bool value);
    bool SetKeyBoardAvoidMode(bool value);

private:
    // app window is full screen
    // todo: remove and only use isNeedAvoidWindow_
    bool isFullScreen_ = false;

    /**
     * @brief Indicates whether the UI within the current window type needs to avoid SafeAreaInsets.
     */
    bool isNeedAvoidWindow_ = false;

    /**
     * @brief Indicates whether to ignore the SafeAreaInsets, specified by the developer from frontend.
     */
    bool ignoreSafeArea_ = false;

    /**
     * @brief Indicates whether the keyboard safe area is enabled. When enabled, UI avoids the keyboard inset and the
     * Page is compressed when the keyboard is up. When disabled, the size of Page doesn't change, but Page would
     * offset vertically according to [keyboardOffset_].
     */
    bool keyboardSafeAreaEnabled_ = false;

    SafeAreaInsets systemSafeArea_;
    SafeAreaInsets cutoutSafeArea_;
    SafeAreaInsets navSafeArea_;
    // keyboard is bottom direction only
    SafeAreaInsets::Inset keyboardInset_;

    /**
     * @brief A set of weak pointers to FrameNode objects whose geometry info are saved before their SafeArea
     * expansion. The geometry info of these nodes need to be restored when their next Layout task begins.
     */
    std::set<WeakPtr<FrameNode>> geoRestoreNodes_;

    // amount of offset to apply to Page when keyboard is up
    float keyboardOffset_ = 0.0f;

    static constexpr float SAFE_AREA_VELOCITY = 0.0f;
    static constexpr float SAFE_AREA_MASS = 1.0f;
    static constexpr float SAFE_AREA_STIFFNESS = 228.0f;
    static constexpr float SAFE_AREA_DAMPING = 30.0f;
    RefPtr<InterpolatingSpring> safeAreaCurve_ = AceType::MakeRefPtr<InterpolatingSpring>(
        SAFE_AREA_VELOCITY, SAFE_AREA_MASS, SAFE_AREA_STIFFNESS, SAFE_AREA_DAMPING);

    ACE_DISALLOW_COPY_AND_MOVE(SafeAreaManager);
};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_MANAGER_SAFE_AREA_SAFE_AREA_MANAGER_H
