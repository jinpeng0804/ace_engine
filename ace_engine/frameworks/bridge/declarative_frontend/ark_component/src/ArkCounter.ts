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

/// <reference path='./import.ts' />
class ArkCounterComponent extends ArkComponent implements CounterAttribute {
  constructor(nativePtr: KNode) {
    super(nativePtr);
  }
  onInc(event: () => void): this {
    throw new Error('Method not implemented.');
  }
  onDec(event: () => void): this {
    throw new Error('Method not implemented.');
  }
  enableDec(value: boolean): this {
    modifierWithKey(this._modifiersWithKeys, EnableDecModifier.identity, EnableDecModifier, value);
    return this;
  }
  enableInc(value: boolean): this {
    modifierWithKey(this._modifiersWithKeys, EnableIncModifier.identity, EnableIncModifier, value);
    return this;
  }
}
class EnableIncModifier extends ModifierWithKey<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity = Symbol('enableInc');
  applyPeer(node: KNode, reset: boolean) {
    if (reset) {
      getUINativeModule().counter.resetEnableInc(node);
    } else {
      getUINativeModule().counter.setEnableInc(node, this.value);
    }
  }
}
class EnableDecModifier extends ModifierWithKey<boolean> {
  constructor(value: boolean) {
    super(value);
  }
  static identity = Symbol('enableDec');
  applyPeer(node: KNode, reset: boolean) {
    if (reset) {
      getUINativeModule().counter.resetEnableDec(node);
    } else {
      getUINativeModule().counter.setEnableDec(node, this.value);
    }
  }
}

// @ts-ignore
globalThis.Counter.attributeModifier = function (modifier) {
  const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
  let nativeNode = getUINativeModule().getFrameNodeById(elmtId);
  let component = this.createOrGetNode(elmtId, () => {
    return new ArkCounterComponent(nativeNode);
  });
  applyUIAttributes(modifier, nativeNode, component);
  component.applyModifierPatch();
};
