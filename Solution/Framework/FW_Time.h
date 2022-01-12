#pragma once

namespace FW_Time
{
	typedef unsigned long long TimeUnit;

	void Init();
	void Update();

	float GetDelta();
	void ClampToFramerate(float aTargetFramerate);

	float GetAverageFramerate();
	TimeUnit GetTime();

	float ConvertTimeUnitToGameTime(TimeUnit aTimeUnit);
	TimeUnit ConvertGameTimeToTimeUnit(float aGameTime);
}