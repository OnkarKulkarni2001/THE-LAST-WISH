#include "cTimeline.h"
#include <algorithm>
#include <cmath>

cTimeline::cTimeline()
    : mCurrentTime(0.0f)
    , mDuration(1.0f)
    , mIsPlaying(false)
    , mIsReversing(false)
    , mEaseMode(EaseMode::None)
    , mAutoRemoveOnFinish(true)
{
}

void cTimeline::Play()
{
    mIsPlaying = true;
    mIsReversing = false;
}

void cTimeline::Reverse()
{
    mIsPlaying = true;
    mIsReversing = true;
}

void cTimeline::SetDuration(float seconds)
{
    mDuration = std::max(seconds, 0.0001f);
}

void cTimeline::SetEaseMode(EaseMode mode)
{
    mEaseMode = mode;
}

void cTimeline::SetAutoRemove(bool autoRemove)
{
    mAutoRemoveOnFinish = autoRemove;
}

bool cTimeline::IsPlaying() const
{
    return mIsPlaying;
}

void cTimeline::Update(float deltaTime)
{
    if (!mIsPlaying)
        return;

    float direction = mIsReversing ? -1.0f : 1.0f;
    mCurrentTime += deltaTime * direction;

    bool finished = false;

    if (mCurrentTime >= mDuration)
    {
        mCurrentTime = mDuration;
        finished = true;
    }
    else if (mCurrentTime <= 0.0f)
    {
        mCurrentTime = 0.0f;
        finished = true;
    }

    float t = mCurrentTime / mDuration;
    float easedAlpha = ApplyEasing(t);

    if (OnUpdate)
        OnUpdate(easedAlpha);

    if (finished)
    {
        mIsPlaying = false;
        if (OnFinished)
            OnFinished();
    }
}

float cTimeline::ApplyEasing(float t) const
{
    switch (mEaseMode)
    {
    case EaseMode::EaseIn:
        return t * t;
    case EaseMode::EaseOut:
        return 1.0f - (1.0f - t) * (1.0f - t);
    case EaseMode::EaseInOut:
        return t < 0.5f
            ? 2 * t * t
            : 1 - std::pow(-2 * t + 2, 2) / 2;
    case EaseMode::None:
    default:
        return t;
    }
}
