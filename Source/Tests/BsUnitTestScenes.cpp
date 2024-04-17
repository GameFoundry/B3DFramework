//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsUnitTestScenes.h"
#include "Scene/BsPrefab.h"

using namespace bs;

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

UnitTestSceneB::UnitTestSceneB(const HSceneObject& parent, const SPtr<Prefab>& childPrefab)
	: SceneInstance(parent->GetScene()), Root(parent)
{
	SceneObject_0 = SceneInstance->CreateSceneObject("SceneB_SceneObject_0");
	SceneObject_0->SetParent(parent);

	SceneObject_1 = SceneInstance->CreateSceneObject("SceneB_SceneObject_1");
	SceneObject_1->SetParent(parent);

	SceneObject_1_0 = SceneInstance->CreateSceneObject("SceneB_SceneObject_1_0");
	SceneObject_1_0->SetParent(SceneObject_1);

	SceneObject_0->SetPosition(Vector3(50.0f, 50.0f, 50.0f));
	Component_1_0 = SceneObject_1_0->AddComponent<UnitTestComponentA>();

	if(childPrefab != nullptr)
	{
		OptionalSceneObject_0_0_PrefabInstance = childPrefab->Instantiate(SceneInstance);
		OptionalSceneObject_0_0_PrefabInstance->SetParent(SceneObject_0);
		OptionalSceneObject_0_0_PrefabInstance->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
	}
}

/** Populates the scene objects and components by looking them up in the provided hierarchy. */
UnitTestSceneB::UnitTestSceneB(const HSceneObject& root)
	: SceneInstance(root->GetScene()), Root(root)
{
	SceneObject_0 = root->FindChild("SceneB_SceneObject_0", false);
	SceneObject_1 = root->FindChild("SceneB_SceneObject_1", false);

	if(SceneObject_1.IsValid())
	{
		SceneObject_1_0 = SceneObject_1->FindChild("SceneB_SceneObject_1_0", false);

		if(SceneObject_1_0.IsValid())
			Component_1_0 = SceneObject_1_0->GetComponent<UnitTestComponentA>();
	}

	if(SceneObject_0->GetChildCount() > 0)
		OptionalSceneObject_0_0_PrefabInstance = SceneObject_0->GetChild(0);

	OptionalSceneObject_2 = root->FindChild("SceneB_SceneObject_2", false);
	if(OptionalSceneObject_2.IsValid())
		OptionalComponent_2 = OptionalSceneObject_2->GetComponent<UnitTestComponentA>();
}

HSceneObject UnitTestSceneB::PopulateNewSceneInstance(const char* name, const SPtr<Prefab>& childPrefab)
{
	SPtr<class SceneInstance> sceneInstance = SceneInstance::Create(name);
	UnitTestSceneB wrapper(sceneInstance->GetRoot(), childPrefab);

	return sceneInstance->GetRoot();
}

void UnitTestSceneB::PopulateParent(const HSceneObject& parent, const SPtr<Prefab>& childPrefab)
{
	UnitTestSceneB wrapper(parent, childPrefab);
}

void UnitTestSceneB::CreateOptionalObjects()
{
	OptionalSceneObject_2 = SceneInstance->CreateSceneObject("SceneB_SceneObject_2");
	OptionalSceneObject_2->SetParent(Root);

	OptionalComponent_2 = OptionalSceneObject_2->AddComponent<UnitTestComponentA>();
}
