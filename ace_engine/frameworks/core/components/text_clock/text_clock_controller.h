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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_CLOCK_TEXT_CLOCK_CONTROLLER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_CLOCK_TEXT_CLOCK_CONTROLLER_H

#include "base/memory/ace_type.h"

#include <functional>

namespace OHOS::Ace {
using StatusCallback = std::function<void()>;
class ACE_EXPORT TextClockController : public AceType {
    DECLARE_ACE_TYPE(TextClockController, AceType);
public:
    TextClockController() = default;
    ~TextClockController() override = default;

    void OnStart(StatusCallback statusCallback)
    {
        start_ = std::move(statusCallback);
    }

    void OnStop(StatusCallback statusCallback)
    {
        stop_ = std::move(statusCallback);
    }

    void Start()
    {
        if (start_) {
            start_();
        }
    }

    void Stop()
    {
        if (stop_) {
            stop_();
        }
    }

    bool HasInitialized() const
    {
        return start_ && stop_;
    }

private:
    StatusCallback start_;
    StatusCallback stop_;
};
}

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_CLOCK_TEXT_CLOCK_CONTROLLER_H
