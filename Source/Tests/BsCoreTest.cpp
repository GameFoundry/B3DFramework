//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/BsConsoleTestOutput.h"
#include "Testing/BsTestSuite.h"
#include "Animation/BsAnimationCurve.h"
#include "Particles/BsParticleDistribution.h"
#include "RTTI/BsStringRTTI.h"
#include "Serialization/BsBinarySerializer.h"
#include "FileSystem/BsDataStream.h"
#include "Scene/BsComponent.h"
#include "Scene/BsGameObjectManager.h"
#include "Scene/BsPrefab.h"
#include "Scene/BsSceneManager.h"
#include "Scene/BsSceneObject.h"
#include "BsApplication.h"
#include "Utility/BsUtility.h"

using namespace bs;

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
	static RTTITypeBase* GetRttiStatic();
	RTTITypeBase* GetRtti() const override;
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

	UnorderedMap<String, UnitTestSerializationObjectB> ObjectMapA = {{ "a", UnitTestSerializationObjectB() }, { "b", UnitTestSerializationObjectB() }};
	UnorderedMap<String, UnitTestSerializationObjectB> ObjectMapB = {{ "a", UnitTestSerializationObjectB() }, { "b", UnitTestSerializationObjectB() }};
	UnorderedMap<String, UnitTestSerializationObjectB> ObjectMapC = { { "a", UnitTestSerializationObjectB() }, { "b", UnitTestSerializationObjectB() }};

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
	static RTTITypeBase* GetRttiStatic();
	RTTITypeBase* GetRtti() const override;
};

class UnitTestComponentA : public Component
{
public:
	HSceneObject SceneObjectReference;
	HComponent ComponentReference;
	String StringValue;

	/************************************************************************/
	/* 							COMPONENT OVERRIDES                    		*/
	/************************************************************************/

protected:
	friend class SceneObject;

	UnitTestComponentA(const HSceneObject& parent);

	/************************************************************************/
	/* 								RTTI		                     		*/
	/************************************************************************/
public:
	friend class UnitTestComponentARTTI;
	static RTTITypeBase* GetRttiStatic();
	RTTITypeBase* GetRtti() const;

protected:
	UnitTestComponentA() {} // Serialization only
};

class UnitTestComponentB : public Component
{
public:
	HSceneObject SceneObjectReference;
	String StringValue;

	/************************************************************************/
	/* 							COMPONENT OVERRIDES                    		*/
	/************************************************************************/

protected:
	friend class SceneObject;

	UnitTestComponentB(const HSceneObject& parent);

	/************************************************************************/
	/* 								RTTI		                     		*/
	/************************************************************************/
public:
	friend class UnitTestComponentBRTTI;
	static RTTITypeBase* GetRttiStatic();
	RTTITypeBase* GetRtti() const;

protected:
	UnitTestComponentB() {} // Serialization only
};

class UnitTestComponentARTTI : public RTTIType<UnitTestComponentA, Component, UnitTestComponentARTTI>
{
private:
	B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_MEMBER(SceneObjectReference, 0)
		B3D_RTTI_MEMBER(ComponentReference, 1)
		B3D_RTTI_MEMBER(StringValue, 2)
	B3D_RTTI_END_MEMBERS

public:
	const String& GetRttiName() override
	{
		static String name = "UnitTestComponentA";
		return name;
	}

	u32 GetRttiId() const override
	{
		return TID_UnitTestComponentA;
	}

	SPtr<IReflectable> NewRttiObject() override
	{
		return SceneObject::CreateEmptyComponent<UnitTestComponentA>();
	}
};

class UnitTestComponentBRTTI : public RTTIType<UnitTestComponentB, Component, UnitTestComponentBRTTI>
{
private:
	B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_MEMBER(SceneObjectReference, 0)
		B3D_RTTI_MEMBER(StringValue, 1)
	B3D_RTTI_END_MEMBERS

public:
	const String& GetRttiName() override
	{
		static String name = "UnitTestComponentB";
		return name;
	}

	u32 GetRttiId() const override
	{
		return TID_UnitTestComponentB;
	}

	SPtr<IReflectable> NewRttiObject() override
	{
		return SceneObject::CreateEmptyComponent<UnitTestComponentB>();
	}
};

class UnitTestSerializationObjectARTTI : public RTTIType<UnitTestSerializationObjectA, IReflectable, UnitTestSerializationObjectARTTI>
{
private:
	B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_MEMBER(IntA, 0)
		B3D_RTTI_MEMBER(StrA, 1)
		B3D_RTTI_MEMBER(StrB, 2)

		B3D_RTTI_MEMBER(ObjA, 3)
		B3D_RTTI_MEMBER(ObjB, 4)

		B3D_RTTI_MEMBER(ObjPtrA, 5)
		B3D_RTTI_MEMBER(ObjPtrB, 6)
		B3D_RTTI_MEMBER(ObjPtrC, 7)
		B3D_RTTI_MEMBER(ObjPtrD, 8)
		B3D_RTTI_MEMBER(ObjPtrE, 9)

		B3D_RTTI_MEMBER_CONTAINER(ArrStrA, 10)
		B3D_RTTI_MEMBER_CONTAINER(ArrStrB, 11)
		B3D_RTTI_MEMBER_CONTAINER(ArrStrC, 12)

		B3D_RTTI_MEMBER_CONTAINER(ArrObjA, 13)
		B3D_RTTI_MEMBER_CONTAINER(ArrObjB, 14)

		B3D_RTTI_MEMBER_CONTAINER(ArrObjPtrA, 15)
		B3D_RTTI_MEMBER_CONTAINER(ArrObjPtrB, 16)

		B3D_RTTI_MEMBER_CONTAINER(PlainMapA, 17)
		B3D_RTTI_MEMBER_CONTAINER(PlainMapB, 18)
		B3D_RTTI_MEMBER_CONTAINER(PlainMapC, 19)
		B3D_RTTI_MEMBER_CONTAINER(PlainMapD, 20)
		B3D_RTTI_MEMBER_CONTAINER(PlainMapE, 21)

		B3D_RTTI_MEMBER_CONTAINER(ObjectMapA, 22)
		B3D_RTTI_MEMBER_CONTAINER(ObjectMapB, 23)
		B3D_RTTI_MEMBER_CONTAINER(ObjectMapC, 24)

		B3D_RTTI_MEMBER_CONTAINER(ObjectPointerMap, 25)

	B3D_RTTI_END_MEMBERS

public:
	const String& GetRttiName() override
	{
		static String name = "UnitTestSerializationObjectA";
		return name;
	}

	u32 GetRttiId() const override
	{
		return TID_UnitTestSerializationObjectA;
	}

	SPtr<IReflectable> NewRttiObject() override
	{
		return B3DMakeShared<UnitTestSerializationObjectA>();
	}
};

class UnitTestSerializationObjectBRTTI : public RTTIType<UnitTestSerializationObjectB, IReflectable, UnitTestSerializationObjectBRTTI>
{
private:
	B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_MEMBER(IntA, 0)
		B3D_RTTI_MEMBER(StrA, 1)
	B3D_RTTI_END_MEMBERS

public:
	const String& GetRttiName() override
	{
		static String name = "UnitTestSerializationObjectB";
		return name;
	}

	u32 GetRttiId() const override
	{
		return TID_UnitTestSerializationObjectB;
	}

	SPtr<IReflectable> NewRttiObject() override
	{
		return B3DMakeShared<UnitTestSerializationObjectB>();
	}
};

RTTITypeBase* UnitTestSerializationObjectB::GetRttiStatic()
{
	return UnitTestSerializationObjectBRTTI::Instance();
}

RTTITypeBase* UnitTestSerializationObjectB::GetRtti() const
{
	return GetRttiStatic();
}

RTTITypeBase* UnitTestSerializationObjectA::GetRttiStatic()
{
	return UnitTestSerializationObjectARTTI::Instance();
}

RTTITypeBase* UnitTestSerializationObjectA::GetRtti() const
{
	return GetRttiStatic();
}

UnitTestComponentA::UnitTestComponentA(const HSceneObject& parent)
	: Component(parent)
{}

RTTITypeBase* UnitTestComponentA::GetRttiStatic()
{
	return UnitTestComponentARTTI::Instance();
}

RTTITypeBase* UnitTestComponentA::GetRtti() const
{
	return GetRttiStatic();
}

UnitTestComponentB::UnitTestComponentB(const HSceneObject& parent)
	: Component(parent)
{}

RTTITypeBase* UnitTestComponentB::GetRttiStatic()
{
	return UnitTestComponentBRTTI::Instance();
}

RTTITypeBase* UnitTestComponentB::GetRtti() const
{
	return GetRttiStatic();
}

using HUnitTestComponentA = GameObjectHandle<UnitTestComponentA>;
using HUnitTestComponentB = GameObjectHandle<UnitTestComponentB>;

static float EvaluatePosition(float acceleration, float velocity, float time)
{
	return acceleration * time * time * 0.5f + velocity * time;
}

static float EvaluateVelocity(float acceleration, float time)
{
	return acceleration * time;
}

class CoreTestSuite : public TestSuite
{
public:
	CoreTestSuite();

private:
	void TestAnimCurveIntegration();
	void TestLookupTable();
	void TestBinarySerialization();
	void TestSerializedObject();
	void TestBinaryDelta();
	void TestSceneSaveLoad();

	template<typename T>
	void TestAssertArraysMatch(const T& lhs, const T& rhs);

	template<typename T>
	void TestAssertArrayContentsMatch(const T& lhs, const T& rhs);

	template<typename T>
	void TestAssertMapsMatch(const T& lhs, const T& rhs);

	void TestAssertObjectsMatch(const SPtr<UnitTestSerializationObjectA>& lhs, const SPtr<UnitTestSerializationObjectA>& rhs, bool isDelta);

	static SPtr<UnitTestSerializationObjectA> CreateSerializationTestObjectVariantA();
	static SPtr<UnitTestSerializationObjectA> CreateSerializationTestObjectVariantB();
};

CoreTestSuite::CoreTestSuite()
{
	B3D_ADD_TEST(CoreTestSuite::TestAnimCurveIntegration)
	B3D_ADD_TEST(CoreTestSuite::TestLookupTable)
	B3D_ADD_TEST(CoreTestSuite::TestBinarySerialization)
	B3D_ADD_TEST(CoreTestSuite::TestSerializedObject)
	B3D_ADD_TEST(CoreTestSuite::TestBinaryDelta)
	B3D_ADD_TEST(CoreTestSuite::TestSceneSaveLoad)

	// TODO - Add unit tests for:
	// - Binary cloner test that restores external references
	// - SceneObject/Component serialization/deserialization
	// - Various Prefab operations
}

SPtr<UnitTestSerializationObjectA> CoreTestSuite::CreateSerializationTestObjectVariantA()
{
	return B3DMakeShared<UnitTestSerializationObjectA>();
}

SPtr<UnitTestSerializationObjectA> CoreTestSuite::CreateSerializationTestObjectVariantB()
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

template<typename T>
void CoreTestSuite::TestAssertArraysMatch(const T& lhs, const T& rhs)
{
	B3D_TEST_ASSERT(lhs.size() == rhs.size())
	for(u32 i = 0; i < (u32)lhs.size(); i++)
		B3D_TEST_ASSERT(lhs[i] == rhs[i])
}

template<typename T>
void CoreTestSuite::TestAssertArrayContentsMatch(const T& lhs, const T& rhs)
{
	B3D_TEST_ASSERT(lhs.size() == rhs.size())
	for(u32 i = 0; i < (u32)lhs.size(); i++)
	{
		if(lhs[i] != nullptr)
		{
			B3D_TEST_ASSERT(rhs[i] != nullptr)
			B3D_TEST_ASSERT(*(lhs[i]) == *(rhs[i]))
		}
		else
		{
			B3D_TEST_ASSERT(rhs[i] == nullptr)
		}
	}
}

template<typename T>
void CoreTestSuite::TestAssertMapsMatch(const T& lhs, const T& rhs)
{
	B3D_TEST_ASSERT(lhs.size() == rhs.size())
	for(const auto& pair : lhs)
	{
		auto found = rhs.find(pair.first);
		B3D_TEST_ASSERT(found != rhs.end())
		B3D_TEST_ASSERT(found->second == pair.second)
	}
}

void CoreTestSuite::TestAssertObjectsMatch(const SPtr<UnitTestSerializationObjectA>& lhs, const SPtr<UnitTestSerializationObjectA>& rhs, bool isDelta = false)
{
	B3D_TEST_ASSERT(lhs->IntA == rhs->IntA)
	B3D_TEST_ASSERT(lhs->StrA == rhs->StrA)
	B3D_TEST_ASSERT(lhs->StrB == rhs->StrB)

	B3D_TEST_ASSERT(lhs->ObjA.IntA == rhs->ObjA.IntA)
	B3D_TEST_ASSERT(lhs->ObjB.IntA == rhs->ObjB.IntA)

	B3D_TEST_ASSERT(*lhs->ObjPtrA == *rhs->ObjPtrA)
	B3D_TEST_ASSERT(*lhs->ObjPtrB == *rhs->ObjPtrB)
	B3D_TEST_ASSERT(lhs->ObjPtrC == rhs->ObjPtrC)
	B3D_TEST_ASSERT(*lhs->ObjPtrD == *rhs->ObjPtrD)
	B3D_TEST_ASSERT(*lhs->ObjPtrE == *rhs->ObjPtrE)

	if(!isDelta) // TODO Skipping these tests for deltas, as it's not a case it handled yet.
	{
		B3D_TEST_ASSERT(lhs->ObjPtrA == lhs->ObjPtrE)
		B3D_TEST_ASSERT(rhs->ObjPtrA == rhs->ObjPtrE)
	}

	TestAssertArraysMatch(lhs->ArrStrA, rhs->ArrStrA);
	TestAssertArraysMatch(lhs->ArrStrB, rhs->ArrStrB);
	TestAssertArraysMatch(lhs->ArrStrC, rhs->ArrStrC);
	TestAssertArraysMatch(lhs->ArrObjA, rhs->ArrObjA);
	TestAssertArraysMatch(lhs->ArrObjB, rhs->ArrObjB);
	TestAssertArrayContentsMatch(lhs->ArrObjPtrA, rhs->ArrObjPtrA);
	TestAssertArrayContentsMatch(lhs->ArrObjPtrB, rhs->ArrObjPtrB);

	TestAssertMapsMatch(lhs->PlainMapA, rhs->PlainMapA);
	TestAssertMapsMatch(lhs->PlainMapB, rhs->PlainMapB);
	TestAssertMapsMatch(lhs->PlainMapC, rhs->PlainMapC);
	TestAssertMapsMatch(lhs->PlainMapD, rhs->PlainMapD);
	TestAssertMapsMatch(lhs->PlainMapE, rhs->PlainMapE);

	TestAssertMapsMatch(lhs->ObjectMapA, rhs->ObjectMapA);
	TestAssertMapsMatch(lhs->ObjectMapB, rhs->ObjectMapB);
	TestAssertMapsMatch(lhs->ObjectMapC, rhs->ObjectMapC);

	B3D_TEST_ASSERT(lhs->ObjectPointerMap.size() == rhs->ObjectPointerMap.size())
	for(const auto& pair : lhs->ObjectPointerMap)
	{
		auto found = rhs->ObjectPointerMap.find(pair.first);
		B3D_TEST_ASSERT(found != rhs->ObjectPointerMap.end())

		if(pair.second != nullptr)
		{
			B3D_TEST_ASSERT(found->second != nullptr)
			B3D_TEST_ASSERT(*(pair.second) == *(found->second))
		}
		else
		{
			B3D_TEST_ASSERT(found->second == nullptr)
		}
	}

	if(!isDelta) // TODO Skipping these tests for deltas, as it's not a case it handled yet.
	{
		B3D_TEST_ASSERT(lhs->ObjectPointerMap.find("a") != lhs->ObjectPointerMap.end())
		B3D_TEST_ASSERT(lhs->ObjectPointerMap.find("f") != lhs->ObjectPointerMap.end())
		B3D_TEST_ASSERT(lhs->ObjectPointerMap["a"] == lhs->ObjectPointerMap["f"])

		B3D_TEST_ASSERT(rhs->ObjectPointerMap.find("a") != rhs->ObjectPointerMap.end())
		B3D_TEST_ASSERT(rhs->ObjectPointerMap.find("f") != rhs->ObjectPointerMap.end())
		B3D_TEST_ASSERT(rhs->ObjectPointerMap["a"] == rhs->ObjectPointerMap["f"])
	}
}

void CoreTestSuite::TestAnimCurveIntegration()
{
	static constexpr float EPSILON = 0.0001f;

	// Construct some curves
	TAnimationCurve<float> curveConstant(
		{
			TKeyframe<float>{ 1.0f, 0.0f, 0.0f, 0.0f },
		});

	TAnimationCurve<float> curveLinear(
		{ TKeyframe<float>{ 0.0f, 0.0f, 1.0f, 0.0f },
		  TKeyframe<float>{ 1.0f, 1.0f, 0.0f, 1.0f } });

	TAnimationCurve<float> curveAcceleration(
		{ TKeyframe<float>{ -9.81f, 0.0f, 0.0f, 0.0f },
		  TKeyframe<float>{ -9.81f, 0.0f, 0.0f, 10.0f } });

	{
		TCurveIntegrationCache<float> cache;
		B3D_TEST_ASSERT(Math::ApproxEquals(curveConstant.EvaluateIntegrated(0.0f, cache), 0.0f, EPSILON));
		B3D_TEST_ASSERT(Math::ApproxEquals(curveConstant.EvaluateIntegrated(0.5f, cache), 0.5f, EPSILON));
		B3D_TEST_ASSERT(Math::ApproxEquals(curveConstant.EvaluateIntegrated(1.0f, cache), 1.0f, EPSILON));
	}

	{
		TCurveIntegrationCache<float> cache;
		B3D_TEST_ASSERT(Math::ApproxEquals(curveLinear.EvaluateIntegrated(0.0f, cache), 0.0f, EPSILON));
		B3D_TEST_ASSERT(Math::ApproxEquals(curveLinear.EvaluateIntegrated(0.5f, cache), 0.125f, EPSILON));
		B3D_TEST_ASSERT(Math::ApproxEquals(curveLinear.EvaluateIntegrated(1.0f, cache), 0.5f, EPSILON));
	}

	{
		TCurveIntegrationCache<float> cache;

		float times[] = { 0.0f, 0.5f, 1.0f };
		for(auto time : times)
		{
			B3D_TEST_ASSERT(Math::ApproxEquals(curveConstant.EvaluateIntegratedDouble(time, cache), EvaluatePosition(1.0f, 0.0f, time), EPSILON));
		}
	}

	{
		TCurveIntegrationCache<float> cache;

		float times[] = { 0.0f, 0.5f, 1.0f, 2.0f, 3.0f, 5.0f, 10.0f };
		for(auto time : times)
		{
			B3D_TEST_ASSERT(Math::ApproxEquals(curveAcceleration.EvaluateIntegrated(time, cache), EvaluateVelocity(-9.81f, time), EPSILON));
		}

		std::pair<float, float> range = curveAcceleration.CalculateRangeIntegrated(cache);
		B3D_TEST_ASSERT(Math::ApproxEquals(range.first, -98.1f, EPSILON));
		B3D_TEST_ASSERT(Math::ApproxEquals(range.second, 0.0f, EPSILON));
	}

	{
		TCurveIntegrationCache<float> cache;

		float times[] = { 0.0f, 0.5f, 1.0f, 2.0f, 3.0f, 5.0f, 10.0f };
		for(auto time : times)
		{
			B3D_TEST_ASSERT(Math::ApproxEquals(curveAcceleration.EvaluateIntegratedDouble(time, cache), EvaluatePosition(-9.81f, 0.0f, time)));
		}

		std::pair<float, float> range = curveAcceleration.CalculateRangeIntegratedDouble(cache);
		B3D_TEST_ASSERT(Math::ApproxEquals(range.first, -490.5f, EPSILON));
		B3D_TEST_ASSERT(Math::ApproxEquals(range.second, 0.0f, EPSILON));
	}
}

void CoreTestSuite::TestLookupTable()
{
	static constexpr float EPSILON = 0.001f;

	TAnimationCurve<Vector3> curve({
		TKeyframe<Vector3>{ Vector3(0.0f, 0.0f, 0.0f), Vector3::kZero, Vector3::kOne, 0.0f },
		TKeyframe<Vector3>{ Vector3(5.0f, 3.0f, 10.0f), Vector3::kOne, Vector3::kZero, 10.0f },
	});

	Vector3Distribution dist = curve;
	auto lookupTable = dist.ToLookupTable(128);

	for(u32 i = 0; i < 10; i++)
	{
		const float* left;
		const float* right;
		float lerp;

		float t = (i / 9.0f) * 1.0f;
		lookupTable.Evaluate(t, left, right, lerp);

		Vector3* leftVec = (Vector3*)left;
		Vector3* rightVec = (Vector3*)right;

		Vector3 valueLookup = Vector3::Lerp(lerp, *leftVec, *rightVec);
		Vector3 valueCurve = curve.Evaluate(t);

		for(u32 j = 0; j < 3; j++)
			B3D_TEST_ASSERT(Math::ApproxEquals(valueLookup[j], valueCurve[j], EPSILON));
	}
}

void CoreTestSuite::TestBinarySerialization()
{
	const SPtr<UnitTestSerializationObjectA> object = CreateSerializationTestObjectVariantB();
	
	SPtr<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
	BinarySerializer serializer;
	serializer.Encode(object.get(), stream, BinarySerializerFlag::None);

	stream->Seek(0);

	const SPtr<UnitTestSerializationObjectA> deserializedObject = B3DRTTICast<UnitTestSerializationObjectA>(serializer.Decode(stream, (u32)stream->Size()));
	TestAssertObjectsMatch(object, deserializedObject);
}

void CoreTestSuite::TestSerializedObject()
{
	const SPtr<UnitTestSerializationObjectA> object = CreateSerializationTestObjectVariantB();

	const SPtr<SerializedObject> serializedObject = SerializedObject::Create(*object);
	const SPtr<UnitTestSerializationObjectA> deserializedObject = B3DRTTICast<UnitTestSerializationObjectA>(serializedObject->Decode());

	TestAssertObjectsMatch(object, deserializedObject);
}

void CoreTestSuite::TestBinaryDelta()
{
	const SPtr<UnitTestSerializationObjectA> objectA = CreateSerializationTestObjectVariantA();
	const SPtr<UnitTestSerializationObjectA> objectB = CreateSerializationTestObjectVariantB();

	const SPtr<SerializedObject> serializedObjectA = SerializedObject::Create(*objectA.get());
	const SPtr<SerializedObject> serializedObjectB = SerializedObject::Create(*objectB.get());

	IDeltaHandler& deltaHandler = objectA->GetRtti()->GetDeltaHandler();
	SPtr<SerializedObject> delta = deltaHandler.GenerateDelta(serializedObjectA, serializedObjectB);
	deltaHandler.ApplyDelta(objectA, delta, nullptr);

	TestAssertObjectsMatch(objectA, objectB, true);
}

/** Wrapper for easier scene creation and object access. */
struct Scene0Wrapper
{
	/** Populates the provided scene instance with the scene. */
	Scene0Wrapper(const SPtr<SceneInstance>& sceneInstance)
	{
		SceneObject_0 = sceneInstance->CreateSceneObject("Scene0_SceneObject_0");
		SceneObject_0_0 = sceneInstance->CreateSceneObject("Scene0_SceneObject_0_0");
		SceneObject_0_1 = sceneInstance->CreateSceneObject("Scene0_SceneObject_0_1");
		SceneObject_0_1_0 = sceneInstance->CreateSceneObject("Scene0_SceneObject_0_1_0");
		SceneObject_1 = sceneInstance->CreateSceneObject("Scene0_SceneObject_1");
		SceneObject_1_0 = sceneInstance->CreateSceneObject("Scene0_SceneObject_1_0");

		SceneObject_0_0->SetParent(SceneObject_0);
		SceneObject_0_1->SetParent(SceneObject_0);
		SceneObject_1_0->SetParent(SceneObject_1);
		SceneObject_0_1_0->SetParent(SceneObject_0_1);

		SceneObject_0_1_0->SetPosition(Vector3(10.0f, 15.0f, 20.0f));
		SceneObject_0_1->SetPosition(Vector3(1.0f, 2.0f, 3.0f));
		SceneObject_1_0->SetPosition(Vector3(0, 123.0f, 0.0f));

		Component_0 = SceneObject_0->AddComponent<UnitTestComponentA>();
		Component_1 = SceneObject_1->AddComponent<UnitTestComponentB>();
		Component_0_1 = SceneObject_0_1->AddComponent<UnitTestComponentA>();
		Component_0_1_0 = SceneObject_0_1_0->AddComponent<UnitTestComponentA>();
	}

	/** Populates the scene objects and components by looking them up in the provided hierarchy. */
	Scene0Wrapper(const HSceneObject& root)
	{
		SceneObject_0 = root->FindChild("Scene0_SceneObject_0", false);
		SceneObject_0_0 = SceneObject_0->FindChild("Scene0_SceneObject_0_0", false);
		SceneObject_0_1 = SceneObject_0->FindChild("Scene0_SceneObject_0_1", false);
		SceneObject_0_1_0 = SceneObject_0_1->FindChild("Scene0_SceneObject_0_1_0", false);
		SceneObject_1 = root->FindChild("Scene0_SceneObject_1", false);
		SceneObject_1_0 = SceneObject_1->FindChild("Scene0_SceneObject_1_0", false);

		Component_0 = SceneObject_0->GetComponent<UnitTestComponentA>();
		Component_1 = SceneObject_1->GetComponent<UnitTestComponentB>();
		Component_0_1 = SceneObject_0_1->GetComponent<UnitTestComponentA>();
		Component_0_1_0 = SceneObject_0_1_0->GetComponent<UnitTestComponentA>();
	}

	template<class T>
	void PerformSceneObjectUnaryOperation(T&& predicate)
	{
		predicate(SceneObject_0);
		predicate(SceneObject_0_0);
		predicate(SceneObject_0_1);
		predicate(SceneObject_0_1_0);
		predicate(SceneObject_1);
		predicate(SceneObject_1_0);
	}

	template<class T>
	void PerformComponentUnaryOperation(T&& predicate)
	{
		predicate(Component_0);
		predicate(Component_1);
		predicate(Component_0_1);
		predicate(Component_0_1_0);
	}

	template<class T>
	void PerformSceneObjectBinaryOperation(Scene0Wrapper& other, T&& predicate)
	{
		predicate(SceneObject_0, other.SceneObject_0);
		predicate(SceneObject_0_0, other.SceneObject_0_0);
		predicate(SceneObject_0_1, other.SceneObject_0_1);
		predicate(SceneObject_0_1_0, other.SceneObject_0_1_0);
		predicate(SceneObject_1, other.SceneObject_1);
		predicate(SceneObject_1_0, other.SceneObject_1_0);
	}

	template<class T>
	void PerformComponentBinaryOperation(Scene0Wrapper& other, T&& predicate)
	{
		predicate(Component_0, other.Component_0);
		predicate(Component_1, other.Component_1);
		predicate(Component_0_1, other.Component_0_1);
		predicate(Component_0_1_0, other.Component_0_1_0);
	}

	HSceneObject SceneObject_0;
	HSceneObject SceneObject_0_0;
	HSceneObject SceneObject_0_1;
	HSceneObject SceneObject_0_1_0;
	HSceneObject SceneObject_1;
	HSceneObject SceneObject_1_0;

	HUnitTestComponentA Component_0;
	HUnitTestComponentB Component_1;
	HUnitTestComponentA Component_0_1;
	HUnitTestComponentA Component_0_1_0;
};

/** Wrapper for easier scene creation and object access. */
struct Scene1Wrapper
{
	/** Populates the provided scene instance with the scene. */
	Scene1Wrapper(const SPtr<SceneInstance>& sceneInstance, const SPtr<Prefab>& childPrefab)
	{
		SceneObject_0 = sceneInstance->CreateSceneObject("Scene0_SceneObject_0");
		SceneObject_0_0_PrefabInstance = childPrefab->Instantiate(sceneInstance);
		SceneObject_1 = sceneInstance->CreateSceneObject("Scene0_SceneObject_1");
		SceneObject_1_0 = sceneInstance->CreateSceneObject("Scene0_SceneObject_1_0");

		SceneObject_0_0_PrefabInstance->SetParent(SceneObject_0);
		SceneObject_1_0->SetParent(SceneObject_1);

		SceneObject_0->SetPosition(Vector3(50.0f, 50.0f, 50.0f));
		SceneObject_0_0_PrefabInstance->SetPosition(Vector3(0.0f, 0.0f, 0.0f));

		Component_1_0 = SceneObject_1_0->AddComponent<UnitTestComponentA>();
	}

	/** Populates the scene objects and components by looking them up in the provided hierarchy. */
	Scene1Wrapper(const HSceneObject& root)
	{
		SceneObject_0 = root->FindChild("Scene0_SceneObject_0", false);
		SceneObject_0_0_PrefabInstance = SceneObject_0->GetChild(0);
		SceneObject_1 = root->FindChild("Scene0_SceneObject_1", false);
		SceneObject_1_0 = SceneObject_1->FindChild("Scene0_SceneObject_1_0", false);

		Component_1_0 = SceneObject_1_0->GetComponent<UnitTestComponentA>();
	}

	HSceneObject SceneObject_0;
	HSceneObject SceneObject_0_0_PrefabInstance;
	HSceneObject SceneObject_1;
	HSceneObject SceneObject_1_0;

	HUnitTestComponentA Component_1_0;
};

void CoreTestSuite::TestSceneSaveLoad()
{
	// Create & serialize scene 0
	SPtr<MemoryDataStream> serializedScene0Stream = B3DMakeShared<MemoryDataStream>();
	{
		SPtr<SceneInstance> scene0Instance = SceneInstance::Create("UnitTestScene0Instance");
		Scene0Wrapper scene0Wrapper(scene0Instance);

		scene0Wrapper.Component_0->SceneObjectReference = scene0Wrapper.SceneObject_0_1_0;
		scene0Wrapper.Component_0->ComponentReference = scene0Wrapper.Component_1;

		scene0Wrapper.Component_0_1->StringValue = "originalValue2";

		scene0Wrapper.Component_0_1_0->StringValue = "testValue";
		scene0Wrapper.Component_0_1_0->SceneObjectReference = scene0Wrapper.SceneObject_0;
		scene0Wrapper.Component_0_1_0->ComponentReference = scene0Wrapper.Component_0;

		HPrefab scene0prefab = Prefab::Create(scene0Instance->GetRoot());

		BinarySerializer serializer;
		serializer.Encode(scene0prefab.Get(), serializedScene0Stream, BinarySerializerFlag::None);
	}

	// Deserialize scene 0 and verify object references remain valid
	SPtr<Prefab> scene0Prefab;
	{
		serializedScene0Stream->Seek(0);

		BinarySerializer serializer;
		CoreSerializationContext serializationContext;
		scene0Prefab = B3DRTTICast<Prefab>(serializer.Decode(serializedScene0Stream, (u32)serializedScene0Stream->Size(), BinarySerializerFlag::None, &serializationContext));

		Scene0Wrapper scene0Wrapper(scene0Prefab->GetRoot());

		B3D_TEST_ASSERT(scene0Wrapper.Component_0_1_0->StringValue == "testValue")
		B3D_TEST_ASSERT(scene0Wrapper.Component_0->SceneObjectReference == scene0Wrapper.SceneObject_0_1_0)
		B3D_TEST_ASSERT(scene0Wrapper.Component_0->ComponentReference == scene0Wrapper.Component_1)
		B3D_TEST_ASSERT(scene0Wrapper.Component_0_1_0->SceneObjectReference == scene0Wrapper.SceneObject_0)
		B3D_TEST_ASSERT(scene0Wrapper.Component_0_1_0->ComponentReference == scene0Wrapper.Component_0)
	}

	// Instantiate the scene and ensure prefab links are valid
	{
		SPtr<SceneInstance> scene0Instance = SceneInstance::Create("UnitTestScene0Instance");
		HSceneObject instancedSceneRoot = scene0Prefab->Instantiate(scene0Instance);
		Scene0Wrapper instancedScene0Wrapper(instancedSceneRoot);

		Scene0Wrapper prefabScene0Wrapper(scene0Prefab->GetRoot());

		instancedScene0Wrapper.PerformSceneObjectBinaryOperation(prefabScene0Wrapper,
			[this, scene0PrefabId = scene0Prefab->GetId()] (const HSceneObject& instanceSceneObject, const HSceneObject& prefabSceneObject) {
			// Instance should be linked to the prefab
			 B3D_TEST_ASSERT(!instanceSceneObject->GetPrefabObjectId().Empty())
			 B3D_TEST_ASSERT(instanceSceneObject->GetPrefabObjectId() == prefabSceneObject->GetId())
			 B3D_TEST_ASSERT(instanceSceneObject->GetPrefabResourceId() == scene0PrefabId)

			 // Prefab hierarchy should have no links
			 B3D_TEST_ASSERT(prefabSceneObject->GetPrefabObjectId().Empty())
			 B3D_TEST_ASSERT(prefabSceneObject->GetPrefabResourceId().Empty())
		});

		instancedScene0Wrapper.PerformComponentBinaryOperation(prefabScene0Wrapper,
			[this, scene0PrefabId = scene0Prefab->GetId()] (const HComponent& instanceComponent, const HComponent& prefabComponent) {
			// Instance should be linked to the prefab
			 B3D_TEST_ASSERT(!instanceComponent->GetPrefabObjectId().Empty())
			 B3D_TEST_ASSERT(instanceComponent->GetPrefabObjectId() == prefabComponent->GetId())

			 // Prefab hierarchy should have no links
			 B3D_TEST_ASSERT(prefabComponent->GetPrefabObjectId().Empty())
		});
	}

	// Create and serialize scene 1 that contains an instance of the above prefab, and they reference eachother objects
	SPtr<MemoryDataStream> serializedScene1Stream = B3DMakeShared<MemoryDataStream>();
	{
		SPtr<SceneInstance> scene1Instance = SceneInstance::Create("UnitTestScene1Instance");

		Scene1Wrapper scene1Wrapper(scene1Instance, scene0Prefab);
		Scene0Wrapper scene0Wrapper(scene1Wrapper.SceneObject_0_0_PrefabInstance);
		HPrefab scene1prefab = Prefab::Create(scene1Instance->GetRoot());

		scene0Wrapper.Component_0_1_0->SceneObjectReference = scene1Wrapper.SceneObject_1_0;
		scene0Wrapper.Component_0_1_0->ComponentReference = scene1Wrapper.Component_1_0;

		scene1Wrapper.Component_1_0->SceneObjectReference = scene0Wrapper.SceneObject_1_0;
		scene1Wrapper.Component_1_0->ComponentReference = scene0Wrapper.Component_0_1_0;

		BinarySerializer serializer;
		serializer.Encode(scene1prefab.Get(), serializedScene1Stream, BinarySerializerFlag::None);
	}

	// Deserialize scene 1 and ensure references remain valid
	SPtr<Prefab> scene1Prefab;
	{
		serializedScene1Stream->Seek(0);

		BinarySerializer serializer;
		CoreSerializationContext serializationContext;
		scene1Prefab = B3DRTTICast<Prefab>(serializer.Decode(serializedScene1Stream, (u32)serializedScene1Stream->Size(), BinarySerializerFlag::None, &serializationContext));

		Scene1Wrapper scene1Wrapper(scene1Prefab->GetRoot());
		Scene0Wrapper scene0Wrapper(scene1Wrapper.SceneObject_0_0_PrefabInstance);

		B3D_TEST_ASSERT(scene0Wrapper.Component_0_1_0->SceneObjectReference = scene1Wrapper.SceneObject_1_0)
		B3D_TEST_ASSERT(scene0Wrapper.Component_0_1_0->ComponentReference = scene1Wrapper.Component_1_0)

		B3D_TEST_ASSERT(scene1Wrapper.Component_1_0->SceneObjectReference = scene0Wrapper.SceneObject_1_0)
		B3D_TEST_ASSERT(scene1Wrapper.Component_1_0->ComponentReference = scene0Wrapper.Component_0_1_0)



		// TODO - Should also verify prefab links are correct (both within the prefab, and in the instantiated hierarchy)
		
	}

	// TODO - Add following tests:
	// 1. [x] Instantiate the above prefab and verify prefab links are valid
	// 2. [x] Create a new scene into which we instantiate the above prefab (See example below)
	//  - [x] Have the prefab reference some of the scene's objects, and ensure those are valid
	//  - [ ] Ensure scene objects have correct prefab links, and the prefab itself does
	// 3. [ ] Update the prefab, verify update has been applied correctly in the scene
	// 4. [ ] Make an instance specific change in the scene, verify the instance specific change persists
	// 5. TODO - Nested prefab test

	//// Modify prefab, reload the scene and ensure it is updated with modified prefab
	//{
	//	{
	//		// unitTest4Scene_0.prefab:
	//		// so0
	//		//  - so0_1 (Comp1)
	//		//    - so0_1_0 (Comp1)
	//		// so1 (Comp1, Comp2)
	//		//  - so1_0
	//		//  - so1_1

	//		Scene.Load("unitTest4Scene_0.prefab");

	//		HSceneObject sceneRoot = Scene.Root;
	//		HSceneObject so0 = sceneRoot.FindChild("so0", false);
	//		HSceneObject so0_0 = so0.FindChild("so0_0", false);
	//		HSceneObject so0_1 = so0.FindChild("so0_1", false);
	//		HSceneObject so1 = sceneRoot.FindChild("so1", false);
	//		HSceneObject so1_0 = so1.FindChild("so1_0", false);
	//		HSceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);

	//		HSceneObject so1_1 = sceneInstance->CreateSceneObject("so1_1");
	//		so1_1.Parent = so1;

	//		so0.RemoveComponent<UnitTestComponentA>();
	//		UnitTestComponentA comp1 = so1->AddComponent<UnitTestComponentA>();
	//		UnitTestComponentA comp0_1_0 = so0_1_0->GetComponent<UnitTestComponentA>();

	//		so0_0.Destroy();

	//		comp1.otherSO = so1_0;
	//		comp1.otherComponent2 = comp0_1_0;

	//		comp0_1_0.otherSO = so1_1;
	//		comp0_1_0.otherComponent2 = comp1;
	//		comp0_1_0.a = 123;
	//		comp0_1_0.b = "modifiedValue";

	//		so1.Name = "so1_modified";
	//		so1.LocalPosition = new Vector3(0, 999.0f, 0.0f);

	//		EditorApplication.SaveScene("unitTest4Scene_0.prefab");
	//	}

	//	{
	//		EditorApplication.LoadScene("unitTest4Scene_1.prefab");

	//		HSceneObject parentSO0 = Scene.Root.FindChild("parentSO0", false);
	//		HSceneObject parentSO1 = Scene.Root.FindChild("parentSO1", false);
	//		HSceneObject parentSO1_0 = parentSO1.FindChild("parentSO1_0", false);

	//		UnitTestComponentA parentComp1_0 = parentSO1_0->GetComponent<UnitTestComponentA>();

	//		HSceneObject prefabInstance = parentSO0.GetChild(0);
	//		HSceneObject so0 = prefabInstance.FindChild("so0", false);
	//		HSceneObject so1 = prefabInstance.FindChild("so1_modified", false);
	//		HSceneObject so0_0 = so0.FindChild("so0_0", false);
	//		HSceneObject so0_1 = so0.FindChild("so0_1", false);
	//		HSceneObject so1_0 = so1.FindChild("so1_0", false);
	//		HSceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);
	//		HSceneObject so1_1 = so1.FindChild("so1_1", false);

	//		UnitTestComponentA comp0 = so0->GetComponent<UnitTestComponentA>();
	//		UnitTestComponentA comp1 = so1->GetComponent<UnitTestComponentA>();
	//		UnitTestComponentA comp0_1_0 = so0_1_0->GetComponent<UnitTestComponentA>();

	//		B3D_TEST_ASSERT(parentComp1_0.otherSO == so1_0);
	//		B3D_TEST_ASSERT(parentComp1_0.otherComponent2 == comp0_1_0);
	//		B3D_TEST_ASSERT(so1_1 != nullptr);
	//		B3D_TEST_ASSERT(so0_0 == nullptr);
	//		B3D_TEST_ASSERT(comp0 == nullptr);
	//		B3D_TEST_ASSERT(comp0_1_0.otherSO == so1_1);
	//		B3D_TEST_ASSERT(comp0_1_0.otherComponent2 == comp1);
	//		B3D_TEST_ASSERT(comp0_1_0.a == 123);
	//		B3D_TEST_ASSERT(comp0_1_0.b == "modifiedValue");
	//		B3D_TEST_ASSERT(comp1.otherSO == so1_0);
	//		B3D_TEST_ASSERT(comp1.otherComponent2 == comp0_1_0);
	//		B3D_TEST_ASSERT(MathEx.ApproxEquals(so1.LocalPosition.y, 999.0f));
	//	}
	//}

	//Debug.Log("Passed stage 3");

	//// Make instance specific changes to the prefab, modify the prefab itself and ensure
	//// both changes persist
	//{
	//	// Create new scene referencing the prefab and make instance modifications
	//	{
	//		// unitTest4Scene_2.prefab:
	//		// parent2SO0
	//		//  - [unitTest4Scene_0.prefab]
	//		// parent2SO1
	//		//  - parent2SO1_0 (Comp1)

	//		// unitTest4Scene_0.prefab (unitTest4Scene_2.prefab instance):
	//		// so0 (Comp1(INSTANCE))
	//		//  - so0_0 (INSTANCE)
	//		//  - so0_1 (Comp1)
	//		//    - so0_1_0 (Comp1)
	//		// so1 (Comp2)
	//		//  - so1_0

	//		Scene.Clear();

	//		HSceneObject parent2SO0 = sceneInstance->CreateSceneObject("parent2SO0");
	//		HSceneObject parent2SO1 = sceneInstance->CreateSceneObject("parent2SO1");
	//		HSceneObject parent2SO1_0 = sceneInstance->CreateSceneObject("parent2SO1_0");

	//		parent2SO1_0.Parent = parent2SO1;

	//		UnitTestComponentA parentComp1_0 = parent2SO1_0->AddComponent<UnitTestComponentA>();

	//		Prefab scene0Prefab = ProjectLibrary.Load<Prefab>("unitTest4Scene_0.prefab");
	//		HSceneObject prefabInstance = scene0Prefab.Instantiate();
	//		prefabInstance.Parent = parent2SO0;

	//		HSceneObject so0 = prefabInstance.FindChild("so0", false);
	//		HSceneObject so1 = prefabInstance.FindChild("so1_modified", false);

	//		HSceneObject so0_1 = so0.FindChild("so0_1", false);
	//		HSceneObject so1_0 = so1.FindChild("so1_0", false);
	//		HSceneObject so1_1 = so1.FindChild("so1_1", false);
	//		HSceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);

	//		UnitTestComponentB comp1 = so1->GetComponent<UnitTestComponentB>();
	//		UnitTestComponentA comp0_1_0 = so0_1_0->GetComponent<UnitTestComponentA>();
	//		UnitTestComponentA comp0_1 = so0_1->GetComponent<UnitTestComponentA>();

	//		HSceneObject so0_0 = sceneInstance->CreateSceneObject("so0_0");
	//		so0_0.Parent = so0;
	//		UnitTestComponentA comp0 = so0->AddComponent<UnitTestComponentA>();

	//		so1.RemoveComponent<UnitTestComponentA>();
	//		so1_1.Destroy();

	//		comp0.otherSO = so0_1_0;
	//		comp0.otherComponent = comp1;

	//		parentComp1_0.otherSO = so1_0;
	//		parentComp1_0.otherComponent2 = comp0_1_0;

	//		comp0_1_0.otherSO = parent2SO1_0;
	//		comp0_1_0.otherComponent2 = parentComp1_0;
	//		comp0_1_0.b = "instanceValue";

	//		comp0_1.b = "instanceValue2";

	//		EditorApplication.SaveScene("unitTest4Scene_2.prefab");
	//	}

	//	Debug.Log("Passed stage 4.1");

	//	// Reload the scene and ensure instance modifications remain
	//	{
	//		EditorApplication.LoadScene("unitTest4Scene_2.prefab");

	//		HSceneObject root = Scene.Root;
	//		HSceneObject parent2SO0 = root.FindChild("parent2SO0", false);
	//		HSceneObject parent2SO1 = root.FindChild("parent2SO1", false);
	//		HSceneObject parent2SO1_0 = parent2SO1.FindChild("parent2SO1_0", false);

	//		HSceneObject prefabInstance = parent2SO0.GetChild(0);

	//		HSceneObject so0 = prefabInstance.FindChild("so0", false);
	//		HSceneObject so1 = prefabInstance.FindChild("so1_modified", false);
	//		HSceneObject so0_0 = so0.FindChild("so0_0", false);
	//		HSceneObject so0_1 = so0.FindChild("so0_1", false);
	//		HSceneObject so1_0 = so1.FindChild("so1_0", false);
	//		HSceneObject so1_1 = so1.FindChild("so1_1", false);
	//		HSceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);

	//		UnitTestComponentA parentComp1_0 = parent2SO1_0->GetComponent<UnitTestComponentA>();
	//		UnitTestComponentA comp0 = so0->GetComponent<UnitTestComponentA>();
	//		UnitTestComponentB comp1 = so1->GetComponent<UnitTestComponentB>();
	//		UnitTestComponentA comp11 = so1->GetComponent<UnitTestComponentA>();
	//		UnitTestComponentA comp0_1_0 = so0_1_0->GetComponent<UnitTestComponentA>();
	//		UnitTestComponentA comp0_1 = so0_1->GetComponent<UnitTestComponentA>();

	//		B3D_TEST_ASSERT(so0_0 != nullptr);
	//		B3D_TEST_ASSERT(comp0 != nullptr);
	//		B3D_TEST_ASSERT(so1_1 == nullptr);
	//		B3D_TEST_ASSERT(comp11 == nullptr);

	//		B3D_TEST_ASSERT(comp0.otherSO == so0_1_0);
	//		B3D_TEST_ASSERT(comp0.otherComponent == comp1);

	//		B3D_TEST_ASSERT(parentComp1_0.otherSO == so1_0);
	//		B3D_TEST_ASSERT(parentComp1_0.otherComponent2 == comp0_1_0);

	//		B3D_TEST_ASSERT(comp0_1_0.otherSO == parent2SO1_0);
	//		B3D_TEST_ASSERT(comp0_1_0.otherComponent2 == parentComp1_0);
	//		B3D_TEST_ASSERT(comp0_1_0.b == "instanceValue");

	//		B3D_TEST_ASSERT(comp0_1.b == "instanceValue2");
	//	}

	//	Debug.Log("Passed stage 4.2");

	//	// Load original scene and ensure instance modifications didn't influence it
	//	{
	//		EditorApplication.LoadScene("unitTest4Scene_1.prefab");

	//		HSceneObject parentSO0 = Scene.Root.FindChild("parentSO0", false);
	//		HSceneObject parentSO1 = Scene.Root.FindChild("parentSO1", false);
	//		HSceneObject parentSO1_0 = parentSO1.FindChild("parentSO1_0", false);

	//		UnitTestComponentA parentComp1_0 = parentSO1_0->GetComponent<UnitTestComponentA>();

	//		HSceneObject prefabInstance = parentSO0.GetChild(0);
	//		HSceneObject so0 = prefabInstance.FindChild("so0", false);
	//		HSceneObject so1 = prefabInstance.FindChild("so1_modified", false);
	//		HSceneObject so0_0 = so0.FindChild("so0_0", false);
	//		HSceneObject so0_1 = so0.FindChild("so0_1", false);
	//		HSceneObject so1_0 = so1.FindChild("so1_0", false);
	//		HSceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);
	//		HSceneObject so1_1 = so1.FindChild("so1_1", false);

	//		UnitTestComponentA comp0 = so0->GetComponent<UnitTestComponentA>();
	//		UnitTestComponentA comp1 = so1->GetComponent<UnitTestComponentA>();
	//		UnitTestComponentA comp0_1_0 = so0_1_0->GetComponent<UnitTestComponentA>();
	//		UnitTestComponentA comp0_1 = so0_1->GetComponent<UnitTestComponentA>();

	//		B3D_TEST_ASSERT(parentComp1_0.otherSO == so1_0);
	//		B3D_TEST_ASSERT(parentComp1_0.otherComponent2 == comp0_1_0);
	//		B3D_TEST_ASSERT(so1_1 != nullptr);
	//		B3D_TEST_ASSERT(so0_0 == nullptr);
	//		B3D_TEST_ASSERT(comp0 == nullptr);
	//		B3D_TEST_ASSERT(comp0_1_0.otherSO == so1_1);
	//		B3D_TEST_ASSERT(comp0_1_0.otherComponent2 == comp1);
	//		B3D_TEST_ASSERT(comp0_1_0.a == 123);
	//		B3D_TEST_ASSERT(comp0_1_0.b == "modifiedValue");
	//		B3D_TEST_ASSERT(comp1.otherSO == so1_0);
	//		B3D_TEST_ASSERT(comp1.otherComponent2 == comp0_1_0);
	//		B3D_TEST_ASSERT(comp0_1.b == "originalValue2");
	//		B3D_TEST_ASSERT(MathEx.ApproxEquals(so1.LocalPosition.y, 999.0f));
	//	}

	//	Debug.Log("Passed stage 4.3");

	//	// Modify prefab and ensure both prefab and instance modifications remain
	//	{
	//		// unitTest4Scene_0.prefab:
	//		// so0 (Comp2)
	//		//  - so0_1
	//		//    - so0_1_0 (Comp1)
	//		// so1 (Comp1, Comp2)
	//		//  - so1_1
	//		//  - so1_2 (Comp1)

	//		// unitTest4Scene_0.prefab (unitTest4Scene_2.prefab instance):
	//		// so0 (Comp1)
	//		//  - so0_0
	//		//  - so0_1 (Comp1)
	//		//    - so0_1_0 (Comp1)
	//		// so1 (Comp2)
	//		//  - so1_2 (Comp1)

	//		Scene.Load("unitTest4Scene_0.prefab");

	//		HSceneObject sceneRoot = Scene.Root;
	//		HSceneObject so0 = sceneRoot.FindChild("so0", false);
	//		HSceneObject so0_1 = so0.FindChild("so0_1", false);
	//		HSceneObject so1 = sceneRoot.FindChild("so1_modified", false);
	//		HSceneObject so1_0 = so1.FindChild("so1_0", false);
	//		HSceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);

	//		HSceneObject so1_2 = sceneInstance->CreateSceneObject("so1_2");
	//		so1_2.Parent = so1;

	//		so0->AddComponent<UnitTestComponentB>();
	//		so0_1.RemoveComponent<UnitTestComponentA>();
	//		so1_0.Destroy();

	//		UnitTestComponentA comp3 = so1_2->AddComponent<UnitTestComponentA>();
	//		UnitTestComponentA comp0_1_0 = so0_1_0->GetComponent<UnitTestComponentA>();
	//		comp0_1_0.b = "modifiedValueAgain";
	//		so1.Name = "so1_modifiedAgain";

	//		comp3.otherSO = so0_1;
	//		comp3.otherComponent2 = comp0_1_0;

	//		EditorApplication.SaveScene("unitTest4Scene_0.prefab");
	//	}

	//	Debug.Log("Passed stage 4.4");

	//	// Reload the scene and ensure both instance and prefab modifications remain
	//	{
	//		EditorApplication.LoadScene("unitTest4Scene_2.prefab");

	//		HSceneObject root = Scene.Root;
	//		HSceneObject parent2SO0 = root.FindChild("parent2SO0", false);
	//		HSceneObject parent2SO1 = root.FindChild("parent2SO1", false);
	//		HSceneObject parent2SO1_0 = parent2SO1.FindChild("parent2SO1_0", false);

	//		HSceneObject prefabInstance = parent2SO0.GetChild(0);

	//		HSceneObject so0 = prefabInstance.FindChild("so0", false);
	//		HSceneObject so1 = prefabInstance.FindChild("so1_modifiedAgain", false);
	//		HSceneObject so0_0 = so0.FindChild("so0_0", false);
	//		HSceneObject so0_1 = so0.FindChild("so0_1", false);
	//		HSceneObject so1_0 = so1.FindChild("so1_0", false);
	//		HSceneObject so1_1 = so1.FindChild("so1_1", false);
	//		HSceneObject so1_2 = so1.FindChild("so1_2", false);
	//		HSceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);

	//		UnitTestComponentA parentComp1_0 = parent2SO1_0->GetComponent<UnitTestComponentA>();
	//		UnitTestComponentA comp0 = so0->GetComponent<UnitTestComponentA>();
	//		UnitTestComponentB comp1 = so1->GetComponent<UnitTestComponentB>();
	//		UnitTestComponentA comp11 = so1->GetComponent<UnitTestComponentA>();
	//		UnitTestComponentA comp0_1_0 = so0_1_0->GetComponent<UnitTestComponentA>();
	//		UnitTestComponentA comp3 = so1_2->GetComponent<UnitTestComponentA>();

	//		// Check instance modifications (they should override any prefab modifications)
	//		B3D_TEST_ASSERT(so0_0 != nullptr);
	//		B3D_TEST_ASSERT(comp0 != nullptr);
	//		B3D_TEST_ASSERT(so1_1 == nullptr);
	//		B3D_TEST_ASSERT(comp11 == nullptr);

	//		B3D_TEST_ASSERT(comp0.otherSO == so0_1_0);
	//		B3D_TEST_ASSERT(comp0.otherComponent == comp1);

	//		B3D_TEST_ASSERT(parentComp1_0.otherSO == so1_0);
	//		B3D_TEST_ASSERT(parentComp1_0.otherComponent2 == comp0_1_0);

	//		B3D_TEST_ASSERT(comp0_1_0.otherSO == parent2SO1_0);
	//		B3D_TEST_ASSERT(comp0_1_0.otherComponent2 == parentComp1_0);
	//		B3D_TEST_ASSERT(comp0_1_0.b == "instanceValue");

	//		// Check prefab modifications
	//		B3D_TEST_ASSERT(so1_0 == nullptr);
	//		B3D_TEST_ASSERT(so1.Name == "so1_modifiedAgain");

	//		B3D_TEST_ASSERT(comp3.otherSO == so0_1);
	//		B3D_TEST_ASSERT(comp3.otherComponent2 == comp0_1_0);
	//	}

	//	Debug.Log("Passed stage 4.5");
	//}
}

using namespace bs;

int main()
{
	VideoMode videoMode(1280, 720);
	Application::StartUp(videoMode, "UnitTests", false);

	SPtr<TestSuite> tests = CoreTestSuite::Create<CoreTestSuite>();

	ExceptionTestOutput testOutput;
	tests->Run(testOutput);

	Application::ShutDown();

	return 0;
}
