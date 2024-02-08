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

declare class __JSBaseNode__ {
  create(builder: (...args: Object[]) => void, params: Object): number | null;
  createRenderNode(): number | null;
}

class BaseNode extends __JSBaseNode__ {
  protected instanceId_?: number;
  protected nodePtr_: number | null;
  constructor(uiContext: UIContext) {
    super();
    var instanceId: number = -1;
    if (uiContext === undefined) {
      throw Error("BuilderNode constructor error, param uiContext error");
    } else {
      if (!(typeof uiContext === "object") || !("instanceId_" in uiContext)) {
        throw Error(
          "BuilderNode constructor error, param uiContext is invalid"
        );
      }
      instanceId = uiContext.instanceId_;
    }
    this.instanceId_ = instanceId;
  }
}
