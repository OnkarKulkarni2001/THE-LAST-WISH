#pragma once

#include <functional>

enum class EaseMode
{
    None,
    EaseIn,
    EaseOut,
    EaseInOut
};

class cTimeline
{
public:
    cTimeline();

    void Play();
    void Reverse();
    void SetDuration(float seconds);
    void SetEaseMode(EaseMode mode);
    void SetAutoRemove(bool autoRemove);

    void Update(float deltaTime);
    bool IsPlaying() const;

    std::function<void(float alpha)> OnUpdate;
    std::function<void()> OnFinished;

    bool mAutoRemoveOnFinish;

private:
    float mCurrentTime;
    float mDuration;
    bool mIsPlaying;
    bool mIsReversing;
    EaseMode mEaseMode;

    float ApplyEasing(float t) const;
};
