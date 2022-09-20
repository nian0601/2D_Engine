#include "FW_Time.h"

#include "FW_CircularArray.h"
#include <windows.h>

namespace FW_Time
{
	typedef unsigned long long TimeUnit;
	static TimeUnit myLastTime;
	static TimeUnit myCurrentTime;
	static TimeUnit myFrequency;
	static FW_CircularArray<float, 64> myRecentFrameTimes;
	static float myAverageFramerate;

	void Init()
	{
		LARGE_INTEGER largeInt;
		QueryPerformanceFrequency(&largeInt);

		myFrequency = largeInt.QuadPart;

		QueryPerformanceCounter(&largeInt);
		myCurrentTime = largeInt.QuadPart * 1000000 / myFrequency;
		myLastTime = myCurrentTime;
	}

	void Update()
	{
		myLastTime = myCurrentTime;
		myCurrentTime = GetCurrentExactTime();

		myRecentFrameTimes.Add(GetDelta());
		myAverageFramerate = 0.f;
		for (float frameTime : myRecentFrameTimes)
			myAverageFramerate += frameTime;

		myAverageFramerate /= myRecentFrameTimes.myTotalCount;
		myAverageFramerate = 1.f / myAverageFramerate;
	}

	void ClampToFramerate(float aTargetFramerate)
	{
		TimeUnit targetTimeUnit = ConvertGameTimeToTimeUnit(aTargetFramerate);
		TimeUnit elasped = GetCurrentExactTime() - myLastTime;

		while(elasped < targetTimeUnit)
			elasped = GetCurrentExactTime() - myLastTime;
	}

	float GetDelta()
	{
		return min(0.1f, ConvertTimeUnitToGameTime(myCurrentTime - myLastTime));
	}

	float GetAverageFramerate()
	{
		return myAverageFramerate;
	}

	TimeUnit GetTime()
	{
		return myCurrentTime;
	}

	TimeUnit GetCurrentExactTime()
	{
		LARGE_INTEGER largeInt;
		QueryPerformanceCounter(&largeInt);
		return largeInt.QuadPart * 1000000 / myFrequency;
	}

	float ConvertTimeUnitToGameTime(TimeUnit aTimeUnit)
	{
		return aTimeUnit / 1000000.f;
	}

	TimeUnit ConvertGameTimeToTimeUnit(float aGameTime)
	{
		return static_cast<TimeUnit>(aGameTime * 1000000.f);
	}
}