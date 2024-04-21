//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsUnitTestPrefabUpdateHelper.h"
#include "BsUnitTestScenes.h"
#include "Testing/BsTestSuite.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsPrefab.h"

namespace bs
{
	void UnitTestPrefabObjectOptions::SetFlagsForObject(const UUID& rootInstanceId, const HPrefab& prefab, const GameObjectHandleBase& gameObject, UnitTestPrefabObjectOptionFlags flags)
	{
		UnorderedMap<UUID, UnorderedMap<String, UnitTestPrefabObjectOptionFlags>>& objectOptionsPerPrefab = ObjectOptionsPerPrefabPerInstance[rootInstanceId];
		UnorderedMap<String, UnitTestPrefabObjectOptionFlags>& objectOptions = objectOptionsPerPrefab[prefab.GetId()];
		objectOptions[gameObject->GetName()] = flags;
	}

	void UnitTestPrefabObjectOptions::SetFlagsForObject(const UUID& rootInstanceId, const HPrefab& prefab, const HSceneObject& sceneObject, UnitTestPrefabObjectOptionFlags flags, bool setOnChildren)
	{
		UnorderedMap<UUID, UnorderedMap<String, UnitTestPrefabObjectOptionFlags>>& objectOptionsPerPrefab = ObjectOptionsPerPrefabPerInstance[rootInstanceId];
		UnorderedMap<String, UnitTestPrefabObjectOptionFlags>& objectOptions = objectOptionsPerPrefab[prefab.GetId()];
		objectOptions[sceneObject->GetName()] = flags;

		if(setOnChildren)
		{
			sceneObject->IterateHierarchy([flags, &objectOptions](const HSceneObject& child)
										  {
				objectOptions[child->GetName()] = flags;

				if(child->IsPrefabInstanceRoot())
					return false; // Don't recurse into other prefab instances
				return true; },
				  [flags, &objectOptions](const HComponent& component)
				  {
					  objectOptions[component->GetName()] = flags;
				  },
				  false);
		}
	}

	UnitTestPrefabObjectOptionFlags UnitTestPrefabObjectOptions::GetFlagsForObject(const UUID& rootInstanceId, const UUID& prefabId, const String& name) const
	{
		if(auto foundInstance = ObjectOptionsPerPrefabPerInstance.find(rootInstanceId); foundInstance != ObjectOptionsPerPrefabPerInstance.end())
		{
			if(auto foundPrefab = foundInstance->second.find(prefabId); foundPrefab != foundInstance->second.end())
			{
				if(auto foundObject = foundPrefab->second.find(name); foundObject != foundPrefab->second.end())
					return GlobalOptions | foundObject->second;
			}
		}

		return GlobalOptions;
	}

	void UnitTestPrefabObjectOptions::ClearAllObjectFlags()
	{
		ObjectOptionsPerPrefabPerInstance.clear();
	}

	template <typename SceneWrapperType>
	void UnitTestPrefabUpdateHelper::TestAssertPrefabLinkValid(TestSuite& testSuite, SceneWrapperType& instanceWrapper, SceneWrapperType& prefabWrapper, const UUID& prefabResourceId, const UUID& instanceRootId, const UUID& instancePrefabId, const UnitTestPrefabObjectOptions& options)
	{
		instanceWrapper.PerformSceneObjectBinaryOperation(
			prefabWrapper, [instanceRootId, instancePrefabId, prefabResourceId, &options, &testSuite](const HSceneObject& instanceSceneObject, const HSceneObject& prefabSceneObject)
			{
				const UnitTestPrefabObjectOptionFlags flags = options.GetFlagsForObject(instanceRootId, instancePrefabId, instanceSceneObject->GetName());

				B3D_TEST_ASSERT_EXTERNAL(testSuite, !instanceSceneObject->GetPrefabObjectId().Empty())
				B3D_TEST_ASSERT_EXTERNAL(testSuite, instanceSceneObject->GetPrefabResourceId() == prefabResourceId)

				if(flags.IsSet(UnitTestPrefabObjectOptionFlag::IsInstanceModification))
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
			prefabWrapper, [instanceRootId, instancePrefabId, &options, &testSuite](const HComponent& instanceComponent, const HComponent& prefabComponent)
			{
				const UnitTestPrefabObjectOptionFlags flags = options.GetFlagsForObject(instanceRootId, instancePrefabId, instanceComponent->GetName());

				B3D_TEST_ASSERT_EXTERNAL(testSuite, !instanceComponent->GetPrefabObjectId().Empty())
				if(flags.IsSet(UnitTestPrefabObjectOptionFlag::IsInstanceModification))
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
	void UnitTestPrefabUpdateHelper::TestAssetRootPrefabLinkValid(TestSuite& testSuite, SceneWrapperType& prefabWrapper, const UUID& prefabId, bool skipOptional)
	{
		prefabWrapper.PerformSceneObjectUnaryOperation([prefabId, &testSuite](const HSceneObject& sceneObject) {
			B3D_TEST_ASSERT_EXTERNAL(testSuite, sceneObject->GetPrefabObjectId() == sceneObject->GetId())
			B3D_TEST_ASSERT_EXTERNAL(testSuite, sceneObject->GetPrefabResourceId() == prefabId) }, skipOptional);
	}

	void UnitTestPrefabUpdateHelper::TestAssertPrefabLinksMatchPrefabInternals_UnitTestSceneB(TestSuite& testSuite, const HSceneObject& instanceRoot, const HSceneObject& prefabRoot, const UUID& prefabId)
	{
		UnitTestSceneB instanceScene(instanceRoot);
		UnitTestSceneB prefabInternalsScene(prefabRoot);

		// Ensure that newly instantiated prefab instances have correct prefab object & resource IDs
		TestAssertPrefabLinkValid(testSuite, instanceScene, prefabInternalsScene, prefabId, UUID::kEmpty, UUID::kEmpty, UnitTestPrefabObjectOptions());

		if(instanceScene.OptionalSceneObject_0_0_PrefabInstance.IsValid())
		{
			TestAssertPrefabLinksMatchPrefabInternals_UnitTestSceneB(testSuite, instanceScene.OptionalSceneObject_0_0_PrefabInstance, prefabInternalsScene.OptionalSceneObject_0_0_PrefabInstance, prefabId);
		}
	}

	void UnitTestPrefabUpdateHelper::TestAssertPrefabInternalsMatch_UnitTestSceneB(TestSuite& testSuite, u32 prefabIndex, const TArray<UnitTestPrefabInformation>& prefabs, const UnitTestPrefabObjectOptions& options)
	{
		if(!B3D_ENSURE(prefabIndex < (u32)prefabs.size()))
			return;

		const u32 prefabCount = (u32)prefabs.size() - prefabIndex;
		const UnitTestPrefabInformation& rootPrefabInformation = prefabs[prefabIndex];

		Vector<UnitTestSceneB> rootPrefabInternals_Parent;
		rootPrefabInternals_Parent.resize(prefabCount);

		rootPrefabInternals_Parent[0] = UnitTestSceneB(rootPrefabInformation.Prefab->GetRoot());
		UnitTestSceneB& internals_Root = rootPrefabInternals_Parent[0];

		// Ensure that prefab internals have correct prefab object & resource IDs (pointing to self for root, pointing to nested otherwise)
		TestAssetRootPrefabLinkValid(testSuite, internals_Root, rootPrefabInformation.Prefab->GetId());

		const u32 firstNestedPrefabIndex = prefabIndex + 1;
		if(firstNestedPrefabIndex >= (u32)prefabs.Size())
			return;

		const UnitTestPrefabInformation& firstNestedPrefabInformation = prefabs[firstNestedPrefabIndex];

		Vector<UnitTestSceneB> firstNestedPrefabInternals_Parent;
		firstNestedPrefabInternals_Parent.resize(prefabCount - 1);

		firstNestedPrefabInternals_Parent[0] = UnitTestSceneB(firstNestedPrefabInformation.Prefab->GetRoot());

		for(u32 nestedPrefabIndex = prefabIndex + 1; nestedPrefabIndex < (u32)prefabs.size(); ++nestedPrefabIndex)
		{
			const u32 nestedPrefabInternalsIndex = nestedPrefabIndex - prefabIndex;
			const UnitTestSceneB& internals_Parent = rootPrefabInternals_Parent[nestedPrefabInternalsIndex - 1];

			const UnitTestPrefabInformation& nestedPrefabInformation = prefabs[nestedPrefabIndex];

			rootPrefabInternals_Parent[nestedPrefabInternalsIndex] = UnitTestSceneB(internals_Parent.OptionalSceneObject_0_0_PrefabInstance);
			UnitTestSceneB& rootInternals_Parent_nested = rootPrefabInternals_Parent[nestedPrefabInternalsIndex];
			UnitTestSceneB& firstNestedInternals_Parent_nested = firstNestedPrefabInternals_Parent[nestedPrefabInternalsIndex - 1];

			if(firstNestedInternals_Parent_nested.OptionalSceneObject_0_0_PrefabInstance.IsValid())
				firstNestedPrefabInternals_Parent[nestedPrefabInternalsIndex] = UnitTestSceneB(firstNestedInternals_Parent_nested.OptionalSceneObject_0_0_PrefabInstance);

			if(nestedPrefabInformation.Flags.IsSet(PrefabCheckFlag::PrefabIsInstanceModification))
			{
				UnitTestSceneB internals_Nested(nestedPrefabInformation.Prefab->GetRoot());

				// Instance modification should link to the original prefab
				TestAssertPrefabLinkValid(testSuite, rootInternals_Parent_nested, internals_Nested, nestedPrefabInformation.Prefab->GetId(), rootPrefabInformation.Prefab->GetId(), nestedPrefabInformation.Prefab.GetId(), options);
			}
			else
			{
				TestAssertPrefabLinkValid(testSuite, rootInternals_Parent_nested, firstNestedInternals_Parent_nested, firstNestedPrefabInformation.Prefab->GetId(), rootPrefabInformation.Prefab.GetId(), nestedPrefabInformation.Prefab.GetId(), options);
			}
		}
	}

	void UnitTestPrefabUpdateHelper::TestAssertPrefabLinksMatch_UnitTestSceneB(TestSuite& testSuite, const HSceneObject& lhsRoot, const HSceneObject& rhsRoot, u32 prefabIndex, const TArray<UnitTestPrefabInformation>& prefabs, const UUID& instanceRootId, const UnitTestPrefabObjectOptions& options)
	{
		UnitTestSceneB unitTestSceneLHS(lhsRoot);
		UnitTestSceneB unitTestSceneRHS(rhsRoot);

		const UnitTestPrefabInformation& prefabInformation = prefabs[prefabIndex];

		unitTestSceneLHS.PerformSceneObjectBinaryOperation(
			unitTestSceneRHS, [&testSuite, &options, &instanceRootId, &prefabInformation](const HSceneObject& lhs, const HSceneObject& rhs) { 
			UnitTestPrefabObjectOptionFlags flags = options.GetFlagsForObject(instanceRootId, prefabInformation.Prefab.GetId(), lhs->GetName());

			B3D_TEST_ASSERT_EXTERNAL(testSuite, flags.IsSet(UnitTestPrefabObjectOptionFlag::SkipGameObjectCheck) || lhs.GetId() == rhs.GetId())
			B3D_TEST_ASSERT_EXTERNAL(testSuite, flags.IsSet(UnitTestPrefabObjectOptionFlag::SkipPrefabObjectCheck) || lhs->GetPrefabObjectId() == rhs->GetPrefabObjectId())
			B3D_TEST_ASSERT_EXTERNAL(testSuite, flags.IsSet(UnitTestPrefabObjectOptionFlag::SkipPrefabResourceCheck) || lhs->GetPrefabResourceId() == rhs->GetPrefabResourceId())
		});
		unitTestSceneLHS.PerformComponentBinaryOperation(
			unitTestSceneRHS, [&testSuite, &options, &instanceRootId, &prefabInformation](const HComponent& lhs, const HComponent& rhs)
			{
			UnitTestPrefabObjectOptionFlags flags = options.GetFlagsForObject(instanceRootId, prefabInformation.Prefab->GetId(), lhs->GetName());
			B3D_TEST_ASSERT_EXTERNAL(testSuite, flags.IsSet(UnitTestPrefabObjectOptionFlag::SkipGameObjectCheck) || lhs.GetId() == rhs.GetId())
			B3D_TEST_ASSERT_EXTERNAL(testSuite, flags.IsSet(UnitTestPrefabObjectOptionFlag::SkipPrefabObjectCheck) || lhs->GetPrefabObjectId() == rhs->GetPrefabObjectId())
		});

		if(unitTestSceneLHS.OptionalSceneObject_0_0_PrefabInstance.IsValid())
			TestAssertPrefabLinksMatch_UnitTestSceneB(testSuite, unitTestSceneLHS.OptionalSceneObject_0_0_PrefabInstance, unitTestSceneRHS.OptionalSceneObject_0_0_PrefabInstance, prefabIndex + 1, prefabs, instanceRootId, options);
	}

} // namespace bs
