//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/BsConsoleTestOutput.h"
#include "Testing/BsTestSuite.h"
#include "Animation/BsAnimationCurve.h"
#include "Particles/BsParticleDistribution.h"
#include "Serialization/BsBinarySerializer.h"
#include "FileSystem/BsDataStream.h"
#include "Scene/BsComponent.h"
#include "Scene/BsGameObjectManager.h"
#include "Scene/BsGameObjectCollection.h"
#include "Scene/BsPrefab.h"
#include "Scene/BsSceneManager.h"
#include "Scene/BsSceneObject.h"
#include "BsApplication.h"
#include "Utility/BsUtility.h"
#include "BsUnitTestScenes.h"
#include "BsUnitTestSerializationHelper.h"
#include "BsUnitTestPrefabUpdateHelper.h"
#include "Scene/BsPrefabUtility.h"

using namespace bs;

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
	void TestPrefabUpdate();
	void TestPrefabScenario1();
	void TestPrefabScenario2();
	void TestPrefabScenario3();
	void TestPrefabScenario4();
	void TestPrefabScenario5();
	void TestPrefabScenario6();
	void TestPrefabScenario7();
	void TestPrefabScenario8();

	void TestAssertPrefabScenario(bool swapCheckOrder = false, bool checkNestedPrefabs = true, bool skipOptionals = false);

	SPtr<SceneInstance> mSceneHierarchy;

	struct PrefabTestInformation
	{
		HSceneObject InstanceRootInScene;
		HPrefab Prefab;

		SPtr<GameObjectCollection> InternalsObjectCollection;
		HSceneObject InternalsCopy;

		PrefabLinkCheckType LinkCheckType = PrefabLinkCheckType::Regular;
	};

	HSceneObject mLastInstantiatedPrefab1InstanceRoot;
	Array<PrefabTestInformation, 3> mPrefabTestInformation;
};

CoreTestSuite::CoreTestSuite()
{
	B3D_ADD_TEST(CoreTestSuite::TestAnimCurveIntegration)
	B3D_ADD_TEST(CoreTestSuite::TestLookupTable)
	B3D_ADD_TEST(CoreTestSuite::TestBinarySerialization)
	B3D_ADD_TEST(CoreTestSuite::TestSerializedObject)
	B3D_ADD_TEST(CoreTestSuite::TestBinaryDelta)
	B3D_ADD_TEST(CoreTestSuite::TestSceneSaveLoad)
	//B3D_ADD_TEST(CoreTestSuite::TestPrefabUpdate)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario1)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario2)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario3)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario4)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario5)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario6)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario7)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario8)

	// TODO - Add unit tests for:
	// - Binary cloner test that restores external references
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

void CoreTestSuite::TestBinarySerialization()
{
	const SPtr<UnitTestSerializationObjectA> object = UnitTestSerializationObjectA::CreateVariantB();
	
	SPtr<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
	BinarySerializer serializer;
	serializer.Encode(object.get(), stream, BinarySerializerFlag::None);

	stream->Seek(0);

	const SPtr<UnitTestSerializationObjectA> deserializedObject = B3DRTTICast<UnitTestSerializationObjectA>(serializer.Decode(stream, (u32)stream->Size()));
	UnitTestSerializationHelpers::TestAssertObjectsMatch(*this, object, deserializedObject, false);
}

void CoreTestSuite::TestSerializedObject()
{
	const SPtr<UnitTestSerializationObjectA> object = UnitTestSerializationObjectA::CreateVariantB();

	const SPtr<SerializedObject> serializedObject = SerializedObject::Create(*object);
	const SPtr<UnitTestSerializationObjectA> deserializedObject = B3DRTTICast<UnitTestSerializationObjectA>(serializedObject->Decode());

	UnitTestSerializationHelpers::TestAssertObjectsMatch(*this, object, deserializedObject, false);
}

void CoreTestSuite::TestBinaryDelta()
{
	const SPtr<UnitTestSerializationObjectA> objectA = UnitTestSerializationObjectA::CreateVariantA();
	const SPtr<UnitTestSerializationObjectA> objectB = UnitTestSerializationObjectA::CreateVariantB();

	const SPtr<SerializedObject> serializedObjectA = SerializedObject::Create(*objectA.get());
	const SPtr<SerializedObject> serializedObjectB = SerializedObject::Create(*objectB.get());

	IDeltaHandler& deltaHandler = objectA->GetRtti()->GetDeltaHandler();
	SPtr<SerializedObject> delta = deltaHandler.GenerateDelta(serializedObjectA, serializedObjectB);
	deltaHandler.ApplyDelta(objectA, delta, nullptr);

	UnitTestSerializationHelpers::TestAssertObjectsMatch(*this, objectA, objectB, true);
}

void CoreTestSuite::TestSceneSaveLoad()
{
	// Create & serialize scene 0
	SPtr<MemoryDataStream> serializedScene0Stream = B3DMakeShared<MemoryDataStream>();
	{
		SPtr<SceneInstance> scene0Instance = SceneInstance::Create("UnitTestScene0Instance");
		UnitTestSceneA scene0Wrapper(scene0Instance);

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

		UnitTestSceneA scene0Wrapper(scene0Prefab->GetRoot());

		B3D_TEST_ASSERT(scene0Wrapper.Component_0_1_0->StringValue == "testValue")
		B3D_TEST_ASSERT(scene0Wrapper.Component_0->SceneObjectReference == scene0Wrapper.SceneObject_0_1_0)
		B3D_TEST_ASSERT(scene0Wrapper.Component_0->ComponentReference == scene0Wrapper.Component_1)
		B3D_TEST_ASSERT(scene0Wrapper.Component_0_1_0->SceneObjectReference == scene0Wrapper.SceneObject_0)
		B3D_TEST_ASSERT(scene0Wrapper.Component_0_1_0->ComponentReference == scene0Wrapper.Component_0)
	}

	// Instantiate the scene and ensure prefab links are valid
	{
		HSceneObject instancedSceneRoot = scene0Prefab->Instantiate();
		UnitTestSceneA instancedScene0Wrapper(instancedSceneRoot);

		UnitTestSceneA prefabScene0Wrapper(scene0Prefab->GetRoot());

		instancedScene0Wrapper.PerformSceneObjectBinaryOperation(prefabScene0Wrapper,
			[this, scene0PrefabId = scene0Prefab->GetId()] (const HSceneObject& instanceSceneObject, const HSceneObject& prefabSceneObject) {
			// Instance should be linked to the prefab
			 B3D_TEST_ASSERT(!instanceSceneObject->GetPrefabObjectId().Empty())
			 B3D_TEST_ASSERT(instanceSceneObject->GetPrefabObjectId() == prefabSceneObject->GetId())
			 B3D_TEST_ASSERT(instanceSceneObject->GetPrefabResourceId() == scene0PrefabId)

			 // Prefab hierarchy should link to itself
			 B3D_TEST_ASSERT(!prefabSceneObject->GetPrefabObjectId().Empty())
			 B3D_TEST_ASSERT(prefabSceneObject->GetPrefabObjectId() == prefabSceneObject->GetId())
			 B3D_TEST_ASSERT(prefabSceneObject->GetPrefabResourceId() == scene0PrefabId)
		});

		instancedScene0Wrapper.PerformComponentBinaryOperation(prefabScene0Wrapper,
			[this, scene0PrefabId = scene0Prefab->GetId()] (const HComponent& instanceComponent, const HComponent& prefabComponent) {
			// Instance should be linked to the prefab
			 B3D_TEST_ASSERT(!instanceComponent->GetPrefabObjectId().Empty())
			 B3D_TEST_ASSERT(instanceComponent->GetPrefabObjectId() == prefabComponent->GetId())

			 // Prefab hierarchy should link to itself
			 B3D_TEST_ASSERT(!prefabComponent->GetPrefabObjectId().Empty())
			 B3D_TEST_ASSERT(prefabComponent->GetPrefabObjectId() == prefabComponent->GetId())
		});
	}

	// Create and serialize scene 1 that contains an instance of the above prefab, and they reference eachother objects
	SPtr<MemoryDataStream> serializedScene1Stream = B3DMakeShared<MemoryDataStream>();
	{
		SPtr<SceneInstance> scene1Instance = SceneInstance::Create("UnitTestScene1Instance");

		UnitTestSceneB scene1Wrapper(scene1Instance->GetRoot(), scene0Prefab);
		UnitTestSceneA scene0Wrapper(scene1Wrapper.OptionalSceneObject_0_0_PrefabInstance);
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

		UnitTestSceneB scene1Wrapper(scene1Prefab->GetRoot());
		UnitTestSceneA scene0Wrapper(scene1Wrapper.OptionalSceneObject_0_0_PrefabInstance);

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

void CoreTestSuite::TestAssertPrefabScenario(bool swapCheckOrder, bool checkNestedPrefabs, bool skipOptionals)
{
	B3D_ASSERT(mPrefabTestInformation[0].Prefab != nullptr);

	{
		auto prefab1TestInformation = mPrefabTestInformation[0];

		const HPrefab prefab = prefab1TestInformation.Prefab;
		const HSceneObject instanceRootInScene = prefab1TestInformation.InstanceRootInScene;
		
		// Ensure the instances correctly link to the prefab (both the hierarchy used to create/update the prefab, and newly instantiated hierarchy)
		HSceneObject instantiatedInstanceRoot = prefab->Instantiate();
		UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(*this, instanceRootInScene, prefab->GetRoot(), prefab->GetId());
		UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(*this, instantiatedInstanceRoot, prefab->GetRoot(), prefab->GetId());

		// Ensure IDs match previous instances
		if(mLastInstantiatedPrefab1InstanceRoot != nullptr)
		{
			if(swapCheckOrder)
			{
				UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, instantiatedInstanceRoot, mLastInstantiatedPrefab1InstanceRoot, true, skipOptionals);
				UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, instanceRootInScene, mLastInstantiatedPrefab1InstanceRoot, true, skipOptionals);
			}
			else
			{
				UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, mLastInstantiatedPrefab1InstanceRoot, instantiatedInstanceRoot, true, skipOptionals);
				UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, mLastInstantiatedPrefab1InstanceRoot, instanceRootInScene, true, skipOptionals);
			}
			
			mLastInstantiatedPrefab1InstanceRoot->Destroy(true);
			mLastInstantiatedPrefab1InstanceRoot = instantiatedInstanceRoot;
		}
	}

	// Ensure internal prefab links are valid
	TArray<UnitTestPrefabInformation> prefabInformation;
	for(auto& entry : mPrefabTestInformation)
	{
		if(entry.Prefab != nullptr)
			prefabInformation.Add(UnitTestPrefabInformation(entry.Prefab, entry.LinkCheckType));
	}

	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabInternalsMatch(*this, 0, prefabInformation, checkNestedPrefabs);

	// Ensure IDs are unchanged from the previous prefab internals
	for(auto& entry : mPrefabTestInformation)
	{
		if(entry.Prefab == nullptr)
			continue;

		if(entry.InternalsCopy != nullptr)
		{
			if(swapCheckOrder)
				UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, entry.Prefab->GetRoot(), entry.InternalsCopy, false, skipOptionals);
			else
				UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, entry.InternalsCopy, entry.Prefab->GetRoot(), false, skipOptionals);
		}

		entry.InternalsObjectCollection = GameObjectCollection::Create();
		entry.InternalsCopy = entry.Prefab->Clone(entry.InternalsObjectCollection, true);
	}
}

// Set up a scene, and create Prefab #1 from a brand new hierarchy. Verify instances of the prefab link to the prefab.
void CoreTestSuite::TestPrefabScenario1()
{
	// Create an instance for the main scene hierarchy
	mSceneHierarchy = SceneInstance::Create("Scene Hierarchy");
	mPrefabTestInformation[0].InstanceRootInScene = mSceneHierarchy->CreateSceneObject("Prefab #1 Instance Root");

	// Create hierarchy for Prefab 1 in the scene hierarchy
	UnitTestSceneB::PopulateParent(mPrefabTestInformation[0].InstanceRootInScene, nullptr);

	// Construct the prefab from Prefab #1 Instance Root in the scene hierarchy
	mPrefabTestInformation[0].Prefab = Prefab::Create(mPrefabTestInformation[0].InstanceRootInScene, false);

	TestAssertPrefabScenario();
}

// Add an object to an instance of Prefab #1, then update the prefab.
void CoreTestSuite::TestPrefabScenario2()
{
	// Add a new object to the scene hierarchy
	UnitTestSceneB prefab1InstanceRoot_Wrapper(mPrefabTestInformation[0].InstanceRootInScene);

	//prefab1InstanceRoot_Wrapper.Component_1_0->Destroy();
	//prefab1InstanceRoot_Wrapper.SceneObject_1_0->Destroy();
	prefab1InstanceRoot_Wrapper.CreateOptionalObjects();

	// Update Prefab1 from Prefab #1 Instance Root in scene hierarchy
	PrefabUtility::UpdatePrefab(mPrefabTestInformation[0].Prefab, mPrefabTestInformation[0].InstanceRootInScene);

	TestAssertPrefabScenario();
}

// Create Prefab2, then add its instance as a child of Prefab #1 Instance Root, and update Prefab1 with Prefab #1 Instance Root
void CoreTestSuite::TestPrefabScenario3()
{
	// Create prefab 2
	mPrefabTestInformation[1].Prefab = Prefab::Create(UnitTestSceneB::PopulateNewSceneInstance("Prefab #2 Scene Instance", nullptr), false);

	// Add Prefab #2 Instance Root in the scene, as child of Prefab 2 Instance Root/SceneObject_0
	UnitTestSceneB prefab1InstanceRoot_Wrapper(mPrefabTestInformation[0].InstanceRootInScene);

	mPrefabTestInformation[1].InstanceRootInScene = mPrefabTestInformation[1].Prefab->Instantiate(mSceneHierarchy);
	mPrefabTestInformation[1].InstanceRootInScene->SetName("Prefab #2 Instance Root");
	mPrefabTestInformation[1].InstanceRootInScene->SetParent(prefab1InstanceRoot_Wrapper.SceneObject_0);

	// Update Prefab1 from Prefab #1 Instance Root in scene hierarchy
	PrefabUtility::UpdatePrefab(mPrefabTestInformation[0].Prefab, mPrefabTestInformation[0].InstanceRootInScene);
	
	TestAssertPrefabScenario();
}

// Add new object as part of Prefab #2 Instance Root, when update Prefab1 with Prefab #1 Instance Root
void CoreTestSuite::TestPrefabScenario4()
{
	UnitTestSceneB prefab1InstanceRoot_Wrapper(mPrefabTestInformation[0].InstanceRootInScene);
	UnitTestSceneB prefab2InstanceRoot_Wrapper(prefab1InstanceRoot_Wrapper.OptionalSceneObject_0_0_PrefabInstance);

	//prefab2InstanceRoot_Wrapper.Component_1_0->Destroy();
	//prefab2InstanceRoot_Wrapper.SceneObject_1_0->Destroy(); // TODO
	prefab2InstanceRoot_Wrapper.CreateOptionalObjects();

	// Update Prefab1 from Prefab #1 Instance Root in scene hierarchy
	PrefabUtility::UpdatePrefab(mPrefabTestInformation[0].Prefab, mPrefabTestInformation[0].InstanceRootInScene);
	
	mPrefabTestInformation[1].LinkCheckType = PrefabLinkCheckType::OptionalsAreInstanceModifications;
	TestAssertPrefabScenario();
}

// Update Prefab2 with Prefab #2 Instance Root
void CoreTestSuite::TestPrefabScenario5()
{
	UnitTestSceneB prefab1InstanceRoot_Wrapper(mPrefabTestInformation[0].InstanceRootInScene);
	UnitTestSceneB prefab2InstanceRoot_Wrapper(prefab1InstanceRoot_Wrapper.OptionalSceneObject_0_0_PrefabInstance);

	UnitTestSceneB prefab1Internals_Wrapper(mPrefabTestInformation[0].Prefab->GetRoot());
	UnitTestSceneB prefab2Internals_Wrapper(prefab1Internals_Wrapper.OptionalSceneObject_0_0_PrefabInstance);

	// Update Prefab2 from Prefab #2 Instance Root in scene hierarchy
	PrefabUtility::UpdatePrefab(mPrefabTestInformation[1].Prefab, mPrefabTestInformation[1].InstanceRootInScene);

	// TODO - The above UpdatePrefab will update Prefab #1 with a brand new copy of SceneObject_2, rather than just linking the existing copy to
	// Prefab2. It remains unsolved how to do this correctly.

	mPrefabTestInformation[1].LinkCheckType = PrefabLinkCheckType::Regular;
	TestAssertPrefabScenario(false, true, true); // TODO - Skipping optionals to avoid error caused above
}

// Update Prefab1 from Prefab #1 Instance Root
void CoreTestSuite::TestPrefabScenario6()
{
	// Update Prefab1 from Prefab #1 Instance Root in scene hierarchy
	PrefabUtility::UpdatePrefab(mPrefabTestInformation[0].Prefab, mPrefabTestInformation[0].InstanceRootInScene);

	TestAssertPrefabScenario();
}

// Create Prefab3, add it as child of Prefab #2 Instance Root, and update Prefab1 with Prefab #1 Instance Root
void CoreTestSuite::TestPrefabScenario7()
{
	// Create prefab 3
	mPrefabTestInformation[2].Prefab = Prefab::Create(UnitTestSceneB::PopulateNewSceneInstance("Prefab #3 Scene Instance", nullptr), false);

	// Add Prefab #2 Instance Root in the scene, as child of Prefab 2 Instance Root/SceneObject_0
	UnitTestSceneB prefab2InstanceRoot_Wrapper(mPrefabTestInformation[1].InstanceRootInScene);

	mPrefabTestInformation[2].InstanceRootInScene = mPrefabTestInformation[2].Prefab->Instantiate(mSceneHierarchy);
	mPrefabTestInformation[2].InstanceRootInScene->SetName("Prefab #3 Instance Root");
	mPrefabTestInformation[2].InstanceRootInScene->SetParent(prefab2InstanceRoot_Wrapper.SceneObject_0);

	// Update Prefab1 from Prefab #1 Instance Root in scene hierarchy
	PrefabUtility::UpdatePrefab(mPrefabTestInformation[0].Prefab, mPrefabTestInformation[0].InstanceRootInScene);
	
	mPrefabTestInformation[2].LinkCheckType = PrefabLinkCheckType::PrefabIsInstanceModification;
	TestAssertPrefabScenario(false, false);
}

// Update Prefab2 from Prefab #2 Instance Root
void CoreTestSuite::TestPrefabScenario8()
{
	// Update Prefab2 from Prefab #2 Instance Root in scene hierarchy
	PrefabUtility::UpdatePrefab(mPrefabTestInformation[1].Prefab, mPrefabTestInformation[1].InstanceRootInScene);

	mPrefabTestInformation[2].LinkCheckType = PrefabLinkCheckType::Regular;
	TestAssertPrefabScenario();
}

void CoreTestSuite::TestPrefabUpdate()
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create first two prefabs (one nested within another)
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Create a scene with a nested prefab
	HSceneObject sourceHierarchy_Child0 = UnitTestSceneB::PopulateNewSceneInstance("UnitTestChild0SceneInstance", nullptr);
	HPrefab prefab_Child0 = Prefab::Create(sourceHierarchy_Child0, false);

	HSceneObject sourceHierarchy_Root = UnitTestSceneB::PopulateNewSceneInstance("UnitTestRootSceneInstance", prefab_Child0.GetShared());
	HPrefab prefab_Root = Prefab::Create(sourceHierarchy_Root, false);

	HSceneObject firstInstantiatedHierarchy_Root = prefab_Root->Instantiate();

	// Keep a copy of the current prefab hierarchy
	SPtr<GameObjectCollection> firstPrefabObjectCollection_Root = GameObjectCollection::Create();
	HSceneObject firstPrefabHierarchy_Root = prefab_Root->Clone(firstPrefabObjectCollection_Root, true);

	SPtr<GameObjectCollection> firstPrefabObjectCollection_Child0 = GameObjectCollection::Create();
	HSceneObject firstPrefabHierarchy_Child0 = prefab_Child0->Clone(firstPrefabObjectCollection_Child0, true);

	// Ensure links from instances to prefab are valid
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(*this, sourceHierarchy_Root, prefab_Root->GetRoot(), prefab_Root->GetId());
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(*this, firstInstantiatedHierarchy_Root, prefab_Root->GetRoot(), prefab_Root->GetId());

	// Ensure internal prefab links are valid
	{
		TArray<UnitTestPrefabInformation> prefabInformation;
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Root, PrefabLinkCheckType::Regular));
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Child0, PrefabLinkCheckType::Regular));

		UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabInternalsMatch(*this, 0, prefabInformation, true);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Modify instantiated hierarchy and update root prefab with modifications
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	{
		// Wrapper for instantiated scene objects
		UnitTestSceneB instancedWrapper_Root(firstInstantiatedHierarchy_Root);
		UnitTestSceneB instancedWrapper_Child0(instancedWrapper_Root.OptionalSceneObject_0_0_PrefabInstance);

		// Destroy objects in both root and nested prefab instance
		instancedWrapper_Root.Component_1_0->Destroy();
		instancedWrapper_Root.SceneObject_1_0->Destroy();
		instancedWrapper_Child0.Component_1_0->Destroy();
		instancedWrapper_Child0.SceneObject_1_0->Destroy();

		// Add objects in both root and nested prefab instance
		instancedWrapper_Root.CreateOptionalObjects();
		instancedWrapper_Child0.CreateOptionalObjects();

		// Update the root prefab
		PrefabUtility::UpdatePrefab(prefab_Root, firstInstantiatedHierarchy_Root);
	}

	// Instantiate the prefab with new modifications
	HSceneObject secondInstantiatedHierarchy_Root = prefab_Root->Instantiate();

	// Keep a copy of the current prefab hierarchy
	SPtr<GameObjectCollection> secondPrefabObjectCollection_Root = GameObjectCollection::Create();
	HSceneObject secondPrefabHierarchy_Root = prefab_Root->Clone(secondPrefabObjectCollection_Root, true);

	// Ensure links from instances to prefab are valid
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(*this, firstInstantiatedHierarchy_Root, prefab_Root->GetRoot(), prefab_Root->GetId());
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(*this, secondInstantiatedHierarchy_Root, prefab_Root->GetRoot(), prefab_Root->GetId());

	// Ensure internal prefab links are valid
	{
		TArray<UnitTestPrefabInformation> prefabInformation;
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Root, PrefabLinkCheckType::Regular));
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Child0, PrefabLinkCheckType::OptionalsAreInstanceModifications));

		UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabInternalsMatch(*this, 0, prefabInformation, true);
	}

	// Ensure IDs are unchanged from the previous instance and prefab internals
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, firstInstantiatedHierarchy_Root, sourceHierarchy_Root, true, true);
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, secondInstantiatedHierarchy_Root, sourceHierarchy_Root, true, true);
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, prefab_Root->GetRoot(), firstPrefabHierarchy_Root, false, true);

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Update the child prefab, then update root prefab again
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	{
		// Wrapper for instantiated scene objects
		UnitTestSceneB instancedWrapper_Root(secondInstantiatedHierarchy_Root);

		// TODO - Original SceneObject_2 from Root gets replaced with another instance from Child after this completes. If something is referencing it, the reference will be broken.
		// It would be nice to find a way to maintain it
		PrefabUtility::UpdatePrefab(prefab_Child0, instancedWrapper_Root.OptionalSceneObject_0_0_PrefabInstance);
		PrefabUtility::UpdatePrefab(prefab_Root, secondInstantiatedHierarchy_Root);
	}

	// Instantiate the prefab with new modifications
	HSceneObject thirdInstantiatedHierarchy_Root = prefab_Root->Instantiate();

	// Keep a copy of the current prefab hierarchy
	SPtr<GameObjectCollection> thirdPrefabObjectCollection_Root = GameObjectCollection::Create();
	HSceneObject thirdPrefabHierarchy_Root = prefab_Root->Clone(thirdPrefabObjectCollection_Root, true);

	SPtr<GameObjectCollection> secondPrefabObjectCollection_Child0 = GameObjectCollection::Create();
	HSceneObject secondPrefabHierarchy_Child0 = prefab_Child0->Clone(secondPrefabObjectCollection_Child0, true);

	// Ensure links from instances to prefab are valid
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(*this, secondInstantiatedHierarchy_Root, prefab_Root->GetRoot(), prefab_Root->GetId());
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(*this, thirdInstantiatedHierarchy_Root, prefab_Root->GetRoot(), prefab_Root->GetId());

	// Ensure internal prefab links are valid
	{
		TArray<UnitTestPrefabInformation> prefabInformation;
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Root, PrefabLinkCheckType::Regular));
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Child0, PrefabLinkCheckType::Regular));

		UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabInternalsMatch(*this, 0, prefabInformation, true);
	}

	// Ensure IDs are unchanged from the previous instance and prefab internals
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, secondInstantiatedHierarchy_Root, firstInstantiatedHierarchy_Root, true);
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, thirdInstantiatedHierarchy_Root, firstInstantiatedHierarchy_Root, true);
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, prefab_Root->GetRoot(), secondPrefabHierarchy_Root, false);
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, prefab_Child0->GetRoot(), firstPrefabHierarchy_Child0, false, true);

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Add another nested prefab to the first nested prefab
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	HSceneObject sourceHierarchy_Child_1 = UnitTestSceneB::PopulateNewSceneInstance("UnitTestChild1SceneInstance", nullptr);
	HPrefab prefab_Child1 = Prefab::Create(sourceHierarchy_Child_1, false);

	// Add another child prefab instance as a child to the current nested prefab instance
	{
		// Wrapper for instantiated scene objects
		UnitTestSceneB instancedWrapper_Root(thirdInstantiatedHierarchy_Root);
		UnitTestSceneB instancedWrapper_Child0(instancedWrapper_Root.OptionalSceneObject_0_0_PrefabInstance);
		
		HSceneObject instantiatedSceneInstance_Child1 = prefab_Child1->Instantiate(instancedWrapper_Root.SceneInstance);
		instantiatedSceneInstance_Child1->SetParent(instancedWrapper_Child0.SceneObject_0);

		PrefabUtility::UpdatePrefab(prefab_Root, thirdInstantiatedHierarchy_Root);
	}

	// Instantiate the prefab with new modifications
	HSceneObject fourthInstantiatedHierarchy_Root = prefab_Root->Instantiate();

	// Keep a copy of the current prefab hierarchy
	SPtr<GameObjectCollection> fourthPrefabObjectCollection_Root = GameObjectCollection::Create();
	HSceneObject fourthPrefabHierarchy_Root = prefab_Root->Clone(fourthPrefabObjectCollection_Root, true);

	// Ensure links from instances to prefab are valid
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(*this, thirdInstantiatedHierarchy_Root, prefab_Root->GetRoot(), prefab_Root->GetId());
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(*this, fourthInstantiatedHierarchy_Root, prefab_Root->GetRoot(), prefab_Root->GetId());

	// Ensure internal prefab links are valid (root only)
	{
		TArray<UnitTestPrefabInformation> prefabInformation;
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Root, PrefabLinkCheckType::Regular));
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Child0, PrefabLinkCheckType::Regular));
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Child1, PrefabLinkCheckType::PrefabIsInstanceModification));

		UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabInternalsMatch(*this, 0, prefabInformation, false);
	}

	// Ensure IDs are unchanged from the previous instance and prefab internals
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, secondInstantiatedHierarchy_Root, thirdInstantiatedHierarchy_Root, true);
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, secondInstantiatedHierarchy_Root, fourthInstantiatedHierarchy_Root, true);
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, thirdPrefabHierarchy_Root, prefab_Root->GetRoot(), false);
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, secondPrefabHierarchy_Child0, prefab_Child0->GetRoot(), false);

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Update the child prefab
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	{
		// Wrapper for instantiated scene objects
		UnitTestSceneB instancedWrapper_Root(fourthInstantiatedHierarchy_Root);

		PrefabUtility::UpdatePrefab(prefab_Child0, instancedWrapper_Root.OptionalSceneObject_0_0_PrefabInstance);
	}

	// Ensure links from instances to prefab are valid
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(*this, thirdInstantiatedHierarchy_Root, prefab_Root->GetRoot(), prefab_Root->GetId());
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(*this, fourthInstantiatedHierarchy_Root, prefab_Root->GetRoot(), prefab_Root->GetId());

	// Ensure internal prefab links are valid (root only)
	{
		TArray<UnitTestPrefabInformation> prefabInformation;
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Root, PrefabLinkCheckType::Regular));
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Child0, PrefabLinkCheckType::Regular));
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Child1, PrefabLinkCheckType::PrefabIsInstanceModification));

		UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabInternalsMatch(*this, 0, prefabInformation, false);
	}

	// Ensure internal prefab links are valid (child0 only)
	{
		TArray<UnitTestPrefabInformation> prefabInformation;
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Child0, PrefabLinkCheckType::Regular));
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Child1, PrefabLinkCheckType::Regular));

		UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabInternalsMatch(*this, 0, prefabInformation, false);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Update root prefab again
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	{
		// Wrapper for instantiated scene objects
		UnitTestSceneB instancedWrapper_Root(fourthInstantiatedHierarchy_Root);

		PrefabUtility::UpdatePrefab(prefab_Root, fourthInstantiatedHierarchy_Root);
	}

	// Instantiate the prefab with new modifications
	HSceneObject fifthInstantiatedHierarchy_Root = prefab_Root->Instantiate();

	// Keep a copy of the current prefab hierarchy
	SPtr<GameObjectCollection> fifthPrefabObjectCollection_Root = GameObjectCollection::Create();
	HSceneObject fifthPrefabHierarchy_Root = prefab_Root->Clone(fifthPrefabObjectCollection_Root, true); // Not yet used

	SPtr<GameObjectCollection> thirdPrefabObjectCollection_Child = GameObjectCollection::Create();
	HSceneObject thirdPrefabHierarchy_Child0 = prefab_Child0->Clone(thirdPrefabObjectCollection_Child, true); // Not yet used

	// Ensure links from instances to prefab are valid
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(*this, fourthInstantiatedHierarchy_Root, prefab_Root->GetRoot(), prefab_Root->GetId());
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(*this, fifthInstantiatedHierarchy_Root, prefab_Root->GetRoot(), prefab_Root->GetId());

	// Ensure internal prefab links are valid
	{
		TArray<UnitTestPrefabInformation> prefabInformation;
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Root, PrefabLinkCheckType::Regular));
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Child0, PrefabLinkCheckType::Regular));
		prefabInformation.Add(UnitTestPrefabInformation(prefab_Child1, PrefabLinkCheckType::Regular));

		UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabInternalsMatch(*this, 0, prefabInformation, true);
	}

	// Ensure IDs are unchanged from the previous instance and prefab internals
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, fourthInstantiatedHierarchy_Root, thirdInstantiatedHierarchy_Root, true);
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, fifthInstantiatedHierarchy_Root, thirdInstantiatedHierarchy_Root, true);
	UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(*this, prefab_Root->GetRoot(), fourthPrefabHierarchy_Root, false);


	// TODO - Add unit test that assigns the nested prefab's root instance to the root prefab
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
