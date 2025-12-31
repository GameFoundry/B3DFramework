//************************************ B3D Framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUnitTestSerializationHelper.h"

using namespace b3d;
template <typename T>
void UnitTestSerializationHelpers::TestAssertArraysMatch(TestSuite& testSuite, const T& lhs, const T& rhs)
{
	B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs.size() == rhs.size())
	for(u32 i = 0; i < (u32)lhs.size(); i++)
		B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs[i] == rhs[i])
}

template <typename T>
void UnitTestSerializationHelpers::TestAssertArrayContentsMatch(TestSuite& testSuite, const T& lhs, const T& rhs)
{
	B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs.size() == rhs.size())
	for(u32 i = 0; i < (u32)lhs.size(); i++)
	{
		if(lhs[i] != nullptr)
		{
			B3D_TEST_ASSERT_EXTERNAL(testSuite, rhs[i] != nullptr)
			B3D_TEST_ASSERT_EXTERNAL(testSuite, *(lhs[i]) == *(rhs[i]))
		}
		else
		{
			B3D_TEST_ASSERT_EXTERNAL(testSuite, rhs[i] == nullptr)
		}
	}
}

template <typename T>
void UnitTestSerializationHelpers::TestAssertMapsMatch(TestSuite& testSuite, const T& lhs, const T& rhs)
{
	B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs.size() == rhs.size())
	for(const auto& pair : lhs)
	{
		auto found = rhs.find(pair.first);
		B3D_TEST_ASSERT_EXTERNAL(testSuite, found != rhs.end())
		B3D_TEST_ASSERT_EXTERNAL(testSuite, found->second == pair.second)
	}
}

void UnitTestSerializationHelpers::TestAssertObjectsMatch(TestSuite& testSuite, const SPtr<UnitTestSerializationObjectA>& lhs, const SPtr<UnitTestSerializationObjectA>& rhs, bool isDelta = false)
{
	B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs->IntA == rhs->IntA)
	B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs->StrA == rhs->StrA)
	B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs->StrB == rhs->StrB)

	B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs->ObjA.IntA == rhs->ObjA.IntA)
	B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs->ObjB.IntA == rhs->ObjB.IntA)

	B3D_TEST_ASSERT_EXTERNAL(testSuite, *lhs->ObjPtrA == *rhs->ObjPtrA)
	B3D_TEST_ASSERT_EXTERNAL(testSuite, *lhs->ObjPtrB == *rhs->ObjPtrB)
	B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs->ObjPtrC == rhs->ObjPtrC)
	B3D_TEST_ASSERT_EXTERNAL(testSuite, *lhs->ObjPtrD == *rhs->ObjPtrD)
	B3D_TEST_ASSERT_EXTERNAL(testSuite, *lhs->ObjPtrE == *rhs->ObjPtrE)

	if(!isDelta) // TODO Skipping these tests for deltas, as it's not a case it handled yet.
	{
		B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs->ObjPtrA == lhs->ObjPtrE)
		B3D_TEST_ASSERT_EXTERNAL(testSuite, rhs->ObjPtrA == rhs->ObjPtrE)
	}

	TestAssertArraysMatch(testSuite, lhs->ArrStrA, rhs->ArrStrA);
	TestAssertArraysMatch(testSuite, lhs->ArrStrB, rhs->ArrStrB);
	TestAssertArraysMatch(testSuite, lhs->ArrStrC, rhs->ArrStrC);
	TestAssertArraysMatch(testSuite, lhs->ArrObjA, rhs->ArrObjA);
	TestAssertArraysMatch(testSuite, lhs->ArrObjB, rhs->ArrObjB);
	TestAssertArrayContentsMatch(testSuite, lhs->ArrObjPtrA, rhs->ArrObjPtrA);
	TestAssertArrayContentsMatch(testSuite, lhs->ArrObjPtrB, rhs->ArrObjPtrB);

	TestAssertMapsMatch(testSuite, lhs->PlainMapA, rhs->PlainMapA);
	TestAssertMapsMatch(testSuite, lhs->PlainMapB, rhs->PlainMapB);
	TestAssertMapsMatch(testSuite, lhs->PlainMapC, rhs->PlainMapC);
	TestAssertMapsMatch(testSuite, lhs->PlainMapD, rhs->PlainMapD);
	TestAssertMapsMatch(testSuite, lhs->PlainMapE, rhs->PlainMapE);

	TestAssertMapsMatch(testSuite, lhs->ObjectMapA, rhs->ObjectMapA);
	TestAssertMapsMatch(testSuite, lhs->ObjectMapB, rhs->ObjectMapB);
	TestAssertMapsMatch(testSuite, lhs->ObjectMapC, rhs->ObjectMapC);

	B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs->ObjectPointerMap.size() == rhs->ObjectPointerMap.size())
	for(const auto& pair : lhs->ObjectPointerMap)
	{
		auto found = rhs->ObjectPointerMap.find(pair.first);
		B3D_TEST_ASSERT_EXTERNAL(testSuite, found != rhs->ObjectPointerMap.end())

		if(pair.second != nullptr)
		{
			B3D_TEST_ASSERT_EXTERNAL(testSuite, found->second != nullptr)
			B3D_TEST_ASSERT_EXTERNAL(testSuite, *(pair.second) == *(found->second))
		}
		else
		{
			B3D_TEST_ASSERT_EXTERNAL(testSuite, found->second == nullptr)
		}
	}

	if(!isDelta) // TODO Skipping these tests for deltas, as it's not a case it handled yet.
	{
		B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs->ObjectPointerMap.find("a") != lhs->ObjectPointerMap.end())
		B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs->ObjectPointerMap.find("f") != lhs->ObjectPointerMap.end())
		B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs->ObjectPointerMap["a"] == lhs->ObjectPointerMap["f"])

		B3D_TEST_ASSERT_EXTERNAL(testSuite, rhs->ObjectPointerMap.find("a") != rhs->ObjectPointerMap.end())
		B3D_TEST_ASSERT_EXTERNAL(testSuite, rhs->ObjectPointerMap.find("f") != rhs->ObjectPointerMap.end())
		B3D_TEST_ASSERT_EXTERNAL(testSuite, rhs->ObjectPointerMap["a"] == rhs->ObjectPointerMap["f"])
	}
}
