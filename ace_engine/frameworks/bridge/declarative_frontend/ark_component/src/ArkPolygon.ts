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
class ArkPolygonComponent extends ArkCommonShapeComponent implements PolygonAttribute {
  points(value: Array<any>): this {
    modifierWithKey(this._modifiersWithKeys, PolygonPointsModifier.identity, PolygonPointsModifier, value);
    return this;
  }
  monopolizeEvents(monopolize: boolean): this {
    throw new Error('Method not implemented.');
  }
}

class PolygonPointsModifier extends ModifierWithKey<Array<any>> {
  constructor(value: Array<any>) {
    super(value);
  }
  static identity: Symbol = Symbol('polygonPoints');
  applyPeer(node: KNode, reset: boolean): void {
    let xPoint = [];
    let yPoint = [];
    if (Array.isArray(this.value)) {
      for (let i = 0; i <= this.value.length; i++) {
        let item = this.value[i];
        if (!Array.isArray(item)) {
          continue;
        }

        if (item.length < ARRAY_LENGTH || isUndefined(item[0]) || isUndefined(item[1])) {
          reset = true;
          break;
        }
        
        xPoint.push(item[0]);
        yPoint.push(item[1]);
      }
    } else {
      reset = true;
    }

    if (reset) {
      GetUINativeModule().polygon.resetPolygonPoints(node);
    } else {
      GetUINativeModule().polygon.setPolygonPoints(node, xPoint, yPoint);
    }
  }

  checkObjectDiff(): boolean {
    return this.stageValue !== this.value;
  }
}

// @ts-ignore
globalThis.Polygon.attributeModifier = function (modifier) {
  const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
  let nativeNode = getUINativeModule().getFrameNodeById(elmtId);
  let component = this.createOrGetNode(elmtId, () => {
    return new ArkPolygonComponent(nativeNode);
  });
  applyUIAttributes(modifier, nativeNode, component);
  component.applyModifierPatch();
};
