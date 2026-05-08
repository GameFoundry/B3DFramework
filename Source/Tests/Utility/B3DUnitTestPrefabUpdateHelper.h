//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "B3DPrerequisites.h"

namespace b3d
{
	struct UnitTestSceneB;

	struct UnitTestPrefabUpdateHelper
	{
		/** Asserts that instance prefab & resource IDs point to the prefab. */
		template <typename UnitTestSceneType>
		static void TestAssertPrefabLinkValid(TestSuite& testSuite, UnitTestSceneType& instanceWrapper, UnitTestSceneType& prefabWrapper, const UUID& prefabResourceId);

		/** Asserts that prefab & resource IDs are valid for root prefab. */
		template <typename UnitTestSceneType>
		static void TestAssetRootPrefabLinkValid(TestSuite& testSuite, UnitTestSceneType& prefabWrapper, const UUID& prefabId);

		/** Checks if prefab instance matches the object and resource IDs in the internal prefab hierarchy. */
		static void TestAssertPrefabLinksMatchPrefabInternals_UnitTestSceneB(TestSuite& testSuite, UnitTestSceneB& instanceScene, const TShared<UnitTestSceneB>& parentPrefabScene, const UUID& parentPrefabId, const UnorderedMap<UUID, TShared<UnitTestSceneB>>& prefabSceneLookup);
	};
} // namespace b3d
