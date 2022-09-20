#include "FW_Profiler.h"
#include "FW_Hashmap.h"

#include "imgui\imgui.h"
#include "FW_CircularArray.h"

namespace FW_Profiler
{
	struct Sample
	{
		Sample() {}
		Sample(const char* aSampleName)
			: myName(aSampleName)
		{}

		FW_String myName;
		FW_Time::TimeUnit myExlusiveTime = 0; // The time spend on just the sample, excluding the time of any sub-samples
		FW_Time::TimeUnit myTotalTime = 0; // Total time of this sample + all sub-samples

		FW_Time::TimeUnit myStartTime = 0;
		FW_Time::TimeUnit myEndTime = 0;

		int myExecutionCount = 0;

		Sample* myParentSample = nullptr;
		FW_GrowingArray<Sample> mySubSamples;
	};

	struct FrameData
	{
		int myFrameNumber = 0;
		FW_GrowingArray<Sample> mySamples;
	};

	struct ScopeData
	{
		FW_String myName;
		FW_Time::TimeUnit myTotalTime = 0;
		int myCallCount = 0;
	};

	static FW_CircularArray<FrameData, 60> ourFrameSamples;
	static FrameData* ourCurrentFrameData = nullptr;
	static Sample* ourCurrentSample = nullptr;
	static FW_Hashmap<FW_String, ScopeData*> ourScopeData;

	void StartScopeSample(const char* aScopeName)
	{
		Sample* newSample = nullptr;
		if (!ourCurrentSample)
		{
			ourCurrentFrameData->mySamples.Add(Sample(aScopeName));
			newSample = &ourCurrentFrameData->mySamples.GetLast();
		}
		else
		{
			ourCurrentSample->mySubSamples.Add(Sample(aScopeName));
			newSample = &ourCurrentSample->mySubSamples.GetLast();
		}

		newSample->myParentSample = ourCurrentSample;
		newSample->myStartTime = FW_Time::GetCurrentExactTime();
		ourCurrentSample = newSample;
	}

	void EndScopeSample()
	{
		FW_ASSERT(ourCurrentSample, "Tried to EndScopeSample before starting one, this only happens if you start a sample without using the Profiler-macro(s)");

		ourCurrentSample->myEndTime = FW_Time::GetCurrentExactTime();
		ourCurrentSample->myTotalTime = ourCurrentSample->myEndTime - ourCurrentSample->myStartTime;
		ourCurrentSample->myExlusiveTime = ourCurrentSample->myTotalTime;
		for (const Sample& subsample : ourCurrentSample->mySubSamples)
			ourCurrentSample->myExlusiveTime -= subsample.myTotalTime;

		if (!ourScopeData.KeyExists(ourCurrentSample->myName))
		{
			ScopeData* scopeData = new ScopeData();
			scopeData->myName = ourCurrentSample->myName;
			ourScopeData[ourCurrentSample->myName] = scopeData;
		}

		ScopeData* scopeData = ourScopeData[ourCurrentSample->myName];
		scopeData->myCallCount++;
		scopeData->myTotalTime += ourCurrentSample->myTotalTime;

		ourCurrentSample = ourCurrentSample->myParentSample;
	}

	void DisplaySample(const Sample& aSample)
	{
		bool displaySubsamples = false;
		if (aSample.mySubSamples.IsEmpty())
		{
			ImGui::Text(aSample.myName.GetBuffer());
		}
		else
		{
			displaySubsamples = ImGui::TreeNode(aSample.myName.GetBuffer());
		}

		ImGui::NextColumn();
		ImGui::Text("%.3f", FW_Time::ConvertTimeUnitToGameTime(aSample.myExlusiveTime));

		ImGui::NextColumn();
		ImGui::Text("%.3f", FW_Time::ConvertTimeUnitToGameTime(aSample.myTotalTime));

		ImGui::NextColumn();
		if (displaySubsamples)
		{
			for (const Sample& subsample : aSample.mySubSamples)
				DisplaySample(subsample);

			ImGui::TreePop();
		}
	}

	void StartNewFrame(int aFrameNumber)
	{
		FrameData& data = ourFrameSamples.Add();
		data.myFrameNumber = aFrameNumber;
		data.mySamples.RemoveAll();

		ourCurrentFrameData = &data;
	}

	void DisplayProfiler()
	{
		FW_String windowTitle = "Profiler (";
		windowTitle += static_cast<int>(FW_Time::GetAverageFramerate());
		windowTitle += " fps)###ProfilerID";
		ImGui::Begin(windowTitle.GetBuffer(), nullptr);

		if (ImGui::TreeNode("Current Frame"))
		{
			ImGui::Columns(3, "tree", true);

			ImGui::Text("Name");
			ImGui::NextColumn();
			ImGui::Text("Exclusive");
			ImGui::NextColumn();
			ImGui::Text("Inclusive");
			ImGui::NextColumn();

			for (const Sample& sample : ourCurrentFrameData->mySamples)
			{
				ImGui::Separator();
				DisplaySample(sample);
			}

			ImGui::Separator();
			ImGui::Columns(1);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Scope View"))
		{
			ImGui::Columns(4, "tree", true);

			ImGui::Text("Name");
			ImGui::NextColumn();
			ImGui::Text("Avg");
			ImGui::NextColumn();
			ImGui::Text("Total");
			ImGui::NextColumn();
			ImGui::Text("Calls");
			ImGui::NextColumn();

			for (auto it = ourScopeData.Begin(); it != ourScopeData.End(); it = ourScopeData.Next(it))
			{
				ImGui::Separator();

				ScopeData* data = it.Second();
				ImGui::Text(data->myName.GetBuffer());
				ImGui::NextColumn();
				ImGui::Text("%.3fms", FW_Time::ConvertTimeUnitToGameTime(data->myTotalTime) / data->myCallCount);
				ImGui::NextColumn();
				ImGui::Text("%.3fms", FW_Time::ConvertTimeUnitToGameTime(data->myTotalTime));
				ImGui::NextColumn();
				ImGui::Text("%i", data->myCallCount);
				ImGui::NextColumn();
			}
			ImGui::Separator();

			ImGui::Columns(1);

			ImGui::TreePop();
		}


		ImGui::End();
	}

}