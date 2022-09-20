#pragma once
#include "FW_String.h"
#include "FW_Time.h"
#include "FW_GrowingArray.h"

namespace FW_Profiler
{
	void StartScopeSample(const char* aScopeName);
	void EndScopeSample();

	void StartNewFrame(int aFrameNumber);
	void DisplayProfiler();

	struct Sampler
	{
		Sampler(const char* aName)
		{
			StartScopeSample(aName);
		}
		~Sampler()
		{
			EndScopeSample();
		}
	};
}

#define FW_PROFILE_FUNCTION() FW_Profiler::Sampler __FUNCTION__##Sampler(__FUNCTION__);
#define FW_PROFILE_SCOPE(aScopeName) FW_Profiler::Sampler scopedSampler(aScopeName);
