//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DCoreTestSuite.h"
#include "Animation/B3DAnimationCurve.h"
#include "Particles/B3DParticleDistribution.h"
#include "Reflection/B3DRTTIObjectWrapper.h"
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
	B3D_ADD_TEST(CoreTestSuite::TestReadOnlySerialization)
	B3D_ADD_TEST(CoreTestSuite::TestRTTIObjectWrapperComparison)
	B3D_ADD_TEST(CoreTestSuite::TestRTTIObjectWrapperFieldFilter)
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

void CoreTestSuite::TestReadOnlySerialization()
{
	static constexpr u32 kReadOnlyFieldId = 3;
	static constexpr u32 kReadOnlyDefaultValue = 100;
	static constexpr u32 kReadOnlyStoredValue = 200;

	const TShared<UnitTestSerializationObjectB> object = B3DMakeShared<UnitTestSerializationObjectB>();
	object->ReadOnlyValue = kReadOnlyStoredValue;

	const TShared<SerializedObject> serializedObject = SerializedObject::Create(*object);
	B3D_TEST_ASSERT(serializedObject->SubObjects.size() == 1)
	B3D_TEST_ASSERT(serializedObject->SubObjects[0].FieldEntries.find(kReadOnlyFieldId) == serializedObject->SubObjects[0].FieldEntries.end())

	const TShared<RTTISchema>& schema = object->GetRtti()->GetSchema();
	const auto readOnlySchema = std::find_if(schema->FieldSchemas.begin(), schema->FieldSchemas.end(), [](const RTTIFieldSchema& fieldSchema)
	{
		return fieldSchema.Id == kReadOnlyFieldId;
	});
	B3D_TEST_ASSERT(readOnlySchema == schema->FieldSchemas.end())

	TShared<MemoryDataStream> binaryStream = B3DMakeShared<MemoryDataStream>();
	BinarySerializer serializer;
	serializer.Encode(object.get(), binaryStream, BinarySerializerFlag::None);

	binaryStream->Seek(0);
	const TShared<UnitTestSerializationObjectB> binaryObject = B3DRTTICast<UnitTestSerializationObjectB>(serializer.Decode(binaryStream, (u32)binaryStream->Size()));
	B3D_TEST_ASSERT(binaryObject->ReadOnlyValue == kReadOnlyDefaultValue)

	TShared<MemoryDataStream> noMetaStream = B3DMakeShared<MemoryDataStream>();
	serializer.Encode(object.get(), noMetaStream, BinarySerializerFlag::NoMeta);

	noMetaStream->Seek(0);
	const TShared<UnitTestSerializationObjectB> noMetaObject = B3DRTTICast<UnitTestSerializationObjectB>(serializer.Decode(noMetaStream, (u32)noMetaStream->Size(), BinarySerializerFlag::NoMeta, nullptr, schema));
	B3D_TEST_ASSERT(noMetaObject->ReadOnlyValue == kReadOnlyDefaultValue)

	TShared<SerializedPlainData> legacyReadOnlyData = B3DMakeShared<SerializedPlainData>();
	legacyReadOnlyData->Value = static_cast<u8*>(B3DAllocate(sizeof(kReadOnlyStoredValue)));
	legacyReadOnlyData->Size = sizeof(kReadOnlyStoredValue);
	legacyReadOnlyData->OwnsMemory = true;
	memcpy(legacyReadOnlyData->Value, &kReadOnlyStoredValue, sizeof(kReadOnlyStoredValue));

	SerializedField legacyReadOnlyField;
	legacyReadOnlyField.FieldId = kReadOnlyFieldId;
	legacyReadOnlyField.Value = legacyReadOnlyData;
	serializedObject->SubObjects[0].FieldEntries[kReadOnlyFieldId] = std::move(legacyReadOnlyField);

	RTTIOperationEngineContext deserializeContext;
	const TShared<UnitTestSerializationObjectB> legacyObject = B3DRTTICast<UnitTestSerializationObjectB>(serializedObject->Decode(deserializeContext));
	B3D_TEST_ASSERT(legacyObject->ReadOnlyValue == kReadOnlyStoredValue)

	const TShared<UnitTestSerializationObjectB> originalObject = B3DMakeShared<UnitTestSerializationObjectB>();
	const TShared<UnitTestSerializationObjectB> modifiedObject = B3DMakeShared<UnitTestSerializationObjectB>();
	modifiedObject->ReadOnlyValue = kReadOnlyStoredValue;

	RTTIOperationEngineContext deltaContext;
	IDeltaHandler& deltaHandler = originalObject->GetRtti()->GetDeltaHandler();
	const TShared<SerializedObject> delta = deltaHandler.GenerateDelta(originalObject, modifiedObject, deltaContext);
	B3D_TEST_ASSERT(delta == nullptr)
}

void CoreTestSuite::TestRTTIObjectWrapperComparison()
{
	static constexpr u32 kComparedFieldId = 0;

	const TShared<UnitTestSerializationObjectB> object = B3DMakeShared<UnitTestSerializationObjectB>();
	const TShared<SerializedObject> serializedObject = SerializedObject::Create(*object);
	const TShared<ISerialized>& serializedField = serializedObject->SubObjects[0].FieldEntries.at(kComparedFieldId).Value;

	FrameAllocator& allocator = GetFrameAllocator();
	FrameAllocatorScope allocatorScope(&allocator);

	const RTTIObjectWrapper::Value<false> serializedValue(0, serializedField, &allocator);
	bool compared = false;
	bool liveToSerializedModified = true;
	bool serializedToLiveModified = true;

	RTTIObjectWrapper::Object<true> wrappedObject(object.get(), object->GetRtti(), &allocator);
	RTTIObjectWrapper::IterateFieldValues(wrappedObject, RTTIOperationType::DeltaGenerate,
		[&](const RTTIFieldSchema&, const RTTIObjectWrapper::Value<true>& value)
		{
			const RTTIObjectWrapper::Value<true> liveValue = value.GetTupleElement(0);
			liveToSerializedModified = liveValue.ComparePlain(serializedValue);
			serializedToLiveModified = serializedValue.ComparePlain(liveValue);
			compared = true;
		},
		[](const RTTIFieldSchema& fieldSchema)
		{
			return fieldSchema.Id == kComparedFieldId;
		});

	B3D_TEST_ASSERT(compared)
	B3D_TEST_ASSERT(!liveToSerializedModified)
	B3D_TEST_ASSERT(!serializedToLiveModified)
}

void CoreTestSuite::TestRTTIObjectWrapperFieldFilter()
{
	static constexpr u32 kFilteredFieldId = 1;
	static constexpr u32 kFieldAfterFilterId = 3;

	const TShared<UnitTestSerializationObjectB> object = B3DMakeShared<UnitTestSerializationObjectB>();
	FrameAllocator& allocator = GetFrameAllocator();
	FrameAllocatorScope allocatorScope(&allocator);

	bool visitedFilteredField = false;
	bool visitedFieldAfterFilter = false;

	RTTIObjectWrapper::Object<true> wrappedObject(object.get(), object->GetRtti(), &allocator);
	RTTIObjectWrapper::IterateFieldValues(wrappedObject, RTTIOperationType::GatherReferences,
		[&](const RTTIFieldSchema& fieldSchema, const RTTIObjectWrapper::Value<true>&)
		{
			visitedFilteredField |= fieldSchema.Id == kFilteredFieldId;
			visitedFieldAfterFilter |= fieldSchema.Id == kFieldAfterFilterId;
		},
		[](const RTTIFieldSchema& fieldSchema)
		{
			return fieldSchema.Id != kFilteredFieldId;
		});

	B3D_TEST_ASSERT(!visitedFilteredField)
	B3D_TEST_ASSERT(visitedFieldAfterFilter)
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
