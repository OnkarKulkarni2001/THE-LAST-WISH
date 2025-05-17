#include "cTimelineManager.h"
#include <algorithm>


void cTimelineManager::AddTimeline(cTimeline* timeline)
{
    if (std::find(mTimelines.begin(), mTimelines.end(), timeline) == mTimelines.end())
        mTimelines.push_back(timeline);
}

void cTimelineManager::RemoveTimeline(cTimeline* timeline)
{
    auto it = std::remove(mTimelines.begin(), mTimelines.end(), timeline);
    if (it != mTimelines.end())
        mTimelines.erase(it, mTimelines.end());
}

void cTimelineManager::UpdateAll(float deltaTime)
{
    for (int i = 0; i < mTimelines.size();)
    {
        cTimeline* timeline = mTimelines[i];
        timeline->Update(deltaTime);

        if (!timeline->IsPlaying() && timeline->mAutoRemoveOnFinish)
        {
            mTimelines.erase(mTimelines.begin() + i);
        }
        else
        {
            ++i;
        }
    }
}
