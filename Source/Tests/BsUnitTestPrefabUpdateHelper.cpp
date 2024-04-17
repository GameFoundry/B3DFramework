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
	void UnitTestPrefabUpdateHelper::TestAssertPrefabLinkValid(TestSuite& testSuite, SceneWrapperType& instanceWrapper, SceneWrapperType& prefabWrapper, const UUID& prefabId, bool skipOptional)
	{
		instanceWrapper.PerformSceneObjectBinaryOperation(
			prefabWrapper, [prefabId, &testSuite](const HSceneObject& instanceSceneObject, const HSceneObject& prefabSceneObject)
			{
				B3D_TEST_ASSERT_EXTERNAL(testSuite, !instanceSceneObject->GetPrefabObjectId().Empty())
				B3D_TEST_ASSERT_EXTERNAL(testSuite, instanceSceneObject->GetPrefabObjectId() == prefabSceneObject->GetId())
				B3D_TEST_ASSERT_EXTERNAL(testSuite, instanceSceneObject->GetPrefabObjectId() != instanceSceneObject->GetId())
				B3D_TEST_ASSERT_EXTERNAL(testSuite, instanceSceneObject->GetPrefabResourceId() == prefabId) },
			skipOptional);

		instanceWrapper.PerformComponentBinaryOperation(
			prefabWrapper, [&testSuite](const HComponent& instanceComponent, const HComponent& prefabComponent)
			{
				B3D_TEST_ASSERT_EXTERNAL(testSuite, !instanceComponent->GetPrefabObjectId().Empty())
				B3D_TEST_ASSERT_EXTERNAL(testSuite, instanceComponent->GetPrefabObjectId() == prefabComponent->GetId())
				B3D_TEST_ASSERT_EXTERNAL(testSuite, instanceComponent->GetPrefabObjectId() != instanceComponent->GetId()) },
			skipOptional);
	}

	template <typename SceneWrapperType>
	void UnitTestPrefabUpdateHelper::TestAssetRootPrefabLinkValid(TestSuite& testSuite, SceneWrapperType& prefabWrapper, const UUID& prefabId, bool skipOptional)
	{
		prefabWrapper.PerformSceneObjectUnaryOperation([prefabId, &testSuite](const HSceneObject& sceneObject) {
			B3D_TEST_ASSERT_EXTERNAL(testSuite, sceneObject->GetPrefabObjectId() == sceneObject->GetId())
			B3D_TEST_ASSERT_EXTERNAL(testSuite, sceneObject->GetPrefabResourceId() == prefabId) }, skipOptional);
	}

	void UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(TestSuite& testSuite, const HSceneObject& instanceRoot, const HSceneObject& prefabRoot, const UUID& prefabId)
	{
		UnitTestSceneB instanceScene(instanceRoot);
		UnitTestSceneB prefabInternalsScene(prefabRoot);

		// Ensure that newly instantiated prefab instances have correct prefab object & resource IDs
		TestAssertPrefabLinkValid(testSuite, instanceScene, prefabInternalsScene, prefabId);

		if(instanceScene.OptionalSceneObject_0_0_PrefabInstance.IsValid())
		{
			TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(testSuite, instanceScene.OptionalSceneObject_0_0_PrefabInstance, prefabInternalsScene.OptionalSceneObject_0_0_PrefabInstance, prefabId);
		}
	}

	void UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabInternalsMatch(TestSuite& testSuite, u32 prefabIndex, const TArray<UnitTestPrefabInformation>& prefabs, bool checkNestedPrefabs)
	{
		if(!B3D_ENSURE(prefabIndex < (u32)prefabs.size()))
			return;

		const u32 prefabCount = (u32)prefabs.size() - prefabIndex;
		const UnitTestPrefabInformation& rootPrefabInformation = prefabs[prefabIndex];

		Vector<UnitTestSceneB> prefabInternals;
		prefabInternals.resize(prefabCount);

		prefabInternals[0] = UnitTestSceneB(rootPrefabInformation.Prefab->GetRoot());
		UnitTestSceneB& internals_Root = prefabInternals[0];

		// Ensure that prefab internals have correct prefab object & resource IDs (pointing to self for root, pointing to nested otherwise)
		TestAssetRootPrefabLinkValid(testSuite, internals_Root, rootPrefabInformation.Prefab->GetId());

		const u32 firstNestedPrefabIndex = prefabIndex + 1;
		if(firstNestedPrefabIndex >= (u32)prefabs.Size())
			return;

		const UnitTestPrefabInformation& firstNestedPrefabInformation = prefabs[firstNestedPrefabIndex];
		UnitTestSceneB internals_FirstNested(firstNestedPrefabInformation.Prefab->GetRoot());

		for(u32 nestedPrefabIndex = prefabIndex + 1; nestedPrefabIndex < (u32)prefabs.size(); ++nestedPrefabIndex)
		{
			const u32 nestedPrefabInternalsIndex = nestedPrefabIndex - prefabIndex;
			const UnitTestSceneB& internals_Parent = prefabInternals[nestedPrefabInternalsIndex - 1];

			const UnitTestPrefabInformation& nestedPrefabInformation = prefabs[nestedPrefabIndex];

			prefabInternals[nestedPrefabInternalsIndex] = UnitTestSceneB(internals_Parent.OptionalSceneObject_0_0_PrefabInstance);
			UnitTestSceneB& internals_Parent_Nested = prefabInternals[nestedPrefabInternalsIndex];

			if(nestedPrefabInformation.CheckType == PrefabLinkCheckType::Regular)
				TestAssertPrefabLinkValid(testSuite, internals_Parent_Nested, internals_FirstNested, firstNestedPrefabInformation.Prefab->GetId());
			else if(nestedPrefabInformation.CheckType == PrefabLinkCheckType::OptionalsAreInstanceModifications)
			{
				TestAssertPrefabLinkValid(testSuite, internals_Parent_Nested, internals_FirstNested, firstNestedPrefabInformation.Prefab->GetId(), true);

				// Optional object is not part of the child prefab resource, so it should have no prefab object (it should be treated as an instance modification)
				B3D_TEST_ASSERT_EXTERNAL(testSuite, !internals_Parent_Nested.OptionalSceneObject_2->GetPrefabObjectId().Empty())
				B3D_TEST_ASSERT_EXTERNAL(testSuite, internals_Parent_Nested.OptionalSceneObject_2->GetPrefabObjectId() == internals_Parent_Nested.OptionalSceneObject_2->GetId())
				B3D_TEST_ASSERT_EXTERNAL(testSuite, internals_Parent_Nested.OptionalSceneObject_2->GetPrefabResourceId() == firstNestedPrefabInformation.Prefab->GetId())

				B3D_TEST_ASSERT_EXTERNAL(testSuite, !internals_Parent_Nested.OptionalComponent_2->GetPrefabObjectId().Empty())
				B3D_TEST_ASSERT_EXTERNAL(testSuite, internals_Parent_Nested.OptionalComponent_2->GetPrefabObjectId() == internals_Parent_Nested.OptionalComponent_2.GetId())
			}
			else if(nestedPrefabInformation.CheckType == PrefabLinkCheckType::PrefabIsInstanceModification)
			{
				UnitTestSceneB internals_Nested(nestedPrefabInformation.Prefab->GetRoot());

				// Instance modification should link to the original prefab
				TestAssertPrefabLinkValid(testSuite, internals_Parent_Nested, internals_Nested, nestedPrefabInformation.Prefab->GetId());
			}

			if(checkNestedPrefabs)
				TestAssertUnitTestSceneBPrefabInternalsMatch(testSuite, nestedPrefabIndex, prefabs, false);
		}
	}

	void UnitTestPrefabUpdateHelper::TestAssertUnitTestSceneBPrefabLinksMatch(TestSuite& testSuite, const HSceneObject& lhsRoot, const HSceneObject& rhsRoot, bool ignoreGameObjectIds, bool skipOptional)
	{
		UnitTestSceneB unitTestSceneLHS(lhsRoot);
		UnitTestSceneB unitTestSceneRHS(rhsRoot);

		unitTestSceneLHS.PerformSceneObjectBinaryOperation(
			unitTestSceneRHS, [&testSuite, ignoreGameObjectIds](const HSceneObject& lhs, const HSceneObject& rhs) {
				B3D_TEST_ASSERT_EXTERNAL(testSuite, ignoreGameObjectIds || lhs.GetId() == rhs.GetId())
				B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs->GetPrefabObjectId() == rhs->GetPrefabObjectId())
				B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs->GetPrefabResourceId() == rhs->GetPrefabResourceId()) },
			skipOptional);

		unitTestSceneLHS.PerformComponentBinaryOperation(
			unitTestSceneRHS, [&testSuite, ignoreGameObjectIds](const HComponent& lhs, const HComponent& rhs) {
				B3D_TEST_ASSERT_EXTERNAL(testSuite, ignoreGameObjectIds || lhs.GetId() == rhs.GetId())
				B3D_TEST_ASSERT_EXTERNAL(testSuite, lhs->GetPrefabObjectId() == rhs->GetPrefabObjectId()) },
			skipOptional);

		if(unitTestSceneLHS.OptionalSceneObject_0_0_PrefabInstance.IsValid())
			TestAssertUnitTestSceneBPrefabLinksMatch(testSuite, unitTestSceneLHS.OptionalSceneObject_0_0_PrefabInstance, unitTestSceneRHS.OptionalSceneObject_0_0_PrefabInstance, ignoreGameObjectIds, skipOptional);
	}

} // namespace bs
