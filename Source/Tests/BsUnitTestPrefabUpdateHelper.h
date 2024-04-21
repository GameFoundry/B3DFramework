//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "BsCorePrerequisites.h"
#include "Scene/BsPrefabUtility.h"

namespace bs
{
	/** Type of check to perform when verifying prefab links. */
	enum class PrefabCheckFlag
	{
		/**
		 * Instances must match object/resource ID of the prefab resource.
		 * Prefab resource internal root must have empty object/resource id.
		 * Nested prefab resource internal must have object and resource id pointing to the nested prefab resource.
		 */
		Regular = 0,

		/**
		 * Same as regular, except if there is a second nested prefab, it will be treated as an instance modification.
		 * Its objects must have empty object id, and resource id pointing to the (first) nested prefab resource.
		 */
		PrefabIsInstanceModification = 1 << 1,

		/** Skips checking prefab internals for a particular prefab. */
		SkipPrefabInternalsCheck = 1 << 2,
	};

	using PrefabCheckFlags = Flags<PrefabCheckFlag>;
	B3D_FLAGS_OPERATORS(PrefabCheckFlag);

	/** Information about nested prefabs used when performing prefab related unit tests. */
	struct UnitTestPrefabInformation
	{
		UnitTestPrefabInformation(const HPrefab& prefab = HPrefab(), PrefabCheckFlags flags = PrefabCheckFlag::Regular)
			: Prefab(prefab), Flags(flags)
		{ }

		HPrefab Prefab;
		PrefabCheckFlags Flags = PrefabCheckFlag::Regular;
	};

	/** Flags that control which checks to skip when performing prefab related unit tests. */
	enum class UnitTestPrefabObjectOptionFlag
	{
		PerformAllChecks = 0,
		SkipGameObjectCheck = 1 << 0,
		SkipPrefabObjectCheck = 1 << 1,
		SkipPrefabResourceCheck = 1 << 2,
		IsInstanceModification = 1 << 3,
		SkipAllChecks = SkipGameObjectCheck | SkipPrefabObjectCheck | SkipPrefabResourceCheck,
	};

	using UnitTestPrefabObjectOptionFlags = Flags<UnitTestPrefabObjectOptionFlag>;
	B3D_FLAGS_OPERATORS(UnitTestPrefabObjectOptionFlag)

	/** Options that control which checks to skip when performing prefab related unit tests. */
	struct UnitTestPrefabObjectOptions
	{
		UnitTestPrefabObjectOptionFlags GlobalOptions = UnitTestPrefabObjectOptionFlag::PerformAllChecks;

		/** Specifies per-object flags that control various prefab checks. */
		UnorderedMap<UUID, UnorderedMap<UUID, UnorderedMap<String, UnitTestPrefabObjectOptionFlags>>> ObjectOptionsPerPrefabPerInstance;

		void SetFlagsForObject(const UUID& rootInstanceId, const HPrefab& prefab, const GameObjectHandleBase& gameObject, UnitTestPrefabObjectOptionFlags flags);
		void SetFlagsForObject(const UUID& rootInstanceId, const HPrefab& prefab, const HSceneObject& sceneObject, UnitTestPrefabObjectOptionFlags flags, bool setOnChildren);
		UnitTestPrefabObjectOptionFlags GetFlagsForObject(const UUID& rootInstanceId, const UUID& prefabId, const String& name) const;
		void ClearAllObjectFlags();
	};

	struct UnitTestPrefabUpdateHelper
	{
		/** Asserts that instance prefab & resource IDs point to the prefab. */
		template <typename UnitTestSceneType>
		static void TestAssertPrefabLinkValid(TestSuite& testSuite, UnitTestSceneType& instanceWrapper, UnitTestSceneType& prefabWrapper, const UUID& prefabResourceId, const UUID& instanceRootId, const UUID& instancePrefabId, const UnitTestPrefabObjectOptions& options);

		/** Asserts that prefab & resource IDs are valid for root prefab. */
		template <typename UnitTestSceneType>
		static void TestAssetRootPrefabLinkValid(TestSuite& testSuite, UnitTestSceneType& prefabWrapper, const UUID& prefabId, bool skipOptional = false);

		/** Checks if prefab instance matches the object and resource IDs in the internal prefab hierarchy. */
		static void TestAssertPrefabLinksMatchPrefabInternals_UnitTestSceneB(TestSuite& testSuite, const HSceneObject& instanceRoot, const HSceneObject& prefabRoot, const UUID& prefabId);

		/**
		 * Checks if object IDs are valid in prefab internals. If prefab contains nested prefabs they should be listed in order from root to nested in
		 * @p prefabs array. All nested prefab instances will be checked against their prefab resources.
		 */
		static void TestAssertPrefabInternalsMatch_UnitTestSceneB(TestSuite& testSuite, u32 prefabIndex, const TArray<UnitTestPrefabInformation>& prefabs, const UnitTestPrefabObjectOptions& options);

		/**
		 * Compares two hierarchies and ensure their prefab object IDs and prefab resource IDs match. Note that order of roots matter - if an object is not present in
		 * the LHS hierarchy, it will be skipped. If the object is present in LHS hierarchy, but not in RHS hierarchy, test will fail. So when adding objects pass
		 * the new hierarchy as RHS, and when destroying objects pass the new hierarchy as LHS.
		 */
		static void TestAssertPrefabLinksMatch_UnitTestSceneB(TestSuite& testSuite, const HSceneObject& lhsRoot, const HSceneObject& rhsRoot, u32 prefabIndex, const TArray<UnitTestPrefabInformation>& prefabs, const UUID& instanceRootId, const UnitTestPrefabObjectOptions& options);
	};
} // namespace bs
