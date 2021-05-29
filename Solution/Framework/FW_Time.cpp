#include "FW_Time.h"
#include <windows.h>

namespace FW_Time
{
	typedef unsigned long long TimeUnit;
	static TimeUnit myLastTime;
	static TimeUnit myCurrentTime;
	static TimeUnit myFrequency;

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

		LARGE_INTEGER largeInt;
		QueryPerformanceCounter(&largeInt);
		myCurrentTime = largeInt.QuadPart * 1000000 / myFrequency;
	}


	float GetDelta()
	{
		return min(0.1f, static_cast<float>(myCurrentTime - myLastTime) / 1000000.f);
	}
}