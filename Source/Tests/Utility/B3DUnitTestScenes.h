//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "B3DUtilityPrerequisites.h"
#include "Scene/B3DSceneManager.h"
#include "Scene/B3DSceneObject.h"
#include "B3DUnitTestComponents.h"
#include "Scene/B3DPrefabUtility.h"
#include "Scene/B3DSceneInstance.h"

namespace b3d
{
	/** Wrapper for easier scene creation and object access. */
	struct UnitTestSceneA
	{
		// Disallow copy, allow move
		UnitTestSceneA(const UnitTestSceneA&) = delete;
		UnitTestSceneA& operator=(const UnitTestSceneA&) = delete;

		UnitTestSceneA(UnitTestSceneA&&) = default;
		UnitTestSceneA& operator=(UnitTestSceneA&&) = default;

		~UnitTestSceneA();

		/** Creates a new scene instance and populates the hierarchy with Test Scene A hierarchy. Releases scene instance when object goes out of scope. */
		static UnitTestSceneA CreateInNewSceneInstance(const char* name);

		/** Instantiates the provided prefab as a new scene instance and looks up the Test Scene A hierarchy in the instantiated objects. Releases instantiated scene instance when object goes out of scope. */
		static UnitTestSceneA InstantateFromPrefab(const TShared<Prefab>& prefab);

		/** Populates the scene objects and components by looking them up in the provided hierarchy. Does not take ownership of the scene instance. */
		static UnitTestSceneA FromExistingHierarchy(const HSceneObject& root);

		HSceneObject GetRoot() const
		{
			if(Scene != nullptr)
				return Scene->GetRoot();

			return nullptr;
		}

		template <class T>
		void PerformSceneObjectUnaryOperation(T&& predicate)
		{
			predicate(SceneObject_0);
			predicate(SceneObject_0_0);
			predicate(SceneObject_0_1);
			predicate(SceneObject_0_1_0);
			predicate(SceneObject_1);
			predicate(SceneObject_1_0);
		}

		template <class T>
		void PerformComponentUnaryOperation(T&& predicate)
		{
			predicate(Component_0);
			predicate(Component_1);
			predicate(Component_0_1);
			predicate(Component_0_1_0);
		}

		template <class T>
		void PerformSceneObjectBinaryOperation(UnitTestSceneA& other, T&& predicate)
		{
			predicate(SceneObject_0, other.SceneObject_0);
			predicate(SceneObject_0_0, other.SceneObject_0_0);
			predicate(SceneObject_0_1, other.SceneObject_0_1);
			predicate(SceneObject_0_1_0, other.SceneObject_0_1_0);
			predicate(SceneObject_1, other.SceneObject_1);
			predicate(SceneObject_1_0, other.SceneObject_1_0);
		}

		template <class T>
		void PerformComponentBinaryOperation(UnitTestSceneA& other, T&& predicate)
		{
			predicate(Component_0, other.Component_0);
			predicate(Component_1, other.Component_1);
			predicate(Component_0_1, other.Component_0_1);
			predicate(Component_0_1_0, other.Component_0_1_0);
		}

		TShared<SceneInstance> Scene;

		HSceneObject SceneObject_0;
		HSceneObject SceneObject_0_0;
		HSceneObject SceneObject_0_1;
		HSceneObject SceneObject_0_1_0;
		HSceneObject SceneObject_1;
		HSceneObject SceneObject_1_0;

		HUnitTestComponentA Component_0;
		HUnitTestComponentB Component_1;
		HUnitTestComponentA Component_0_1;
		HUnitTestComponentA Component_0_1_0;

	private:
		/** Populates the provided scene instance with the scene. */
		UnitTestSceneA(const TShared<SceneInstance>& sceneInstance);

		/** Populates the scene objects and components by looking them up in the provided hierarchy. */
		UnitTestSceneA(const HSceneObject& root, bool ownsSceneInstance);

		bool mOwnsSceneInstance = false;
	};

	/** Flags that control the checks we perform on objects. */
	enum class UnitTestSceneObjectFlag
	{
		None = 0,
		IsInstanceModification = 1 << 0,
		IsPrefabRootInstanceModification = 1 << 1,
	};

	using UnitTestSceneObjectFlags = Flags<UnitTestSceneObjectFlag>;
	B3D_FLAGS_OPERATORS(UnitTestSceneObjectFlag)

	/* Information about a game object within a unit test scene. */
	struct UnitTestGameObjectInformation
	{
		UnitTestGameObjectInformation(PrefabLinkInformation prefabLink = PrefabLinkInformation(), UnitTestSceneObjectFlags flags = UnitTestSceneObjectFlag::None)
			: PrefabLink(prefabLink), Flags(flags)
		{ }

		PrefabLinkInformation PrefabLink;
		UnitTestSceneObjectFlags Flags;
	};

	/**
	 * Allows you to easily set up the following scene object hierarchy:
	 * Root
	 *  SceneObject_0 [Components: None]
	 *    OptionalSceneObject_0_0_PrefabInstance (Optional child prefab instance)
	 *  SceneObject_1 [Components: None]
	 *    SceneObject_1_0 [Components: UnitTestComponentA]
	 *    OptionalSceneObject_1_1_PrefabInstance (Optional child prefab instance)
	 *  OptionalSceneObject_2 [Components: UnitTestComponentA] (Optional object that can be created after initial construction)
	 */
	struct UnitTestSceneB
	{
		UnitTestSceneB() = default;

		// Disallow copy, allow move
		UnitTestSceneB(const UnitTestSceneB&) = delete;
		UnitTestSceneB& operator=(const UnitTestSceneB&) = delete;

		UnitTestSceneB(UnitTestSceneB&&) = default;
		UnitTestSceneB& operator=(UnitTestSceneB&&) = default;

		~UnitTestSceneB();

		/** Creates a new scene instance and populates the hierarchy with Test Scene B hierarchy. Releases scene instance when object goes out of scope. */
		static UnitTestSceneB CreateInNewSceneInstance(const char* name);

		/** Instantiates the provided prefab as a new scene instance and looks up the Test Scene B hierarchy in the instantiated objects. Releases instantiated scene instance when object goes out of scope. */
		static UnitTestSceneB InstantateFromPrefab(const HPrefab& prefab);

		/** Populates the scene objects and components by looking them up in the provided hierarchy. Does not take ownership of the scene instance. */
		static UnitTestSceneB FromExistingHierarchy(const HSceneObject& root);

		/** Populates the scene objects and components by looking them up in the provided hierarchy. Does not take ownership of the scene instance. */
		static TShared<UnitTestSceneB> FromExistingHierarchyAsShared(const HSceneObject& root);

		/** Creates the Test Scene B hierarchy as child of the provided object. Does not take ownership of the scene instance. */
		static UnitTestSceneB CreateAsChild(const HSceneObject& parent);

		HSceneObject GetRoot() const
		{
			if(Scene != nullptr)
				return Scene->GetRoot();

			return nullptr;
		}

		HSceneObject SetUnitTestSceneAChildPrefab_0_0(const Prefab& prefab);
		HSceneObject SetUnitTestSceneBChildPrefab_0_0(const Prefab& prefab);
		HSceneObject SetUnitTestSceneBChildPrefab_1_1(const Prefab& prefab);

		/** Creates OptionalSceneObject_2 and its associated component. */
		void CreateOptionalSceneObject_2();

		/** Destroys SceneObject_1_0 and its associated component. */
		void DestroySceneObject_1_0();

		/** Refreshes the hierarchy by assigning a new root object. Original IDs are not updated, except for newly added optional objects, or objects that were destroyed. */
		void RefreshHierarchy(const HSceneObject& root);

		/** Resets all the values to default. */
		void Reset();

		/** Destroys the scene object hierarchy and resets the object. */
		void Destroy();

		/** Performs an operation over scene objects in the scene. If an object has been destroyed, the predicate won't be called on it. */
		template <class T>
		void PerformSceneObjectUnaryOperation(T&& predicate)
		{
			predicate(Root);
			if(SceneObject_0.IsValid()) predicate(SceneObject_0);
			if(SceneObject_1.IsValid()) predicate(SceneObject_1);
			if(SceneObject_1_0.IsValid()) predicate(SceneObject_1_0);
			if(OptionalSceneObject_2.IsValid()) predicate(OptionalSceneObject_2);
		}

		/** Performs an operation over components in the scene. If a component has been destroyed, the predicate won't be called on it. */
		template <class T>
		void PerformComponentUnaryOperation(T&& predicate)
		{
			if(Component_1_0.IsValid()) predicate(Component_1_0);
			if(OptionalComponent_2.IsValid()) predicate(OptionalComponent_2);
		}

		/** Performs an operation over matching scene objects in two scenes. If an object has been destroyed in this scene, the predicate won't be called on it. */
		template <class T>
		void PerformSceneObjectBinaryOperation(UnitTestSceneB& other, T&& predicate)
		{
			predicate(Root, other.Root);
			if(SceneObject_0.IsValid()) predicate(SceneObject_0, other.SceneObject_0);
			if(SceneObject_1.IsValid()) predicate(SceneObject_1, other.SceneObject_1);
			if(SceneObject_1_0.IsValid()) predicate(SceneObject_1_0, other.SceneObject_1_0);
			if(OptionalSceneObject_2.IsValid()) predicate(OptionalSceneObject_2, other.OptionalSceneObject_2);
		}

		/** Performs an operation over matching components in two scenes. If a component has been destroyed in this scene, the predicate won't be called on it. */
		template <class T>
		void PerformComponentBinaryOperation(UnitTestSceneB& other, T&& predicate)
		{
			if(Component_1_0.IsValid()) predicate(Component_1_0, other.Component_1_0);
			if(OptionalComponent_2.IsValid()) predicate(OptionalComponent_2, other.OptionalComponent_2);
		}

		void AddOrUpdateIds(HSceneObject object, bool updatePrefabObjectId = true, bool updatePrefabResourceId = false, bool allowAddNew = false);
		void AddOrUpdateIds(bool updatePrefabObjectId = true, bool updatePrefabResourceId = false, bool allowAddNew = false) { return AddOrUpdateIds(Root, updatePrefabObjectId, updatePrefabResourceId, allowAddNew); }

		void UpdatePrefabLinkIds(HSceneObject object) { AddOrUpdateIds(object, true, true, false); }
		void UpdatePrefabLinkIds() { UpdatePrefabLinkIds(Root); }

		void UpdatePrefabObjectIds(HSceneObject object) { AddOrUpdateIds(object, true, false, false); }
		void UpdatePrefabObjectIds() { UpdatePrefabObjectIds(Root); }

		void AddNewObjectIds(HSceneObject object) { AddOrUpdateIds(object, true, true, true); }
		void AddNewObjectIds() { AddNewObjectIds(Root); }

		void SetFlagOnObject(const GameObjectHandle& object, UnitTestSceneObjectFlags flags);

		/** Check if all current game object match the original recorded IDs. */
		void TestAssertHierarchyMatchesOriginalIds(TestSuite& testSuite);

		// TODO - Do
		void TestAssertHierarchyMatchesPrefabLinks(TestSuite& testSuite, const UnorderedMap<UUID, TShared<UnitTestSceneB>>& prefabSceneLookup, u32 nestingLevel = 0, const UUID& parentPrefabId = UUID::kEmpty, const TShared<UnitTestSceneB>& parentPrefabScene = nullptr);

		TShared<SceneInstance> Scene;
		HSceneObject Root;

		HSceneObject SceneObject_0;
		HSceneObject OptionalSceneObject_0_0_PrefabInstance;
		HSceneObject SceneObject_1;
		HSceneObject SceneObject_1_0;
		HSceneObject OptionalSceneObject_1_1_PrefabInstance;
		HUnitTestComponentA Component_1_0;

		UUID SceneObject_1_0_Id;
		UUID Component_1_0_Id;

		TShared<UnitTestSceneB> OptionalPrefabInstance_0_0;
		TShared<UnitTestSceneB> OptionalPrefabInstance_1_1;

		UnorderedMap<UUID, UnitTestGameObjectInformation> ObjectInformation;

		// These objects may be created after initial construction
		HSceneObject OptionalSceneObject_2;
		HComponent OptionalComponent_2;

	private:
		UnitTestSceneB(const HSceneObject& root, bool ownsSceneInstance);

		/** Creates the test scene hierarchy as a child of the root object. Initializes the member variables to the created hierarchy. */
		void PopulateHierarchy();

		bool mOwnsSceneInstance = false;
	};
} // namespace b3d
