#pragma once

#include <vector>
#include "cTimeline.h"

class cTimelineManager
{
public:
    void AddTimeline(cTimeline* timeline);
    void RemoveTimeline(cTimeline* timeline);
    void UpdateAll(float deltaTime);

private:
    std::vector<cTimeline*> mTimelines;
};

