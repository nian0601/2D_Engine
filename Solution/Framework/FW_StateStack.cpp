#include "FW_StateStack.h"
#include "FW_Assert.h"

void FW_StateStack::Update()
{
	if (myStates.IsEmpty())
		return;

	FW_GrowingArray<State*>& currentMajorState = myStates.GetLast();
	for (int i = currentMajorState.Count() - 1; i >= 0; --i)
	{
		State* minorState = currentMajorState[i];

		State::UpdateResult result = minorState->UpdateState();
		if (result == State::UpdateResult::REMOVE_STATE)
		{
			currentMajorState.DeleteCyclicAtIndex(i);
			if (currentMajorState.IsEmpty())
				myStates.RemoveLast();
		}
	}
}

void FW_StateStack::PushMajorState(State* aState)
{
	FW_GrowingArray<State*>& newMajorState = myStates.Add();
	newMajorState.Add(aState);
}

void FW_StateStack::PushMinorState(State* aState)
{
	FW_ASSERT(!myStates.IsEmpty(), "Cannot push a MinorState without first pushing a MajorState");
	FW_GrowingArray<State*>& currentMajorState = myStates.GetLast();
	currentMajorState.Add(aState);
}

void FW_StateStack::PopMajorState()
{
	FW_ASSERT(!myStates.IsEmpty(), "StateStack is empty, cannot PopMajorState");
	FW_GrowingArray<State*>& currentMajorState = myStates.GetLast();
	currentMajorState.DeleteAll();
	myStates.RemoveLast();
}

void FW_StateStack::PopMinorState()
{
	FW_ASSERT(!myStates.IsEmpty(), "StateStack is empty, cannot PopMinorStateState");

	FW_GrowingArray<State*>& currentMajorState = myStates.GetLast();
	FW_ASSERT(currentMajorState.Count() > 1, "Current MajorState doesnt contain any MinorStates, cannot PopMinorState");
	currentMajorState.DeleteLast();
}

FW_StateStack::State::UpdateResult FW_StateStack::State::UpdateState()
{
	return OnUpdate();
}
