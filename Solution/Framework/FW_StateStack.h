#pragma once
#include "FW_GrowingArray.h"

class FW_StateStack
{
public:
	class State
	{
	public:
		enum UpdateResult
		{
			KEEP_STATE,
			REMOVE_STATE,
		};

		UpdateResult UpdateState();

	protected:
		virtual UpdateResult OnUpdate() = 0;
	};

	void Update();

	void PushMajorState(State* aState);
	void PushMinorState(State* aState);

	void PopMajorState();
	void PopMinorState();

private:
	FW_GrowingArray<FW_GrowingArray<State*>> myStates;
};
