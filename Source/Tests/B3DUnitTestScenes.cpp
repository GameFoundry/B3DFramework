//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUnitTestScenes.h"

#include "Scene/B3DPrefab.h"
#include "Testing/B3DTestSuite.h"
#include "B3DUnitTestPrefabUpdateHelper.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

UnitTestSceneA::UnitTestSceneA(const SPtr<SceneInstance>& sceneInstance)
{
	SceneObject_0 = sceneInstance->CreateSceneObject("SceneA_SceneObject_0");
	SceneObject_0_0 = sceneInstance->CreateSceneObject("SceneA_SceneObject_0_0");
	SceneObject_0_1 = sceneInstance->CreateSceneObject("SceneA_SceneObject_0_1");
	SceneObject_0_1_0 = sceneInstance->CreateSceneObject("SceneA_SceneObject_0_1_0");
	SceneObject_1 = sceneInstance->CreateSceneObject("SceneA_SceneObject_1");
	SceneObject_1_0 = sceneInstance->CreateSceneObject("SceneA_SceneObject_1_0");

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

UnitTestSceneA::UnitTestSceneA(const HSceneObject& root)
{
	SceneObject_0 = root->FindChild("SceneA_SceneObject_0", false);
	SceneObject_0_0 = SceneObject_0->FindChild("SceneA_SceneObject_0_0", false);
	SceneObject_0_1 = SceneObject_0->FindChild("SceneA_SceneObject_0_1", false);
	SceneObject_0_1_0 = SceneObject_0_1->FindChild("SceneA_SceneObject_0_1_0", false);
	SceneObject_1 = root->FindChild("SceneA_SceneObject_1", false);
	SceneObject_1_0 = SceneObject_1->FindChild("SceneA_SceneObject_1_0", false);

	Component_0 = SceneObject_0->GetComponent<UnitTestComponentA>();
	Component_1 = SceneObject_1->GetComponent<UnitTestComponentB>();
	Component_0_1 = SceneObject_0_1->GetComponent<UnitTestComponentA>();
	Component_0_1_0 = SceneObject_0_1_0->GetComponent<UnitTestComponentA>();
}

/** Populates the scene objects and components by looking them up in the provided hierarchy. */
UnitTestSceneB::UnitTestSceneB(const HSceneObject& root)
	: SceneInstance(root->GetScene()), Root(root)
{
	RefreshHierarchy(root);

	// Record IDs
	PerformSceneObjectUnaryOperation([this](const HSceneObject& sceneObject) {
		ObjectInformation[sceneObject.GetId()] = PrefabLinkInformation(sceneObject->GetPrefabObjectId(), sceneObject->GetPrefabResourceId());
		for(const auto& component : sceneObject->GetComponents())
			ObjectInformation[component.GetId()] = PrefabLinkInformation(component->GetPrefabObjectId(), sceneObject->GetPrefabResourceId());
	 });
}

HSceneObject UnitTestSceneB::PopulateNewSceneInstance(const char* name)
{
	SPtr<class SceneInstance> sceneInstance = SceneInstance::Create(name);
	UnitTestSceneB wrapper = PopulateParent(sceneInstance->GetRoot());

	return sceneInstance->GetRoot();
}

UnitTestSceneB UnitTestSceneB::PopulateParent(const HSceneObject& parent)
{
	UnitTestSceneB scene;

	scene.SceneInstance = parent->GetScene();
	scene.Root = parent;

	scene.SceneObject_0 = scene.SceneInstance->CreateSceneObject("SceneB_SceneObject_0");
	scene.SceneObject_0->SetParent(parent);

	scene.SceneObject_1 = scene.SceneInstance->CreateSceneObject("SceneB_SceneObject_1");
	scene.SceneObject_1->SetParent(parent);

	scene.SceneObject_1_0 = scene.SceneInstance->CreateSceneObject("SceneB_SceneObject_1_0");
	scene.SceneObject_1_0->SetParent(scene.SceneObject_1);
	scene.SceneObject_1_0_Id = scene.SceneObject_1_0.GetId();

	scene.SceneObject_0->SetPosition(Vector3(50.0f, 50.0f, 50.0f));
	scene.Component_1_0 = scene.SceneObject_1_0->AddComponent<UnitTestComponentA>();
	scene.Component_1_0->SetName("SceneB_Component_1_0");
	scene.Component_1_0_Id = scene.Component_1_0.GetId();

	scene.AddOrUpdateIds(scene.Root, true, true, true);

	return scene;
}

HSceneObject UnitTestSceneB::SetUnitTestSceneAChildPrefab_0_0(const Prefab& prefab)
{
	B3D_ASSERT(!OptionalSceneObject_0_0_PrefabInstance.IsValid());

	OptionalSceneObject_0_0_PrefabInstance = prefab.Instantiate(SceneInstance);
	OptionalSceneObject_0_0_PrefabInstance->SetParent(SceneObject_0);
	OptionalSceneObject_0_0_PrefabInstance->SetPosition(Vector3(0.0f, 0.0f, 0.0f));

	return OptionalSceneObject_0_0_PrefabInstance;
}

HSceneObject UnitTestSceneB::SetUnitTestSceneBChildPrefab_0_0(const Prefab& prefab)
{
	B3D_ASSERT(!OptionalSceneObject_0_0_PrefabInstance.IsValid());

	OptionalSceneObject_0_0_PrefabInstance = prefab.Instantiate(SceneInstance);
	OptionalSceneObject_0_0_PrefabInstance->SetParent(SceneObject_0);
	OptionalSceneObject_0_0_PrefabInstance->SetPosition(Vector3(0.0f, 0.0f, 0.0f));

	OptionalPrefabInstance_0_0 = B3DMakeShared<UnitTestSceneB>(OptionalSceneObject_0_0_PrefabInstance);
	return OptionalSceneObject_0_0_PrefabInstance;
}

HSceneObject UnitTestSceneB::SetUnitTestSceneBChildPrefab_1_1(const Prefab& prefab)
{
	B3D_ASSERT(!OptionalSceneObject_1_1_PrefabInstance.IsValid());

	OptionalSceneObject_1_1_PrefabInstance = prefab.Instantiate(SceneInstance);
	OptionalSceneObject_1_1_PrefabInstance->SetParent(SceneObject_1);
	OptionalSceneObject_1_1_PrefabInstance->SetPosition(Vector3(0.0f, 0.0f, 0.0f));

	OptionalPrefabInstance_1_1 = B3DMakeShared<UnitTestSceneB>(OptionalSceneObject_1_1_PrefabInstance);
	return OptionalSceneObject_1_1_PrefabInstance;
}

void UnitTestSceneB::CreateOptionalSceneObject_2()
{
	OptionalSceneObject_2 = SceneInstance->CreateSceneObject("SceneB_SceneObject_2");
	OptionalSceneObject_2->SetParent(Root);

	OptionalComponent_2 = OptionalSceneObject_2->AddComponent<UnitTestComponentA>();
	OptionalComponent_2->SetName("SceneB_Component_2");
}

void UnitTestSceneB::DestroySceneObject_1_0()
{
	ObjectInformation.erase(SceneObject_1_0.GetId());
	ObjectInformation.erase(Component_1_0.GetId());

	SceneObject_1_0->Destroy();
	Component_1_0->Destroy();

	SceneObject_1_0 = nullptr;
	Component_1_0 = nullptr;

	SceneObject_1_0_Id = UUID::kEmpty;
	Component_1_0_Id = UUID::kEmpty;
}

void UnitTestSceneB::RefreshHierarchy(const HSceneObject& root)
{
	Root = root;
	SceneObject_0 = root->FindChild("SceneB_SceneObject_0", false);
	SceneObject_1 = root->FindChild("SceneB_SceneObject_1", false);

	HSceneObject newSceneObject_1_0;
	HUnitTestComponentA newComponent_1_0;
	if(SceneObject_1.IsValid())
	{
		newSceneObject_1_0 = SceneObject_1->FindChild("SceneB_SceneObject_1_0", false);

		if(newSceneObject_1_0.IsValid())
			newComponent_1_0 = newSceneObject_1_0->GetComponent<UnitTestComponentA>();

		const u32 childCount = SceneObject_1->GetChildCount();
		for(u32 childIndex = 0; childIndex < childCount; childIndex++)
		{
			HSceneObject child = SceneObject_1->GetChild(childIndex);
			if(child != newSceneObject_1_0)
			{
				OptionalSceneObject_1_1_PrefabInstance = child;

				// Prefab may be added after initial construction
				if(OptionalPrefabInstance_1_1 == nullptr)
					OptionalPrefabInstance_1_1 = B3DMakeShared<UnitTestSceneB>(OptionalSceneObject_1_1_PrefabInstance);
				else
					OptionalPrefabInstance_1_1->RefreshHierarchy(OptionalSceneObject_1_1_PrefabInstance);

				break;
			}
		}
	}

	if(SceneObject_0.IsValid() && SceneObject_0->GetChildCount() > 0)
	{
		OptionalSceneObject_0_0_PrefabInstance = SceneObject_0->GetChild(0);

		// Prefab may be added after initial construction
		if(OptionalPrefabInstance_0_0 == nullptr)
			OptionalPrefabInstance_0_0 = B3DMakeShared<UnitTestSceneB>(OptionalSceneObject_0_0_PrefabInstance);
		else
			OptionalPrefabInstance_0_0->RefreshHierarchy(OptionalSceneObject_0_0_PrefabInstance);
	}

	bool hadOptionalSceneObject_2 = OptionalSceneObject_2 != nullptr;

	// These objects may be added after initial construction
	OptionalSceneObject_2 = root->FindChild("SceneB_SceneObject_2", false);
	if(OptionalSceneObject_2.IsValid())
	{
		OptionalComponent_2 = OptionalSceneObject_2->GetComponent<UnitTestComponentA>();

		if(!hadOptionalSceneObject_2)
			AddNewObjectIds(OptionalSceneObject_2);
	}

	// These objects can be destroyed after initial construction
	if(!SceneObject_1_0_Id.Empty() && newSceneObject_1_0 == nullptr)
	{
		ObjectInformation.erase(SceneObject_1_0_Id);
		ObjectInformation.erase(Component_1_0_Id);

		SceneObject_1_0 = nullptr;
		Component_1_0 = nullptr;

		SceneObject_1_0_Id = UUID::kEmpty;
		Component_1_0_Id = UUID::kEmpty;
	}
	else
	{
		SceneObject_1_0 = newSceneObject_1_0;
		Component_1_0 = newComponent_1_0;

		SceneObject_1_0_Id = SceneObject_1_0.GetId();
		Component_1_0_Id = Component_1_0.GetId();
	}
}

void UnitTestSceneB::Reset()
{
	Root = nullptr;
	SceneObject_0 = nullptr;
	OptionalSceneObject_0_0_PrefabInstance = nullptr;
	SceneObject_1 = nullptr;
	SceneObject_1_0 = nullptr;
	OptionalSceneObject_1_1_PrefabInstance = nullptr;
	OptionalSceneObject_2 = nullptr;

	Component_1_0 = nullptr;
	OptionalComponent_2 = nullptr;

	SceneObject_1_0_Id = UUID::kEmpty;
	Component_1_0_Id = UUID::kEmpty;

	SceneInstance = nullptr;
	OptionalPrefabInstance_0_0 = nullptr;
	OptionalPrefabInstance_1_1 = nullptr;

	ObjectInformation.clear();
}

void UnitTestSceneB::AddOrUpdateIds(HSceneObject object, bool updatePrefabObjectId, bool updatePrefabResourceId, bool allowAddNew)
{
	object->IterateHierarchy([this, updatePrefabObjectId, updatePrefabResourceId, allowAddNew](const HSceneObject& sceneObject)
	{
		if(sceneObject == OptionalSceneObject_0_0_PrefabInstance || sceneObject == OptionalSceneObject_1_1_PrefabInstance)
			return false;

		{
			auto foundSceneObject = ObjectInformation.find(sceneObject.GetId());
			if(foundSceneObject != ObjectInformation.end())
			{
				if(updatePrefabObjectId)
					foundSceneObject->second.PrefabLink.PrefabObjectId = sceneObject->GetPrefabObjectId();

				if(updatePrefabResourceId)
					foundSceneObject->second.PrefabLink.PrefabResourceId = sceneObject->GetPrefabResourceId();
			}
			else if(B3D_ENSURE(allowAddNew))
				ObjectInformation[sceneObject.GetId()] = PrefabLinkInformation(sceneObject->GetPrefabObjectId(), sceneObject->GetPrefabResourceId());
		}

		for(const auto& component : sceneObject->GetComponents())
		{
			auto foundComponent = ObjectInformation.find(component.GetId());
			if(foundComponent != ObjectInformation.end())
			{
				if(updatePrefabObjectId)
					foundComponent->second.PrefabLink.PrefabObjectId = component->GetPrefabObjectId();

				if(updatePrefabResourceId)
					foundComponent->second.PrefabLink.PrefabResourceId = sceneObject->GetPrefabResourceId();
			}
			else if(B3D_ENSURE(allowAddNew))
				ObjectInformation[component.GetId()] = PrefabLinkInformation(component->GetPrefabObjectId(), sceneObject->GetPrefabResourceId());
		}

		return true;
		
	}, nullptr, true);
}

void UnitTestSceneB::SetFlagOnObject(const GameObjectHandle& object, UnitTestSceneObjectFlags flags)
{
	auto found = ObjectInformation.find(object.GetId());
	if(!B3D_ENSURE(found != ObjectInformation.end()))
		return;

	found->second.Flags = flags;
}

void UnitTestSceneB::TestAssertHierarchyMatchesOriginalIds(TestSuite& testSuite)
{
	u32 foundIdCount = 0;
	PerformSceneObjectUnaryOperation([this, &testSuite, &foundIdCount](const HSceneObject& sceneObject) {
		auto foundSceneObject = ObjectInformation.find(sceneObject.GetId());
		B3D_TEST_ASSERT_EXTERNAL(testSuite, foundSceneObject != ObjectInformation.end())

		if(foundSceneObject != ObjectInformation.end())
		{
			B3D_TEST_ASSERT_EXTERNAL(testSuite, foundSceneObject->second.PrefabLink.PrefabObjectId == sceneObject->GetPrefabObjectId())
			B3D_TEST_ASSERT_EXTERNAL(testSuite, foundSceneObject->second.PrefabLink.PrefabResourceId == sceneObject->GetPrefabResourceId())

			foundIdCount++;
		}

		for(const auto& component : sceneObject->GetComponents())
		{
			auto foundComponent = ObjectInformation.find(component.GetId());
			B3D_TEST_ASSERT_EXTERNAL(testSuite, foundComponent != ObjectInformation.end())

			if(foundComponent != ObjectInformation.end())
			{
				B3D_TEST_ASSERT_EXTERNAL(testSuite, foundComponent->second.PrefabLink.PrefabObjectId == component->GetPrefabObjectId())
				foundIdCount++;
			}
		}


		if(OptionalPrefabInstance_0_0)
			OptionalPrefabInstance_0_0->TestAssertHierarchyMatchesOriginalIds(testSuite);

		if(OptionalPrefabInstance_1_1)
			OptionalPrefabInstance_1_1->TestAssertHierarchyMatchesOriginalIds(testSuite);
	 });

	B3D_TEST_ASSERT_EXTERNAL(testSuite, foundIdCount == (u32)ObjectInformation.size())
}


void UnitTestSceneB::TestAssertHierarchyMatchesPrefabLinks(TestSuite& testSuite, const UnorderedMap<UUID, SPtr<UnitTestSceneB>>& prefabSceneLookup, u32 nestingLevel, const UUID& parentPrefabId, const SPtr<UnitTestSceneB>& parentPrefabScene)
{
	if(nestingLevel == 0)
	{
		const UUID rootPrefabId = Root->GetPrefabResourceId();
		UnitTestPrefabUpdateHelper::TestAssetRootPrefabLinkValid(testSuite, *this, rootPrefabId);
	}
	else
	{
		if(!B3D_ENSURE(parentPrefabScene != nullptr))
			return;

		if(!B3D_ENSURE(!parentPrefabId.Empty()))
			return;

		UnitTestPrefabUpdateHelper::TestAssertPrefabLinkValid(testSuite, *this, *parentPrefabScene, parentPrefabId);
	}

	auto fnVisitChildPrefabInstance = [&testSuite, &prefabSceneLookup, nestingLevel, parentPrefabId](const SPtr<UnitTestSceneB>& childPrefabInstance, const SPtr<UnitTestSceneB>& otherChildPrefabInstance)
	{
		if(childPrefabInstance == nullptr)
			return;

		const auto found = childPrefabInstance->ObjectInformation.find(childPrefabInstance->Root.GetId());
		if(!B3D_ENSURE(found != childPrefabInstance->ObjectInformation.end()))
			return;

		const bool isInstanceModification = found->second.Flags.IsSet(UnitTestSceneObjectFlag::IsPrefabRootInstanceModification);

		UUID nestedPrefabId;
		SPtr<UnitTestSceneB> nestedPrefabScene;
		if(isInstanceModification || nestingLevel == 0)
		{
			nestedPrefabId = childPrefabInstance->Root->GetPrefabResourceId();

			if(auto foundPrefab = prefabSceneLookup.find(nestedPrefabId); foundPrefab != prefabSceneLookup.end())
				nestedPrefabScene = foundPrefab->second;

			if(!B3D_ENSURE(nestedPrefabScene))
				return;
		}
		else
		{
			nestedPrefabId = parentPrefabId;
			nestedPrefabScene = otherChildPrefabInstance;
		}

		childPrefabInstance->TestAssertHierarchyMatchesPrefabLinks(testSuite, prefabSceneLookup, nestingLevel + 1, nestedPrefabId, nestedPrefabScene);
	};

	fnVisitChildPrefabInstance(OptionalPrefabInstance_0_0, parentPrefabScene != nullptr ? parentPrefabScene->OptionalPrefabInstance_0_0 : nullptr);
	fnVisitChildPrefabInstance(OptionalPrefabInstance_1_1, parentPrefabScene != nullptr ? parentPrefabScene->OptionalPrefabInstance_1_1 : nullptr);
}
