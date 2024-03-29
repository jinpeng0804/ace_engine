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

#include "core/components/indexer/render_indexer_list.h"

#include "core/components/indexer/indexer_list_component.h"

namespace OHOS::Ace {

RefPtr<RenderNode> RenderIndexerList::Create()
{
    return AceType::MakeRefPtr<RenderIndexerList>();
}

void RenderIndexerList::Update(const RefPtr<Component>& component)
{
    LOGI("[indexer] Update RenderIndexerList");
    RefPtr<IndexerListComponent> itemComponent = AceType::DynamicCast<IndexerListComponent>(component);
    if (!itemComponent) {
        LOGE("[indexer] IndexerListComponent is null");
    }
}

void RenderIndexerList::PerformLayout()
{
    if (GetChildren().empty()) {
        LOGE("RenderIndexerList: no child found in RenderIndexerList!");
    } else {
        auto child = GetChildren().front();
        child->Layout(GetLayoutParam());
        child->SetPosition(Offset::Zero());
        SetLayoutSize(child->GetLayoutSize());
    }
}

} // namespace OHOS::Ace
