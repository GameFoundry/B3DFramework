//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUnitTestPrefabUpdateHelper.h"
#include "B3DUnitTestScenes.h"
#include "Testing/B3DTestSuite.h"
#include "Scene/B3DSceneObject.h"
#include "Scene/B3DPrefab.h"

namespace b3d
{
	template <typename SceneWrapperType>
	void UnitTestPrefabUpdateHelper::TestAssertPrefabLinkValid(TestSuite& testSuite, SceneWrapperType& instanceWrapper, SceneWrapperType& prefabWrapper, const UUID& prefabResourceId)
	{
		instanceWrapper.PerformSceneObjectBinaryOperation(
			prefabWrapper, [prefabResourceId, &testSuite, &instanceWrapper](const HSceneObject& instanceSceneObject, const HSceneObject& prefabSceneObject)
			{
				B3D_TEST_ASSERT_EXTERNAL(testSuite, !instanceSceneObject->GetPrefabObjectId().Empty())
				B3D_TEST_ASSERT_EXTERNAL(testSuite, instanceSceneObject->GetPrefabResourceId() == prefabResourceId)

				bool isInstanceModification = false;
				auto foundObjectInformation = instanceWrapper.ObjectInformation.find(instanceSceneObject.GetId());
				B3D_TEST_ASSERT_EXTERNAL(testSuite, foundObjectInformation != instanceWrapper.ObjectInformation.end())
				if(foundObjectInformation != instanceWrapper.ObjectInformation.end())
					isInstanceModification = foundObjectInformation->second.Flags.IsSet(UnitTestSceneObjectFlag::IsInstanceModification);

				if(isInstanceModification)
				{
					B3D_TEST_ASSERT_EXTERNAL(testSuite, instanceSceneObject->GetPrefabObjectId() == instanceSceneObject->GetId())
				}
				else
				{
					B3D_TEST_ASSERT_EXTERNAL(testSuite, instanceSceneObject->GetPrefabObjectId() == prefabSceneObject->GetId())
					B3D_TEST_ASSERT_EXTERNAL(testSuite, instanceSceneObject->GetPrefabObjectId() != instanceSceneObject->GetId())
				}
			});

		instanceWrapper.PerformComponentBinaryOperation(
			prefabWrapper, [&testSuite, &instanceWrapper](const HComponent& instanceComponent, const HComponent& prefabComponent)
			{
				bool isInstanceModification = false;
				auto foundObjectInformation = instanceWrapper.ObjectInformation.find(instanceComponent.GetId());
				B3D_TEST_ASSERT_EXTERNAL(testSuite, foundObjectInformation != instanceWrapper.ObjectInformation.end())
				if(foundObjectInformation != instanceWrapper.ObjectInformation.end())
					isInstanceModification = foundObjectInformation->second.Flags.IsSet(UnitTestSceneObjectFlag::IsInstanceModification);

				B3D_TEST_ASSERT_EXTERNAL(testSuite, !instanceComponent->GetPrefabObjectId().Empty())
				if(isInstanceModification)
				{
					B3D_TEST_ASSERT_EXTERNAL(testSuite, instanceComponent->GetPrefabObjectId() == instanceComponent->GetId())
				}
				else
				{
					B3D_TEST_ASSERT_EXTERNAL(testSuite, instanceComponent->GetPrefabObjectId() == prefabComponent->GetId())
					B3D_TEST_ASSERT_EXTERNAL(testSuite, instanceComponent->GetPrefabObjectId() != instanceComponent->GetId()) }
				}
			);
	}

	template <typename SceneWrapperType>
	void UnitTestPrefabUpdateHelper::TestAssetRootPrefabLinkValid(TestSuite& testSuite, SceneWrapperType& prefabWrapper, const UUID& prefabId)
	{
		prefabWrapper.PerformSceneObjectUnaryOperation([prefabId, &testSuite](const HSceneObject& sceneObject) {
			B3D_TEST_ASSERT_EXTERNAL(testSuite, sceneObject->GetPrefabObjectId() == sceneObject->GetId())
			B3D_TEST_ASSERT_EXTERNAL(testSuite, sceneObject->GetPrefabResourceId() == prefabId) });
	}

	void UnitTestPrefabUpdateHelper::TestAssertPrefabLinksMatchPrefabInternals_UnitTestSceneB(TestSuite& testSuite, UnitTestSceneB& instanceScene, const SPtr<UnitTestSceneB>& parentPrefabScene, const UUID& parentPrefabId, const UnorderedMap<UUID, SPtr<UnitTestSceneB>>& prefabSceneLookup)
	{
		const auto found = instanceScene.ObjectInformation.find(instanceScene.Root.GetId());
		if(!B3D_ENSURE(found != instanceScene.ObjectInformation.end()))
			return;

		const bool isInstanceModification = found->second.Flags.IsSet(UnitTestSceneObjectFlag::IsPrefabRootInstanceModification);

		UUID prefabId;
		SPtr<UnitTestSceneB> prefabScene;
		if(isInstanceModification || parentPrefabScene == nullptr)
		{
			prefabId = instanceScene.Root->GetPrefabResourceId();

			if(auto foundPrefab = prefabSceneLookup.find(prefabId); foundPrefab != prefabSceneLookup.end())
				prefabScene = foundPrefab->second;

			if(!B3D_ENSURE(prefabScene))
				return;
		}
		else
		{
			prefabId = parentPrefabId;
			prefabScene = parentPrefabScene;
		}

		// Ensure that newly instantiated prefab instances have correct prefab object & resource IDs
		TestAssertPrefabLinkValid(testSuite, instanceScene, *prefabScene, prefabId);

		if(instanceScene.OptionalSceneObject_0_0_PrefabInstance.IsValid())
		{
			TestAssertPrefabLinksMatchPrefabInternals_UnitTestSceneB(testSuite, *instanceScene.OptionalPrefabInstance_0_0, prefabScene->OptionalPrefabInstance_0_0, prefabId, prefabSceneLookup);
		}

		if(instanceScene.OptionalSceneObject_1_1_PrefabInstance.IsValid())
		{
			TestAssertPrefabLinksMatchPrefabInternals_UnitTestSceneB(testSuite, *instanceScene.OptionalPrefabInstance_1_1,  prefabScene->OptionalPrefabInstance_1_1, prefabId, prefabSceneLookup);
		}
	}

	template void UnitTestPrefabUpdateHelper::TestAssetRootPrefabLinkValid(TestSuite& testSuite, UnitTestSceneB& prefabWrapper, const UUID& prefabId);
} // namespace b3d
