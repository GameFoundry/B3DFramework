//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUnitTestSerializableObjects.h"
#include "RTTI/B3DUnitTestSerializableObjectsRTTI.h"

namespace b3d
{
	RTTIType* UnitTestSerializationObjectB::GetRttiStatic()
	{
		return UnitTestSerializationObjectBRTTI::Instance();
	}

	RTTIType* UnitTestSerializationObjectB::GetRtti() const
	{
		return GetRttiStatic();
	}

	RTTIType* UnitTestSerializationObjectA::GetRttiStatic()
	{
		return UnitTestSerializationObjectARTTI::Instance();
	}

	RTTIType* UnitTestSerializationObjectA::GetRtti() const
	{
		return GetRttiStatic();
	}

	SPtr<UnitTestSerializationObjectA> UnitTestSerializationObjectA::CreateVariantA()
	{
		return B3DMakeShared<UnitTestSerializationObjectA>();
	}

	SPtr<UnitTestSerializationObjectA> UnitTestSerializationObjectA::CreateVariantB()
	{
		SPtr<UnitTestSerializationObjectA> object = B3DMakeShared<UnitTestSerializationObjectA>();

		object->IntA = 995;
		object->StrA = "potato";
		object->ArrStrB = { "orange", "carrot" };
		object->ArrStrC[2] = "banana";
		object->ObjB.IntA = 9940;
		object->ObjPtrB->StrA = "kiwi";
		object->ObjPtrC = nullptr;
		object->ObjPtrD = B3DMakeShared<UnitTestSerializationObjectB>();
		object->ObjPtrE = object->ObjPtrA;
		object->ArrObjB[1].StrA = "strawberry";
		object->ArrObjPtrB[0]->IntA = 99100;
		object->PlainMapB =  { { 25, "newValue25" }, { 210, "newValue210" }, { 215, "newValue215" } };
		object->PlainMapC[5] = "newValue5";
		object->PlainMapD[225] = "newValue225";
		object->PlainMapE.erase(object->PlainMapE.find(10));
		object->ObjectMapB["a"].IntA = 9940;
		object->ObjectMapB["c"] = UnitTestSerializationObjectB();
		object->ObjectMapC["b"].StrA = "strawberry";
		object->ObjectMapC.erase("a");
		object->ObjectPointerMap["b"] = nullptr;
		object->ObjectPointerMap["f"] = object->ObjectPointerMap["a"];
		object->ObjectPointerMap.erase("c");
		object->ObjectPointerMap["d"]->IntA = 9940;

		return object;
	}
} // namespace b3d
