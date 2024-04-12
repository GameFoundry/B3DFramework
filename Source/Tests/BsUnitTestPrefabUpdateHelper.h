//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "BsCorePrerequisites.h"

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
		SecondNestedPrefabIsInstanceModification
	};

	struct UnitTestPrefabUpdateHelper
	{
		/** Asserts that instance prefab & resource IDs point to the prefab. */
		template <typename UnitTestSceneType>
		static void TestAssertPrefabLinkValid(TestSuite& testSuite, UnitTestSceneType& instanceWrapper, UnitTestSceneType& prefabWrapper, const UUID& prefabId, bool skipOptional = false);

		/** Asserts that prefab & resource IDs are valid for root prefab. */
		template <typename SceneWrapperType>
		static void TestAssetRootPrefabLinkValid(TestSuite& testSuite, SceneWrapperType& prefabWrapper, const UUID& prefabId, bool skipOptional = false);

		static void TestAssertUnitTestSceneBPrefabLinksMatchPrefabInternals(TestSuite& testSuite, const HSceneObject& sourceInstanceRoot, const HSceneObject& newInstanceRoot, const HPrefab& rootPrefab, const HPrefab& firstNestedPrefab, const HPrefab& secondNestedPrefab, PrefabLinkCheckType checkType);

		/** Compares two hierarchies and ensure their prefab object IDs and prefab resource IDs match. */
		static void TestAssertUnitTestSceneBPrefabLinksMatch(TestSuite& testSuite, const HSceneObject& lhsRoot, const HSceneObject& rhsRoot, bool ignoreGameObjectIds, bool skipOptional = false);
	};
} // namespace bs
