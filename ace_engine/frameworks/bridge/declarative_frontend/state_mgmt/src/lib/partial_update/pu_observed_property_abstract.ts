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

/**
 * ObservedPropertyAbstractPU aka ObservedPropertyAbstract for partial update
 * 
 * all definitions in this file are framework internal
 */

abstract class ObservedPropertyAbstractPU<T> extends ObservedPropertyAbstract<T> 
implements ISinglePropertyChangeSubscriber<T>, IMultiPropertiesChangeSubscriber, IMultiPropertiesReadSubscriber
// these interfaces implementations are all empty functions, overwrite FU base class implementations.
{
  static readonly DelayedNotifyChangesEnum=class  {
    static readonly do_not_delay = 0;
    static readonly delay_none_pending = 1;
    static readonly delay_notification_pending = 2;
  };
  
  private owningView_ : ViewPU = undefined;
  
  // PU code stores object references to dependencies directly as class variable
  // SubscriberManager is not used for lookup in PU code path to speedup updates
  protected subscriberRefs_: Set<IPropertySubscriber>;
  
  // when owning ViewPU is inActive, delay notifying changes
  private delayedNotification_: number = ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.do_not_delay;

  // install when current value is ObservedObject and the value type is not using compatibility mode
  // note value may change for union type variables when switching an object from one class to another.
  protected shouldInstallTrackedObjectReadCb : boolean = true;
  private dependentElmtIdsByProperty_ = new class PropertyDependencies {

    // dependencies for property -> elmtId
    // variable read during render adds elmtId
    // variable assignment causes elmtId to need re-render.
    // UINode with elmtId deletion needs elmtId to be removed from all records, see purgeDependenciesForElmtId
    private propertyDependencies_: Set<number> = new Set<number>();

    public getAllPropertyDependencies(): Set<number> {
      stateMgmtConsole.debug(`  ... variable value assignment: returning affected elmtIds ${JSON.stringify(Array.from(this.propertyDependencies_))}`);
      return this.propertyDependencies_;
    }

    public addPropertyDependency(elmtId: number): void {
      this.propertyDependencies_.add(elmtId);
      stateMgmtConsole.debug(`   ... variable value read: add dependent elmtId ${elmtId} - updated list of dependent elmtIds: ${JSON.stringify(Array.from(this.propertyDependencies_))}`);
    }

    public purgeDependenciesForElmtId(rmElmtId: number): void {
      stateMgmtConsole.debug(`   ...purge all dependencies for elmtId ${rmElmtId} `);
      this.propertyDependencies_.delete(rmElmtId);
      stateMgmtConsole.debug(`      ... updated list of elmtIds dependent on variable assignment: ${JSON.stringify(Array.from(this.propertyDependencies_))}`);
      this.trackedObjectPropertyDependencies_.forEach((propertyElmtId, propertyName) => {
        propertyElmtId.delete(rmElmtId);
        stateMgmtConsole.debug(`      ... updated dependencies on objectProperty '${propertyName}' changes: ${JSON.stringify(Array.from(propertyElmtId))}`);
      });
    }

    // dependencies on individual object properties
    private trackedObjectPropertyDependencies_: Map<string, Set<number>> = new Map<string, Set<number>>();

    public addTrackedObjectPropertyDependency(readProperty: string, elmtId: number): void {
      let dependentElmtIds = this.trackedObjectPropertyDependencies_.get(readProperty);
      if (!dependentElmtIds) {
        dependentElmtIds = new Set<number>();
        this.trackedObjectPropertyDependencies_.set(readProperty, dependentElmtIds);
      }
      dependentElmtIds.add(elmtId);
      stateMgmtConsole.debug(`   ... object property '${readProperty}' read: add dependent elmtId ${elmtId} - updated list of dependent elmtIds: ${JSON.stringify(Array.from(dependentElmtIds))}`);
    }

    public getTrackedObjectPropertyDependencies(changedObjectProperty: string, debugInfo: string): Set<number> {
      const dependentElmtIds = this.trackedObjectPropertyDependencies_.get(changedObjectProperty) || new Set<number>();
      stateMgmtConsole.debug(`  ... property '@Track ${changedObjectProperty}': returning affected elmtIds ${JSON.stringify(Array.from(dependentElmtIds))}`);
      return dependentElmtIds;
    }

    public dumpInfoDependencies(): string {
      let result = `dependencies: variable assignment (or object prop change in compat mode) affects elmtIds: ${JSON.stringify(Array.from(this.propertyDependencies_))} \n`;
      this.trackedObjectPropertyDependencies_.forEach((propertyElmtId, propertyName) => {
        result += `  property '@Track ${propertyName}' change affects elmtIds: ${JSON.stringify(Array.from(propertyElmtId))} \n`;
      });
      return result;
    }

  }; // inner class PropertyDependencies


  constructor(subscriber: IPropertySubscriber, viewName: PropertyInfo) {
    super(subscriber, viewName);
    Object.defineProperty(this, 'owningView_', {writable: true, enumerable: false});
    Object.defineProperty(this, 'subscriberRefs_',
      {writable: true, enumerable: false, value: new Set<IPropertySubscriber>()});
    if(subscriber) {
      if (subscriber instanceof ViewPU) {
        this.owningView_ = subscriber;
      } else {
        this.subscriberRefs_.add(subscriber);
      }
    }
  }

  aboutToBeDeleted() {
    super.aboutToBeDeleted();
    this.subscriberRefs_.clear();
    this.owningView_ = undefined;
  }

  // dump info about variable decorator to string
  // e.g. @State/Provide, @Link/Consume, etc.
  public abstract debugInfoDecorator() : string;

  // dump basic info about this variable to a string, non-recursive, no subscriber info
  public debugInfo() : string {
    const propSource : string | false = this.isPropSourceObservedPropertyFakeName();
    return (propSource)
    ? `internal source (ObservedPropertyPU) of @Prop ${propSource} [${this.id__()}]`
    : `${this.debugInfoDecorator()} '${this.info()}'[${this.id__()}] <${this.debugInfoOwningView()}>`;
  }

  public debugInfoOwningView() : string {
    return `${this.owningView_ ? this.owningView_.debugInfo() : "owning @Component UNKNOWN"}`;
  }

  // dump info about owning view and subscribers (PU ones only)
  // use function only for debug output and DFX.
  public debugInfoSubscribers(): string {
    return (this.owningView_)
      ? `|--Owned by ${this.debugInfoOwningView()} `
      : `|--Owned by: owning view not known`;
  }

  public debugInfoSyncPeers(): string {
    if (!this.subscriberRefs_.size) {
      return "|--Sync peers: none";
    }
    let result: string = `|--Sync peers: {`;
    let sepa: string = "";
    this.subscriberRefs_.forEach((subscriber: IPropertySubscriber) => {
      if ("debugInfo" in subscriber) {
        result += `\n    ${sepa}${(subscriber as ObservedPropertyAbstractPU<any>).debugInfo()}`;
        sepa = ", ";
      }
    });
    result += "\n  }"
    return result;
  }

  public debugInfoDependentElmtIds(): string {
    return this.dependentElmtIdsByProperty_.dumpInfoDependencies();
  }

  /* for @Prop value from source we need to generate a @State
     that observes when this value changes. This ObservedPropertyPU
     sits inside SynchedPropertyOneWayPU.
     below methods invent a fake variable name for it
  */
  protected getPropSourceObservedPropertyFakeName(): string {
    return `${this.info()}_prop_fake_state_source___`;
  }

  protected isPropSourceObservedPropertyFakeName(): string | false {
    return this.info().endsWith("_prop_fake_state_source___")
      ? this.info().substring(0, this.info().length - "_prop_fake_state_source___".length)
      : false;
  }

  /*
    Virtualized version of the subscription mechanism - add subscriber
    Overrides implementation in ObservedPropertyAbstract<T>
  */
  public addSubscriber(subscriber: ISinglePropertyChangeSubscriber<T>):void {
    if (subscriber) {
      // ObservedPropertyAbstract will also add subscriber to
      // SubscriberManager map and to its own Set of subscribers as well
      // Something to improve in the future for PU path.
      // subscribeMe should accept IPropertySubscriber interface
      super.subscribeMe(subscriber as ISinglePropertyChangeSubscriber<T>);
      this.subscriberRefs_.add(subscriber);
    }
  }

  /*
    Virtualized version of the subscription mechanism - remove subscriber
    Overrides implementation in ObservedPropertyAbstract<T>
  */
  public removeSubscriber(subscriber: IPropertySubscriber, id?: number):void {
    if (subscriber) {
      this.subscriberRefs_.delete(subscriber);
      if (!id) {
        id = subscriber.id__();
      }
    }
    super.unlinkSuscriber(id);
  }

  /**
   * put the property to delayed notification mode
   * feature is only used for @StorageLink/Prop, @LocalStorageLink/Prop
   */
  public enableDelayedNotification() : void {
  if (this.delayedNotification_ != ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.delay_notification_pending) {
      stateMgmtConsole.debug(`${this.constructor.name}: enableDelayedNotification.`);
      this.delayedNotification_ = ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.delay_none_pending;
    }
  }

  /*
     when moving from inActive to active state the owning ViewPU calls this function
     This solution is faster than ViewPU polling each variable to send back a viewPropertyHasChanged event
     with the elmtIds

    returns undefined if variable has _not_ changed
    returns dependentElementIds_ Set if changed. This Set is empty if variable is not used to construct the UI
  */
  public moveElmtIdsForDelayedUpdate(): Set<number> | undefined {
    const result = (this.delayedNotification_ == ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.delay_notification_pending)
      ? this.dependentElmtIdsByProperty_.getAllPropertyDependencies()
      : undefined;
    stateMgmtConsole.debug(`${this.debugInfo()}: moveElmtIdsForDelayedUpdate: elmtIds that need delayed update \
                      ${result ? Array.from(result).toString() : 'no delayed notifications'} .`);
    this.delayedNotification_ = ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.do_not_delay;
    return result;
  }

  protected notifyPropertyRead() {
    stateMgmtConsole.error(`${this.debugInfo()}: notifyPropertyRead, DO NOT USE with PU. Use notifyReadCb mechanism.`);

  }

  // notify owning ViewPU and peers of a variable assignment
  // also property/item changes to  ObservedObjects of class object type, which use compat mode
  // Date and Array are notified as if there had been an assignment.
  protected notifyPropertyHasChangedPU() {
    stateMgmtProfiler.begin("ObservedPropertyAbstractPU.notifyPropertyHasChangedPU");
    stateMgmtConsole.debug(`${this.debugInfo()}: notifyPropertyHasChangedPU.`)
    if (this.owningView_) {
      if (this.delayedNotification_ == ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.do_not_delay) {
        // send viewPropertyHasChanged right away
        this.owningView_.viewPropertyHasChanged(this.info_, this.dependentElmtIdsByProperty_.getAllPropertyDependencies());
      } else {
        // mark this @StorageLink/Prop or @LocalStorageLink/Prop variable has having changed and notification of viewPropertyHasChanged delivery pending
        this.delayedNotification_ = ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.delay_notification_pending;
      }
    }
    this.subscriberRefs_.forEach((subscriber) => {
      if (subscriber) {
        if ('syncPeerHasChanged' in subscriber) {
          (subscriber as unknown as PeerChangeEventReceiverPU<T>).syncPeerHasChanged(this);
        } else  {
          stateMgmtConsole.warn(`${this.debugInfo()}: notifyPropertyHasChangedPU: unknown subscriber ID 'subscribedId' error!`);
        }
      }
    });
    stateMgmtProfiler.end();
  }  


  // notify owning ViewPU and peers of a ObservedObject @Track property's assignment
  protected notifyTrackedObjectPropertyHasChanged(changedPropertyName : string) : void {
    stateMgmtProfiler.begin("ObservedPropertyAbstract.notifyTrackedObjectPropertyHasChanged");
    stateMgmtConsole.debug(`${this.debugInfo()}: notifyTrackedObjectPropertyHasChanged.`)
    if (this.owningView_) {
      if (this.delayedNotification_ == ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.do_not_delay) {
        // send viewPropertyHasChanged right away
        this.owningView_.viewPropertyHasChanged(this.info_, this.dependentElmtIdsByProperty_.getTrackedObjectPropertyDependencies(changedPropertyName, "notifyTrackedObjectPropertyHasChanged"));
      } else {
        // mark this @StorageLink/Prop or @LocalStorageLink/Prop variable has having changed and notification of viewPropertyHasChanged delivery pending
        this.delayedNotification_ = ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.delay_notification_pending;
      }
    }
    this.subscriberRefs_.forEach((subscriber) => {
      if (subscriber) {
        if ('syncPeerTrackedPropertyHasChanged' in subscriber) {
          (subscriber as unknown as PeerChangeEventReceiverPU<T>).syncPeerTrackedPropertyHasChanged(this, changedPropertyName);
        } else  {
          stateMgmtConsole.warn(`${this.debugInfo()}: notifyTrackedObjectPropertyHasChanged: unknown subscriber ID 'subscribedId' error!`);
        }
      }
    });
    stateMgmtProfiler.end();
  }

  protected abstract onOptimisedObjectPropertyRead(readObservedObject : T, readPropertyName : string, isTracked : boolean) : void;

  public markDependentElementsDirty(view: ViewPU) {
    // TODO ace-ets2bundle, framework, complicated apps need to update together
    // this function will be removed after a short transition period.
    stateMgmtConsole.warn(`${this.debugInfo()}: markDependentElementsDirty no longer supported. App will work ok, but
        please update your ace-ets2bundle and recompile your application!`);
  }

  public numberOfSubscrbers(): number {
    return this.subscriberRefs_.size + (this.owningView_ ? 1 : 0);
  }

  /*
   type checking for any supported type, as required for union type support
    see 1st parameter for explanation what is allowed

    FIXME this expects the Map, Set patch to go in
   */

  protected checkIsSupportedValue(value: T): boolean {
    return this.checkNewValue(
      `undefined, null, number, boolean, string, or Object but not function`,
      value,
      () => ((typeof value == "object" && typeof value != "function")
        || typeof value == "number" || typeof value == "string" || typeof value == "boolean")
        || (value == undefined || value == null)
    );
  }

  /*
    type checking for allowed Object type value
    see 1st parameter for explanation what is allowed

      FIXME this expects the Map, Set patch to go in
   */
  protected checkIsObject(value: T): boolean {
    return this.checkNewValue(
      `undefined, null, Object including Array and instance of SubscribableAbstract and excluding function, Set, and Map`,
      value,
      () => (value == undefined || value == null || (typeof value == "object"))
    );
  }

  /*
    type checking for allowed simple types value
    see 1st parameter for explanation what is allowed
   */
  protected checkIsSimple(value: T): boolean {
    return this.checkNewValue(
      `undefined, number, boolean, string`,
      value,
      () => (value == undefined || typeof value == "number" || typeof value == "string" || typeof value == "boolean")
    );
  }

  protected checkNewValue(isAllowedComment : string, newValue: T, validator: (value: T) => boolean) : boolean {
    if (validator(newValue)) {
      return true;
    } 

    // report error
    // current implementation throws an Exception
    errorReport.varValueCheckFailed({
      customComponent: this.debugInfoOwningView(),
      variableDeco: this.debugInfoDecorator(),
      variableName: this.info(),
      expectedType: isAllowedComment,
      value: newValue
    });

    // never gets here if errorReport.varValueCheckFailed throws an exception
    // but should not depend on its implementation
    return false;
  }

  
  /**
   * factory function for concrete 'object' or 'simple' ObservedProperty object
   * depending if value is Class object
   * or simple type (boolean | number | string)
   * @param value
   * @param owningView
   * @param thisPropertyName
   * @returns either
   */
  static CreateObservedObject<C>(value: C, owningView: IPropertySubscriber, thisPropertyName: PropertyInfo)
    : ObservedPropertyAbstract<C> {
    return (typeof value === "object") ?
      new ObservedPropertyObject(value, owningView, thisPropertyName)
      : new ObservedPropertySimple(value, owningView, thisPropertyName);
  }


  /**
   * If owning viewPU is currently rendering or re-rendering a UINode, return its elmtId
   * return -1 otherwise
   * ViewPU caches the info, it does not request the info from C++ side (by calling 
   * ViewStackProcessor.GetElmtIdToAccountFor(); as done in earlier implementation
   */
  protected getRenderingElmtId() : number {
    return (this.owningView_) ? this.owningView_.getCurrentlyRenderedElmtId() : -1;
  }


  /**
   * during 'get' access recording take note of the created component and its elmtId
   * and add this component to the list of components who are dependent on this property
   */
  protected recordPropertyDependentUpdate() : void {
    const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
    if (elmtId < 0) {
      // not access recording 
      return;
    }
    stateMgmtConsole.debug(`${this.debugInfo()}: recordPropertyDependentUpdate: add (state) variable dependency for elmtId ${elmtId}.`)
    this.dependentElmtIdsByProperty_.addPropertyDependency(elmtId);
  }

  /** record dependency ObservedObject + propertyName -> elmtId 
   * caller ensures renderingElmtId >= 0
   */
  protected recordTrackObjectPropertyDependencyForElmtId(renderingElmtId : number, readTrackedPropertyName : string) : void {
    stateMgmtConsole.debug(`${this.debugInfo()}: recordTrackObjectPropertyDependency on elmtId ${renderingElmtId}.`)
    this.dependentElmtIdsByProperty_.addTrackedObjectPropertyDependency(readTrackedPropertyName, renderingElmtId);
  }
  
  public purgeDependencyOnElmtId(rmElmtId: number): void {
    stateMgmtConsole.debug(`${this.debugInfo()}: purgeDependencyOnElmtId ${rmElmtId}`);
    this.dependentElmtIdsByProperty_.purgeDependenciesForElmtId(rmElmtId);
  }

  public SetPropertyUnchanged(): void {
    // function to be removed
    // keep it here until transpiler is updated.
  }

  // unified Appstorage, what classes to use, and the API
  public createLink(subscribeOwner?: IPropertySubscriber,
    linkPropName?: PropertyInfo): ObservedPropertyAbstractPU<T> {
      throw new Error(`${this.debugInfo()}: createLink: Can not create a AppStorage 'Link' from this property.`);
  }

  public createProp(subscribeOwner?: IPropertySubscriber,
    linkPropName?: PropertyInfo): ObservedPropertyAbstractPU<T> {
      throw new Error(`${this.debugInfo()}: createProp: Can not create a AppStorage 'Prop' from a @State property. `);
  }

  /*
    Below empty functions required to keep as long as this class derives from FU version
    ObservedPropertyAbstract. Need to overwrite these functions to do nothing for PU
   */
    protected notifyHasChanged(_: T) {
      stateMgmtConsole.error(`${this.debugInfo()}: notifyHasChanged, DO NOT USE with PU. Use syncPeerHasChanged() \ 
                                            or onTrackedObjectProperty(CompatMode)HasChangedPU()`);
    }

    
 /**
  * event emitted by wrapped ObservedObject, when one of its property values changes
  * for class objects when in compatibility mode
  * for Array, Date instances always
  * @param souceObject 
  * @param changedPropertyName 
  */
  public onTrackedObjectPropertyHasChangedPU(sourceObject: ObservedObject<T>, changedPropertyName: string) {
    stateMgmtConsole.debug(`${this.debugInfo()}: onTrackedObjectPropertyHasChangedPU: property '${changedPropertyName}' of \
      object value has changed.`)

    this.notifyTrackedObjectPropertyHasChanged(changedPropertyName);
  }

 /**
  * event emitted by wrapped ObservedObject, when one of its property values changes
  * for class objects when in compatibility mode
  * for Array, Date instances always
  * @param souceObject 
  * @param changedPropertyName 
  */
  public onTrackedObjectPropertyCompatModeHasChangedPU(sourceObject: ObservedObject<T>, changedPropertyName: string) {
    stateMgmtConsole.debug(`${this.debugInfo()}: onTrackedObjectPropertyCompatModeHasChangedPU: property '${changedPropertyName}' of \
      object value has changed.`)

    this.notifyPropertyHasChangedPU();
  }


  hasChanged(_: T): void {
    // unused for PU
    // need to overwrite impl of base class with empty function.
  }
  
  propertyHasChanged(_?: PropertyInfo): void {
    // unused for PU
    // need to overwrite impl of base class with empty function.
  }
  
  propertyRead(_?: PropertyInfo): void {
    // unused for PU
    // need to overwrite impl of base class with empty function.
  }
}