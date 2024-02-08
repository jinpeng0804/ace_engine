/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_ITEM_GROUP_MODEL_NG_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_ITEM_GROUP_MODEL_NG_H

#include <functional>
#include <memory>

#include "base/geometry/axis.h"
#include "base/geometry/dimension.h"
#include "core/components/common/layout/constants.h"
#include "core/components_v2/list/list_properties.h"
#include "core/components_ng/pattern/list/list_item_group_model.h"

namespace OHOS::Ace::NG {

class ACE_EXPORT ListItemGroupModelNG : public ListItemGroupModel {
public:
    void Create(V2::ListItemGroupStyle listItemGroupStyle) override;
    void SetSpace(const Dimension& space) override;
    void SetDivider(const V2::ItemDivider& divider) override;
    void SetHeader(std::function<void()>&& header) override;
    void SetFooter(std::function<void()>&& footer) override;

    static void SetDivider(FrameNode* frameNode, const V2::ItemDivider& divider);
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_LIST_LIST_ITEM_MODEL_H
