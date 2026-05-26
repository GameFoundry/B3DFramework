//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DCoreTestSuite.h"
#include "Animation/B3DAnimationCurve.h"
#include "Particles/B3DParticleDistribution.h"
#include "Serialization/B3DBinarySerializer.h"
#include "FileSystem/B3DDataStream.h"
#include "B3DUnitTestSerializableObjects.h"
#include "B3DUnitTestSerializationHelper.h"
#include "Utility/B3DUtility.h"

using namespace b3d;

static float EvaluatePosition(float acceleration, float velocity, float time)
{
	return acceleration * time * time * 0.5f + velocity * time;
}

static float EvaluateVelocity(float acceleration, float time)
{
	return acceleration * time;
}

CoreTestSuite::CoreTestSuite()
	: TestSuite("CoreTestSuite")
{
	B3D_ADD_TEST(CoreTestSuite::TestAnimCurveIntegration)
	B3D_ADD_TEST(CoreTestSuite::TestLookupTable)
	B3D_ADD_TEST(CoreTestSuite::TestBinarySerialization)
	B3D_ADD_TEST(CoreTestSuite::TestDataBlockSerialization)
	B3D_ADD_TEST(CoreTestSuite::TestSerializedObject)
	B3D_ADD_TEST(CoreTestSuite::TestBinaryDelta)

	// TODO - Add unit test for binary cloner test that restores external references
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
	const TShared<UnitTestSerializationObjectA> object = UnitTestSerializationObjectA::CreateVariantB();

	TShared<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
	BinarySerializer serializer;
	serializer.Encode(object.get(), stream, BinarySerializerFlag::None);

	stream->Seek(0);

	const TShared<UnitTestSerializationObjectA> deserializedObject = B3DRTTICast<UnitTestSerializationObjectA>(serializer.Decode(stream, (u32)stream->Size()));
	UnitTestSerializationHelpers::TestAssertObjectsMatch(*this, object, deserializedObject, false);
}

void CoreTestSuite::TestDataBlockSerialization()
{
	// Round-trip an object carrying a data block through a MemoryDataStream. This exercises the serializer's in-memory
	// data-block decode path and verifies the stream handed to the RTTI setter reports the correct Size() (the fix for
	// the MemoryDataStream capacity-constructor leaving Size() == 0).
	const TShared<UnitTestSerializationObjectB> object = B3DMakeShared<UnitTestSerializationObjectB>();
	object->IntA = 42;
	object->StrA = "data-block";
	object->DataBlock = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	TShared<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
	BinarySerializer serializer;
	serializer.Encode(object.get(), stream, BinarySerializerFlag::None);

	stream->Seek(0);

	const TShared<UnitTestSerializationObjectB> deserializedObject = B3DRTTICast<UnitTestSerializationObjectB>(serializer.Decode(stream, (u32)stream->Size()));

	B3D_TEST_ASSERT(deserializedObject != nullptr);
	B3D_TEST_ASSERT(deserializedObject->IntA == object->IntA);
	B3D_TEST_ASSERT(deserializedObject->StrA == object->StrA);
	B3D_TEST_ASSERT(deserializedObject->DataBlock == object->DataBlock);
	B3D_TEST_ASSERT(deserializedObject->DataBlockStreamSize == (u32)object->DataBlock.size());
}

void CoreTestSuite::TestSerializedObject()
{
	const TShared<UnitTestSerializationObjectA> object = UnitTestSerializationObjectA::CreateVariantB();

	const TShared<SerializedObject> serializedObject = SerializedObject::Create(*object);
	RTTIOperationEngineContext rttiOperationContext;
	const TShared<UnitTestSerializationObjectA> deserializedObject = B3DRTTICast<UnitTestSerializationObjectA>(serializedObject->Decode(rttiOperationContext));

	UnitTestSerializationHelpers::TestAssertObjectsMatch(*this, object, deserializedObject, false);
}

void CoreTestSuite::TestBinaryDelta()
{
	const TShared<UnitTestSerializationObjectA> objectA = UnitTestSerializationObjectA::CreateVariantA();
	const TShared<UnitTestSerializationObjectA> objectB = UnitTestSerializationObjectA::CreateVariantB();

	const TShared<SerializedObject> serializedObjectA = SerializedObject::Create(*objectA.get());
	const TShared<SerializedObject> serializedObjectB = SerializedObject::Create(*objectB.get());

	IDeltaHandler& deltaHandler = objectA->GetRtti()->GetDeltaHandler();
	RTTIOperationEngineContext generateDeltaRTTIOperationContext;
	TShared<SerializedObject> delta = deltaHandler.GenerateDelta(serializedObjectA, serializedObjectB, generateDeltaRTTIOperationContext);

	RTTIOperationEngineContext applyDeltaRTTIOperationContext;
	deltaHandler.ApplyDelta(objectA, delta, applyDeltaRTTIOperationContext);

	UnitTestSerializationHelpers::TestAssertObjectsMatch(*this, objectA, objectB, true);
}
