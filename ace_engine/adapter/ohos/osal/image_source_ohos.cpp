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

#include "image_source_ohos.h"

#include "image_source.h"
#include "image_type.h"
#include "media_errors.h"

#include "base/image/pixel_map.h"

namespace OHOS::Ace {
RefPtr<ImageSource> ImageSource::Create(int32_t fd)
{
    uint32_t errorCode;
    Media::SourceOptions options;
    auto src = Media::ImageSource::CreateImageSource(fd, options, errorCode);
    if (errorCode != Media::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_IMAGE, " create image source failed, errorCode = %{public}u", errorCode);
        return nullptr;
    }
    return MakeRefPtr<ImageSourceOhos>(std::move(src));
}

RefPtr<ImageSource> ImageSource::Create(const uint8_t* data, uint32_t size)
{
    uint32_t errorCode;
    Media::SourceOptions options;
    auto src = Media::ImageSource::CreateImageSource(data, size, options, errorCode);
    if (errorCode != Media::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_IMAGE, "create image source failed, errorCode = %{public}u", errorCode);
        return nullptr;
    }
    return MakeRefPtr<ImageSourceOhos>(std::move(src));
}

bool ImageSource::IsAstc(const uint8_t* data, size_t size)
{
    return Media::ImageSource::IsASTC(data, size);
}

ImageSource::Size ImageSource::GetASTCInfo(const uint8_t* data, size_t size)
{
    Media::ASTCInfo astcInfo;
    Media::ImageSource::GetASTCInfo(data, size, astcInfo);
    return { astcInfo.size.width, astcInfo.size.height };
}

std::string ImageSourceOhos::GetProperty(const std::string& key)
{
    std::string value;
    uint32_t res = imageSource_->GetImagePropertyString(0, key, value);
    if (res != Media::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_IMAGE, "Get ImageSource property %{public}s failed, errorCode = %{public}u",
            key.c_str(), res);
    }
    return value;
}

RefPtr<PixelMap> ImageSourceOhos::CreatePixelMap(const Size& size)
{
    return CreatePixelMap(0, size);
}

RefPtr<PixelMap> ImageSourceOhos::CreatePixelMap(uint32_t index, const Size& size)
{
    Media::DecodeOptions options;
    if (size.first > 0 && size.second > 0) {
        options.desiredSize = { size.first, size.second };
    }
    uint32_t errorCode;
    auto pixmap = imageSource_->CreatePixelMapEx(index, options, errorCode);
    if (errorCode != Media::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_IMAGE,
            "create PixelMap from ImageSource failed, index = %{public}u, errorCode = %{public}u", index, errorCode);
        return nullptr;
    }
    return PixelMap::Create(std::move(pixmap));
}

ImageSource::Size ImageSourceOhos::GetImageSize()
{
    Media::ImageInfo info;
    auto errorCode = imageSource_->GetImageInfo(info);
    if (errorCode != Media::SUCCESS) {
        TAG_LOGW(AceLogTag::ACE_IMAGE, "Get ImageSource info failed, errorCode = %{public}u", errorCode);
        return { 0, 0 };
    }
    return { info.size.width, info.size.height };
}
} // namespace OHOS::Ace
