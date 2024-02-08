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

#include "bridge/declarative_frontend/jsview/js_view_functions.h"

#include <memory>
#include <string>

#include "base/geometry/calc_dimension.h"
#include "base/geometry/dimension.h"
#include "base/log/ace_performance_check.h"
#include "base/log/ace_trace.h"
#include "base/utils/utils.h"
#include "bridge/declarative_frontend/engine/js_execution_scope_defines.h"
#include "bridge/declarative_frontend/engine/js_ref_ptr.h"
#include "bridge/declarative_frontend/jsview/js_view.h"
#include "bridge/declarative_frontend/jsview/js_view_measure_layout.h"
#include "bridge/declarative_frontend/view_stack_processor.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/layout/layout_property.h"
#include "core/components_ng/layout/layout_wrapper.h"
#include "core/pipeline/base/composed_element.h"

namespace OHOS::Ace::Framework {

#ifdef USE_ARK_ENGINE

namespace {
const std::string EMPTY_STATUS_DATA = "empty_status_data";
JSRef<JSObject> GenConstraint(const std::optional<NG::LayoutConstraintF>& parentConstraint)
{
    auto minSize = parentConstraint->minSize;
    auto maxSize = parentConstraint->maxSize;
    JSRef<JSObject> constraint = JSRef<JSObject>::New();
    constraint->SetProperty<double>("minWidth", minSize.Width());
    constraint->SetProperty<double>("minHeight", minSize.Height());
    constraint->SetProperty<double>("maxWidth", maxSize.Width());
    constraint->SetProperty<double>("maxHeight", maxSize.Height());
    return constraint;
}

JSRef<JSObject> GenConstraintNG(const std::optional<NG::LayoutConstraintF>& parentConstraint)
{
    auto minSize = parentConstraint->minSize;
    auto maxSize = parentConstraint->maxSize;
    JSRef<JSObject> constraint = JSRef<JSObject>::New();
    auto pipeline = PipelineBase::GetCurrentContext();
    constraint->SetProperty<double>("minWidth", minSize.Width() / pipeline->GetDipScale());
    constraint->SetProperty<double>("minHeight", minSize.Height() / pipeline->GetDipScale());
    constraint->SetProperty<double>("maxWidth", maxSize.Width() / pipeline->GetDipScale());
    constraint->SetProperty<double>("maxHeight", maxSize.Height() / pipeline->GetDipScale());
    return constraint;
}

JSRef<JSObject> GenPadding(const std::unique_ptr<NG::PaddingProperty>& paddingNative)
{
    JSRef<JSObject> padding = JSRef<JSObject>::New();
    padding->SetProperty("top", paddingNative->top->GetDimension().ConvertToVp());
    padding->SetProperty("right", paddingNative->right->GetDimension().ConvertToVp());
    padding->SetProperty("bottom", paddingNative->bottom->GetDimension().ConvertToVp());
    padding->SetProperty("left", paddingNative->left->GetDimension().ConvertToVp());
    return padding;
}

JSRef<JSObject> GenEdgeWidths(const std::unique_ptr<NG::BorderWidthProperty>& edgeWidthsNative)
{
    JSRef<JSObject> edgeWidths = JSRef<JSObject>::New();
    edgeWidths->SetProperty("top", edgeWidthsNative->topDimen->ConvertToVp());
    edgeWidths->SetProperty("right", edgeWidthsNative->rightDimen->ConvertToVp());
    edgeWidths->SetProperty("bottom", edgeWidthsNative->bottomDimen->ConvertToVp());
    edgeWidths->SetProperty("left", edgeWidthsNative->leftDimen->ConvertToVp());
    return edgeWidths;
}

JSRef<JSObject> GenBorderInfo(const RefPtr<NG::LayoutWrapper>& layoutWrapper)
{
    JSRef<JSObject> borderInfo = JSRef<JSObject>::New();
    auto layoutProperty = layoutWrapper->GetLayoutProperty();
    if (!layoutProperty) {
        return borderInfo;
    }

    const std::unique_ptr<NG::PaddingProperty> defaultPadding = std::make_unique<NG::PaddingProperty>();
    const std::unique_ptr<NG::BorderWidthProperty>& defaultEdgeWidth = std::make_unique<NG::BorderWidthProperty>();
    borderInfo->SetPropertyObject("borderWidth",
        GenEdgeWidths(
            layoutProperty->GetBorderWidthProperty() ? layoutProperty->GetBorderWidthProperty() : defaultEdgeWidth));

    borderInfo->SetPropertyObject("margin",
        GenPadding(layoutProperty->GetMarginProperty() ? layoutProperty->GetMarginProperty() : defaultPadding));
    borderInfo->SetPropertyObject("padding",
        GenPadding(layoutProperty->GetPaddingProperty() ? layoutProperty->GetPaddingProperty() : defaultPadding));

    return borderInfo;
}

JSRef<JSObject> GenPositionInfo(const RefPtr<NG::LayoutWrapper>& layoutWrapper)
{
    auto offset = layoutWrapper->GetGeometryNode()->GetFrameOffset();
    JSRef<JSObject> position = JSRef<JSObject>::New();
    position->SetProperty("x", offset.GetX());
    position->SetProperty("y", offset.GetY());
    return position;
}

JSRef<JSObject> GenSelfLayoutInfo(RefPtr<NG::LayoutProperty> layoutProperty)
{
    JSRef<JSObject> selfLayoutInfo = JSRef<JSObject>::New();
    if (!layoutProperty) {
        return selfLayoutInfo;
    }
    auto parentNode = AceType::DynamicCast<NG::FrameNode>(layoutProperty->GetHost()->GetParent());
    auto pipeline = PipelineBase::GetCurrentContext();
    if (parentNode->GetTag() == V2::COMMON_VIEW_ETS_TAG) {
        layoutProperty = parentNode->GetLayoutProperty();
    }
    auto width =
        layoutProperty->GetLayoutConstraint()
            ? layoutProperty->GetLayoutConstraint()->selfIdealSize.Width().value_or(0.0) / pipeline->GetDipScale()
            : 0.0f;
    auto height =
        layoutProperty->GetLayoutConstraint()
            ? layoutProperty->GetLayoutConstraint()->selfIdealSize.Height().value_or(0.0) / pipeline->GetDipScale()
            : 0.0f;

    const std::unique_ptr<NG::PaddingProperty> defaultPadding = std::make_unique<NG::PaddingProperty>();
    const std::unique_ptr<NG::BorderWidthProperty>& defaultEdgeWidth = std::make_unique<NG::BorderWidthProperty>();
    selfLayoutInfo->SetPropertyObject("borderWidth",
        GenEdgeWidths(
            layoutProperty->GetBorderWidthProperty() ? layoutProperty->GetBorderWidthProperty() : defaultEdgeWidth));
    selfLayoutInfo->SetPropertyObject("margin",
        GenPadding(layoutProperty->GetMarginProperty() ? layoutProperty->GetMarginProperty() : defaultPadding));
    selfLayoutInfo->SetPropertyObject("padding",
        GenPadding(layoutProperty->GetPaddingProperty() ? layoutProperty->GetPaddingProperty() : defaultPadding));
    selfLayoutInfo->SetProperty(
        "width", NearEqual(width, 0.0f)
                     ? layoutProperty->GetLayoutConstraint()->percentReference.Width() / pipeline->GetDipScale()
                     : width);
    selfLayoutInfo->SetProperty(
        "height", NearEqual(height, 0.0f)
                      ? layoutProperty->GetLayoutConstraint()->percentReference.Height() / pipeline->GetDipScale()
                      : height);
    return selfLayoutInfo;
}

void FillSubComponetProperty(JSRef<JSObject>& info, const RefPtr<NG::LayoutWrapper>& layoutWrapper, const size_t& index)
{
    info->SetProperty<std::string>("name", layoutWrapper->GetHostNode()->GetTag());
    info->SetProperty<std::string>("id", std::to_string(layoutWrapper->GetHostNode()->GetId()));
    info->SetPropertyObject("constraint", GenConstraint(layoutWrapper->GetLayoutProperty()->GetLayoutConstraint()));
    info->SetPropertyObject("borderInfo", GenBorderInfo(layoutWrapper));
    info->SetPropertyObject("position", GenPositionInfo(layoutWrapper));
}

void FillPlaceSizeProperty(JSRef<JSObject>& info, const RefPtr<NG::LayoutWrapper>& layoutWrapper)
{
    auto frameRect = layoutWrapper->GetGeometryNode()->GetFrameRect();
    JSRef<JSObject> size = JSRef<JSObject>::New();

    Dimension measureWidth(frameRect.GetSize().Width(), DimensionUnit::PX);
    Dimension measureHeight(frameRect.GetSize().Height(), DimensionUnit::PX);
    size->SetProperty("width", measureWidth.ConvertToVp());
    size->SetProperty("height", measureHeight.ConvertToVp());
    info->SetPropertyObject("measureResult", size);
}

JSRef<JSArray> GenLayoutChildArray(std::list<RefPtr<NG::LayoutWrapper>> children)
{
    JSRef<JSArray> childInfo = JSRef<JSArray>::New();
    JSRef<JSFunc> layoutFunc = JSRef<JSFunc>::New<FunctionCallback>(ViewMeasureLayout::JSLayout);
    size_t index = 0;

    for (const auto& iter : children) {
        JSRef<JSObject> info = JSRef<JSObject>::New();
        FillSubComponetProperty(info, iter, index);
        info->SetPropertyObject("layout", layoutFunc);
        childInfo->SetValueAt(index++, info);
    }

    return childInfo;
}

JSRef<JSArray> GenPlaceChildrenArray(std::list<RefPtr<NG::LayoutWrapper>> children)
{
    JSRef<JSArray> childInfo = JSRef<JSArray>::New();
    JSRef<JSFunc> layoutFunc = JSRef<JSFunc>::New<FunctionCallback>(ViewMeasureLayout::JSPlaceChildren);
    size_t index = 0;

    for (const auto& iter : children) {
        JSRef<JSObject> info = JSRef<JSObject>::New();
        FillPlaceSizeProperty(info, iter);
        info->SetPropertyObject("layout", layoutFunc);
        childInfo->SetValueAt(index++, info);
    }

    return childInfo;
}

JSRef<JSArray> GenMeasureChildArray(std::list<RefPtr<NG::LayoutWrapper>> children, bool isMeasureSize)
{
    JSRef<JSArray> childInfo = JSRef<JSArray>::New();
    JSRef<JSFunc> measureFunc = JSRef<JSFunc>::New<FunctionCallback>(ViewMeasureLayout::JSMeasure);
    size_t index = 0;

    for (const auto& iter : children) {
        JSRef<JSObject> info = JSRef<JSObject>::New();
        if (!isMeasureSize) {
            FillSubComponetProperty(info, iter, index);
        }
        info->SetPropertyObject("measure", measureFunc);
        childInfo->SetValueAt(index++, info);
    }

    return childInfo;
}

} // namespace

void ViewFunctions::ExecuteLayout(NG::LayoutWrapper* layoutWrapper)
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    ACE_SCOPED_TRACE("ViewFunctions::ExecuteLayout");
    auto children = layoutWrapper->GetAllChildrenWithBuild();
    auto parentConstraint = layoutWrapper->GetGeometryNode()->GetParentLayoutConstraint();
    auto constraint = GenConstraint(parentConstraint);
    auto childArray = GenLayoutChildArray(children);
    JSRef<JSVal> params[2] = { childArray, constraint };

    ViewMeasureLayout::SetLayoutChildren(layoutWrapper->GetAllChildrenWithBuild());
    ViewMeasureLayout::SetDefaultMeasureConstraint(parentConstraint.value());
    jsLayoutFunc_.Lock()->Call(jsObject_.Lock(), 2, params);
}

void ViewFunctions::ExecuteMeasure(NG::LayoutWrapper* layoutWrapper)
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    ACE_SCOPED_TRACE("ViewFunctions::ExecuteMeasure");
    auto children = layoutWrapper->GetAllChildrenWithBuild();
    auto parentConstraint = layoutWrapper->GetGeometryNode()->GetParentLayoutConstraint();
    auto constraint = GenConstraint(parentConstraint);
    auto childArray = GenMeasureChildArray(children, false);
    JSRef<JSVal> params[2];

    params[0] = childArray;
    params[1] = constraint;

    ViewMeasureLayout::SetMeasureChildren(children);
    ViewMeasureLayout::SetDefaultMeasureConstraint(parentConstraint.value());
    jsMeasureFunc_.Lock()->Call(jsObject_.Lock(), 2, params);
}

void ViewFunctions::ExecutePlaceChildren(NG::LayoutWrapper* layoutWrapper)
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    ACE_SCOPED_TRACE("ViewFunctions::ExecutePlaceChildren");
    auto children = layoutWrapper->GetAllChildrenWithBuild();
    auto parentNode = AceType::DynamicCast<NG::FrameNode>(layoutWrapper->GetHostNode()->GetParent());
    std::optional<NG::LayoutConstraintF> parentConstraint;
    if (parentNode->GetTag() == V2::COMMON_VIEW_ETS_TAG) {
        parentConstraint = parentNode->GetGeometryNode()->GetParentLayoutConstraint();
    } else {
        parentConstraint = layoutWrapper->GetGeometryNode()->GetParentLayoutConstraint();
    }

    auto selfLayoutInfo = GenSelfLayoutInfo(layoutWrapper->GetLayoutProperty());
    auto childArray = GenPlaceChildrenArray(children);
    auto constraint = GenConstraintNG(parentConstraint);

    JSRef<JSVal> params[3] = { selfLayoutInfo, childArray, constraint };

    ViewMeasureLayout::SetLayoutChildren(layoutWrapper->GetAllChildrenWithBuild());
    ViewMeasureLayout::SetDefaultMeasureConstraint(parentConstraint.value());
    jsPlaceChildrenFunc_.Lock()->Call(jsObject_.Lock(), 3, params);
}

void ViewFunctions::ExecuteMeasureSize(NG::LayoutWrapper* layoutWrapper)
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    ACE_SCOPED_TRACE("ViewFunctions::ExecutePlaceChildren");
    auto children = layoutWrapper->GetAllChildrenWithBuild();
    auto parentNode = AceType::DynamicCast<NG::FrameNode>(layoutWrapper->GetHostNode()->GetParent());
    std::optional<NG::LayoutConstraintF> parentConstraint;
    if (parentNode->GetTag() == V2::COMMON_VIEW_ETS_TAG) {
        parentConstraint = parentNode->GetGeometryNode()->GetParentLayoutConstraint();
    } else {
        parentConstraint = layoutWrapper->GetGeometryNode()->GetParentLayoutConstraint();
    }

    auto selfLayoutInfo = GenSelfLayoutInfo(layoutWrapper->GetLayoutProperty());
    auto childArray = GenMeasureChildArray(children, true);
    auto constraint = GenConstraintNG(parentConstraint);

    JSRef<JSVal> params[3];

    params[0] = selfLayoutInfo;
    params[1] = childArray;
    params[2] = constraint;

    ViewMeasureLayout::SetMeasureChildren(children);
    ViewMeasureLayout::SetDefaultMeasureConstraint(parentConstraint.value());
    JSRef<JSObject> result = jsMeasureSizeFunc_.Lock()->Call(jsObject_.Lock(), 3, params);

    CalcDimension measureWidth;
    CalcDimension measureHeight;
    if (!JSViewAbstract::ParseJsDimensionVp(result->GetProperty("width"), measureWidth)) {
        measureWidth = { -1.0f };
    }
    if (!JSViewAbstract::ParseJsDimensionVp(result->GetProperty("height"), measureHeight)) {
        measureWidth = { -1.0f };
    }
    NG::SizeF frameSize = { measureWidth.ConvertToPx(), measureHeight.ConvertToPx() };
    NG::CalcSize idealSize = { NG::CalcLength(measureWidth.ConvertToPx()),
        NG::CalcLength(measureHeight.ConvertToPx()) };
    if (parentNode->GetTag() == V2::COMMON_VIEW_ETS_TAG) {
        auto parentLayoutProperty = parentNode->GetLayoutProperty();
        parentLayoutProperty->UpdateUserDefinedIdealSize(idealSize);
        parentNode->GetGeometryNode()->SetFrameSize(frameSize);
        parentLayoutProperty->UpdateMarginSelfIdealSize(
            NG::SizeF { measureWidth.ConvertToPx(), measureHeight.ConvertToPx() });
    }
    layoutWrapper->GetLayoutProperty()->UpdateUserDefinedIdealSize(idealSize);
    layoutWrapper->GetGeometryNode()->SetFrameSize(frameSize);
}

void ViewFunctions::ExecuteReload(bool deep)
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    ACE_SCOPED_TRACE("ViewFunctions::ExecuteReload");
    auto func = jsReloadFunc_.Lock();
    if (!func.IsEmpty()) {
        JSRef<JSVal> params[1];
        params[0] = JSRef<JSVal>(JSVal(JsiValueConvertor::toJsiValue(deep)));
        func->Call(jsObject_.Lock(), 1, params);
    } else {
        LOGE("the reload func is null");
    }
}

void ViewFunctions::ExecuteForceNodeRerender(int32_t elemId)
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    ACE_SCOPED_TRACE("ViewFunctions::ExecuteForceNodeRerender");
    auto func = jsForceRerenderNodeFunc_.Lock();
    if (!func.IsEmpty()) {
        JSRef<JSVal> params[1];
        params[0] = JSRef<JSVal>(JSVal(JsiValueConvertor::toJsiValue(elemId)));
        func->Call(jsObject_.Lock(), 1, params);
    } else {
        LOGE("the force node rerender func is null");
    }
}

void ViewFunctions::ExecuteRecycle(const std::string& viewName)
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    ACE_SCOPED_TRACE("ViewFunctions::ExecuteRecycle");
    auto func = jsRecycleFunc_.Lock();
    if (!func->IsEmpty()) {
        auto recycleNodeName = JSRef<JSVal>::Make(ToJSValue(viewName));
        func->Call(jsObject_.Lock(), 1, &recycleNodeName);
    } else {
        LOGE("the recycle func is null");
    }
}

void ViewFunctions::ExecuteSetActive(bool active)
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    auto func = jsSetActive_.Lock();
    if (!func->IsEmpty()) {
        auto isActive = JSRef<JSVal>::Make(ToJSValue(active));
        func->Call(jsObject_.Lock(), 1, &isActive);
    } else {
        LOGE("the set active func is null");
    }
}

void ViewFunctions::ExecuteOnDumpInfo(const std::vector<std::string>& params)
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    auto func = jsOnDumpInfo_.Lock();
    if (!func->IsEmpty()) {
        JSRef<JSArray> arr = JSRef<JSArray>::New();
        for (size_t i = 0; i < params.size(); ++i) {
            arr->SetValueAt(i, JSRef<JSVal>::Make(ToJSValue(params.at(i))));
        }
        JSRef<JSVal> argv = arr;
        func->Call(jsObject_.Lock(), 1, &argv);
    } else {
        LOGE("the on dump info func is null");
    }
}

#else

void ViewFunctions::ExecuteLayout(NG::LayoutWrapper* layoutWrapper) {}

void ViewFunctions::ExecuteMeasure(NG::LayoutWrapper* layoutWrapper) {}

void ViewFunctions::ExecutePlaceChildren(NG::LayoutWrapper* layoutWrapper) {}

void ViewFunctions::ExecuteMeasureSize(NG::LayoutWrapper* layoutWrapper) {}

void ViewFunctions::ExecuteReload(bool deep) {}

void ViewFunctions::ExecuteForceNodeRerender(int32_t elemId) {}

#endif

void ViewFunctions::InitViewFunctions(
    const JSRef<JSObject>& jsObject, const JSRef<JSFunc>& jsRenderFunction, bool partialUpdate)
{
    jsObject_ = jsObject;

    if (partialUpdate) {
        if (jsObject->GetProperty("initialRender")->IsFunction()) {
            JSRef<JSVal> jsRenderFunc = jsObject->GetProperty("initialRenderView");
            if (jsRenderFunc->IsFunction()) {
                jsRenderFunc_ = JSRef<JSFunc>::Cast(jsRenderFunc);
            } else {
                LOGE("View lacks mandatory 'initialRenderView()' function, fatal internal error.");
            }
        } else {
            LOGE("View lacks mandatory 'initialRender()' function, fatal internal error.");
        }

        JSRef<JSVal> jsRerenderFunc = jsObject->GetProperty("rerender");
        if (jsRerenderFunc->IsFunction()) {
            jsRerenderFunc_ = JSRef<JSFunc>::Cast(jsRerenderFunc);
        } else {
            LOGE("View lacks mandatory 'rerender()' function, fatal internal error.");
        }

        JSRef<JSVal> jsReloadFunc = jsObject->GetProperty("forceCompleteRerender");
        if (jsReloadFunc->IsFunction()) {
            jsReloadFunc_ = JSRef<JSFunc>::Cast(jsReloadFunc);
        } else {
            LOGE("View lacks mandatory 'forceCompleteRerender()' function, fatal internal error.");
        }

        JSRef<JSVal> jsForceRerenderNodeFunc = jsObject->GetProperty("forceRerenderNode");
        if (jsForceRerenderNodeFunc->IsFunction()) {
            jsForceRerenderNodeFunc_ = JSRef<JSFunc>::Cast(jsForceRerenderNodeFunc);
        } else {
            LOGE("View lacks mandatory 'forceRerenderNode()' function, fatal internal error.");
        }

        JSRef<JSVal> jsRecycleFunc = jsObject->GetProperty("recycleSelf");
        if (jsRecycleFunc->IsFunction()) {
            jsRecycleFunc_ = JSRef<JSFunc>::Cast(jsRecycleFunc);
        }

        JSRef<JSVal> jsAboutToRecycleFunc = jsObject->GetProperty("aboutToRecycleInternal");
        if (jsAboutToRecycleFunc->IsFunction()) {
            jsAboutToRecycleFunc_ = JSRef<JSFunc>::Cast(jsAboutToRecycleFunc);
        }

        JSRef<JSVal> jsSetActive = jsObject->GetProperty("setActiveInternal");
        if (jsSetActive->IsFunction()) {
            jsSetActive_ = JSRef<JSFunc>::Cast(jsSetActive);
        }

        JSRef<JSVal> jsOnDumpInfo = jsObject->GetProperty("onDumpInfo");
        if (jsOnDumpInfo->IsFunction()) {
            jsOnDumpInfo_ = JSRef<JSFunc>::Cast(jsOnDumpInfo);
        }
    }

    JSRef<JSVal> jsAppearFunc = jsObject->GetProperty("aboutToAppear");
    if (jsAppearFunc->IsFunction()) {
        jsAppearFunc_ = JSRef<JSFunc>::Cast(jsAppearFunc);
    }

    JSRef<JSVal> jsDisappearFunc = jsObject->GetProperty("aboutToDisappear");
    if (jsDisappearFunc->IsFunction()) {
        jsDisappearFunc_ = JSRef<JSFunc>::Cast(jsDisappearFunc);
    }

    JSRef<JSVal> jsLayoutFunc = jsObject->GetProperty("onLayout");
    if (jsLayoutFunc->IsFunction()) {
        jsLayoutFunc_ = JSRef<JSFunc>::Cast(jsLayoutFunc);
    }

    JSRef<JSVal> jsMeasureFunc = jsObject->GetProperty("onMeasure");
    if (jsMeasureFunc->IsFunction()) {
        jsMeasureFunc_ = JSRef<JSFunc>::Cast(jsMeasureFunc);
    }

    JSRef<JSVal> jsPlaceChildrenFunc = jsObject->GetProperty("onPlaceChildren");
    if (jsPlaceChildrenFunc->IsFunction()) {
        jsPlaceChildrenFunc_ = JSRef<JSFunc>::Cast(jsPlaceChildrenFunc);
    }

    JSRef<JSVal> jsMeasureSizeFunc = jsObject->GetProperty("onMeasureSize");
    if (jsMeasureSizeFunc->IsFunction()) {
        jsMeasureSizeFunc_ = JSRef<JSFunc>::Cast(jsMeasureSizeFunc);
    }

    JSRef<JSVal> jsAboutToBeDeletedFunc = jsObject->GetProperty("aboutToBeDeleted");
    if (jsAboutToBeDeletedFunc->IsFunction()) {
        jsAboutToBeDeletedFunc_ = JSRef<JSFunc>::Cast(jsAboutToBeDeletedFunc);
    }

    JSRef<JSVal> jsAboutToRenderFunc = jsObject->GetProperty("aboutToRender");
    if (jsAboutToRenderFunc->IsFunction()) {
        jsAboutToRenderFunc_ = JSRef<JSFunc>::Cast(jsAboutToRenderFunc);
    }

    JSRef<JSVal> jsRenderDoneFunc = jsObject->GetProperty("onRenderDone");
    if (jsRenderDoneFunc->IsFunction()) {
        jsRenderDoneFunc_ = JSRef<JSFunc>::Cast(jsRenderDoneFunc);
    }

    JSRef<JSVal> jsAboutToBuildFunc = jsObject->GetProperty("aboutToBuild");
    if (jsAboutToBuildFunc->IsFunction()) {
        jsAboutToBuildFunc_ = JSRef<JSFunc>::Cast(jsAboutToBuildFunc);
    }

    JSRef<JSVal> jsBuildDoneFunc = jsObject->GetProperty("onBuildDone");
    if (jsBuildDoneFunc->IsFunction()) {
        jsBuildDoneFunc_ = JSRef<JSFunc>::Cast(jsBuildDoneFunc);
    }

    JSRef<JSVal> jsTransitionFunc = jsObject->GetProperty("pageTransition");
    if (jsTransitionFunc->IsFunction()) {
        jsTransitionFunc_ = JSRef<JSFunc>::Cast(jsTransitionFunc);
    }

    JSRef<JSVal> jsOnHideFunc = jsObject->GetProperty("onPageHide");
    if (jsOnHideFunc->IsFunction()) {
        jsOnHideFunc_ = JSRef<JSFunc>::Cast(jsOnHideFunc);
    }

    JSRef<JSVal> jsOnShowFunc = jsObject->GetProperty("onPageShow");
    if (jsOnShowFunc->IsFunction()) {
        jsOnShowFunc_ = JSRef<JSFunc>::Cast(jsOnShowFunc);
    }

    JSRef<JSVal> jsBackPressFunc = jsObject->GetProperty("onBackPress");
    if (jsBackPressFunc->IsFunction()) {
        jsBackPressFunc_ = JSRef<JSFunc>::Cast(jsBackPressFunc);
    }

    JSRef<JSVal> jsSetInitiallyProvidedValueFunc = jsObject->GetProperty("setInitiallyProvidedValue");
    if (jsSetInitiallyProvidedValueFunc->IsFunction()) {
        jsSetInitiallyProvidedValueFunc_ = JSRef<JSFunc>::Cast(jsSetInitiallyProvidedValueFunc);
    }

    if (!partialUpdate) {
        JSRef<JSVal> jsUpdateWithValueParamsFunc = jsObject->GetProperty("updateWithValueParams");
        if (jsUpdateWithValueParamsFunc->IsFunction()) {
            jsUpdateWithValueParamsFunc_ = JSRef<JSFunc>::Cast(jsUpdateWithValueParamsFunc);
        }
        jsRenderFunc_ = jsRenderFunction;
    }

    JSRef<JSVal> jsOnFormRecycleFunc = jsObject->GetProperty("onFormRecycle");
    if (jsOnFormRecycleFunc->IsFunction()) {
        jsOnFormRecycleFunc_ = JSRef<JSFunc>::Cast(jsOnFormRecycleFunc);
    }

    JSRef<JSVal> jsOnFormRecoverFunc = jsObject->GetProperty("onFormRecover");
    if (jsOnFormRecoverFunc->IsFunction()) {
        jsOnFormRecoverFunc_ = JSRef<JSFunc>::Cast(jsOnFormRecoverFunc);
    }
}

ViewFunctions::ViewFunctions(const JSRef<JSObject>& jsObject, const JSRef<JSFunc>& jsRenderFunction)
{
    ACE_DCHECK(jsObject);
    InitViewFunctions(jsObject, jsRenderFunction, false);
}

void ViewFunctions::ExecuteRender()
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    if (jsRenderFunc_.IsEmpty()) {
        LOGE("no render function in View!");
        return;
    }

    auto func = jsRenderFunc_.Lock();
    JSRef<JSVal> jsThis = jsObject_.Lock();
    if (!jsThis->IsUndefined()) {
        jsRenderResult_ = func->Call(jsThis);
    } else {
        LOGE("jsView Object is undefined and will not execute render function");
    }
}

void ViewFunctions::ExecuteAppear()
{
    ExecuteFunction(jsAppearFunc_, "aboutToAppear");
}

void ViewFunctions::ExecuteDisappear()
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    if (jsDisappearFunc_.IsEmpty()) {
        return;
    }
    ACE_SCOPED_TRACE("%s", "aboutToDisappear");
    JSRef<JSVal> jsObject = jsObject_.Lock();
    std::string functionName("aboutToDisappear");
    AceScopedPerformanceCheck scoped(functionName);
    if (!jsObject->IsUndefined()) {
        jsDisappearFunc_.Lock()->Call(jsObject);
    } else {
        LOGE("jsView Object is undefined and will not execute aboutToDisappear function");
    }
}

void ViewFunctions::ExecuteAboutToRecycle()
{
    ExecuteFunction(jsAboutToRecycleFunc_, "aboutToRecycleInternal");
}

bool ViewFunctions::HasLayout() const
{
    return !jsLayoutFunc_.IsEmpty();
}

bool ViewFunctions::HasMeasure() const
{
    return !jsMeasureFunc_.IsEmpty();
}

bool ViewFunctions::HasPlaceChildren() const
{
    return !jsPlaceChildrenFunc_.IsEmpty();
}

bool ViewFunctions::HasMeasureSize() const
{
    return !jsMeasureSizeFunc_.IsEmpty();
}

void ViewFunctions::ExecuteAboutToBeDeleted()
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    if (jsAboutToBeDeletedFunc_.IsEmpty()) {
        return;
    }
    ACE_SCOPED_TRACE("%s", "aboutToBeDeleted");
    JSRef<JSVal> jsObject = jsObject_.Lock();
    std::string functionName("aboutToBeDeleted");
    AceScopedPerformanceCheck scoped(functionName);
    if (!jsObject->IsUndefined()) {
        jsAboutToBeDeletedFunc_.Lock()->Call(jsObject);
    } else {
        LOGE("jsView Object is undefined and will not execute aboutToBeDeleted function");
    }
}

void ViewFunctions::ExecuteAboutToRender()
{
    // for developer callback.
    ExecuteFunction(jsAboutToBuildFunc_, "aboutToBuild");
    // for state manager mark rendering progress.
    ExecuteFunction(jsAboutToRenderFunc_, "aboutToRender");
}

void ViewFunctions::ExecuteOnRenderDone()
{
    // for state manager reset rendering progress.
    ExecuteFunction(jsRenderDoneFunc_, "onRenderDone");
    // for developer callback.
    ExecuteFunction(jsBuildDoneFunc_, "onBuildDone");
}

void ViewFunctions::ExecuteTransition()
{
    ExecuteFunction(jsTransitionFunc_, "pageTransition");
}

bool ViewFunctions::HasPageTransition() const
{
    return !jsTransitionFunc_.IsEmpty();
}

void ViewFunctions::ExecuteShow()
{
    ExecuteFunction(jsOnShowFunc_, "onPageShow");
}

void ViewFunctions::ExecuteHide()
{
    ExecuteFunction(jsOnHideFunc_, "onPageHide");
}

void ViewFunctions::ExecuteInitiallyProvidedValue(const std::string& jsonData)
{
    ExecuteFunctionWithParams(jsSetInitiallyProvidedValueFunc_, "setInitiallyProvidedValue", jsonData);
}

// Method not needed for Partial Update code path
void ViewFunctions::ExecuteUpdateWithValueParams(const std::string& jsonData)
{
    ExecuteFunctionWithParams(jsUpdateWithValueParamsFunc_, "updateWithValueParams", jsonData);
}

bool ViewFunctions::ExecuteOnBackPress()
{
    auto ret = ExecuteFunctionWithReturn(jsBackPressFunc_, "onBackPress");
    if (!ret->IsEmpty() && ret->IsBoolean()) {
        return ret->ToBoolean();
    }
    return false;
}

void ViewFunctions::ExecuteFunction(JSWeak<JSFunc>& func, const char* debugInfo)
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    if (func.IsEmpty()) {
        return;
    }
    ACE_SCOPED_TRACE("%s", debugInfo);
    JSRef<JSVal> jsObject = jsObject_.Lock();
    if (!jsObject->IsUndefined()) {
        std::string functionName(debugInfo);
        AceScopedPerformanceCheck scoped(functionName);
        func.Lock()->Call(jsObject);
    } else {
        LOGE("jsObject is undefined. Internal error while trying to exec %{public}s", debugInfo);
    }
}

JSRef<JSVal> ViewFunctions::ExecuteFunctionWithReturn(JSWeak<JSFunc>& func, const char* debugInfo)
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_, JSRef<JSVal>::Make())
    if (func.IsEmpty()) {
        return JSRef<JSVal>::Make();
    }
    ACE_SCOPED_TRACE("%s", debugInfo);
    JSRef<JSVal> jsObject = jsObject_.Lock();
    std::string functionName(debugInfo);
    AceScopedPerformanceCheck scoped(functionName);
    JSRef<JSVal> result = func.Lock()->Call(jsObject);
    if (result.IsEmpty()) {
        LOGE("Error calling %{public}s", debugInfo);
    }
    return result;
}

void ViewFunctions::ExecuteFunctionWithParams(JSWeak<JSFunc>& func, const char* debugInfo, const std::string& jsonData)
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    if (func.IsEmpty()) {
        return;
    }

    JSRef<JSObject> obj = JSRef<JSObject>::New();
    JSRef<JSVal> param = obj->ToJsonObject(jsonData.c_str());

    JSRef<JSVal> jsObject = jsObject_.Lock();
    JSRef<JSVal> result = func.Lock()->Call(jsObject, 1, &param);
    if (result.IsEmpty()) {
        LOGE("Error calling %{public}s", debugInfo);
    }
}

// Baseline version of Destroy
void ViewFunctions::Destroy(JSView* parentCustomView)
{
    // Might be called from parent view, before any result has been produced??
    if (jsRenderResult_.IsEmpty()) {
        return;
    }

    auto renderRes = jsRenderResult_.Lock();
    if (renderRes.IsEmpty() || !renderRes->IsObject()) {
        return;
    }

    JSRef<JSObject> obj = JSRef<JSObject>::Cast(renderRes);
    if (!obj.IsEmpty()) {
        // jsRenderResult_ maybe an js exception, not a JSView
        JSView* view = obj->Unwrap<JSView>();
        if (view != nullptr) {
            view->Destroy(parentCustomView);
        }
    }
    jsRenderResult_.Reset();
}

// PartialUpdate version of Destroy
void ViewFunctions::Destroy()
{
    // Might be called from parent view, before any result has been produced??
    if (jsRenderResult_.IsEmpty()) {
        return;
    }

    auto renderRes = jsRenderResult_.Lock();
    if (renderRes.IsEmpty() || !renderRes->IsObject()) {
        return;
    }

    // merging: when would a render function return a JSView ?
    JSRef<JSObject> obj = JSRef<JSObject>::Cast(renderRes);
    if (!obj.IsEmpty()) {
        // jsRenderResult_ maybe an js exception, not a JSView
        JSView* view = obj->Unwrap<JSView>();
        if (view != nullptr) {
            LOGE("NOTE NOTE NOTE render returned a JSView object that's dangling!");
        }
    }
    jsObject_.Reset();
    jsRenderResult_.Reset();
}

// Partial update method
void ViewFunctions::ExecuteRerender()
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    if (jsRerenderFunc_.IsEmpty()) {
        LOGE("no rerender function in View!");
        return;
    }

    auto func = jsRerenderFunc_.Lock();
    JSRef<JSVal> jsThis = jsObject_.Lock();
    if (!jsThis->IsUndefined()) {
        jsRenderResult_ = func->Call(jsThis);
    } else {
        LOGE("jsView Object is undefined and will not execute rerender function");
    }
}

// Partial update method
ViewFunctions::ViewFunctions(const JSRef<JSObject>& jsObject)
{
    InitViewFunctions(jsObject, JSRef<JSFunc>(), true);
}

std::string ViewFunctions::ExecuteOnFormRecycle()
{
    auto ret = ExecuteFunctionWithReturn(jsOnFormRecycleFunc_, "OnFormRecycle");
    if (!ret->IsEmpty() && ret->IsString()) {
        std::string statusData = ret->ToString();
        return statusData.empty() ? EMPTY_STATUS_DATA : statusData;
    }
    LOGE("ExecuteOnFormRecycle failed");
    return "";
}

void ViewFunctions::ExecuteOnFormRecover(const std::string &statusData)
{
    JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(context_)
    if (jsOnFormRecoverFunc_.IsEmpty()) {
        LOGE("jsOnFormRecoverFunc_ is null");
        return;
    }

    std::string data;
    if (statusData != EMPTY_STATUS_DATA) {
        data = statusData;
    }
    auto jsData = JSRef<JSVal>::Make(ToJSValue(data));
    auto func = jsOnFormRecoverFunc_.Lock();
    func->Call(jsObject_.Lock(), 1, &jsData);
}
} // namespace OHOS::Ace::Framework
