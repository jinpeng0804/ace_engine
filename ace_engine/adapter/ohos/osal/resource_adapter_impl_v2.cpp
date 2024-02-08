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

#include "adapter/ohos/osal/resource_adapter_impl_v2.h"

#include <dirent.h>

#include "drawable_descriptor.h"

#include "adapter/ohos/entrance/ace_container.h"
#include "adapter/ohos/osal/resource_convertor.h"
#include "adapter/ohos/osal/resource_theme_style.h"
#include "base/log/log_wrapper.h"
#include "base/utils/system_properties.h"
#include "base/utils/utils.h"
#include "core/components/theme/theme_attributes.h"
namespace OHOS::Ace {
namespace {
constexpr uint32_t OHOS_THEME_ID = 125829872; // ohos_theme
const Color ERROR_VALUE_COLOR = Color(0xff000000);

void CheckThemeId(int32_t& themeId)
{
    if (themeId >= 0) {
        return;
    }
    themeId = OHOS_THEME_ID;
}

const char* PATTERN_MAP[] = {
    THEME_PATTERN_BUTTON,
    THEME_PATTERN_CHECKBOX,
    THEME_PATTERN_DATA_PANEL,
    THEME_PATTERN_RADIO,
    THEME_PATTERN_SWIPER,
    THEME_PATTERN_SWITCH,
    THEME_PATTERN_TOOLBAR,
    THEME_PATTERN_TOGGLE,
    THEME_PATTERN_TOAST,
    THEME_PATTERN_DIALOG,
    THEME_PATTERN_DRAG_BAR,
    THEME_PATTERN_CLOSE_ICON,
    THEME_PATTERN_SEMI_MODAL,
    THEME_PATTERN_BADGE,
    THEME_PATTERN_CALENDAR,
    THEME_PATTERN_CAMERA,
    THEME_PATTERN_CLOCK,
    THEME_PATTERN_CARD,
    THEME_PATTERN_COUNTER,
    THEME_PATTERN_DIVIDER,
    THEME_PATTERN_FOCUS_ANIMATION,
    THEME_PATTERN_GRID,
    THEME_PATTERN_HYPERLINK,
    THEME_PATTERN_IMAGE,
    THEME_PATTERN_LIST,
    THEME_PATTERN_LIST_ITEM,
    THEME_PATTERN_MARQUEE,
    THEME_PATTERN_NAVIGATION_BAR,
    THEME_PATTERN_PICKER,
    THEME_PATTERN_PIECE,
    THEME_PATTERN_POPUP,
    THEME_PATTERN_PROGRESS,
    THEME_PATTERN_QRCODE,
    THEME_PATTERN_RATING,
    THEME_PATTERN_REFRESH,
    THEME_PATTERN_SCROLL_BAR,
    THEME_PATTERN_SEARCH,
    THEME_PATTERN_SELECT,
    THEME_PATTERN_SLIDER,
    THEME_PATTERN_STEPPER,
    THEME_PATTERN_TAB,
    THEME_PATTERN_TEXT,
    THEME_PATTERN_TEXTFIELD,
    THEME_PATTERN_TEXT_OVERLAY,
    THEME_PATTERN_VIDEO,
    THEME_PATTERN_ICON,
    THEME_PATTERN_INDEXER,
    THEME_PATTERN_APP_BAR,
    THEME_PATTERN_ADVANCED_PATTERN,
    THEME_PATTERN_SECURITY_COMPONENT,
    THEME_PATTERN_FORM,
    THEME_PATTERN_SIDE_BAR,
    THEME_PATTERN_RICH_EDITOR,
    THEME_PATTERN_PATTERN_LOCK,
    THEME_PATTERN_GAUGE,
    THEME_PATTERN_SHEET,
    THEME_BLUR_STYLE_COMMON,
    THEME_PATTERN_SHADOW
};

bool IsDirExist(const std::string& path)
{
    char realPath[PATH_MAX] = { 0x00 };
    CHECK_NULL_RETURN(realpath(path.c_str(), realPath), false);
    DIR* dir = opendir(realPath);
    CHECK_NULL_RETURN(dir, false);
    closedir(dir);
    return true;
}

DimensionUnit ParseDimensionUnit(const std::string& unit)
{
    if (unit == "px") {
        return DimensionUnit::PX;
    } else if (unit == "fp") {
        return DimensionUnit::FP;
    } else if (unit == "lpx") {
        return DimensionUnit::LPX;
    } else if (unit == "%") {
        return DimensionUnit::PERCENT;
    } else {
        return DimensionUnit::VP;
    }
};
} // namespace

RefPtr<ResourceAdapter> ResourceAdapter::CreateV2()
{
    return AceType::MakeRefPtr<ResourceAdapterImplV2>();
}

RefPtr<ResourceAdapter> ResourceAdapter::CreateNewResourceAdapter(
    const std::string& bundleName, const std::string& moduleName)
{
    auto container = Container::Current();
    CHECK_NULL_RETURN(container, nullptr);
    auto aceContainer = AceType::DynamicCast<Platform::AceContainer>(container);
    CHECK_NULL_RETURN(aceContainer, nullptr);
    
    RefPtr<ResourceAdapter> newResourceAdapter = nullptr;
    auto context = aceContainer->GetAbilityContextByModule(bundleName, moduleName);
    if (context) {
        auto resourceManager = context->GetResourceManager();
        newResourceAdapter = AceType::MakeRefPtr<ResourceAdapterImplV2>(resourceManager);
    } else {
        newResourceAdapter = ResourceAdapter::CreateV2();
        auto resourceInfo = aceContainer->GetResourceInfo();
        newResourceAdapter->Init(resourceInfo);
    }

    auto resConfig = aceContainer->GetResourceConfiguration();
    newResourceAdapter->UpdateConfig(resConfig);

    return newResourceAdapter;
}

ResourceAdapterImplV2::ResourceAdapterImplV2(std::shared_ptr<Global::Resource::ResourceManager> resourceManager)
{
    sysResourceManager_ = resourceManager;
}

void ResourceAdapterImplV2::Init(const ResourceInfo& resourceInfo)
{
    std::string resPath = resourceInfo.GetPackagePath();
    std::string hapPath = resourceInfo.GetHapPath();
    auto resConfig = ConvertConfigToGlobal(resourceInfo.GetResourceConfiguration());
    std::shared_ptr<Global::Resource::ResourceManager> newResMgr(Global::Resource::CreateResourceManager());
    std::string resIndexPath = hapPath.empty() ? (resPath + "resources.index") : hapPath;
    newResMgr->AddResource(resIndexPath.c_str());
    if (resConfig != nullptr) {
        newResMgr->UpdateResConfig(*resConfig);
    }
    sysResourceManager_ = newResMgr;
    packagePathStr_ = (hapPath.empty() || IsDirExist(resPath)) ? resPath : std::string();
    resConfig_ = resConfig;
}

bool ResourceAdapterImplV2::NeedUpdateResConfig(const std::shared_ptr<Global::Resource::ResConfig>& oldResConfig,
    const std::shared_ptr<Global::Resource::ResConfig>& newResConfig)
{
    if (oldResConfig == nullptr) {
        return true;
    }
    auto oldLocaleInfo = oldResConfig->GetLocaleInfo();
    auto newLocaleInfo = newResConfig->GetLocaleInfo();
    bool isLocaleChange = false;
    if (newLocaleInfo == nullptr) {
        isLocaleChange = false;
    } else if (oldLocaleInfo == nullptr) {
        isLocaleChange = true;
    } else {
        isLocaleChange = std::string(oldLocaleInfo->getLanguage()) != std::string(newLocaleInfo->getLanguage()) ||
                         std::string(oldLocaleInfo->getScript()) != std::string(newLocaleInfo->getScript()) ||
                         std::string(oldLocaleInfo->getCountry()) != std::string(newLocaleInfo->getCountry());
    }

    return oldResConfig->GetDeviceType() != newResConfig->GetDeviceType() ||
           oldResConfig->GetDirection() != newResConfig->GetDirection() ||
           oldResConfig->GetScreenDensity() != newResConfig->GetScreenDensity() ||
           oldResConfig->GetColorMode() != newResConfig->GetColorMode() ||
           oldResConfig->GetInputDevice() != newResConfig->GetInputDevice() || isLocaleChange;
}

void ResourceAdapterImplV2::UpdateConfig(const ResourceConfiguration& config, bool themeFlag)
{
    auto resConfig = ConvertConfigToGlobal(config);
    auto needUpdateResConfig = NeedUpdateResConfig(resConfig_, resConfig) || themeFlag;
    if (sysResourceManager_ && resConfig != nullptr && needUpdateResConfig) {
        sysResourceManager_->UpdateResConfig(*resConfig, themeFlag);
    }
    resConfig_ = resConfig;
}

RefPtr<ThemeStyle> ResourceAdapterImplV2::GetTheme(int32_t themeId)
{
    CheckThemeId(themeId);
    auto theme = AceType::MakeRefPtr<ResourceThemeStyle>(AceType::Claim(this));
    constexpr char OHFlag[] = "ohos_"; // fit with resource/base/theme.json and pattern.json
    {
        auto manager = GetResourceManager();
        if (manager) {
            auto ret = manager->GetThemeById(themeId, theme->rawAttrs_);
            for (size_t i = 0; i < sizeof(PATTERN_MAP) / sizeof(PATTERN_MAP[0]); i++) {
                ResourceThemeStyle::RawAttrMap attrMap;
                std::string patternTag = PATTERN_MAP[i];
                std::string patternName = std::string(OHFlag) + PATTERN_MAP[i];
                ret = manager->GetPatternByName(patternName.c_str(), attrMap);
                if (attrMap.empty()) {
                    continue;
                }
                theme->patternAttrs_[patternTag] = attrMap;
            }
        }
    }

    if (theme->patternAttrs_.empty() && theme->rawAttrs_.empty()) {
        return nullptr;
    }

    theme->ParseContent();
    theme->patternAttrs_.clear();
    return theme;
}

Color ResourceAdapterImplV2::GetColor(uint32_t resId)
{
    uint32_t result = 0;
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, Color(result));
    auto state = manager->GetColorById(resId, result);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get color by id error, id=%{public}u", resId);
        return ERROR_VALUE_COLOR;
    }
    return Color(result);
}

Color ResourceAdapterImplV2::GetColorByName(const std::string& resName)
{
    uint32_t result = 0;
    auto actualResName = GetActualResourceName(resName);
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, Color(result));
    auto state = manager->GetColorByName(actualResName.c_str(), result);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get color by name error, name=%{public}s, errorCode=%{public}d",
            resName.c_str(), state);
    }
    return Color(result);
}

Dimension ResourceAdapterImplV2::GetDimension(uint32_t resId)
{
    float dimensionFloat = 0.0f;
#ifdef NG_BUILD
    std::string unit;
    auto manager = GetResourceManager();
    if (manager) {
        auto state = manager->GetFloatById(resId, dimensionFloat, unit);
        if (state != Global::Resource::SUCCESS) {
            TAG_LOGW(AceLogTag::ACE_RESOURCE, "NG Get dimension by id error, id=%{public}u, errorCode=%{public}d",
                resId, state);
        }
    }
    return Dimension(static_cast<double>(dimensionFloat), ParseDimensionUnit(unit));
#else
    if (Container::IsCurrentUseNewPipeline()) {
        std::string unit;
        auto manager = GetResourceManager();
        if (manager) {
            auto state = manager->GetFloatById(resId, dimensionFloat, unit);
            if (state != Global::Resource::SUCCESS) {
                TAG_LOGW(AceLogTag::ACE_RESOURCE, "NG: Get dimension by id error, id=%{public}u, errorCode=%{public}d",
                    resId, state);
            }
        }
        return Dimension(static_cast<double>(dimensionFloat), ParseDimensionUnit(unit));
    }

    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, Dimension(static_cast<double>(dimensionFloat)));
    auto state = manager->GetFloatById(resId, dimensionFloat);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(
            AceLogTag::ACE_RESOURCE, "Get dimension by id error, id=%{public}u, errorCode=%{public}d", resId, state);
    }
    return Dimension(static_cast<double>(dimensionFloat));
#endif
}

Dimension ResourceAdapterImplV2::GetDimensionByName(const std::string& resName)
{
    float dimensionFloat = 0.0f;
    auto actualResName = GetActualResourceName(resName);
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, Dimension());
    std::string unit;
    auto state = manager->GetFloatByName(actualResName.c_str(), dimensionFloat, unit);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get dimension by name error, resName=%{public}s, errorCode=%{public}d",
            resName.c_str(), state);
    }
    return Dimension(static_cast<double>(dimensionFloat), ParseDimensionUnit(unit));
}

std::string ResourceAdapterImplV2::GetString(uint32_t resId)
{
    std::string strResult = "";
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, strResult);
    auto state = manager->GetStringById(resId, strResult);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get string by id error, id=%{public}u, errorCode=%{public}d", resId, state);
    }
    return strResult;
}

std::string ResourceAdapterImplV2::GetStringByName(const std::string& resName)
{
    std::string strResult = "";
    auto actualResName = GetActualResourceName(resName);
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, strResult);
    auto state = manager->GetStringByName(actualResName.c_str(), strResult);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get string by name error, resName=%{public}s, errorCode=%{public}d",
            resName.c_str(), state);
    }
    return strResult;
}

std::string ResourceAdapterImplV2::GetPluralString(uint32_t resId, int quantity)
{
    std::string strResult = "";
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, strResult);
    auto state = manager->GetPluralStringById(resId, quantity, strResult);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get plural string by id error, id=%{public}u, errorCode=%{public}d", resId,
            state);
    }
    return strResult;
}

std::string ResourceAdapterImplV2::GetPluralStringByName(const std::string& resName, int quantity)
{
    std::string strResult = "";
    auto actualResName = GetActualResourceName(resName);
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, strResult);
    auto state = manager->GetPluralStringByName(actualResName.c_str(), quantity, strResult);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get plural string by name error, resName=%{public}s, errorCode=%{public}d",
            resName.c_str(), state);
    }
    return strResult;
}

std::vector<std::string> ResourceAdapterImplV2::GetStringArray(uint32_t resId) const
{
    std::vector<std::string> strResults;
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, strResults);
    auto state = manager->GetStringArrayById(resId, strResults);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(
            AceLogTag::ACE_RESOURCE, "Get stringArray by id error, id=%{public}u, errorCode=%{public}d", resId, state);
    }
    return strResults;
}

std::vector<std::string> ResourceAdapterImplV2::GetStringArrayByName(const std::string& resName) const
{
    std::vector<std::string> strResults;
    auto actualResName = GetActualResourceName(resName);
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, strResults);
    auto state = manager->GetStringArrayByName(actualResName.c_str(), strResults);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get stringArray by name error, resName=%{public}s, errorCode=%{public}d",
            resName.c_str(), state);
    }
    return strResults;
}

double ResourceAdapterImplV2::GetDouble(uint32_t resId)
{
    float result = 0.0f;
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, static_cast<double>(result));
    auto state = manager->GetFloatById(resId, result);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get double by id error, id=%{public}u, errorCode=%{public}d", resId, state);
    }
    return static_cast<double>(result);
}

double ResourceAdapterImplV2::GetDoubleByName(const std::string& resName)
{
    float result = 0.0f;
    auto actualResName = GetActualResourceName(resName);
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, static_cast<double>(result));
    auto state = manager->GetFloatByName(actualResName.c_str(), result);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get double by name error, resName=%{public}s, errorCode=%{public}d",
            resName.c_str(), state);
    }
    return static_cast<double>(result);
}

int32_t ResourceAdapterImplV2::GetInt(uint32_t resId)
{
    int32_t result = 0;
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, result);
    auto state = manager->GetIntegerById(resId, result);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get int by id error, id=%{public}u, errorCode=%{public}d", resId, state);
    }
    return result;
}

int32_t ResourceAdapterImplV2::GetIntByName(const std::string& resName)
{
    int32_t result = 0;
    auto actualResName = GetActualResourceName(resName);
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, result);
    auto state = manager->GetIntegerByName(actualResName.c_str(), result);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get int by name error, resName=%{public}s, errorCode=%{public}d",
            resName.c_str(), state);
    }
    return result;
}

std::vector<uint32_t> ResourceAdapterImplV2::GetIntArray(uint32_t resId) const
{
    std::vector<int> intVectorResult;
    {
        auto manager = GetResourceManager();
        if (manager) {
            auto state = manager->GetIntArrayById(resId, intVectorResult);
            if (state != Global::Resource::SUCCESS) {
                TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get intArray by id error, id=%{public}u, errorCode=%{public}d",
                    resId, state);
            }
        }
    }

    std::vector<uint32_t> result;
    std::transform(
        intVectorResult.begin(), intVectorResult.end(), result.begin(), [](int x) { return static_cast<uint32_t>(x); });
    return result;
}

std::vector<uint32_t> ResourceAdapterImplV2::GetIntArrayByName(const std::string& resName) const
{
    std::vector<int> intVectorResult;
    auto actualResName = GetActualResourceName(resName);
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, {});
    auto state = manager->GetIntArrayByName(actualResName.c_str(), intVectorResult);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get intArray by name error, resName=%{public}s, errorCode=%{public}d",
            resName.c_str(), state);
    }

    std::vector<uint32_t> result;
    std::transform(
        intVectorResult.begin(), intVectorResult.end(), result.begin(), [](int x) { return static_cast<uint32_t>(x); });
    return result;
}

bool ResourceAdapterImplV2::GetBoolean(uint32_t resId) const
{
    bool result = false;
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, result);
    auto state = manager->GetBooleanById(resId, result);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get boolean by id error, id=%{public}u, errorCode=%{public}d", resId, state);
    }
    return result;
}

bool ResourceAdapterImplV2::GetBooleanByName(const std::string& resName) const
{
    bool result = false;
    auto actualResName = GetActualResourceName(resName);
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, result);
    auto state = manager->GetBooleanByName(actualResName.c_str(), result);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get boolean by name error, resName=%{public}s, errorCode=%{public}d",
            resName.c_str(), state);
    }
    return result;
}

std::shared_ptr<Media::PixelMap> ResourceAdapterImplV2::GetPixelMap(uint32_t resId)
{
    auto manager = GetResourceManager();

    CHECK_NULL_RETURN(manager, nullptr);
    Napi::DrawableDescriptor::DrawableType drawableType;
    Global::Resource::RState state;
    auto drawableDescriptor =
        Napi::DrawableDescriptorFactory::Create(resId, sysResourceManager_, state, drawableType, 0);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Failed to Create drawableDescriptor by %{public}d, errorCode=%{public}d",
            resId, state);
        return nullptr;
    }
    CHECK_NULL_RETURN(drawableDescriptor, nullptr);
    return drawableDescriptor->GetPixelMap();
}

std::string ResourceAdapterImplV2::GetMediaPath(uint32_t resId)
{
    std::string mediaPath = "";
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, "");
    auto state = manager->GetMediaById(resId, mediaPath);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get media by id error, id=%{public}u, errorCode=%{public}u", resId, state);
        return "";
    }
    if (SystemProperties::GetUnZipHap()) {
        return "file:///" + mediaPath;
    }
    auto pos = mediaPath.find_last_of('.');
    if (pos == std::string::npos) {
        return "";
    }
    return "resource:///" + std::to_string(resId) + mediaPath.substr(pos);
}

std::string ResourceAdapterImplV2::GetMediaPathByName(const std::string& resName)
{
    std::string mediaPath = "";
    auto actualResName = GetActualResourceName(resName);
    {
        auto manager = GetResourceManager();
        CHECK_NULL_RETURN(manager, "");
        auto state = manager->GetMediaByName(actualResName.c_str(), mediaPath);
        if (state != Global::Resource::SUCCESS) {
            TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get media path by name error, resName=%{public}s, errorCode=%{public}u",
                resName.c_str(), state);
            return "";
        }
    }
    if (SystemProperties::GetUnZipHap()) {
        return "file:///" + mediaPath;
    }
    auto pos = mediaPath.find_last_of('.');
    if (pos == std::string::npos) {
        return "";
    }
    return "resource:///" + actualResName + mediaPath.substr(pos);
}

std::string ResourceAdapterImplV2::GetRawfile(const std::string& fileName)
{
    // as web component not support resource format: resource://RAWFILE/{fileName}, use old format
    if (!packagePathStr_.empty()) {
        std::string outPath;
        auto manager = GetResourceManager();
        CHECK_NULL_RETURN(manager, "");
        // Adapt to the input like: "file:///index.html?a=1", before the new solution comes.
        auto it = std::find_if(fileName.begin(), fileName.end(), [](char c) { return (c == '#') || (c == '?'); });
        std::string params;
        std::string newFileName = fileName;
        if (it != fileName.end()) {
            newFileName = std::string(fileName.begin(), it);
            params = std::string(it, fileName.end());
        }
        auto state = manager->GetRawFilePathByName(newFileName, outPath);
        if (state != Global::Resource::SUCCESS) {
            TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get rawFile error, filename:%{public}s, error:%{public}u",
                fileName.c_str(), state);
            return "";
        }
        return "file:///" + outPath + params;
    }
    return "resource://RAWFILE/" + fileName;
}

bool ResourceAdapterImplV2::GetRawFileData(const std::string& rawFile, size_t& len, std::unique_ptr<uint8_t[]>& dest)
{
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, false);
    auto state = manager->GetRawFileFromHap(rawFile, len, dest);
    if (state != Global::Resource::SUCCESS || !dest) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get rawFile from hap error, raw filename:%{public}s, error:%{public}u",
            rawFile.c_str(), state);
        return false;
    }
    return true;
}

bool ResourceAdapterImplV2::GetRawFileData(const std::string& rawFile, size_t& len, std::unique_ptr<uint8_t[]>& dest,
    const std::string& bundleName, const std::string& moduleName)
{
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, false);
    auto state = manager->GetRawFileFromHap(rawFile, len, dest);
    if (state != Global::Resource::SUCCESS || !dest) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE,
            "Get rawFile from hap error, raw filename:%{public}s, bundleName:%{public}s, moduleName:%{public}s, "
            "error:%{public}u",
            rawFile.c_str(), bundleName.c_str(), moduleName.c_str(), state);
        return false;
    }
    return true;
}

bool ResourceAdapterImplV2::GetMediaData(uint32_t resId, size_t& len, std::unique_ptr<uint8_t[]>& dest)
{
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, false);
    auto state = manager->GetMediaDataById(resId, len, dest);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get media data by id error, id:%{public}u, error:%{public}u", resId, state);
        return false;
    }
    return true;
}

bool ResourceAdapterImplV2::GetMediaData(uint32_t resId, size_t& len, std::unique_ptr<uint8_t[]>& dest,
    const std::string& bundleName, const std::string& moduleName)
{
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, false);
    auto state = manager->GetMediaDataById(resId, len, dest);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE,
            "Get media data by id error, id:%{public}u, bundleName:%{public}s, moduleName:%{public}s, error:%{public}u",
            resId, bundleName.c_str(), moduleName.c_str(), state);
        return false;
    }
    return true;
}

bool ResourceAdapterImplV2::GetMediaData(const std::string& resName, size_t& len, std::unique_ptr<uint8_t[]>& dest)
{
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, false);
    auto state = manager->GetMediaDataByName(resName.c_str(), len, dest);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get media data by name error, resName:%{public}s, error:%{public}u",
            resName.c_str(), state);
        return false;
    }
    return true;
}

bool ResourceAdapterImplV2::GetMediaData(const std::string& resName, size_t& len, std::unique_ptr<uint8_t[]>& dest,
    const std::string& bundleName, const std::string& moduleName)
{
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, false);
    auto state = manager->GetMediaDataByName(resName.c_str(), len, dest);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE,
            "Get media data by name error, resName:%{public}s, bundleName:%{public}s, moduleName:%{public}s, "
            "error:%{public}u",
            resName.c_str(), bundleName.c_str(), moduleName.c_str(), state);
        return false;
    }
    return true;
}

bool ResourceAdapterImplV2::GetRawFileDescription(
    const std::string& rawfileName, RawfileDescription& rawfileDescription) const
{
    OHOS::Global::Resource::ResourceManager::RawFileDescriptor descriptor;
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, false);
    auto state = manager->GetRawFileDescriptorFromHap(rawfileName, descriptor);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get raw file description error, rawFileName:%{public}s, error:%{public}u",
            rawfileName.c_str(), state);
        return false;
    }
    rawfileDescription.fd = descriptor.fd;
    rawfileDescription.offset = descriptor.offset;
    rawfileDescription.length = descriptor.length;
    return true;
}

bool ResourceAdapterImplV2::GetMediaById(const int32_t& resId, std::string& mediaPath) const
{
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, false);
    auto state = manager->GetMediaById(resId, mediaPath);
    if (state != Global::Resource::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_RESOURCE, "Get media by id error, resId:%{public}d, error:%{public}u", resId, state);
        return false;
    }
    return true;
}

std::string ResourceAdapterImplV2::GetActualResourceName(const std::string& resName) const
{
    auto index = resName.find_last_of('.');
    if (index == std::string::npos) {
        return {};
    }
    return resName.substr(index + 1, resName.length() - index - 1);
}

uint32_t ResourceAdapterImplV2::GetResourceLimitKeys() const
{
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, 0);
    return manager->GetResourceLimitKeys();
}

uint32_t ResourceAdapterImplV2::GetSymbolByName(const char* resName) const
{
    uint32_t result = 0;
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, -1);
    manager->GetSymbolByName(resName, result);
    return result;
}

uint32_t ResourceAdapterImplV2::GetSymbolById(uint32_t resId) const
{
    uint32_t result = 0;
    auto manager = GetResourceManager();
    CHECK_NULL_RETURN(manager, -1);
    manager->GetSymbolById(resId, result);
    return result;
}

} // namespace OHOS::Ace
