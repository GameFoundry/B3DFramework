//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Scene/BsGameObject.h"
#include "Resources/BsResource.h"

namespace bs
{
	/** @addtogroup Scene
	 *  @{
	 */

	B3D_CORE_EXPORT B3D_LOG_CATEGORY_EXTERN(Prefab, Log)

	/** Keeps track of all live prefabs and ensures they are kept up to date. */
	class B3D_CORE_EXPORT PrefabManager : public Module<PrefabManager>
	{
	public:
		/** Returns all prefabs that are currently loaded. */
		const UnorderedSet<Prefab*>& GetLivePrefabs() const { return mLivePrefabs; }

	private:
		friend class Prefab;

		void RegisterPrefab(Prefab& prefab);
		void UnregisterPrefab(Prefab* prefab);

		UnorderedSet<Prefab*> mLivePrefabs;
	};

	/**
	 * Prefab is a saveable hierarchy of scene objects. It can be instanced, and instances will maintain  link to the
	 * original prefab they were created from, allowing you to update them to latest version if the prefab changes.
	 * Prefabs can also be nested within each-other, as long as there are no circular dependencies.
	 */
	class B3D_CORE_EXPORT Prefab : public Resource
	{
	public:
		Prefab();
		~Prefab();

		/**
		 * Creates a new prefab from the provided scene object. If the scene object has an existing prefab link it will
		 * be broken. After the prefab is created the scene object will be automatically linked to it.
		 *
		 * @param[in]	sceneObject		Scene object to create the prefab from.
		 * @param[in]	isScene			Determines if the prefab represents a scene or just a generic group of objects.
		 *								@see isScene().
		 */
		static HPrefab Create(const HSceneObject& sceneObject, bool isScene = true);

		/**
		 * Instantiates a prefab by creating an instance of the prefab's scene object hierarchy. The returned hierarchy
		 * will be parented to the provided scene instance root.
		 *
		 * @param	sceneInstance	Scene instance into which to instantiate the prefab instance in. If null, prefab will be instantiated
		 *							in a brand new scene instance.
		 * @return					Instantiated clone of the prefab's scene object hierarchy.
		 */
		HSceneObject Instantiate(const SPtr<SceneInstance>& sceneInstance = nullptr) const { return Instantiate(sceneInstance, false); }

		/**
		 * Returns a version value that gets updated every time the prefab contents update. Can be used for detecting if a prefab instance
		 * is up to date.
		 */
		UUID GetPrefabVersion() const { return mPrefabVersion; }

		/** Determines if the prefab represents a scene or just a generic group of objects. */
		bool IsScene() const { return mIsScene; }

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Returns a reference to the internal prefab hierarchy. Returned hierarchy is not instantiated and cannot be
		 * interacted with in a manner you would with normal scene objects.
		 */
		HSceneObject GetRoot() const { return mRoot; }

		/** Returns the game object collection that owns all the game objects in the prefab. */
		SPtr<GameObjectCollection> GetGameObjectCollection() const { return mGameObjectCollection; }

		/**
		 * Creates the clone of the prefab's current hierarchy but doesn't instantiate it.
		 *
		 * @param	cloneOwnerCollection	Collection into which to place the cloned scene objects. If @p preserveIds is true
		 *									this must be a different collection that the prefab's internal collection,
		 *									otherwise IDs would conflict.
		 * @param	preserveIds				If false, each cloned game object will be assigned a brand new ID. Otherwise
		 *									the ID of the original game objects will be preserved. Note that two instantiated
		 *									scene objects should never have the same ID, so if preserving ID's make sure
		 *									the original is destroyed before instantiating.
		 * @return							Clone of the prefab's scene object hierarchy.
		 */
		HSceneObject Clone(const SPtr<GameObjectCollection>& cloneOwnerCollection, bool preserveIds = false) const;

		/**
		 * Instantiates a prefab by creating an instance of the prefab's scene object hierarchy. The returned hierarchy
		 * will be parented to world root by default.
		 *
		 * @param	sceneInstance	Scene instance into which to instantiate the prefab instance in.
		 * @param	preserveIds		If false, each cloned game object will be assigned a brand new UUID. Otherwise
		 *							the UUID of the original game objects will be preserved. Note that two instantiated
		 *							scene objects should never have the same UUID, so if preserving UUID's make sure
		 *							the original is destroyed before instantiating.
		 * @return					Instantiated clone of the prefab's scene object hierarchy.
		 */
		HSceneObject Instantiate(const SPtr<SceneInstance>& sceneInstance, bool preserveIds) const;

		/**
		 * Replaces the contents of this prefab with new contents from the provided object. Returns a map of @p sceneObject IDs
		 * that were remapped to new IDs within the prefab.
		 */
		UnorderedMap<UUID, UUID> ReplaceInternalHierarchy(const HSceneObject& sceneObject);

		/** Updates the internal prefab version to a new value. You should call this after modifying the prefab hierarchy. */
		void TickPrefabVersion();

		/** Updates deltas for any nested prefab instances. */
		void RecordNestedPrefabInstanceDeltas();

		/** @} */

	private:
		/**	Creates an empty and uninitialized prefab. */
		static SPtr<Prefab> CreateEmpty();

		void Initialize() override;
		void Destroy() override;

		HSceneObject mRoot;
		UUID mPrefabVersion = UUID::kEmpty;
		UUID mUUID;
		bool mIsScene = true;
		SPtr<GameObjectCollection> mGameObjectCollection; /**< Collection owning the internal hierarchy. */

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class PrefabRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */
} // namespace bs
