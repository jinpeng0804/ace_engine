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

interface PeerChangeEventReceiverPU<T> {
    /**
     * emitted by sync peer when it changes
     * @Prop - SynchedPropertySimple/ObjectOneWay: ObservedPropertyAbstractPU in parent 
     * @Link - SynchedPropertySimple/ObjectTwoWay: ObservedPropertyAbstractPU in parent 
     * 
     * ObservedPropertyAbstractPU given as source has changed ('set')
     * the new value is available via ObservedPropertyAbstractPU.Get() 
     * or ObservedPropertyAbstractPU.GetUnmonitored()
     * the property name is available via ObservedPropertyAbstractPU.Info()
     * @see  ObservedPropertyAbstractPU.notifyPropertryHasChangedPU()
     * @param eventSource The 
     */
    syncPeerHasChanged(eventSource: ObservedPropertyAbstractPU<T>) : void;

    syncPeerTrackedPropertyHasChanged(eventSource: ObservedPropertyAbstractPU<T>, changedTrackedObjectPropertyName : string) : void;
}

interface PropertyReadEventListener<T> {
    /**
     * ObservedPropertyAbstractPU given as source has been read ('get')
     * @param eventSource 
     */
    propertyHasBeenReadPU(eventSource: ObservedPropertyAbstractPU<T>);
}


/**
 * Events generated by an ObservedObject
 */
interface ObservedObjectEventsPUReceiver<C extends Object> {
    // called from ObservedObject proxy when property has changes
    // for class objects when in compatibility mode
    // for Array, Date instances always
    onTrackedObjectPropertyCompatModeHasChangedPU?(eventSource: ObservedObject<C>, changedPropName: string);
    
    // called from ObservedObject proxy when @Track property has changes
    onTrackedObjectPropertyHasChangedPU?(eventSource: ObservedObject<C>, changedPropName: string);
}
