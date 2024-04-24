//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsUnitTestPrefabUpdateHelper.h"
#include "BsUnitTestScenes.h"
#include "Testing/BsTestSuite.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsPrefab.h"

namespace bs
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

	void UnitTestPrefabUpdateHelper::TestAssertPrefabLinksMatchPrefabInternals_UnitTestSceneB(TestSuite& testSuite, const HSceneObject& instanceRoot, const HSceneObject& prefabRoot, const UUID& prefabId)
	{
		UnitTestSceneB instanceScene(instanceRoot);
		UnitTestSceneB prefabInternalsScene(prefabRoot);

		// Ensure that newly instantiated prefab instances have correct prefab object & resource IDs
		TestAssertPrefabLinkValid(testSuite, instanceScene, prefabInternalsScene, prefabId);

		if(instanceScene.OptionalSceneObject_0_0_PrefabInstance.IsValid())
		{
			TestAssertPrefabLinksMatchPrefabInternals_UnitTestSceneB(testSuite, instanceScene.OptionalSceneObject_0_0_PrefabInstance, prefabInternalsScene.OptionalSceneObject_0_0_PrefabInstance, prefabId);
		}

		if(instanceScene.OptionalSceneObject_1_1_PrefabInstance.IsValid())
		{
			TestAssertPrefabLinksMatchPrefabInternals_UnitTestSceneB(testSuite, instanceScene.OptionalSceneObject_1_1_PrefabInstance, prefabInternalsScene.OptionalSceneObject_1_1_PrefabInstance, prefabId);
		}
	}

	template void UnitTestPrefabUpdateHelper::TestAssetRootPrefabLinkValid(TestSuite& testSuite, UnitTestSceneB& prefabWrapper, const UUID& prefabId);
} // namespace bs
