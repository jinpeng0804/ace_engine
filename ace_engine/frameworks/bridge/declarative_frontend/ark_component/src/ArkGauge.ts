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
class ArkGaugeComponent extends ArkComponent implements GaugeAttribute {
  constructor(nativePtr: KNode) {
    super(nativePtr);
  }
  value(value: number): this {
    modifierWithKey(this._modifiersWithKeys, GaugeVauleModifier.identity, GaugeVauleModifier, value);
    return this;
  }
  startAngle(angle: number): this {
    modifierWithKey(this._modifiersWithKeys, GaugeStartAngleModifier.identity, GaugeStartAngleModifier, angle);
    return this;
  }
  endAngle(angle: number): this {
    modifierWithKey(this._modifiersWithKeys, GaugeEndAngleModifier.identity, GaugeEndAngleModifier, angle);
    return this;
  }
  colors(colors: ResourceColor | LinearGradient | Array<[ResourceColor | LinearGradient, number]>): this {
    modifierWithKey(this._modifiersWithKeys, GaugeColorsModifier.identity, GaugeColorsModifier, colors);
    return this;
  }
  strokeWidth(length: any): this {
    modifierWithKey(this._modifiersWithKeys, GaugeStrokeWidthModifier.identity, GaugeStrokeWidthModifier, length);
    return this;
  }
  description(value: CustomBuilder): this {
    throw new Error('Method not implemented.');
  }
  trackShadow(value: GaugeShadowOptions): this {
    modifierWithKey(this._modifiersWithKeys, GaugeTrackShadowModifier.identity, GaugeTrackShadowModifier, value);
    return this;
  }
  indicator(value: GaugeIndicatorOptions): this {
    modifierWithKey(this._modifiersWithKeys, GaugeIndicatorModifier.identity, GaugeIndicatorModifier, value);
    return this;
  }
  monopolizeEvents(monopolize: boolean): this {
    throw new Error('Method not implemented.');
  }
}

class GaugeIndicatorModifier extends ModifierWithKey<GaugeIndicatorOptions> {
  static identity: Symbol = Symbol('gaugeIndicator');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().gauge.resetGaugeIndicator(node, this.value);
    } else {
      getUINativeModule().gauge.setGaugeIndicator(node, this.value.icon, this.value.space);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue.icon, this.value.icon) ||
      !isBaseOrResourceEqual(this.stageValue.space, this.value.space);
  }
}

class GaugeColorsModifier extends ModifierWithKey<ResourceColor | LinearGradient | Array<[ResourceColor | LinearGradient, number]>> {
  static identity = Symbol('gaugeColors');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().gauge.resetGaugeColors(node);
    } else {
      getUINativeModule().gauge.setGaugeColors(node, this.value!);
    }
  }
  checkObjectDiff(): boolean {
    return true;
  }
}

class GaugeVauleModifier extends ModifierWithKey<number> {
  static identity: Symbol = Symbol('gaugeVaule');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().gauge.resetGaugeVaule(node);
    } else {
      getUINativeModule().gauge.setGaugeVaule(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class GaugeStartAngleModifier extends ModifierWithKey<number> {
  static identity: Symbol = Symbol('gaugeStartAngle');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().gauge.resetGaugeStartAngle(node);
    } else {
      getUINativeModule().gauge.setGaugeStartAngle(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class GaugeEndAngleModifier extends ModifierWithKey<number> {
  static identity: Symbol = Symbol('gaugeEndAngle');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().gauge.resetGaugeEndAngle(node);
    } else {
      getUINativeModule().gauge.setGaugeEndAngle(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class GaugeStrokeWidthModifier extends ModifierWithKey<Length> {
  static identity: Symbol = Symbol('gaugeStrokeWidth');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().gauge.resetGaugeStrokeWidth(node);
    } else {
      getUINativeModule().gauge.setGaugeStrokeWidth(node, this.value);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

class GaugeTrackShadowModifier extends ModifierWithKey<GaugeShadowOptions> {
  static identity: Symbol = Symbol('gaugeTrackShadow');
  applyPeer(node: KNode, reset: boolean): void {
    if (reset) {
      getUINativeModule().gauge.resetGaugeTrackShadow(node);
    } else {
      getUINativeModule().gauge.setGaugeTrackShadow(node, this.value, this.value.radius, this.value.offsetX, this.value.offsetY);
    }
  }

  checkObjectDiff(): boolean {
    return !isBaseOrResourceEqual(this.stageValue, this.value);
  }
}

// @ts-ignore
globalThis.Gauge.attributeModifier = function (modifier) {
  const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
  let nativeNode = getUINativeModule().getFrameNodeById(elmtId);
  let component = this.createOrGetNode(elmtId, () => {
    return new ArkGaugeComponent(nativeNode);
  });
  applyUIAttributes(modifier, nativeNode, component);
  component.applyModifierPatch();
};
