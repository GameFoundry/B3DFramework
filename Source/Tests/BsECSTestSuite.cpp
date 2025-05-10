//************************************ bs::framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsECSTestSuite.h"
#include "ECS/BsEntitySparseSet.h"

using namespace bs;
using namespace bs::ecs;

void ECSTestSuite::StartUp() { }
void ECSTestSuite::ShutDown() { }

ECSTestSuite::ECSTestSuite()
{
	B3D_ADD_TEST(ECSTestSuite::TestEntitySparseSet)
}

void ECSTestSuite::TestEntitySparseSet()
{
	TEntitySparseSet EntitySparseSet;
	EntitySparseSet.Add(Entity());

}
