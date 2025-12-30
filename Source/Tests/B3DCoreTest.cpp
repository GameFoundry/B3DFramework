//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/B3DConsoleTestOutput.h"
#include "Testing/B3DJSONTestOutput.h"
#include "Testing/B3DTestSuite.h"
#include "Animation/B3DAnimationCurve.h"
#include "Particles/B3DParticleDistribution.h"
#include "Serialization/B3DBinarySerializer.h"
#include "FileSystem/B3DDataStream.h"
#include "Scene/B3DComponent.h"
#include "Scene/B3DGameObjectManager.h"
#include "Scene/B3DGameObjectCollection.h"
#include "Scene/B3DPrefab.h"
#include "Scene/B3DSceneManager.h"
#include "Scene/B3DSceneObject.h"
#include "B3DApplication.h"
#include "Utility/B3DUtility.h"
#include "B3DUnitTestScenes.h"
#include "B3DUnitTestSerializationHelper.h"
#include "B3DUnitTestPrefabUpdateHelper.h"
#include "Resources/B3DResources.h"
#include "Scene/B3DPrefabUtility.h"
#include "Scene/B3DSceneInstance.h"
#include "Utility/B3DCommandLine.h"

using namespace b3d;

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
	void TestPrefabSaveLoad();
	void TestPrefabScenario1();
	void TestPrefabScenario2();
	void TestPrefabScenario3();
	void TestPrefabScenario4();
	void TestPrefabScenario5();
	void TestPrefabScenario6();
	void TestPrefabScenario7();
	void TestPrefabScenario8();
	void TestPrefabScenario9();
	void TestPrefabScenario10();

	void TestAssertPrefabScenario();

	SPtr<SceneInstance> mSceneHierarchy;
	UnitTestSceneB mScene;

	struct PrefabTestInformation
	{
		HPrefab Prefab;
		SPtr<UnitTestSceneB> PrefabInternalsScene;
	};

	Array<PrefabTestInformation, 4> mPrefabTestInformation;
};

CoreTestSuite::CoreTestSuite()
	: TestSuite("CoreTestSuite")
{
	B3D_ADD_TEST(CoreTestSuite::TestAnimCurveIntegration)
	B3D_ADD_TEST(CoreTestSuite::TestLookupTable)
	B3D_ADD_TEST(CoreTestSuite::TestBinarySerialization)
	B3D_ADD_TEST(CoreTestSuite::TestSerializedObject)
	B3D_ADD_TEST(CoreTestSuite::TestBinaryDelta)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabSaveLoad)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario1)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario2)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario3)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario4)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario5)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario6)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario7)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario8)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario9)
	B3D_ADD_TEST(CoreTestSuite::TestPrefabScenario10)

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
	RTTIOperationEngineContext rttiOperationContext;
	const SPtr<UnitTestSerializationObjectA> deserializedObject = B3DRTTICast<UnitTestSerializationObjectA>(serializedObject->Decode(rttiOperationContext));

	UnitTestSerializationHelpers::TestAssertObjectsMatch(*this, object, deserializedObject, false);
}

void CoreTestSuite::TestBinaryDelta()
{
	const SPtr<UnitTestSerializationObjectA> objectA = UnitTestSerializationObjectA::CreateVariantA();
	const SPtr<UnitTestSerializationObjectA> objectB = UnitTestSerializationObjectA::CreateVariantB();

	const SPtr<SerializedObject> serializedObjectA = SerializedObject::Create(*objectA.get());
	const SPtr<SerializedObject> serializedObjectB = SerializedObject::Create(*objectB.get());

	IDeltaHandler& deltaHandler = objectA->GetRtti()->GetDeltaHandler();
	RTTIOperationEngineContext generateDeltaRTTIOperationContext;
	SPtr<SerializedObject> delta = deltaHandler.GenerateDelta(serializedObjectA, serializedObjectB, generateDeltaRTTIOperationContext);

	RTTIOperationEngineContext applyDeltaRTTIOperationContext;
	deltaHandler.ApplyDelta(objectA, delta, applyDeltaRTTIOperationContext);

	UnitTestSerializationHelpers::TestAssertObjectsMatch(*this, objectA, objectB, true);
}

void CoreTestSuite::TestPrefabSaveLoad()
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
		RTTIOperationEngineContext rttiOperationContext;
		scene0Prefab = B3DRTTICast<Prefab>(serializer.Decode(serializedScene0Stream, (u32)serializedScene0Stream->Size(), rttiOperationContext));

		UnitTestSceneA scene0Wrapper(scene0Prefab->GetRoot());

		B3D_TEST_ASSERT(scene0Wrapper.Component_0_1_0->StringValue == "testValue")
		B3D_TEST_ASSERT(scene0Wrapper.Component_0->SceneObjectReference == scene0Wrapper.SceneObject_0_1_0)
		B3D_TEST_ASSERT(scene0Wrapper.Component_0->ComponentReference == scene0Wrapper.Component_1)
		B3D_TEST_ASSERT(scene0Wrapper.Component_0_1_0->SceneObjectReference == scene0Wrapper.SceneObject_0)
		B3D_TEST_ASSERT(scene0Wrapper.Component_0_1_0->ComponentReference == scene0Wrapper.Component_0)

		// Ensure the prefab stays loaded so tests below don't fail
		HResource scene0PRefabHandle = GetResources().CreateResourceHandle(scene0Prefab);
		GetResources().Load(scene0Prefab->GetId(), ResourceLoadOptions(false));
	}

	// Instantiate the scene and ensure prefab links are valid
	{
		SPtr<SceneInstance> instancedScene = scene0Prefab->InstantiateAsScene();
		HSceneObject instancedSceneRoot = instancedScene->GetRoot();
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

		UnitTestSceneB scene1Wrapper = UnitTestSceneB::PopulateParent(scene1Instance->GetRoot());
		scene1Wrapper.SetUnitTestSceneAChildPrefab_0_0(*scene0Prefab);

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
		RTTIOperationEngineContext rttiOperationContext;
		scene1Prefab = B3DRTTICast<Prefab>(serializer.Decode(serializedScene1Stream, (u32)serializedScene1Stream->Size(), rttiOperationContext));

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

void CoreTestSuite::TestAssertPrefabScenario()
{
	B3D_ASSERT(mPrefabTestInformation[0].Prefab != nullptr);

	UnorderedMap<UUID, SPtr<UnitTestSceneB>> prefabSceneLookup;
	for(auto& entry : mPrefabTestInformation)
	{
		if(entry.Prefab != nullptr)
			prefabSceneLookup[entry.Prefab.GetId()] = entry.PrefabInternalsScene;
	}

	{
		auto prefab1TestInformation = mPrefabTestInformation[0];

		const HPrefab prefab = prefab1TestInformation.Prefab;
		
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Ensure the instances correctly link to the prefab (both the hierarchy used to create/update the prefab, and newly instantiated hierarchy)
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		UnitTestPrefabUpdateHelper::TestAssertPrefabLinksMatchPrefabInternals_UnitTestSceneB(*this, mScene, nullptr, UUID::kEmpty, prefabSceneLookup);

		{
			SPtr<SceneInstance> instantiatedSceneInstance = prefab->InstantiateAsScene();
			HSceneObject instantiatedInstanceRoot = instantiatedSceneInstance->GetRoot();
			UnitTestSceneB instantiatedScene(instantiatedInstanceRoot);
			UnitTestPrefabUpdateHelper::TestAssertPrefabLinksMatchPrefabInternals_UnitTestSceneB(*this, instantiatedScene, nullptr, UUID::kEmpty, prefabSceneLookup);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Ensure IDs are unchanged from the previous instances
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		mScene.TestAssertHierarchyMatchesOriginalIds(*this);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Ensure nested prefabs within other prefabs correctly link to the nested prefab
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for(auto& entry : mPrefabTestInformation)
	{
		if(entry.PrefabInternalsScene != nullptr)
			entry.PrefabInternalsScene->TestAssertHierarchyMatchesPrefabLinks(*this, prefabSceneLookup);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Ensure IDs are unchanged from the previous prefab internals
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for(auto& entry : mPrefabTestInformation)
	{
		if(entry.Prefab == nullptr)
			continue;

		entry.PrefabInternalsScene->TestAssertHierarchyMatchesOriginalIds(*this);
	}
}

// Set up a scene, and create Prefab #1 from a brand new hierarchy. Verify instances of the prefab link to the prefab.
void CoreTestSuite::TestPrefabScenario1()
{
	// Create an instance for the main scene hierarchy
	mSceneHierarchy = SceneInstance::Create("Scene Hierarchy");
	HSceneObject sceneRoot = mSceneHierarchy->CreateSceneObject("Prefab #1 Instance Root");

	// Create hierarchy for Prefab 1 in the scene hierarchy
	mScene = UnitTestSceneB::PopulateParent(sceneRoot);

	// Construct the prefab from Prefab #1 Instance Root in the scene hierarchy
	mPrefabTestInformation[0].Prefab = Prefab::Create(mScene.Root);
	mPrefabTestInformation[0].PrefabInternalsScene = B3DMakeShared<UnitTestSceneB>(mPrefabTestInformation[0].Prefab->GetRoot());

	// Update scene information used for checks
	mScene.UpdatePrefabLinkIds();

	// Current state:
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//  
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]

	// Run the checks
	TestAssertPrefabScenario();
}

// Add an object to an instance of Prefab #1, then update the prefab.
void CoreTestSuite::TestPrefabScenario2()
{
	// Initial state:
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//  
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]

	// Add a new object to the scene hierarchy
	mScene.DestroySceneObject_1_0();
	mScene.CreateOptionalSceneObject_2();

	// Current state:
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     Optional object [Game Object ID = OBI2, Prefab Object ID = None, Prefab Resource ID = None] <- Modified

	// Update Prefab1 from Prefab #1 Instance Root in scene hierarchy
	PrefabUtility::UpdatePrefab(mPrefabTestInformation[0].Prefab, mScene.Root);

	// Current state:
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//   Optional object [Game Object ID = OB12, Prefab Object ID = OB12, Prefab Resource ID = PFB1] <- Modified
	//
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     Optional object [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1] <- Modified

	// Update scene information used for checks
	mPrefabTestInformation[0].PrefabInternalsScene->RefreshHierarchy(mPrefabTestInformation[0].Prefab->GetRoot());

	mScene.AddNewObjectIds(mScene.OptionalSceneObject_2);

	// Run the checks
	TestAssertPrefabScenario();
}

// Create Prefab2, then add its instance as a child of Prefab #1 Instance Root, and update Prefab1 with Prefab #1 Instance Root
void CoreTestSuite::TestPrefabScenario3()
{
	// Initial state:
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]

	// Create prefab 2
	NewSceneInstanceResult prefab2NewScene = UnitTestSceneB::PopulateNewSceneInstance("Prefab #2 Scene Instance");
	mPrefabTestInformation[1].Prefab = Prefab::Create(prefab2NewScene.Root);
	mPrefabTestInformation[1].PrefabInternalsScene = B3DMakeShared<UnitTestSceneB>(mPrefabTestInformation[1].Prefab->GetRoot());
	prefab2NewScene.Root->Destroy();

	// Add Prefab #2 Instance Root #1 in the scene, as child of Prefab 1 Instance Root/SceneObject_0
	HSceneObject prefab2Instance1 = mScene.SetUnitTestSceneBChildPrefab_0_0(*mPrefabTestInformation[1].Prefab);
	prefab2Instance1->SetName("Prefab #2 Instance Root #1");

	// Add Prefab #2 Instance Root #2 in the scene, as child of Prefab 1 Instance Root/SceneObject_1
	HSceneObject prefab2Instance2 = mScene.SetUnitTestSceneBChildPrefab_1_1(*mPrefabTestInformation[1].Prefab);
	prefab2Instance2->SetName("Prefab #2 Instance Root #2");

	// Current state:
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB21, Prefab Resource ID = PFB2] <- Modified
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB21, Prefab Resource ID = PFB2] <- Modified
	// 
	// ***PFB2***
	// PFB2 Instance [Game Object ID = OB21, Prefab Object ID = OB21, Prefab Resource ID = PFB2]

	// Update Prefab1 from Prefab #1 Instance Root in scene hierarchy
	PrefabUtility::UpdatePrefab(mPrefabTestInformation[0].Prefab, mScene.Root);

	// Current state:
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//   PFB2 Instance #1 [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2] <- Modified
	//   PFB2 Instance #2 [Game Object ID = OB13, Prefab Object ID = OB21, Prefab Resource ID = PFB2] <- Modified
	//
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1] <- Modified
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1] <- Modified

	// Update scene information used for checks
	mPrefabTestInformation[0].PrefabInternalsScene->RefreshHierarchy(mPrefabTestInformation[0].Prefab->GetRoot());

	mScene.OptionalPrefabInstance_0_0->UpdatePrefabLinkIds(mScene.OptionalPrefabInstance_0_0->Root);
	mScene.OptionalPrefabInstance_1_1->UpdatePrefabLinkIds(mScene.OptionalPrefabInstance_1_1->Root);
	
	// Run the checks
	TestAssertPrefabScenario();
}

// Add new object as part of Prefab #2 Instance Root #1, then update Prefab1 with Prefab #1 Instance Root
void CoreTestSuite::TestPrefabScenario4()
{
	// Initial state:
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]
	//
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//   PFB2 Instance #1 [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//   PFB2 Instance #2 [Game Object ID = OB13, Prefab Object ID = OB21, Prefab Resource ID = PFB2]

	mScene.OptionalPrefabInstance_0_0->DestroySceneObject_1_0();
	mScene.OptionalPrefabInstance_0_0->CreateOptionalSceneObject_2();

	// Current state:
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//       Optional object [Game Object ID = OBI4, Prefab Object ID = None, Prefab Resource ID = None] <- Modified
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]

	// Update Prefab1 from Prefab #1 Instance Root in scene hierarchy
	PrefabUtility::UpdatePrefab(mPrefabTestInformation[0].Prefab, mScene.Root);

	// Current state:
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//   PFB2 Instance #1 [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//     Optional object [Game Object ID = OB14, Prefab Object ID = OB14, Prefab Resource ID = PFB2] <- Modified
	//   PFB2 Instance #2 [Game Object ID = OB13, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//       Optional object [Game Object ID = OBI4, Prefab Object ID = OB14, Prefab Resource ID = PFB1] <- Modified
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]

	// Update scene information used for checks
	mPrefabTestInformation[0].PrefabInternalsScene->RefreshHierarchy(mPrefabTestInformation[0].Prefab->GetRoot());
	mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->SetFlagOnObject(mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->OptionalSceneObject_2, UnitTestSceneObjectFlag::IsInstanceModification);
	mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->SetFlagOnObject(mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->OptionalComponent_2, UnitTestSceneObjectFlag::IsInstanceModification);

	mScene.OptionalPrefabInstance_0_0->AddNewObjectIds(mScene.OptionalPrefabInstance_0_0->OptionalSceneObject_2);

	// Run the checks
	TestAssertPrefabScenario();
}

// Update Prefab2 with Prefab #2 Instance Root
void CoreTestSuite::TestPrefabScenario5()
{
	// Initial state:
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//       Optional object [Game Object ID = OBI4, Prefab Object ID = OB14, Prefab Resource ID = PFB1]
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]
	//
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//   PFB2 Instance #1 [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//     Optional object [Game Object ID = OB14, Prefab Object ID = OB14, Prefab Resource ID = PFB2]
	//   PFB2 Instance #2 [Game Object ID = OB13, Prefab Object ID = OB21, Prefab Resource ID = PFB2]

	// Update Prefab2 from Prefab #2 Instance Root in scene hierarchy
	PrefabUtility::UpdatePrefab(mPrefabTestInformation[1].Prefab, mScene.OptionalPrefabInstance_0_0->Root);

	// Current state:
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//   PFB2 Instance #1 [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//     Optional object [Game Object ID = OB14, Prefab Object ID = OB22, Prefab Resource ID = PFB2] <- Modified
	//   PFB2 Instance #2 [Game Object ID = OB13, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//
	// ***PFB2***
	// PFB2 Instance [Game Object ID = OB21, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//   Optional object [Game Object ID = OB22, Prefab Object ID = OB22, Prefab Resource ID = PFB2] <- Modified
	//
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//       Optional object [Game Object ID = OBI4, Prefab Object ID = OB14, Prefab Resource ID = PFB1]
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]

	// Update scene information used for checks
	mPrefabTestInformation[1].PrefabInternalsScene->RefreshHierarchy(mPrefabTestInformation[1].Prefab->GetRoot());

	mPrefabTestInformation[0].PrefabInternalsScene->RefreshHierarchy(mPrefabTestInformation[0].Prefab->GetRoot());
	mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->UpdatePrefabObjectIds(mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->OptionalSceneObject_2);
	mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->SetFlagOnObject(mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->OptionalSceneObject_2, UnitTestSceneObjectFlag::None);
	mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->SetFlagOnObject(mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->OptionalComponent_2, UnitTestSceneObjectFlag::None);

	mScene.OptionalPrefabInstance_1_1->RefreshHierarchy(mScene.OptionalPrefabInstance_1_1->Root);

	// Run the checks
	TestAssertPrefabScenario();
}

// Update Prefab1 from Prefab #1 Instance Root
void CoreTestSuite::TestPrefabScenario6()
{
	// Initial state:
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//       Optional object [Game Object ID = OBI4, Prefab Object ID = OB14, Prefab Resource ID = PFB1]
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]
	//
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//   PFB2 Instance #1 [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//     Optional object [Game Object ID = OB14, Prefab Object ID = OB22, Prefab Resource ID = PFB2]
	//   PFB2 Instance #2 [Game Object ID = OB13, Prefab Object ID = OB21, Prefab Resource ID = PFB2]

	// Update Prefab1 from Prefab #1 Instance Root in scene hierarchy
	PrefabUtility::UpdatePrefab(mPrefabTestInformation[0].Prefab, mScene.Root);

	// Current state:
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//       Optional object [Game Object ID = OBI4, Prefab Object ID = OB14, Prefab Resource ID = PFB1]
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]
	//
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//   PFB2 Instance #1 [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//     Optional object [Game Object ID = OB14, Prefab Object ID = OB22, Prefab Resource ID = PFB2]
	//   PFB2 Instance #2 [Game Object ID = OB13, Prefab Object ID = OB21, Prefab Resource ID = PFB2]

	// Update scene information used for checks
	mPrefabTestInformation[0].PrefabInternalsScene->RefreshHierarchy(mPrefabTestInformation[0].Prefab->GetRoot());

	// Run the checks
	TestAssertPrefabScenario();
}

// Create Prefab3, add it as child of Prefab #2 Instance Root, and update Prefab1 with Prefab #1 Instance Root
void CoreTestSuite::TestPrefabScenario7()
{
	// Initial state:
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]
	//  
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//   PFB2 Instance #1 [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//   PFB2 Instance #2 [Game Object ID = OB13, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//
	// ***PFB2***
	// PFB2 Instance [Game Object ID = OB21, Prefab Object ID = OB21, Prefab Resource ID = PFB2]

	// Create prefab 3
	NewSceneInstanceResult prefab3NewScene = UnitTestSceneB::PopulateNewSceneInstance("Prefab #3 Scene Instance");
	mPrefabTestInformation[2].Prefab = Prefab::Create(prefab3NewScene.Root);
	mPrefabTestInformation[2].PrefabInternalsScene = B3DMakeShared<UnitTestSceneB>(mPrefabTestInformation[2].Prefab->GetRoot());
	prefab3NewScene.Root->Destroy();

	// Add Prefab #3 Instance Root in the scene, as child of Prefab 2 Instance Root/SceneObject_0
	HSceneObject prefab3Instance = mScene.OptionalPrefabInstance_0_0->SetUnitTestSceneBChildPrefab_0_0(*mPrefabTestInformation[2].Prefab);
	prefab3Instance->SetName("Prefab #3 Instance Root");

	// Current state:
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//       PFB3 Instance #1 [Game Object ID = OBI4, Prefab Object ID = OB31, Prefab Resource ID = PFB3] <- Modified
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]

	// Update Prefab1 from Prefab #1 Instance Root in scene hierarchy
	PrefabUtility::UpdatePrefab(mPrefabTestInformation[0].Prefab, mScene.Root);

	// Current state:
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//   PFB2 Instance #1 [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//     PFB3 Instance #1 [Game Object ID = OB14, Prefab Object ID = OB31 Prefab Resource ID = PFB3] <- Modified
	//   PFB2 Instance #2 [Game Object ID = OB13, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//       PFB3 Instance #1 [Game Object ID = OBI4, Prefab Object ID = OB14, Prefab Resource ID = PFB1] <- Modified
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]

	// Update scene information used for checks
	mPrefabTestInformation[0].PrefabInternalsScene->RefreshHierarchy(mPrefabTestInformation[0].Prefab->GetRoot());
	mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->OptionalPrefabInstance_0_0->SetFlagOnObject(mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->OptionalPrefabInstance_0_0->Root, UnitTestSceneObjectFlag::IsPrefabRootInstanceModification);

	mScene.OptionalPrefabInstance_0_0->OptionalPrefabInstance_0_0->UpdatePrefabLinkIds();

	// Run the checks
	TestAssertPrefabScenario();
}

// Create Prefab4, add it a as a second child of Prefab #2 instanceRoot
void CoreTestSuite::TestPrefabScenario8()
{
	// Initial state:
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//       PFB3 Instance #1 [Game Object ID = OBI4, Prefab Object ID = OB14, Prefab Resource ID = PFB1]
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]
	//
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//   PFB2 Instance #1 [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//     PFB3 Instance #1 [Game Object ID = OB14, Prefab Object ID = OB31 Prefab Resource ID = PFB3]
	//   PFB2 Instance #2 [Game Object ID = OB13, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//
	// ***PFB2***
	// PFB2 Instance [Game Object ID = OB21, Prefab Object ID = OB21, Prefab Resource ID = PFB2]

	// Create prefab 4
	NewSceneInstanceResult prefab4NewScene = UnitTestSceneB::PopulateNewSceneInstance("Prefab #4 Scene Instance");
	mPrefabTestInformation[3].Prefab = Prefab::Create(prefab4NewScene.Root);
	mPrefabTestInformation[3].PrefabInternalsScene = B3DMakeShared<UnitTestSceneB>(mPrefabTestInformation[3].Prefab->GetRoot());
	prefab4NewScene.Root->Destroy();

	// Add Prefab #4 Instance Root in the scene, as child of Prefab 2 Instance Root/SceneObject_1
	HSceneObject prefab4Instance = mScene.OptionalPrefabInstance_0_0->SetUnitTestSceneBChildPrefab_1_1(*mPrefabTestInformation[3].Prefab);
	prefab4Instance->SetName("Prefab #4 Instance Root");

	// Current state:
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//       PFB3 Instance #1 [Game Object ID = OBI4, Prefab Object ID = OB14, Prefab Resource ID = PFB1]
	//       PFB4 Instance #1 [Game Object ID = OBI5, Prefab Object ID = OB41, Prefab Resource ID = PFB4] <- Modified
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]

	//// Update scene information used for checks
	mScene.OptionalPrefabInstance_0_0->OptionalPrefabInstance_1_1->SetFlagOnObject(mScene.OptionalPrefabInstance_0_0->OptionalPrefabInstance_1_1->Root, UnitTestSceneObjectFlag::IsPrefabRootInstanceModification);

	// Run the checks
	TestAssertPrefabScenario();
}

// Update Prefab2 from Prefab #2 Instance Root
void CoreTestSuite::TestPrefabScenario9()
{
	// Initial state:
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//       PFB3 Instance #1 [Game Object ID = OBI4, Prefab Object ID = OB14, Prefab Resource ID = PFB1]
	//       PFB4 Instance #1 [Game Object ID = OBI5, Prefab Object ID = OB41, Prefab Resource ID = PFB4]
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]
	//
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//   PFB2 Instance #1 [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//     PFB3 Instance #1 [Game Object ID = OB14, Prefab Object ID = OB31 Prefab Resource ID = PFB3]
	//   PFB2 Instance #2 [Game Object ID = OB13, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//
	// ***PFB2***
	// PFB2 Instance [Game Object ID = OB21, Prefab Object ID = OB21, Prefab Resource ID = PFB2]

	// Update Prefab2 from Prefab #2 Instance Root in scene hierarchy
	PrefabUtility::UpdatePrefab(mPrefabTestInformation[1].Prefab, mScene.OptionalPrefabInstance_0_0->Root);

	// Current state
	// ***PFB2***
	// PFB2 Instance [Game Object ID = OB21, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//   PFB3 Instance #1 [Game Object ID = OB22, Prefab Object ID = OB31, Prefab Resource ID = PFB3] <- Modified
	//   PFB4 Instance #1 [Game Object ID = OB23, Prefab Object ID = OB41, Prefab Resource ID = PFB4] <- Modified
	//
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//   PFB2 Instance #1 [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//     PFB3 Instance #1 [Game Object ID = OB14, Prefab Object ID = OB22 Prefab Resource ID = PFB2] <- Modified
	//     PFB4 Instance #1 [Game Object ID = OB15, Prefab Object ID = OB23, Prefab Resource ID = PFB2] <- Modified
	//   PFB2 Instance #2 [Game Object ID = OB13, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//     PFB3 Instance #2 [Game Object ID = OB16, Prefab Object ID = OB22 Prefab Resource ID = PFB2] <- Modified
	//     PFB4 Instance #2 [Game Object ID = OB17, Prefab Object ID = OB23, Prefab Resource ID = PFB2] <- Modified
	//
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//       PFB3 Instance #1 [Game Object ID = OBI4, Prefab Object ID = OB14, Prefab Resource ID = PFB1]
	//       PFB4 Instance #1 [Game Object ID = OBI5, Prefab Object ID = OB15, Prefab Resource ID = PFB1] <- Modified
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]
	//       PFB3 Instance #2 [Game Object ID = OBI6, Prefab Object ID = OB16, Prefab Resource ID = PFB1] <- Modified
	//       PFB4 Instance #2 [Game Object ID = OBI7, Prefab Object ID = OB17, Prefab Resource ID = PFB1] <- Modified

	// Update scene information used for checks
	mPrefabTestInformation[1].PrefabInternalsScene->RefreshHierarchy(mPrefabTestInformation[1].Prefab->GetRoot());

	mPrefabTestInformation[0].PrefabInternalsScene->RefreshHierarchy(mPrefabTestInformation[0].Prefab->GetRoot());
	mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->OptionalPrefabInstance_0_0->UpdatePrefabLinkIds();
	mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->OptionalPrefabInstance_0_0->SetFlagOnObject(mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->OptionalPrefabInstance_0_0->Root, UnitTestSceneObjectFlag::None);
	mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_1_1->OptionalPrefabInstance_0_0 = B3DMakeShared<UnitTestSceneB>(mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_1_1->OptionalSceneObject_0_0_PrefabInstance);
	mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_1_1->OptionalPrefabInstance_1_1 = B3DMakeShared<UnitTestSceneB>(mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_1_1->OptionalSceneObject_1_1_PrefabInstance);

	B3D_TEST_ASSERT(mScene.OptionalPrefabInstance_1_1->OptionalPrefabInstance_0_0 == nullptr)
	B3D_TEST_ASSERT(mScene.OptionalPrefabInstance_1_1->OptionalPrefabInstance_1_1 == nullptr)
	mScene.OptionalPrefabInstance_1_1->RefreshHierarchy(mScene.OptionalPrefabInstance_1_1->Root);
	mScene.OptionalPrefabInstance_1_1->OptionalPrefabInstance_0_0 = B3DMakeShared<UnitTestSceneB>(mScene.OptionalPrefabInstance_1_1->OptionalSceneObject_0_0_PrefabInstance);
	mScene.OptionalPrefabInstance_1_1->OptionalPrefabInstance_1_1 = B3DMakeShared<UnitTestSceneB>(mScene.OptionalPrefabInstance_1_1->OptionalSceneObject_1_1_PrefabInstance);
	mScene.OptionalPrefabInstance_0_0->OptionalPrefabInstance_1_1->SetFlagOnObject(mScene.OptionalPrefabInstance_0_0->OptionalPrefabInstance_1_1->Root, UnitTestSceneObjectFlag::None);
	mScene.OptionalPrefabInstance_0_0->OptionalPrefabInstance_1_1->UpdatePrefabLinkIds();

	// Run the checks
	TestAssertPrefabScenario();
}

// Add an object to Prefab #4 Instance, update Prefab #4
void CoreTestSuite::TestPrefabScenario10()
{
	// Initial state
	// ***PFB3***
	// PFB4 Instance [Game Object ID = OB41, Prefab Object ID = OB41, Prefab Resource ID = PFB4]
	//
	// ***PFB2***
	// PFB2 Instance [Game Object ID = OB21, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//   PFB3 Instance #1 [Game Object ID = OB22, Prefab Object ID = OB31, Prefab Resource ID = PFB3]
	//   PFB4 Instance #1 [Game Object ID = OB23, Prefab Object ID = OB41, Prefab Resource ID = PFB4]
	//
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//   PFB2 Instance #1 [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//     PFB3 Instance #1 [Game Object ID = OB14, Prefab Object ID = OB22 Prefab Resource ID = PFB2]
	//     PFB4 Instance #1 [Game Object ID = OB15, Prefab Object ID = OB23, Prefab Resource ID = PFB2]
	//   PFB2 Instance #2 [Game Object ID = OB13, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//     PFB3 Instance #2 [Game Object ID = OB16, Prefab Object ID = OB22 Prefab Resource ID = PFB2]
	//     PFB4 Instance #2 [Game Object ID = OB17, Prefab Object ID = OB23, Prefab Resource ID = PFB2]
	//
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//       PFB3 Instance #1 [Game Object ID = OBI4, Prefab Object ID = OB14, Prefab Resource ID = PFB1]
	//       PFB4 Instance #1 [Game Object ID = OBI5, Prefab Object ID = OB15, Prefab Resource ID = PFB1]
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]
	//       PFB3 Instance #2 [Game Object ID = OBI6, Prefab Object ID = OB16, Prefab Resource ID = PFB1]
	//       PFB4 Instance #2 [Game Object ID = OBI7, Prefab Object ID = OB17, Prefab Resource ID = PFB1]

	mScene.OptionalPrefabInstance_0_0->OptionalPrefabInstance_1_1->CreateOptionalSceneObject_2();
	PrefabUtility::UpdatePrefab(mPrefabTestInformation[3].Prefab, mScene.OptionalPrefabInstance_0_0->OptionalPrefabInstance_1_1->Root);

	// Current state
	// ***PFB3***
	// PFB4 Instance [Game Object ID = OB41, Prefab Object ID = OB41, Prefab Resource ID = PFB4]
	//   Optional object [Game Object ID = OB42, Prefab Object ID = OB42, Prefab Resource ID = PFB4] <- Modified
	//
	// ***PFB2***
	// PFB2 Instance [Game Object ID = OB21, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//   PFB3 Instance #1 [Game Object ID = OB22, Prefab Object ID = OB31, Prefab Resource ID = PFB3]
	//   PFB4 Instance #1 [Game Object ID = OB23, Prefab Object ID = OB41, Prefab Resource ID = PFB4]
	//     Optional object [Game Object ID = OB24, Prefab Object ID = OB42, Prefab Resource ID = PFB4] <- Modified
	//
	// ***PFB1***
	// PFB1 Instance [Game Object ID = OB11, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//   PFB2 Instance #1 [Game Object ID = OB12, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//     PFB3 Instance #1 [Game Object ID = OB14, Prefab Object ID = OB22 Prefab Resource ID = PFB2]
	//     PFB4 Instance #1 [Game Object ID = OB15, Prefab Object ID = OB23, Prefab Resource ID = PFB2]
	//       Optional object [Game Object ID = OB18, Prefab Object ID = OB24, Prefab Resource ID = PFB2] <- Modified
	//   PFB2 Instance #2 [Game Object ID = OB13, Prefab Object ID = OB21, Prefab Resource ID = PFB2]
	//     PFB3 Instance #2 [Game Object ID = OB16, Prefab Object ID = OB22 Prefab Resource ID = PFB2]
	//     PFB4 Instance #2 [Game Object ID = OB17, Prefab Object ID = OB23, Prefab Resource ID = PFB2]
	//       Optional object [Game Object ID = OB19, Prefab Object ID = OB24, Prefab Resource ID = PFB2] <- Modified
	//
	// ***Scene***
	// Root
	//   PFB1 Instance #1 [Game Object ID = OBI1, Prefab Object ID = OB11, Prefab Resource ID = PFB1]
	//     PFB2 Instance #1 [Game Object ID = OBI2, Prefab Object ID = OB12, Prefab Resource ID = PFB1]
	//       PFB3 Instance #1 [Game Object ID = OBI4, Prefab Object ID = OB14, Prefab Resource ID = PFB1]
	//       PFB4 Instance #1 [Game Object ID = OBI5, Prefab Object ID = OB15, Prefab Resource ID = PFB1]
	//         Optional object [Game Object ID = OBI8, Prefab Object ID = OB18, Prefab Resource ID = PFB1] <- Modified
	//     PFB2 Instance #2 [Game Object ID = OBI3, Prefab Object ID = OB13, Prefab Resource ID = PFB1]
	//       PFB3 Instance #2 [Game Object ID = OBI6, Prefab Object ID = OB16, Prefab Resource ID = PFB1]
	//       PFB4 Instance #2 [Game Object ID = OBI7, Prefab Object ID = OB17, Prefab Resource ID = PFB1]
	//         Optional object [Game Object ID = OBI9, Prefab Object ID = OB18, Prefab Resource ID = PFB1] <- Modified

	// Update scene information used for checks
	mPrefabTestInformation[3].PrefabInternalsScene->RefreshHierarchy(mPrefabTestInformation[3].Prefab->GetRoot());
	mPrefabTestInformation[3].PrefabInternalsScene->AddNewObjectIds(mPrefabTestInformation[3].PrefabInternalsScene->OptionalSceneObject_2);

	mPrefabTestInformation[1].PrefabInternalsScene->RefreshHierarchy(mPrefabTestInformation[1].Prefab->GetRoot());
	mPrefabTestInformation[1].PrefabInternalsScene->OptionalPrefabInstance_1_1->AddNewObjectIds(mPrefabTestInformation[1].PrefabInternalsScene->OptionalPrefabInstance_1_1->OptionalSceneObject_2);

	mPrefabTestInformation[0].PrefabInternalsScene->RefreshHierarchy(mPrefabTestInformation[0].Prefab->GetRoot());
	mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->OptionalPrefabInstance_1_1->AddNewObjectIds(mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_0_0->OptionalPrefabInstance_1_1->OptionalSceneObject_2);
	mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_1_1->OptionalPrefabInstance_1_1->AddNewObjectIds(mPrefabTestInformation[0].PrefabInternalsScene->OptionalPrefabInstance_1_1->OptionalPrefabInstance_1_1->OptionalSceneObject_2);

	mScene.OptionalPrefabInstance_0_0->OptionalPrefabInstance_1_1->AddNewObjectIds(mScene.OptionalPrefabInstance_0_0->OptionalPrefabInstance_1_1->OptionalSceneObject_2);
	mScene.OptionalPrefabInstance_1_1->OptionalPrefabInstance_1_1->RefreshHierarchy(mScene.OptionalPrefabInstance_1_1->OptionalPrefabInstance_1_1->Root);
	mScene.OptionalPrefabInstance_1_1->OptionalPrefabInstance_1_1->AddNewObjectIds(mScene.OptionalPrefabInstance_1_1->OptionalPrefabInstance_1_1->OptionalSceneObject_2);

	// Run the checks
	TestAssertPrefabScenario();

	// Clean up
	mScene.Reset();
	mSceneHierarchy = nullptr;

	for(auto& prefabTestInformation : mPrefabTestInformation)
	{
		prefabTestInformation.Prefab = nullptr;
		prefabTestInformation.PrefabInternalsScene = nullptr;
	}
}

// TODO - Add check to ensure circular references aren't possible
// TODO - Add check that I can't update Prefab A with Prefab B's instance (doesn't make sense from the editor perspective, so no need to handle this case)
// TODO - Add a check where I move PFB3 Instance #1 as a parent of PFB2 Instance #1
// - Actually, should disallow this as well? But how. I still want to allow adding a new prefab instance of another prefab
// TODO - Disallow moving objects between prefabs? Or automatically reset the prefab & object IDs when an object is reparented, so it's treated as a brand new instance
// - Handling this reparenting gracefully otherwise becomes very hard to reason about. It's better to just disallow it. It's a rare case anyway.
// TODO - Each nested prefab should have a root object that must always exist. Attempting to delete it without breaking a prefab link should fail in code, and in editor
//   show a dialog box warning. Moving an object out of that root, or within a nested prefab's root breaks the prefab link, as mentioned above.
// TODO - Perhaps even disallow direct changes to nested prefabs altogether, it makes tracing deltas tricky. If a nested prefab needs to be changed the user needs to
//   enter editing mode for that nested prefab only, and editor limits the operations to that prefab only. When done, user applies the changes to that prefab.
//  - But this doesn't really work. How to even perform instance changes at all then?
// TODO - Allow instance modifications only for prefab instances in Scene, i.e. a Prefab with nested prefabs is not allowed instance modifications?
//  - That might be limiting though. I might want to have a chunk of a scene as a Prefab

using namespace b3d;

int main(int argc, char* argv[])
{
	CrashHandler::StartUp();
	CommandLine::Initialize(argc, argv);

	String outputFormat = CommandLine::GetParameterValue("test-output-format", "console");
	String outputPath = CommandLine::GetParameterValue("test-output-path", "");

	VideoMode videoMode(1280, 720);
	Application::StartUp(videoMode, "UnitTests", false);

	SPtr<TestSuite> tests = CoreTestSuite::Create<CoreTestSuite>();

	i32 exitCode;
	if(outputFormat == "json")
	{
		Path jsonPath = outputPath.empty() ? Path("test_results.json") : Path(outputPath);
		JSONTestOutput testOutput(jsonPath);
		tests->Run(testOutput);
		exitCode = testOutput.GetExitCode();
	}
	else
	{
		ConsoleTestOutput testOutput;
		tests->Run(testOutput);
		exitCode = testOutput.GetExitCode();
	}

	Application::ShutDown();
	CrashHandler::ShutDown();

	return exitCode;
}
