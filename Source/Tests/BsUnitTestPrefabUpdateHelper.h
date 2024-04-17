//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "BsCorePrerequisites.h"
#include "Scene/BsPrefabUtility.h"

namespace bs
{
	/** Type of check to perform when verifying prefab links. */
	enum class PrefabLinkCheckType
	{
		/**
		 * Instances must match object/resource ID of the prefab resource.
		 * Prefab resource internal root must have empty object/resource id.
		 * Nested prefab resource internal must have object and resource id pointing to the nested prefab resource.
		 */
		Regular,

		/**
		 * Same as regular, except for treatment of optionals in the nested prefab.
		 * Optionals in the nested prefab resource must have empty object id, and resource id pointing to the nested prefab resource.
		 */
		OptionalsAreInstanceModifications,

		/**
		 * Same as regular, except if there is a second nested prefab, it will be treated as an instance modification.
		 * Its objects must have empty object id, and resource id pointing to the (first) nested prefab resource.
		 */
		PrefabIsInstanceModification
	};

	/** Information about nested prefabs used when performing prefab related unit tests. */
	struct UnitTestPrefabInformation
	{
		UnitTestPrefabInformation(const HPrefab& prefab = HPrefab(), PrefabLinkCheckType checkType = PrefabLinkCheckType::Regular)
			: Prefab(prefab), CheckType(checkType)
		{ }

		HPrefab Prefab;
		PrefabLinkCheckType CheckType = PrefabLinkCheckType::Regular;
	};

	struct UnitTestPrefabUpdateHelper
	{
		/** Asserts that instance prefab & resource IDs point to the prefab. */
		template <typename UnitTestSceneType>
		static void TestAssertPrefabLinkValid(TestSuite& testSuite, UnitTestSceneType& instanceWrapper, UnitTestSceneType& prefabWrapper, const UUID& prefabId, bool skipOptional = false);

		/** Asserts that prefab & resource IDs are valid for root prefab. */
		template <typename SceneWrapperType>
		static void TestAssetRootPrefabLinkValid(TestSuite& testSuite, SceneWrapperType& prefabWrapper, const UUID& prefabId, bool skipOptional = false);

		/** Checks if prefab instance matches the object and resource IDs in the internal prefab hierarchy. */
		static void TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(TestSuite& testSuite, const HSceneObject& instanceRoot, const HSceneObject& prefabRoot, const UUID& prefabId);

		/**
		 * Checks if object IDs are valid in prefab internals. If prefab contains nested prefabs they should be listed in order from root to nested in
		 * @p prefabs array. All nested prefab instances will be checked against their prefab resources.
		 */
		static void TestAssertUnitTestSceneBPrefabInternalsMatch(TestSuite& testSuite, u32 prefabIndex, const TArray<UnitTestPrefabInformation>& prefabs, bool checkNestedPrefabs);

		/**
		 * Compares two hierarchies and ensure their prefab object IDs and prefab resource IDs match. Note that order of roots matter - if an object is not present in
		 * the LHS hierarchy, it will be skipped. If the object is present in LHS hierarchy, but not in RHS hierarchy, test will fail. So when adding objects pass
		 * the new hierarchy as RHS, and when destroying objects pass the new hierarchy as LHS.
		 */
		static void TestAssertUnitTestSceneBPrefabLinksMatch(TestSuite& testSuite, const HSceneObject& lhsRoot, const HSceneObject& rhsRoot, bool ignoreGameObjectIds, bool skipOptional = false);
	};
} // namespace bs
