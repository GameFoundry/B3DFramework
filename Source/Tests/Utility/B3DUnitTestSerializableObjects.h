//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "B3DPrerequisites.h"


namespace b3d
{
	struct UnitTestSerializationObjectB : IReflectable
	{
		u32 IntA = 100;
		String StrA = "100";

		bool operator==(const UnitTestSerializationObjectB& other) const
		{
			return IntA == other.IntA && StrA == other.StrA;
		}

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializationTestObjectBRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	struct UnitTestSerializationObjectA : IReflectable
	{
		UnitTestSerializationObjectA()
		{
			ArrStrA = { "10", "11", "12" };
			ArrStrB = { "13", "14", "15" };
			ArrStrC = { "16", "17", "18" };

			ArrObjA = { UnitTestSerializationObjectB(), UnitTestSerializationObjectB(), UnitTestSerializationObjectB() };
			ArrObjB = { UnitTestSerializationObjectB(), UnitTestSerializationObjectB(), UnitTestSerializationObjectB() };

			ArrObjPtrA = { B3DMakeShared<UnitTestSerializationObjectB>(), B3DMakeShared<UnitTestSerializationObjectB>(), B3DMakeShared<UnitTestSerializationObjectB>() };
			ArrObjPtrB = { B3DMakeShared<UnitTestSerializationObjectB>(), B3DMakeShared<UnitTestSerializationObjectB>(), B3DMakeShared<UnitTestSerializationObjectB>() };
		}

		static SPtr<UnitTestSerializationObjectA> CreateVariantA();
		static SPtr<UnitTestSerializationObjectA> CreateVariantB();

		u32 IntA = 5;
		String StrA = "5";
		String StrB = "7";

		UnitTestSerializationObjectB ObjA;
		UnitTestSerializationObjectB ObjB;

		SPtr<UnitTestSerializationObjectB> ObjPtrA = B3DMakeShared<UnitTestSerializationObjectB>();
		SPtr<UnitTestSerializationObjectB> ObjPtrB = B3DMakeShared<UnitTestSerializationObjectB>();
		SPtr<UnitTestSerializationObjectB> ObjPtrC = B3DMakeShared<UnitTestSerializationObjectB>();
		SPtr<UnitTestSerializationObjectB> ObjPtrD = nullptr;
		SPtr<UnitTestSerializationObjectB> ObjPtrE = nullptr;

		Vector<String> ArrStrA;
		Vector<String> ArrStrB;
		Vector<String> ArrStrC;

		Vector<UnitTestSerializationObjectB> ArrObjA;
		Vector<UnitTestSerializationObjectB> ArrObjB;

		Vector<SPtr<UnitTestSerializationObjectB>> ArrObjPtrA;
		Vector<SPtr<UnitTestSerializationObjectB>> ArrObjPtrB;

		UnorderedMap<u32, String> PlainMapA = { { 5, "value5" }, { 10, "value10" }, { 15, "value15" } };
		UnorderedMap<u32, String> PlainMapB = { { 5, "value5" }, { 10, "value10" }, { 15, "value15" } };
		UnorderedMap<u32, String> PlainMapC = { { 5, "value5" }, { 10, "value10" }, { 15, "value15" } };
		UnorderedMap<u32, String> PlainMapD = { { 5, "value5" }, { 10, "value10" }, { 15, "value15" } };
		UnorderedMap<u32, String> PlainMapE = { { 5, "value5" }, { 10, "value10" }, { 15, "value15" } };

		UnorderedMap<String, UnitTestSerializationObjectB> ObjectMapA = { { "a", UnitTestSerializationObjectB() }, { "b", UnitTestSerializationObjectB() } };
		UnorderedMap<String, UnitTestSerializationObjectB> ObjectMapB = { { "a", UnitTestSerializationObjectB() }, { "b", UnitTestSerializationObjectB() } };
		UnorderedMap<String, UnitTestSerializationObjectB> ObjectMapC = { { "a", UnitTestSerializationObjectB() }, { "b", UnitTestSerializationObjectB() } };

		UnorderedMap<String, SPtr<UnitTestSerializationObjectB>> ObjectPointerMap = { { "a", B3DMakeShared<UnitTestSerializationObjectB>() },
																					  { "b", B3DMakeShared<UnitTestSerializationObjectB>() },
																					  { "c", B3DMakeShared<UnitTestSerializationObjectB>() },
																					  { "d", B3DMakeShared<UnitTestSerializationObjectB>() },
																					  { "e", nullptr } };

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class UnitTestSerializationObjectARTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};
} // namespace b3d
