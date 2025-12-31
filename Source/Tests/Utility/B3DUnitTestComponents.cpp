//************************************ B3D Framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUnitTestComponents.h"
#include "RTTI/B3DUnitTestComponentsRTTI.h"

using namespace b3d;

UnitTestComponentA::UnitTestComponentA(const HSceneObject& parent)
	: Component(parent)
{}

RTTIType* UnitTestComponentA::GetRttiStatic()
{
	return UnitTestComponentARTTI::Instance();
}

RTTIType* UnitTestComponentA::GetRtti() const
{
	return GetRttiStatic();
}

UnitTestComponentB::UnitTestComponentB(const HSceneObject& parent)
	: Component(parent)
{}

RTTIType* UnitTestComponentB::GetRttiStatic()
{
	return UnitTestComponentBRTTI::Instance();
}

RTTIType* UnitTestComponentB::GetRtti() const
{
	return GetRttiStatic();
}
