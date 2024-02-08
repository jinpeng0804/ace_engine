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
class ArkSelectComponent extends ArkComponent implements SelectAttribute {
  constructor(nativePtr: KNode) {
    super(nativePtr);
  }
  onGestureJudgeBegin(callback: (gestureInfo: GestureInfo, event: BaseGestureEvent) => GestureJudgeResult): this {
    throw new Error('Method not implemented.');
  }
  optionWidth(value: Dimension | OptionWidthMode): this {
    throw new Error('Method not implemented.');
  }
  optionHeight(value: Dimension): this {
    throw new Error('Method not implemented.');
  }
  selected(value: number | Resource): this {
    modifierWithKey(
      this._modifiersWithKeys, SelectedModifier.identity, SelectedModifier, value);
    return this;
  }
  value(value: ResourceStr): this {
    modifierWithKey(
      this._modifiersWithKeys, ValueModifier.identity, ValueModifier, value);
    return this;
  }
  font(value: Font): this {
    modifierWithKey(
      this._modifiersWithKeys, FontModifier.identity, FontModifier, value);
    return this;
  }
  fontColor(value: ResourceColor): this {
    modifierWithKey(
      this._modifiersWithKeys, SelectFontColorModifier.identity, SelectFontColorModifier, value);
    return this;
  }
  selectedOptionBgColor(value: ResourceColor): this {
    modifierWithKey(
      this._modifiersWithKeys, SelectedOptionBgColorModifier.identity, SelectedOptionBgColorModifier, value);
    return this;
  }
  selectedOptionFont(value: Font): this {
    modifierWithKey(
      this._modifiersWithKeys, SelectedOptionFontModifier.identity, SelectedOptionFontModifier, value);
    return this;
  }
  selectedOptionFontColor(value: ResourceColor): this {
    modifierWithKey(
      this._modifiersWithKeys, SelectedOptionFontColorModifier.identity, SelectedOptionFontColorModifier, value);
    return this;
  }
  optionBgColor(value: ResourceColor): this {
    modifierWithKey(
      this._modifiersWithKeys, OptionBgColorModifier.identity, OptionBgColorModifier, value);
    return this;
  }
  optionFont(value: Font): this {
    modifierWithKey(
      this._modifiersWithKeys, OptionFontModifier.identity, OptionFontModifier, value);
    return this;
  }
  optionFontColor(value: ResourceColor): this {
    modifierWithKey(
      this._modifiersWithKeys, OptionFontColorModifier.identity, OptionFontColorModifier, value);
    return this;
  }
  onSelect(callback: (index: number, value: string) => void): this {
    throw new Error('Method not implemented.');
  }
  space(value: Length): this {
    modifierWithKey(
      this._modifiersWithKeys, SpaceModifier.identity, SpaceModifier, value);
    return this;
  }
  arrowPosition(value: ArrowPosition): this {
    modifierWithKey(
      this._modifiersWithKeys, ArrowPositionModifier.identity, ArrowPositionModifier, value);
    return this;
  }
  menuAlign(alignType: MenuAlignType, offset?: Offset): this {
    let menuAlign = new ArkMenuAlignType(alignType, offset);
    modifierWithKey(
      this._modifiersWithKeys, MenuAlignModifier.identity, MenuAlignModifier, menuAlign);
    return this;
  }
}

class FontModifier extends ModifierWithKey<Font> {
  constructor(value: Font) {
    super(value);
  }
  static identity: Symbol = Symbol('selectFont');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().select.resetFont(node);
    } else {
      getUINativeModule().select.setFont(node, this.value.size, this.value.weight, this.value.family, this.value.style);
    }
  }

  checkObjectDiff(): boolean {
    let sizeEQ = isBaseOrResourceEqual(this.stageValue.size, this.value.size);
    let weightEQ = this.stageValue.weight === this.value.weight;
    let familyEQ = isBaseOrResourceEqual(this.stageValue.family, this.value.family);
    let styleEQ = this.stageValue.style === this.value.style;
    return !sizeEQ || !weightEQ || !familyEQ || !styleEQ;
  }
}

class OptionFontModifier extends ModifierWithKey<Font> {
  constructor(value: Font) {
    super(value);
  }
  static identity: Symbol = Symbol('selectOptionFont');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().select.resetOptionFont(node);
    } else {
      getUINativeModule().select.setOptionFont(node, this.value.size, this.value.weight, this.value.family, this.value.style);
    }
  }

  checkObjectDiff(): boolean {
    let sizeEQ = isBaseOrResourceEqual(this.stageValue.size, this.value.size);
    let weightEQ = this.stageValue.weight === this.value.weight;
    let familyEQ = isBaseOrResourceEqual(this.stageValue.family, this.value.family);
    let styleEQ = this.stageValue.style === this.value.style;
    return !sizeEQ || !weightEQ || !familyEQ || !styleEQ;
  }
}

class SelectedOptionFontModifier extends ModifierWithKey<Font> {
  constructor(value: Font) {
    super(value);
  }
  static identity: Symbol = Symbol('selectSelectedOptionFont');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().select.resetSelectedOptionFont(node);
    } else {
      getUINativeModule().select.setSelectedOptionFont(node, this.value.size, this.value.weight, this.value.family, this.value.style);
    }
  }

  checkObjectDiff(): boolean {
    let sizeEQ = isBaseOrResourceEqual(this.stageValue.size, this.value.size);
    let weightEQ = this.stageValue.weight === this.value.weight;
    let familyEQ = isBaseOrResourceEqual(this.stageValue.family, this.value.family);
    let styleEQ = this.stageValue.style === this.value.style;
    return !sizeEQ || !weightEQ || !familyEQ || !styleEQ;
  }
}

class MenuAlignModifier extends ModifierWithKey<ArkMenuAlignType> {
  constructor(value: ArkMenuAlignType) {
    super(value);
  }
  static identity: Symbol = Symbol('selectMenuAlign');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().select.resetMenuAlign(node);
    } else {
      getUINativeModule().select.setMenuAlign(node, this.value.alignType, this.value.dx, this.value.dy);
    }
  }

  checkObjectDiff(): boolean {
    let alignTypeEQ = this.stageValue.alignType === this.value.alignType;
    let dxEQ = isBaseOrResourceEqual(this.stageValue, this.value);
    let dyEQ = isBaseOrResourceEqual(this.stageValue, this.value);

    return !alignTypeEQ || !dxEQ || !dyEQ;
  }

  private isEqual(stageValue: Length, value: Length) {
    if ((!isUndefined(stageValue) && isResource(stageValue)) &&
      (!isUndefined(value) && isResource(value))) {
      return !isResourceEqual(stageValue, value);
    } else {
      return stageValue !== value;
    }
  }
}

class ArrowPositionModifier extends ModifierWithKey<ArrowPosition> {
  constructor(value: ArrowPosition) {
    super(value);
  }
  static identity: Symbol = Symbol('selectArrowPosition');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().select.resetArrowPosition(node);
    } else {
      getUINativeModule().select.setArrowPosition(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return this.stageValue !== this.value;
  }
}
class SpaceModifier extends ModifierWithKey<Length> {
  constructor(value: Length) {
    super(value);
  }
  static identity: Symbol = Symbol('selectSpace');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().select.resetSpace(node);
    } else {
      getUINativeModule().select.setSpace(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}
class ValueModifier extends ModifierWithKey<ResourceStr> {
  constructor(value: ResourceStr) {
    super(value);
  }
  static identity: Symbol = Symbol('selectValue');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().select.resetValue(node);
    } else {
      getUINativeModule().select.setValue(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}
class SelectedModifier extends ModifierWithKey<number | Resource> {
  constructor(value: number | Resource) {
    super(value);
  }
  static identity: Symbol = Symbol('selectSelected');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().select.resetSelected(node);
    } else {
      getUINativeModule().select.setSelected(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}
class SelectFontColorModifier extends ModifierWithKey<ResourceColor> {
  constructor(value: ResourceColor) {
    super(value);
  }
  static identity: Symbol = Symbol('selectFontColor');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().select.resetFontColor(node);
    } else {
      getUINativeModule().select.setFontColor(node, this.value);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}
class SelectedOptionBgColorModifier extends ModifierWithKey<ResourceColor> {
  constructor(value: ResourceColor) {
    super(value);
  }
  static identity: Symbol = Symbol('selectSelectedOptionBgColor');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().select.resetSelectedOptionBgColor(node);
    } else {
      getUINativeModule().select.setSelectedOptionBgColor(node, this.value);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}
class OptionBgColorModifier extends ModifierWithKey<ResourceColor> {
  constructor(value: ResourceColor) {
    super(value);
  }
  static identity: Symbol = Symbol('selectOptionBgColor');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().select.resetOptionBgColor(node);
    } else {
      getUINativeModule().select.setOptionBgColor(node, this.value);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}
class OptionFontColorModifier extends ModifierWithKey<ResourceColor> {
  constructor(value: ResourceColor) {
    super(value);
  }
  static identity: Symbol = Symbol('selectOptionFontColor');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().select.resetOptionFontColor(node);
    } else {
      getUINativeModule().select.setOptionFontColor(node, this.value);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}
class SelectedOptionFontColorModifier extends ModifierWithKey<ResourceColor> {
  constructor(value: ResourceColor) {
    super(value);
  }
  static identity: Symbol = Symbol('selectSelectedOptionFontColor');

  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().select.resetSelectedOptionFontColor(node);
    } else {
      getUINativeModule().select.setSelectedOptionFontColor(node, this.value);
    }
  }
  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

// @ts-ignore
globalThis.Select.attributeModifier = function (modifier) {
  const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
  let nativeNode = getUINativeModule().getFrameNodeById(elmtId);
  let component = this.createOrGetNode(elmtId, () => {
    return new ArkSelectComponent(nativeNode);
  });
  applyUIAttributes(modifier, nativeNode, component);
  component.applyModifierPatch();
};
