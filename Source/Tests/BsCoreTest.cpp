//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/BsConsoleTestOutput.h"
#include "Testing/BsTestSuite.h"
#include "Animation/BsAnimationCurve.h"
#include "Particles/BsParticleDistribution.h"
#include "RTTI/BsStringRTTI.h"
#include "Serialization/BsBinarySerializer.h"
#include "FileSystem/BsDataStream.h"

using namespace bs;

struct UnitTestSerializationObjectB : IReflectable
{
	u32 IntA = 100;
	String StrA = "100";

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

	Vector<String> ArrStrA;
	Vector<String> ArrStrB;
	Vector<String> ArrStrC;

	Vector<UnitTestSerializationObjectB> ArrObjA;
	Vector<UnitTestSerializationObjectB> ArrObjB;

	Vector<SPtr<UnitTestSerializationObjectB>> ArrObjPtrA;
	Vector<SPtr<UnitTestSerializationObjectB>> ArrObjPtrB;

	/************************************************************************/
	/* 								RTTI		                     		*/
	/************************************************************************/
public:
	friend class UnitTestSerializationObjectARTTI;
	static RTTITypeBase* GetRttiStatic();
	RTTITypeBase* GetRtti() const override;
};

class UnitTestSerializationObjectARTTI : public RTTIType<UnitTestSerializationObjectA, IReflectable, UnitTestSerializationObjectARTTI>
{
private:
	B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_MEMBER_PLAIN(IntA, 0)
		B3D_RTTI_MEMBER_PLAIN(StrA, 1)
		B3D_RTTI_MEMBER_PLAIN(StrB, 2)

		B3D_RTTI_MEMBER_REFL(ObjA, 3)
		B3D_RTTI_MEMBER_REFL(ObjB, 4)

		B3D_RTTI_MEMBER_REFLPTR(ObjPtrA, 5)
		B3D_RTTI_MEMBER_REFLPTR(ObjPtrB, 6)
		B3D_RTTI_MEMBER_REFLPTR(ObjPtrC, 7)
		B3D_RTTI_MEMBER_REFLPTR(ObjPtrD, 8)

		B3D_RTTI_MEMBER_PLAIN_ARRAY(ArrStrA, 9)
		B3D_RTTI_MEMBER_PLAIN_ARRAY(ArrStrB, 10)
		B3D_RTTI_MEMBER_PLAIN_ARRAY(ArrStrC, 11)

		B3D_RTTI_MEMBER_REFL_ARRAY(ArrObjA, 12)
		B3D_RTTI_MEMBER_REFL_ARRAY(ArrObjB, 13)

		B3D_RTTI_MEMBER_REFLPTR_ARRAY(ArrObjPtrA, 14)
		B3D_RTTI_MEMBER_REFLPTR_ARRAY(ArrObjPtrB, 15)
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
		B3D_RTTI_MEMBER_PLAIN(IntA, 0)
		B3D_RTTI_MEMBER_PLAIN(StrA, 1)
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

	void AssertObjectsMatch(const SPtr<UnitTestSerializationObjectA>& lhs, const SPtr<UnitTestSerializationObjectA>& rhs);

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

	// TODO - Add unit tests for:
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
	object->ArrObjB[1].StrA = "strawberry";
	object->ArrObjPtrB[0]->IntA = 99100;

	return object;
}

void CoreTestSuite::AssertObjectsMatch(const SPtr<UnitTestSerializationObjectA>& lhs, const SPtr<UnitTestSerializationObjectA>& rhs)
{
	B3D_TEST_ASSERT(lhs->IntA == rhs->IntA)
	B3D_TEST_ASSERT(lhs->StrA == rhs->StrA)
	B3D_TEST_ASSERT(lhs->StrB == rhs->StrB)

	B3D_TEST_ASSERT(lhs->ObjA.IntA == rhs->ObjA.IntA)
	B3D_TEST_ASSERT(lhs->ObjB.IntA == rhs->ObjB.IntA)

	B3D_TEST_ASSERT(lhs->ObjPtrA->StrA == rhs->ObjPtrA->StrA)
	B3D_TEST_ASSERT(lhs->ObjPtrB->StrA == rhs->ObjPtrB->StrA)
	B3D_TEST_ASSERT(lhs->ObjPtrD->StrA == rhs->ObjPtrD->StrA)
	B3D_TEST_ASSERT(lhs->ObjPtrC == rhs->ObjPtrC);

	B3D_TEST_ASSERT(lhs->ArrStrA.size() == rhs->ArrStrA.size())
	for(u32 i = 0; i < (u32)lhs->ArrStrA.size(); i++)
		B3D_TEST_ASSERT(lhs->ArrStrA[i] == rhs->ArrStrA[i])

	B3D_TEST_ASSERT(lhs->ArrStrB.size() == rhs->ArrStrB.size())
	for(u32 i = 0; i < (u32)lhs->ArrStrB.size(); i++)
		B3D_TEST_ASSERT(lhs->ArrStrB[i] == rhs->ArrStrB[i])

	B3D_TEST_ASSERT(lhs->ArrStrC.size() == rhs->ArrStrC.size())
	for(u32 i = 0; i < (u32)lhs->ArrStrC.size(); i++)
		B3D_TEST_ASSERT(lhs->ArrStrC[i] == rhs->ArrStrC[i])

	B3D_TEST_ASSERT(lhs->ArrObjA.size() == rhs->ArrObjA.size())
	for(u32 i = 0; i < (u32)lhs->ArrObjA.size(); i++)
		B3D_TEST_ASSERT(lhs->ArrObjA[i].StrA == rhs->ArrObjA[i].StrA)

	B3D_TEST_ASSERT(lhs->ArrObjB.size() == rhs->ArrObjB.size())
	for(u32 i = 0; i < (u32)lhs->ArrObjB.size(); i++)
		B3D_TEST_ASSERT(lhs->ArrObjB[i].StrA == rhs->ArrObjB[i].StrA)

	B3D_TEST_ASSERT(lhs->ArrObjPtrA.size() == rhs->ArrObjPtrA.size())
	for(u32 i = 0; i < (u32)lhs->ArrObjPtrA.size(); i++)
		B3D_TEST_ASSERT(lhs->ArrObjPtrA[i]->IntA == rhs->ArrObjPtrA[i]->IntA)

	B3D_TEST_ASSERT(lhs->ArrObjPtrB.size() == rhs->ArrObjPtrB.size())
	for(u32 i = 0; i < (u32)lhs->ArrObjPtrB.size(); i++)
		B3D_TEST_ASSERT(lhs->ArrObjPtrB[i]->IntA == rhs->ArrObjPtrB[i]->IntA)
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
	AssertObjectsMatch(object, deserializedObject);
}

void CoreTestSuite::TestSerializedObject()
{
	const SPtr<UnitTestSerializationObjectA> object = CreateSerializationTestObjectVariantB();

	const SPtr<SerializedObject> serializedObject = SerializedObject::Create(*object);
	const SPtr<UnitTestSerializationObjectA> deserializedObject = B3DRTTICast<UnitTestSerializationObjectA>(serializedObject->Decode());

	AssertObjectsMatch(object, deserializedObject);
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

	AssertObjectsMatch(objectA, objectB);
}

using namespace bs;

int main()
{
	MemStack::BeginThread();

	SPtr<TestSuite> tests = CoreTestSuite::Create<CoreTestSuite>();

	ExceptionTestOutput testOutput;
	tests->Run(testOutput);

	MemStack::EndThread();
	return 0;
}
