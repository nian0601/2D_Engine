#include "PhysicsPlayground.h"
#include "FW_Includes.h"

#include "PhysicsTestingTestbed.h"
#include "Spareparty.h"

#define RUN_PHYSICS_TESTBED 0
void PhysicsPlayground::OnStartup()
{
#if RUN_PHYSICS_TESTBED
	myPhysicsTestbed = new PhysicsTestingTestbed();
#else
	mySpareparty = new Spareparty();
#endif
}

void PhysicsPlayground::OnShutdown()
{
	delete myPhysicsTestbed;
	delete mySpareparty;
}

bool PhysicsPlayground::Run()
{
#if RUN_PHYSICS_TESTBED
	myPhysicsTestbed->Run();
#else
	mySpareparty->Run();
#endif

	return true;
}