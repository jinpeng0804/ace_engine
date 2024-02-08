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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_PATTERNS_PARTICLE_MODEL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_PATTERNS_PARTICLE_MODEL_H
#include <functional>
#include <list>
#include <memory>
#include <mutex>

#include "base/utils/macros.h"
#include "core/components_ng/property/particle_property.h"
namespace OHOS::Ace {
class ACE_EXPORT ParticleModel {
public:
    static ParticleModel* GetInstance();
    virtual ~ParticleModel() = default;
    virtual void Create(std::list<NG::ParticleOption>& arrayValue) = 0;

private:
    static std::unique_ptr<ParticleModel> instance_;
    static std::mutex mutex_;
};
} // namespace OHOS::Ace
#endif