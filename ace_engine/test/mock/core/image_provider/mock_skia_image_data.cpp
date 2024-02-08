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

#ifndef USE_ROSEN_DRAWING
#include "core/components_ng/image_provider/adapter/skia_image_data.h"
#else
#include "core/components_ng/image_provider/adapter/rosen/drawing_image_data.h"
#endif

namespace OHOS::Ace::NG {
static int32_t frameCount { 0 };
#ifndef USE_ROSEN_DRAWING
SkiaImageData::SkiaImageData(const void* data, size_t length)
#else
DrawingImageData::DrawingImageData(const void* data, size_t length)
#endif
{
    frameCount = length;
}

RefPtr<ImageData> ImageData::MakeFromDataWithCopy(const void* data, size_t length)
{
    return nullptr;
}

RefPtr<ImageData> ImageData::MakeFromDataWrapper(void* dataWrapper)
{
    return nullptr;
}

#ifndef USE_ROSEN_DRAWING
size_t SkiaImageData::GetSize() const
{
    CHECK_NULL_RETURN(skData_, 0);
    return skData_->size();
}
#else
size_t DrawingImageData::GetSize() const
{
    CHECK_NULL_RETURN(rsData_, 0);
    return rsData_->GetSize();
}
#endif

#ifndef USE_ROSEN_DRAWING
const void* SkiaImageData::GetData() const
{
    CHECK_NULL_RETURN(skData_, nullptr);
    return skData_->data();
}
#else
const void* DrawingImageData::GetData() const
{
    CHECK_NULL_RETURN(rsData_, nullptr);
    return rsData_->GetData();
}
#endif

#ifndef USE_ROSEN_DRAWING
sk_sp<SkData> SkiaImageData::GetSkData() const
{
    CHECK_NULL_RETURN(skData_, nullptr);
    return skData_;
}
#else
std::shared_ptr<RSData> DrawingImageData::GetRSData() const
{
    CHECK_NULL_RETURN(rsData_, nullptr);
    return rsData_;
}
#endif

#ifndef USE_ROSEN_DRAWING
RefPtr<SvgDomBase> SkiaImageData::MakeSvgDom(const std::optional<Color>& svgFillColor)
#else
RefPtr<SvgDomBase> DrawingImageData::MakeSvgDom(const std::optional<Color>& svgFillColor)
#endif
{
    return nullptr;
}

#ifndef USE_ROSEN_DRAWING
std::pair<SizeF, int32_t> SkiaImageData::Parse() const
#else
std::pair<SizeF, int32_t> DrawingImageData::Parse() const
#endif
{
    return { SizeF(1, 1), frameCount };
}

#ifndef USE_ROSEN_DRAWING
const void* SkiaImageData::GetDataWrapper() const
{
    return nullptr;
}
#endif
} // namespace OHOS::Ace::NG
