//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/BsConsoleTestOutput.h"
#include "Testing/BsTestSuite.h"
#include "Animation/BsAnimationCurve.h"
#include "Particles/BsParticleDistribution.h"
#include "RTTI/BsStringRTTI.h"

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
	void TestBinaryDelta();
};

CoreTestSuite::CoreTestSuite()
{
	B3D_ADD_TEST(CoreTestSuite::TestAnimCurveIntegration)
	B3D_ADD_TEST(CoreTestSuite::TestLookupTable)
	B3D_ADD_TEST(CoreTestSuite::TestBinaryDelta)

	// TODO - Add unit tests for:
	// - Binary serialization/deserialization
	// - Intermediate serialization/deserialization
	// - SceneObject/Component serialization/deserialization
	// - Various Prefab operations
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

void CoreTestSuite::TestBinaryDelta()
{
	SPtr<UnitTestSerializationObjectA> orgObj = B3DMakeShared<UnitTestSerializationObjectA>();
	SPtr<UnitTestSerializationObjectA> newObj = B3DMakeShared<UnitTestSerializationObjectA>();

	newObj->IntA = 995;
	newObj->StrA = "potato";
	newObj->ArrStrB = { "orange", "carrot" };
	newObj->ArrStrC[2] = "banana";
	newObj->ObjB.IntA = 9940;
	newObj->ObjPtrB->StrA = "kiwi";
	newObj->ObjPtrC = nullptr;
	newObj->ObjPtrD = B3DMakeShared<UnitTestSerializationObjectB>();
	newObj->ArrObjB[1].StrA = "strawberry";
	newObj->ArrObjPtrB[0]->IntA = 99100;

	SPtr<SerializedObject> orgSerialized = SerializedObject::Create(*orgObj.get());
	SPtr<SerializedObject> newSerialized = SerializedObject::Create(*newObj.get());

	IDiff& diffHandler = orgObj->GetRtti()->GetDiffHandler();
	SPtr<SerializedObject> objDiff = diffHandler.GenerateDiff(orgSerialized, newSerialized);
	diffHandler.ApplyDiff(orgObj, objDiff, nullptr);

	B3D_TEST_ASSERT(orgObj->IntA == newObj->IntA);
	B3D_TEST_ASSERT(orgObj->StrA == newObj->StrA);
	B3D_TEST_ASSERT(orgObj->StrB == newObj->StrB);

	B3D_TEST_ASSERT(orgObj->ObjA.IntA == newObj->ObjA.IntA);
	B3D_TEST_ASSERT(orgObj->ObjB.IntA == newObj->ObjB.IntA);

	B3D_TEST_ASSERT(orgObj->ObjPtrA->StrA == newObj->ObjPtrA->StrA);
	B3D_TEST_ASSERT(orgObj->ObjPtrB->StrA == newObj->ObjPtrB->StrA);
	B3D_TEST_ASSERT(orgObj->ObjPtrD->StrA == newObj->ObjPtrD->StrA);
	B3D_TEST_ASSERT(orgObj->ObjPtrC == newObj->ObjPtrC);

	B3D_TEST_ASSERT(orgObj->ArrStrA.size() == newObj->ArrStrA.size());
	for(u32 i = 0; i < (u32)orgObj->ArrStrA.size(); i++)
		B3D_TEST_ASSERT(orgObj->ArrStrA[i] == newObj->ArrStrA[i]);

	B3D_TEST_ASSERT(orgObj->ArrStrB.size() == newObj->ArrStrB.size());
	for(u32 i = 0; i < (u32)orgObj->ArrStrB.size(); i++)
		B3D_TEST_ASSERT(orgObj->ArrStrB[i] == newObj->ArrStrB[i]);

	B3D_TEST_ASSERT(orgObj->ArrStrC.size() == newObj->ArrStrC.size());
	for(u32 i = 0; i < (u32)orgObj->ArrStrC.size(); i++)
		B3D_TEST_ASSERT(orgObj->ArrStrC[i] == newObj->ArrStrC[i]);

	B3D_TEST_ASSERT(orgObj->ArrObjA.size() == newObj->ArrObjA.size());
	for(u32 i = 0; i < (u32)orgObj->ArrObjA.size(); i++)
		B3D_TEST_ASSERT(orgObj->ArrObjA[i].StrA == newObj->ArrObjA[i].StrA);

	B3D_TEST_ASSERT(orgObj->ArrObjB.size() == newObj->ArrObjB.size());
	for(u32 i = 0; i < (u32)orgObj->ArrObjB.size(); i++)
		B3D_TEST_ASSERT(orgObj->ArrObjB[i].StrA == newObj->ArrObjB[i].StrA);

	B3D_TEST_ASSERT(orgObj->ArrObjPtrA.size() == newObj->ArrObjPtrA.size());
	for(u32 i = 0; i < (u32)orgObj->ArrObjPtrA.size(); i++)
		B3D_TEST_ASSERT(orgObj->ArrObjPtrA[i]->IntA == newObj->ArrObjPtrA[i]->IntA);

	B3D_TEST_ASSERT(orgObj->ArrObjPtrB.size() == newObj->ArrObjPtrB.size());
	for(u32 i = 0; i < (u32)orgObj->ArrObjPtrB.size(); i++)
		B3D_TEST_ASSERT(orgObj->ArrObjPtrB[i]->IntA == newObj->ArrObjPtrB[i]->IntA);
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
