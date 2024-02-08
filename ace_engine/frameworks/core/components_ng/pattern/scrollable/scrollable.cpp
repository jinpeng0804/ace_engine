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

#include "core/components_ng/pattern/scrollable/scrollable.h"

#include <chrono>

#include "base/log/ace_trace.h"
#include "base/log/frame_report.h"
#include "base/log/jank_frame_report.h"
#include "base/log/log.h"
#include "base/ressched/ressched_report.h"
#include "base/utils/time_util.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/common/layout_inspector.h"
#include "core/event/ace_events.h"

namespace OHOS::Ace::NG {
namespace {

constexpr double CAP_COEFFICIENT = 0.45;
constexpr int32_t FIRST_THRESHOLD = 5;
constexpr int32_t SECOND_THRESHOLD = 10;
constexpr double CAP_FIXED_VALUE = 16.0;
constexpr uint32_t DRAG_INTERVAL_TIME = 900;

#ifndef WEARABLE_PRODUCT
constexpr double FRICTION = 0.6;
constexpr double VELOCITY_SCALE = 1.0;
constexpr double ADJUSTABLE_VELOCITY = 3000.0;
#else
constexpr double DISTANCE_EPSILON = 1.0;
constexpr double FRICTION = 0.9;
constexpr double VELOCITY_SCALE = 0.8;
constexpr double ADJUSTABLE_VELOCITY = 0.0;
#endif
constexpr float FRICTION_SCALE = -4.2f;
constexpr uint32_t CUSTOM_SPRING_ANIMATION_DURION = 1000;
constexpr uint32_t MILLOS_PER_SECONDS = 1000;
constexpr float DEFAULT_THRESHOLD = 0.75f;
constexpr float DEFAULT_SPRING_RESPONSE = 0.416f;
constexpr float DEFAULT_SPRING_DAMP = 0.99f;
constexpr uint32_t MIN_DIFF_TIME = 1;
#ifdef OHOS_PLATFORM
constexpr int64_t INCREASE_CPU_TIME_ONCE = 4000000000; // 4s(unit: ns)
#endif
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
} // namespace

// Static Functions.
double Scrollable::sFriction_ = FRICTION;
double Scrollable::sVelocityScale_ = VELOCITY_SCALE;

void Scrollable::SetVelocityScale(double sVelocityScale)
{
    if (LessOrEqual(sVelocityScale, 0.0)) {
        return;
    }
    sVelocityScale_ = sVelocityScale;
}

void Scrollable::SetFriction(double sFriction)
{
    if (LessOrEqual(sFriction, 0.0)) {
        return;
    }
    sFriction_ = sFriction;
}

Scrollable::~Scrollable()
{
    // If animation still runs, force stop it.
    StopFrictionAnimation();
    StopSpringAnimation();
    StopSnapAnimation();
}

void Scrollable::Initialize(const WeakPtr<PipelineBase>& context)
{
    context_ = context;
    PanDirection panDirection;
    if (axis_ == Axis::VERTICAL) {
        panDirection.type = PanDirection::VERTICAL;
    } else {
        panDirection.type = PanDirection::HORIZONTAL;
    }

    auto actionStart = [weakScroll = AceType::WeakClaim(this)](const GestureEvent& info) {
        auto scroll = weakScroll.Upgrade();
        if (scroll) {
            // Send event to accessibility when scroll start.
            auto context = scroll->GetContext().Upgrade();
            if (context) {
                AccessibilityEvent scrollEvent;
                scrollEvent.nodeId = scroll->nodeId_;
                scrollEvent.eventType = "scrollstart";
                context->SendEventToAccessibility(scrollEvent);
            }
            scroll->isDragging_ = true;
            scroll->HandleDragStart(info);
        }
    };

    auto actionUpdate = [weakScroll = AceType::WeakClaim(this)](const GestureEvent& info) {
        auto scroll = weakScroll.Upgrade();
        if (scroll) {
            scroll->HandleDragUpdate(info);
        }
    };

    auto actionEnd = [weakScroll = AceType::WeakClaim(this)](const GestureEvent& info) {
        auto scroll = weakScroll.Upgrade();
        if (scroll) {
            scroll->HandleDragEnd(info);
            auto context = scroll->GetContext().Upgrade();
            if (context && scroll->IsStopped()) {
                AccessibilityEvent scrollEvent;
                scrollEvent.nodeId = scroll->nodeId_;
                scrollEvent.eventType = "scrollend";
                context->SendEventToAccessibility(scrollEvent);
            }
            if (scroll->actionEnd_) {
                auto gestureEvent = info;
                scroll->actionEnd_(gestureEvent);
            }
            scroll->isDragging_ = false;
        }
    };

    auto actionCancel = [weakScroll = AceType::WeakClaim(this)]() {
        auto scroll = weakScroll.Upgrade();
        if (!scroll) {
            return;
        }
        if (scroll->dragCancelCallback_) {
            scroll->dragCancelCallback_();
        }
        scroll->isDragging_ = false;
    };

    if (Container::IsCurrentUseNewPipeline()) {
        panRecognizerNG_ = AceType::MakeRefPtr<NG::PanRecognizer>(
            DEFAULT_PAN_FINGER, panDirection, DEFAULT_PAN_DISTANCE.ConvertToPx());
        panRecognizerNG_->SetIsAllowMouse(false);
        panRecognizerNG_->SetOnActionStart(actionStart);
        panRecognizerNG_->SetOnActionUpdate(actionUpdate);
        panRecognizerNG_->SetOnActionEnd(actionEnd);
        panRecognizerNG_->SetOnActionCancel(actionCancel);
    }

    // use RawRecognizer to receive next touch down event to stop animation.
    rawRecognizer_ = AceType::MakeRefPtr<RawRecognizer>();

    rawRecognizer_->SetOnTouchDown([weakScroll = AceType::WeakClaim(this)](const TouchEventInfo&) {
        auto scroll = weakScroll.Upgrade();
        if (scroll) {
            scroll->HandleTouchDown();
        }
    });
    rawRecognizer_->SetOnTouchUp([weakScroll = AceType::WeakClaim(this)](const TouchEventInfo&) {
        auto scroll = weakScroll.Upgrade();
        if (scroll) {
            scroll->HandleTouchUp();
        }
    });
    rawRecognizer_->SetOnTouchCancel([weakScroll = AceType::WeakClaim(this)](const TouchEventInfo&) {
        auto scroll = weakScroll.Upgrade();
        if (scroll) {
            scroll->HandleTouchCancel();
        }
    });

    available_ = true;
}

void Scrollable::SetAxis(Axis axis)
{
    axis_ = axis;
    PanDirection panDirection;
    if (axis_ == Axis::NONE) {
        panDirection.type = PanDirection::NONE;
    } else if (axis_ == Axis::VERTICAL) {
        panDirection.type = PanDirection::VERTICAL;
    } else {
        panDirection.type = PanDirection::HORIZONTAL;
    }
    if (panRecognizerNG_) {
        panRecognizerNG_->SetDirection(panDirection);
    }
}

void Scrollable::HandleTouchDown()
{
    isTouching_ = true;
    // If animation still runs, first stop it.
    StopSpringAnimation();
    if (!isFrictionAnimationStop_) {
        StopFrictionAnimation();
    } else if (!isSnapAnimationStop_ || !isSnapScrollAnimationStop_) {
        StopSnapAnimation();
    } else {
        // Resets values.
        currentPos_ = 0.0;
    }
}

void Scrollable::HandleTouchUp()
{
    // Two fingers are alternately drag, one finger is released without triggering spring animation.
    if (isDragging_) {
        return;
    }
    isTouching_ = false;
    if (outBoundaryCallback_ && !outBoundaryCallback_()) {
        if (isSnapScrollAnimationStop_ && scrollSnapCallback_) {
            scrollSnapCallback_(0.0, 0.0);
        }
        return;
    }
    if (isSpringAnimationStop_ && scrollOverCallback_) {
        ProcessScrollOverCallback(0.0);
    }
}

void Scrollable::HandleTouchCancel()
{
    isTouching_ = false;
    if (isSpringAnimationStop_ && scrollOverCallback_) {
        ProcessScrollOverCallback(0.0);
    }
}

bool Scrollable::IsAnimationNotRunning() const
{
    return !isTouching_ && isFrictionAnimationStop_ && isSpringAnimationStop_
        && isSnapAnimationStop_ && isSnapScrollAnimationStop_;
}

bool Scrollable::Idle() const
{
    return !isTouching_ && isFrictionAnimationStop_ && isSpringAnimationStop_
        && isSnapAnimationStop_ && isSnapScrollAnimationStop_;
}

bool Scrollable::IsStopped() const
{
    return isSpringAnimationStop_ && isFrictionAnimationStop_
        && isSnapAnimationStop_ && isSnapScrollAnimationStop_;
}

bool Scrollable::IsSpringStopped() const
{
    return isSpringAnimationStop_;
}

bool Scrollable::IsSnapStopped() const
{
    return isSnapAnimationStop_;
}

void Scrollable::StopScrollable()
{
    if (!isFrictionAnimationStop_) {
        StopFrictionAnimation();
    }
    if (!isSpringAnimationStop_) {
        StopSpringAnimation();
    }
    if (!isSnapAnimationStop_ || !isSnapScrollAnimationStop_) {
        StopSnapAnimation();
    }
}

void Scrollable::HandleScrollEnd()
{
    // priority:
    //  1. onScrollEndRec_ (would internally call onScrollEnd)
    //  2. scrollEndCallback_
    if (onScrollEndRec_) {
        onScrollEndRec_();
        return;
    }
    if (scrollEndCallback_) {
        scrollEndCallback_();
    }
}

void Scrollable::HandleDragStart(const OHOS::Ace::GestureEvent& info)
{
    ACE_FUNCTION_TRACE();
    currentVelocity_ = info.GetMainVelocity();
    if (dragFRCSceneCallback_) {
        dragFRCSceneCallback_(currentVelocity_, NG::SceneStatus::START);
    }
    if (continuousDragStatus_) {
        IncreaseContinueDragCount();
        task_.Cancel();
    }
    SetDragStartPosition(GetMainOffset(Offset(info.GetGlobalPoint().GetX(), info.GetGlobalPoint().GetY())));
    const auto dragPositionInMainAxis =
        axis_ == Axis::VERTICAL ? info.GetGlobalLocation().GetY() : info.GetGlobalLocation().GetX();
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE, "Scroll drag start, localLocation: %{public}s, globalLocation: %{public}s",
        info.GetLocalLocation().ToString().c_str(), info.GetGlobalLocation().ToString().c_str());
#ifdef OHOS_PLATFORM
    // Increase the cpu frequency when sliding start.
    auto currentTime = GetSysTimestamp();
    auto increaseCpuTime = currentTime - startIncreaseTime_;
    if (!moved_ || increaseCpuTime >= INCREASE_CPU_TIME_ONCE) {
        startIncreaseTime_ = currentTime;
        ResSchedReport::GetInstance().ResSchedDataReport("slide_on");
        if (FrameReport::GetInstance().GetEnable()) {
            FrameReport::GetInstance().BeginListFling();
        }
    }
#endif
    JankFrameReport::GetInstance().SetFrameJankFlag(JANK_RUNNING_SCROLL);
    if (onScrollStartRec_) {
        onScrollStartRec_(static_cast<float>(dragPositionInMainAxis));
    }
}

ScrollResult Scrollable::HandleScroll(double offset, int32_t source, NestedState state)
{
    if (!handleScrollCallback_) {
        ExecuteScrollBegin(offset);
        moved_ = UpdateScrollPosition(offset, source);
        canOverScroll_ = false;
        return { 0, false };
    }
    // call NestableScrollContainer::HandleScroll
    return handleScrollCallback_(static_cast<float>(offset), source, state);
}

void Scrollable::HandleDragUpdate(const GestureEvent& info)
{
    ACE_FUNCTION_TRACE();
    currentVelocity_ = info.GetMainVelocity();
    if (dragFRCSceneCallback_) {
        dragFRCSceneCallback_(currentVelocity_, NG::SceneStatus::RUNNING);
    }
    if (!NearZero(info.GetMainVelocity()) && dragCount_ >= FIRST_THRESHOLD) {
        if (Negative(lastVelocity_ / info.GetMainVelocity())) {
            ResetContinueDragCount();
        }
    }
    if (!isSpringAnimationStop_ || !isFrictionAnimationStop_ ||
        !isSnapAnimationStop_ || !isSnapScrollAnimationStop_) {
        // If animation still runs, first stop it.
        isDragUpdateStop_ = true;
        StopFrictionAnimation();
        StopSpringAnimation();
        StopSnapAnimation();
        currentPos_ = 0.0;
    }
#ifdef OHOS_PLATFORM
    // Handle the case where you keep sliding past limit time(4s).
    auto currentTime = GetSysTimestamp();
    auto increaseCpuTime = currentTime - startIncreaseTime_;
    if (increaseCpuTime >= INCREASE_CPU_TIME_ONCE) {
        startIncreaseTime_ = currentTime;
        ResSchedReport::GetInstance().ResSchedDataReport("slide_on");
        if (FrameReport::GetInstance().GetEnable()) {
            FrameReport::GetInstance().BeginListFling();
        }
    }
#endif
    auto mainDelta = info.GetMainDelta();
    JankFrameReport::GetInstance().RecordFrameUpdate();
    auto source = info.GetInputEventType() == InputEventType::AXIS ? SCROLL_FROM_AXIS : SCROLL_FROM_UPDATE;
    HandleScroll(mainDelta, source, NestedState::GESTURE);
}

void Scrollable::HandleDragEnd(const GestureEvent& info)
{
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE, "Scroll drag end, position is %{public}lf and %{public}lf, "
        "velocity is %{public}lf",
        info.GetGlobalPoint().GetX(), info.GetGlobalPoint().GetY(), info.GetMainVelocity());
    if (dragFRCSceneCallback_) {
        dragFRCSceneCallback_(info.GetMainVelocity(), NG::SceneStatus::END);
    }
    isTouching_ = false;
    isDragUpdateStop_ = false;
    touchUp_ = false;
    scrollPause_ = false;
    lastVelocity_ = info.GetMainVelocity();
    double correctVelocity =
        std::clamp(info.GetMainVelocity(), -maxFlingVelocity_ + slipFactor_, maxFlingVelocity_ - slipFactor_);
    SetDragEndPosition(GetMainOffset(Offset(info.GetGlobalPoint().GetX(), info.GetGlobalPoint().GetY())));
    correctVelocity = correctVelocity * sVelocityScale_ * GetGain(GetDragOffset());
    currentVelocity_ = correctVelocity;

    lastPos_ = GetDragOffset();
    JankFrameReport::GetInstance().ClearFrameJankFlag(JANK_RUNNING_SCROLL);
    if (dragEndCallback_) {
        dragEndCallback_();
    }
    double mainPosition = GetMainOffset(Offset(info.GetGlobalPoint().GetX(), info.GetGlobalPoint().GetY()));
    if (!moved_ || info.GetInputEventType() == InputEventType::AXIS) {
        if (calePredictSnapOffsetCallback_) {
            std::optional<float> predictSnapOffset = calePredictSnapOffsetCallback_(0.0f, 0.0f, 0.0f);
            if (predictSnapOffset.has_value() && !NearZero(predictSnapOffset.value())) {
                currentPos_ = mainPosition;
                ProcessScrollSnapSpringMotion(predictSnapOffset.value(), correctVelocity);
                return;
            }
        }
        HandleScrollEnd();
        currentVelocity_ = 0.0;
#ifdef OHOS_PLATFORM
        ResSchedReport::GetInstance().ResSchedDataReport("slide_off");
        if (FrameReport::GetInstance().GetEnable()) {
            FrameReport::GetInstance().EndListFling();
        }
#endif
    } else if (!Container::IsCurrentUseNewPipeline() && outBoundaryCallback_ && outBoundaryCallback_() &&
               scrollOverCallback_) {
        ResetContinueDragCount();
        ProcessScrollOverCallback(correctVelocity);
    } else if (canOverScroll_) {
        ResetContinueDragCount();
        HandleOverScroll(correctVelocity);
    } else {
        StartScrollAnimation(mainPosition, correctVelocity);
    }
    SetDelayedTask();
}

void Scrollable::StartScrollAnimation(float mainPosition, float correctVelocity)
{
    if (!isSpringAnimationStop_) {
        StopSpringAnimation();
    }
    if (!frictionOffsetProperty_) {
        GetFrictionProperty();
    }
    StopSnapController();
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE, "The position of scroll motion is %{public}lf, velocity is %{public}lf",
        mainPosition, correctVelocity);
    float friction = friction_ > 0 ? friction_ : sFriction_;
    initVelocity_ = correctVelocity;
    finalPosition_ = mainPosition + correctVelocity / (friction * -FRICTION_SCALE);

    if (calePredictSnapOffsetCallback_) {
        std::optional<float> predictSnapOffset =
            calePredictSnapOffsetCallback_(GetFinalPosition() - mainPosition, GetDragOffset(), correctVelocity);
        if (predictSnapOffset.has_value() && !NearZero(predictSnapOffset.value())) {
            currentPos_ = mainPosition;
            ProcessScrollSnapSpringMotion(predictSnapOffset.value(), correctVelocity);
            return;
        }
    }

    if (scrollSnapCallback_ && scrollSnapCallback_(GetFinalPosition() - mainPosition, correctVelocity)) {
        currentVelocity_ = 0.0;
        return;
    }
    // change motion param when list item need to be center of screen on watch
    FixScrollMotion(mainPosition, correctVelocity);

    // Resets values.
    currentPos_ = mainPosition;
    currentVelocity_ = 0.0;

    frictionOffsetProperty_->Set(mainPosition);
    float response = fabs(2 * M_PI / (FRICTION_SCALE * friction));
    auto curve = AceType::MakeRefPtr<ResponsiveSpringMotion>(response, 1.0f, 0.0f);
    AnimationOption option;
    option.SetCurve(curve);
    option.SetDuration(CUSTOM_SPRING_ANIMATION_DURION);

    frictionOffsetProperty_->AnimateWithVelocity(option, finalPosition_, initVelocity_,
        [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto scroll = weak.Upgrade();
            CHECK_NULL_VOID(scroll);
            scroll->frictionVelocity_ = 0.0f;
            scroll->isFrictionAnimationStop_ = true;
            scroll->ProcessScrollMotionStop();
            auto context = scroll->GetContext().Upgrade();
            if (context && scroll->Idle()) {
                AccessibilityEvent scrollEvent;
                scrollEvent.nodeId = scroll->nodeId_;
                scrollEvent.eventType = "scrollend";
                context->SendEventToAccessibility(scrollEvent);
            }
    });
    isFrictionAnimationStop_ = false;
    if (scrollMotionFRCSceneCallback_) {
        scrollMotionFRCSceneCallback_(frictionVelocity_, NG::SceneStatus::START);
    }
}

void Scrollable::SetDelayedTask()
{
    SetContinuousDragStatus(true);
    auto context = OHOS::Ace::PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto taskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::UI);
    task_.Reset([weak = WeakClaim(this)] {
        auto drag = weak.Upgrade();
        if (drag) {
            drag->ResetContinueDragCount();
            drag->SetContinuousDragStatus(false);
        }
    });
    taskExecutor.PostDelayedTask(task_, DRAG_INTERVAL_TIME);
}

double Scrollable::ComputeCap(int dragCount)
{
    if (dragCount < FIRST_THRESHOLD) {
        return 1.0;
    }
    auto cap = ComputeCap(dragCount - 1) + CAP_COEFFICIENT * (dragCount - 1);
    return cap;
}

double Scrollable::GetGain(double delta)
{
    auto cap = 1.0;
    auto gain = 1.0;
    if (!continuousSlidingCallback_) {
        return gain;
    }
    auto screenHeight = continuousSlidingCallback_();
    if (delta == 0 || screenHeight == 0) {
        return gain;
    }
    if (dragCount_ >= FIRST_THRESHOLD && dragCount_ < SECOND_THRESHOLD) {
        if (Negative(lastPos_ / delta)) {
            ResetContinueDragCount();
            return gain;
        }
        cap = ComputeCap(dragCount_);
        gain = (LessNotEqual(cap, std::abs(delta) / screenHeight * (dragCount_ - 1))) ? cap :
            std::abs(delta) / screenHeight * (dragCount_ - 1);
    } else if (dragCount_ >= SECOND_THRESHOLD) {
        if (Negative(lastPos_ / delta)) {
            ResetContinueDragCount();
            return gain;
        }
        cap = CAP_FIXED_VALUE;
        gain = (LessNotEqual(cap, std::abs(delta) / screenHeight * (dragCount_ - 1))) ? cap :
            std::abs(delta) / screenHeight * (dragCount_ - 1);
    }
    return gain;
}

void Scrollable::ExecuteScrollBegin(double& mainDelta)
{
    auto context = context_.Upgrade();
    if (!scrollBeginCallback_ || !context) {
        return;
    }

    ScrollInfo scrollInfo;
    if (axis_ == Axis::VERTICAL) {
        scrollInfo = scrollBeginCallback_(0.0_vp, Dimension(mainDelta / context->GetDipScale(), DimensionUnit::VP));
        mainDelta = context->NormalizeToPx(scrollInfo.dy);
    } else if (axis_ == Axis::HORIZONTAL) {
        scrollInfo = scrollBeginCallback_(Dimension(mainDelta / context->GetDipScale(), DimensionUnit::VP), 0.0_vp);
        mainDelta = context->NormalizeToPx(scrollInfo.dx);
    }
}

float Scrollable::GetFrictionVelocityByFinalPosition(float final, float position, float friction,
    float signum, float threshold)
{
    return DEFAULT_THRESHOLD * threshold * signum - (final - position) * friction;
}

void Scrollable::FixScrollMotion(float position, float initVelocity)
{
#ifdef WEARABLE_PRODUCT
    float signum = 0.0;
    if (!NearZero(initVelocity)) {
        signum = GreatNotEqual(initVelocity, 0.0) ? 1.0 : -1.0;
    }
    if (frictionOffsetProperty_ && needCenterFix_ && watchFixCallback_) {
        float finalPosition = watchFixCallback_(GetFinalPosition(), position);
        if (!NearEqual(finalPosition, GetFinalPosition(), DISTANCE_EPSILON)) {
            float friction = friction_ > 0 ? friction_ : sFriction_;
            float velocity = GetFrictionVelocityByFinalPosition(finalPosition, position, friction, signum);

            // fix again when velocity is less than velocity threshold
            if (!NearEqual(finalPosition, GetFinalPosition(), DISTANCE_EPSILON)) {
                velocity = GetFrictionVelocityByFinalPosition(finalPosition, position, friction, signum, 0.0f);
            }
            initVelocity_ = velocity;
            finalPosition_ = mainPosition + initVelocity_ / (friction * -FRICTION_SCALE);
        }
    }
#endif
}

void Scrollable::StartScrollSnapMotion(float predictSnapOffset, float scrollSnapVelocity)
{
    endPos_ = currentPos_ + predictSnapOffset;
    AnimationOption option;
    option.SetDuration(CUSTOM_SPRING_ANIMATION_DURION);
    auto curve = AceType::MakeRefPtr<ResponsiveSpringMotion>(DEFAULT_SPRING_RESPONSE, DEFAULT_SPRING_DAMP, 0.0f);
    option.SetCurve(curve);
    if (!snapOffsetProperty_) {
        GetSnapProperty();
    }
    snapOffsetProperty_->Set(currentPos_);
    snapOffsetProperty_->AnimateWithVelocity(option, endPos_, scrollSnapVelocity,
        [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto scroll = weak.Upgrade();
            CHECK_NULL_VOID(scroll);
            if (scroll->updateSnapAnimationCount_ == 0) {
                scroll->isSnapScrollAnimationStop_ = true;
                scroll->snapVelocity_ = 0.0f;
                scroll->ProcessScrollSnapStop();
            }
    });
    isSnapScrollAnimationStop_ = false;
    if (scrollMotionFRCSceneCallback_) {
        scrollMotionFRCSceneCallback_(snapVelocity_, NG::SceneStatus::START);
    }
}

void Scrollable::ProcessScrollSnapSpringMotion(float scrollSnapDelta, float scrollSnapVelocity)
{
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE, "The snap delta of scroll motion is %{public}f, "
        "The snap velocity of scroll motion is %{public}f",
        scrollSnapDelta, scrollSnapVelocity);
    endPos_ = currentPos_ + scrollSnapDelta;
    AnimationOption option;
    option.SetDuration(CUSTOM_SPRING_ANIMATION_DURION);
    auto curve = AceType::MakeRefPtr<ResponsiveSpringMotion>(DEFAULT_SPRING_RESPONSE, DEFAULT_SPRING_DAMP, 0.0f);
    option.SetCurve(curve);
    if (!snapOffsetProperty_) {
        GetSnapProperty();
    }
    snapOffsetProperty_->Set(currentPos_);
    snapOffsetProperty_->AnimateWithVelocity(option, endPos_, scrollSnapVelocity,
        [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto scroll = weak.Upgrade();
            CHECK_NULL_VOID(scroll);
            scroll->isSnapAnimationStop_ = true;
            scroll->snapVelocity_ = 0.0f;
            scroll->ProcessScrollMotionStop();
            auto context = scroll->GetContext().Upgrade();
            CHECK_NULL_VOID(context && scroll->Idle());
            AccessibilityEvent scrollEvent;
            scrollEvent.nodeId = scroll->nodeId_;
            scrollEvent.eventType = "scrollend";
            context->SendEventToAccessibility(scrollEvent);
    });
    isSnapAnimationStop_ = false;
    if (scrollMotionFRCSceneCallback_) {
        scrollMotionFRCSceneCallback_(snapVelocity_, NG::SceneStatus::START);
    }
}

void Scrollable::UpdateScrollSnapStartOffset(double offset)
{
    UpdateScrollSnapEndWithOffset(offset);
}

void Scrollable::ProcessScrollSnapMotion(double position)
{
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE, "Current Pos is %{public}lf, position is %{public}lf",
        currentPos_, position);
    currentVelocity_ = snapVelocity_;
    if (scrollMotionFRCSceneCallback_) {
        scrollMotionFRCSceneCallback_(currentVelocity_, NG::SceneStatus::RUNNING);
    }
    if (NearEqual(currentPos_, position)) {
        UpdateScrollPosition(0.0, SCROLL_FROM_ANIMATION_SPRING);
    } else {
        auto mainDelta = position - currentPos_;
        HandleScroll(mainDelta, SCROLL_FROM_ANIMATION, NestedState::GESTURE);
        if (!moved_ && !isSnapScrollAnimationStop_) {
            StopSnapAnimation();
        } else if (!touchUp_) {
            if (scrollTouchUpCallback_) {
                scrollTouchUpCallback_();
            }
            touchUp_ = true;
        }
    }
    currentPos_ = position;
    if (outBoundaryCallback_ && outBoundaryCallback_()  && !isSnapScrollAnimationStop_) {
        scrollPause_ = true;
        skipRestartSpring_ = true;
        StopSnapAnimation();
    }
}

void Scrollable::ProcessScrollSnapStop()
{
    if (snapOffsetProperty_ && scrollMotionFRCSceneCallback_) {
        scrollMotionFRCSceneCallback_(snapVelocity_, NG::SceneStatus::END);
    }
    if (scrollPause_) {
        scrollPause_ = false;
        HandleOverScroll(currentVelocity_);
    } else {
        OnAnimateStop();
    }
}

void Scrollable::OnAnimateStop()
{
    if (scrollMotionFRCSceneCallback_) {
        scrollMotionFRCSceneCallback_(GetCurrentVelocity(), NG::SceneStatus::END);
    }
    if (moved_) {
        HandleScrollEnd();
    }
    currentVelocity_ = 0.0;
    if (isTouching_ || isDragUpdateStop_) {
        return;
    }
    moved_ = false;
#ifdef OHOS_PLATFORM
    ResSchedReport::GetInstance().ResSchedDataReport("slide_off");
    if (FrameReport::GetInstance().GetEnable()) {
        FrameReport::GetInstance().EndListFling();
    }
#endif
    if (scrollEnd_) {
        scrollEnd_();
    }
    auto context = GetContext().Upgrade();
    if (context) {
        AccessibilityEvent scrollEvent;
        scrollEvent.nodeId = nodeId_;
        scrollEvent.eventType = "scrollend";
        context->SendEventToAccessibility(scrollEvent);
    }
#if !defined(PREVIEW)
    LayoutInspector::SupportInspector();
#endif
}

void Scrollable::StartSpringMotion(
    double mainPosition, double mainVelocity, const ExtentPair& extent, const ExtentPair& initExtent)
{
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE, "position is %{public}lf, mainVelocity is %{public}lf, "
        "minExtent is %{public}lf, maxExtent is %{public}lf, initMinExtent is %{public}lf, "
        "initMaxExtent is %{public}lf",
        mainPosition, mainVelocity, extent.Leading(), extent.Trailing(), initExtent.Leading(), initExtent.Trailing());
    // skip repeat spring, and handle over scroll spring first, restart spring handle later
    if (!isSpringAnimationStop_ || (skipRestartSpring_ && NearEqual(mainVelocity, 0.0f, 0.001f))) {
        return;
    }
    currentPos_ = mainPosition;
    bool validPos = false;
    if (mainPosition > initExtent.Trailing() || NearEqual(mainPosition, initExtent.Trailing(), 0.01f)) {
        finalPosition_ = extent.Trailing();
        validPos = true;
    } else if (mainPosition <  initExtent.Leading() || NearEqual(mainPosition, initExtent.Leading(), 0.01f)) {
        finalPosition_ = extent.Leading();
        validPos = true;
    }
    if (!validPos) {
        return;
    }

    if (scrollMotionFRCSceneCallback_) {
        scrollMotionFRCSceneCallback_(GetCurrentVelocity(), NG::SceneStatus::START);
    }
    if (!springOffsetProperty_) {
        GetSpringProperty();
    }
    springAnimationCount_++;
    springOffsetProperty_->Set(mainPosition);
    AnimationOption option;
    auto curve = AceType::MakeRefPtr<ResponsiveSpringMotion>(DEFAULT_SPRING_RESPONSE, DEFAULT_SPRING_DAMP, 0.0f);
    option.SetCurve(curve);
    option.SetDuration(CUSTOM_SPRING_ANIMATION_DURION);
    springOffsetProperty_->AnimateWithVelocity(
        option, finalPosition_, mainVelocity,
        [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
            ContainerScope scope(id);
            auto scroll = weak.Upgrade();
            CHECK_NULL_VOID(scroll);
            scroll->springAnimationCount_--;
            // avoid current animation being interrupted by the prev animation's finish callback
            if (scroll->springAnimationCount_ > 0) {
                return;
            }
            scroll->isSpringAnimationStop_ = true;
            scroll->currentVelocity_ = 0.0;
            scroll->OnAnimateStop();
    });
    isSpringAnimationStop_ = false;
    skipRestartSpring_ = false;
}

void Scrollable::ProcessScrollMotionStop()
{
    if (frictionOffsetProperty_ && scrollMotionFRCSceneCallback_) {
        scrollMotionFRCSceneCallback_(frictionVelocity_, NG::SceneStatus::END);
    }
    if (snapOffsetProperty_ && scrollMotionFRCSceneCallback_) {
        scrollMotionFRCSceneCallback_(snapVelocity_, NG::SceneStatus::END);
    }
    if (needScrollSnapChange_ && calePredictSnapOffsetCallback_ && frictionOffsetProperty_) {
        needScrollSnapChange_ = false;
        auto predictSnapOffset = calePredictSnapOffsetCallback_(GetFinalPosition() - currentPos_, 0.0f, 0.0f);
        if (predictSnapOffset.has_value() && !NearZero(predictSnapOffset.value())) {
            ProcessScrollSnapSpringMotion(predictSnapOffset.value(), currentVelocity_);
            return;
        }
    }
    // spring effect special process
    if (scrollPause_) {
        scrollPause_ = false;
        HandleOverScroll(currentVelocity_);
    } else {
        if (isDragUpdateStop_) {
            return;
        }
        moved_ = false;
        HandleScrollEnd();
#ifdef OHOS_PLATFORM
        ResSchedReport::GetInstance().ResSchedDataReport("slide_off");
        if (FrameReport::GetInstance().GetEnable()) {
            FrameReport::GetInstance().EndListFling();
        }
#endif
        if (scrollEnd_) {
            scrollEnd_();
        }
        currentVelocity_ = 0.0;
#if !defined(PREVIEW)
        LayoutInspector::SupportInspector();
#endif
    }
}

void Scrollable::ProcessSpringMotion(double position)
{
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE, "Current Pos is %{public}lf, position is %{public}lf",
        currentPos_, position);
    high_resolution_clock::time_point currentTime = high_resolution_clock::now();
    milliseconds diff = std::chrono::duration_cast<milliseconds>(currentTime - lastTime_);
    if (diff.count() > MIN_DIFF_TIME) {
        currentVelocity_ = (position - currentPos_) / diff.count() * MILLOS_PER_SECONDS;
    }
    lastTime_ = currentTime;
    if (scrollMotionFRCSceneCallback_) {
        scrollMotionFRCSceneCallback_(currentVelocity_, NG::SceneStatus::RUNNING);
    }
    if (NearEqual(currentPos_, position)) {
        UpdateScrollPosition(0.0, SCROLL_FROM_ANIMATION_SPRING);
    } else {
        moved_ = UpdateScrollPosition(position - currentPos_, SCROLL_FROM_ANIMATION_SPRING);
        if (!moved_) {
            StopSpringAnimation();
        } else if (!touchUp_) {
            if (scrollTouchUpCallback_) {
                scrollTouchUpCallback_();
            }
            touchUp_ = true;
        }
    }
    currentPos_ = position;
}

void Scrollable::ProcessScrollMotion(double position)
{
    currentVelocity_ = frictionVelocity_;
    if (scrollMotionFRCSceneCallback_) {
        scrollMotionFRCSceneCallback_(currentVelocity_, NG::SceneStatus::RUNNING);
    }
    if (snapOffsetProperty_ && scrollMotionFRCSceneCallback_) {
        scrollMotionFRCSceneCallback_(snapVelocity_, NG::SceneStatus::RUNNING);
    }
    if (needScrollSnapToSideCallback_) {
        needScrollSnapChange_ = needScrollSnapToSideCallback_(position - currentPos_);
    }
    TAG_LOGD(AceLogTag::ACE_SCROLLABLE, "position is %{public}lf, currentVelocity_ is %{public}lf, "
        "needScrollSnapChange_ is %{public}u",
        position, currentVelocity_, needScrollSnapChange_);
    if ((NearEqual(currentPos_, position))) {
        UpdateScrollPosition(0.0, SCROLL_FROM_ANIMATION);
    } else {
        // UpdateScrollPosition return false, means reach to scroll limit.
        auto mainDelta = position - currentPos_;
        HandleScroll(mainDelta, SCROLL_FROM_ANIMATION, NestedState::GESTURE);
        if (!moved_) {
            StopFrictionAnimation();
        } else if (!touchUp_) {
            if (scrollTouchUpCallback_) {
                scrollTouchUpCallback_();
            }
            touchUp_ = true;
        }
    }
    currentPos_ = position;

    // spring effect special process
    if ((IsSnapStopped() && canOverScroll_) || needScrollSnapChange_ ||
        (!Container::IsCurrentUseNewPipeline() && outBoundaryCallback_ && outBoundaryCallback_())) {
        scrollPause_ = true;
        skipRestartSpring_ = true;
        StopFrictionAnimation();
    }
}

bool Scrollable::UpdateScrollPosition(const double offset, int32_t source) const
{
    bool ret = true;
    if (callback_) {
        ret = callback_(offset, source);
    }
    return ret;
}

void Scrollable::ProcessScrollOverCallback(double velocity)
{
    if (outBoundaryCallback_ && !outBoundaryCallback_() && !canOverScroll_) {
        return;
    }
    // In the case of chain animation enabled, you need to switch the control point first,
    // and then correct the offset value in notification process
    if (notifyScrollOverCallback_) {
        notifyScrollOverCallback_(velocity);
    }
    // then use corrected offset to make scroll motion.
    if (scrollOverCallback_) {
        scrollOverCallback_(velocity);
    }
}

bool Scrollable::HandleOverScroll(double velocity)
{
    if (!overScrollCallback_) {
        if (edgeEffect_ == EdgeEffect::SPRING) {
            ProcessScrollOverCallback(velocity);
            return true;
        }
        if (scrollEndCallback_) {
            scrollEndCallback_();
        }
        return false;
    }
    // call NestableScrollContainer::HandleOverScroll
    return overScrollCallback_(velocity);
}

void Scrollable::SetSlipFactor(double SlipFactor)
{
    slipFactor_ = std::clamp(SlipFactor, -ADJUSTABLE_VELOCITY, ADJUSTABLE_VELOCITY);
}

void Scrollable::UpdateScrollSnapEndWithOffset(double offset)
{
    if (!isSnapScrollAnimationStop_) {
        AnimationOption option;
        option.SetDuration(CUSTOM_SPRING_ANIMATION_DURION);
        auto curve = AceType::MakeRefPtr<ResponsiveSpringMotion>(DEFAULT_SPRING_RESPONSE, DEFAULT_SPRING_DAMP, 0.0f);
        option.SetCurve(curve);
        if (!snapOffsetProperty_) {
            GetSnapProperty();
        }
        updateSnapAnimationCount_++;
        endPos_ -= offset;
        AnimationUtils::StartAnimation(
            option,
            [weak = AceType::WeakClaim(this)]() {
                auto scroll = weak.Upgrade();
                CHECK_NULL_VOID(scroll);
                scroll->snapOffsetProperty_->Set(scroll->endPos_);
                scroll->isSnapScrollAnimationStop_ = false;
            },
            [weak = AceType::WeakClaim(this), id = Container::CurrentId()]() {
                ContainerScope scope(id);
                auto scroll = weak.Upgrade();
                CHECK_NULL_VOID(scroll);
                scroll->updateSnapAnimationCount_--;
                // avoid current animation being interrupted by the prev animation's finish callback
                if (scroll->updateSnapAnimationCount_ == 0) {
                    scroll->isSnapScrollAnimationStop_ = true;
                    scroll->snapVelocity_ = 0.0f;
                    scroll->ProcessScrollSnapStop();
                }
        });
        isSnapScrollAnimationStop_ = false;
    }
}

RefPtr<NodeAnimatablePropertyFloat> Scrollable::GetFrictionProperty()
{
    auto propertyCallback = [weak = AceType::WeakClaim(this)](float position) {
        auto scroll = weak.Upgrade();
        CHECK_NULL_VOID(scroll);
        if (scroll->isFrictionAnimationStop_ || scroll->isTouching_) {
            return;
        }
        high_resolution_clock::time_point currentTime = high_resolution_clock::now();
        milliseconds diff = std::chrono::duration_cast<milliseconds>(currentTime - scroll->lastTime_);
        if (diff.count() > MIN_DIFF_TIME) {
            scroll->frictionVelocity_ = (position - scroll->lastPosition_) /
                diff.count() * MILLOS_PER_SECONDS;
        }
        scroll->lastTime_ = currentTime;
        scroll->lastPosition_ = position;
        scroll->ProcessScrollMotion(position);
    };
    frictionOffsetProperty_ = AceType::MakeRefPtr<NodeAnimatablePropertyFloat>(0.0, std::move(propertyCallback));
    return frictionOffsetProperty_;
}

RefPtr<NodeAnimatablePropertyFloat> Scrollable::GetSpringProperty()
{
    auto propertyCallback = [weak = AceType::WeakClaim(this)](float position) {
        auto scroll = weak.Upgrade();
        CHECK_NULL_VOID(scroll);
        if (!scroll->isSpringAnimationStop_) {
            scroll->ProcessSpringMotion(position);
        }
    };
    springOffsetProperty_ = AceType::MakeRefPtr<NodeAnimatablePropertyFloat>(0.0, std::move(propertyCallback));
    return springOffsetProperty_;
}

RefPtr<NodeAnimatablePropertyFloat> Scrollable::GetSnapProperty()
{
    auto propertyCallback = [weak = AceType::WeakClaim(this)](float position) {
        auto scroll = weak.Upgrade();
        CHECK_NULL_VOID(scroll);
        high_resolution_clock::time_point currentTime = high_resolution_clock::now();
        milliseconds diff = std::chrono::duration_cast<milliseconds>(currentTime - scroll->lastTime_);
        if (diff.count() > MIN_DIFF_TIME) {
            scroll->snapVelocity_ = (position - scroll->currentPos_) /
                diff.count() * MILLOS_PER_SECONDS;
        }
        scroll->lastTime_ = currentTime;
        if (!scroll->isSnapScrollAnimationStop_) {
            scroll->ProcessScrollSnapMotion(position);
        } else if (!scroll->isSnapAnimationStop_) {
            scroll->ProcessScrollMotion(position);
        }
    };
    snapOffsetProperty_ = AceType::MakeRefPtr<NodeAnimatablePropertyFloat>(0.0, std::move(propertyCallback));
    return snapOffsetProperty_;
}

void Scrollable::StopFrictionAnimation()
{
    if (!isFrictionAnimationStop_) {
        isFrictionAnimationStop_ = true;
        AnimationOption option;
        option.SetCurve(Curves::EASE);
        option.SetDuration(0);
        AnimationUtils::StartAnimation(
            option,
            [weak = AceType::WeakClaim(this)]() {
                auto scroll = weak.Upgrade();
                CHECK_NULL_VOID(scroll);
                scroll->frictionOffsetProperty_->Set(0.0f);
            },
            nullptr);
    }
    frictionVelocity_ = 0.0f;
}

void Scrollable::StopSpringAnimation()
{
    if (!isSpringAnimationStop_) {
        isSpringAnimationStop_ = true;
        AnimationOption option;
        option.SetCurve(Curves::EASE);
        option.SetDuration(0);
        AnimationUtils::StartAnimation(
            option,
            [weak = AceType::WeakClaim(this)]() {
                auto scroll = weak.Upgrade();
                CHECK_NULL_VOID(scroll);
                scroll->springOffsetProperty_->Set(0.0f);
            },
            nullptr);
        OnAnimateStop();
    }
    currentVelocity_ = 0.0;
}

void Scrollable::StopSnapAnimation()
{
    if (!isSnapAnimationStop_ || !isSnapScrollAnimationStop_) {
        isSnapAnimationStop_ = true;
        isSnapScrollAnimationStop_ = true;
        AnimationOption option;
        option.SetCurve(Curves::EASE);
        option.SetDuration(0);
        AnimationUtils::StartAnimation(
            option,
            [weak = AceType::WeakClaim(this)]() {
                auto scroll = weak.Upgrade();
                CHECK_NULL_VOID(scroll);
                scroll->snapOffsetProperty_->Set(0.0f);
            },
            nullptr);
    }
    snapVelocity_ = 0.0f;
}
} // namespace OHOS::Ace::NG
