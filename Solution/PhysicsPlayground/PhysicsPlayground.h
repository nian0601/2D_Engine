#pragma once

#include "FW_IGame.h"

class PhysicsTestingTestbed;
class Spareparty;

class PhysicsPlayground : public FW_IGame
{
public:
	void OnStartup() override;
	void OnShutdown() override;
	bool Run() override;


	const char* GetGameName() override { return "PhysicsPlayground"; }
	const char* GetDataFolderName() override { return "PhysicsPlayground"; }

private:
	PhysicsTestingTestbed* myPhysicsTestbed = nullptr;
	Spareparty* mySpareparty = nullptr;
};
