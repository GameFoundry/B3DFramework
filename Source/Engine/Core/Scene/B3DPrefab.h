//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DGameObject.h"
#include "Resources/B3DResource.h"

namespace b3d
{
	/** @addtogroup Scene
	 *  @{
	 */

	B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogPrefab, Log)

	/** Keeps track of all live prefabs and ensures they are kept up to date. */
	class B3D_EXPORT PrefabManager : public Module<PrefabManager>
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
	 * Prefab is a saveable hierarchy of scene objects. It can be instanced, and instances will maintain a link to the
	 * original prefab they were created from, allowing you to update them to latest version if the prefab changes.
	 * Prefabs can also be nested within each-other, as long as there are no circular dependencies. Instanced prefabs
	 * can also contain per instance modifications that will be preserved even if the prefab they were created from
	 * changes.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Scene)) Prefab : public Resource
	{
	public:
		Prefab();
		~Prefab();

		/**
		 * Instantiates a prefab by creating an instance of the prefab's scene object hierarchy. The returned hierarchy
		 * will be parented to the provided scene instance root.
		 *
		 * @param	sceneInstance	Scene instance into which to instantiate the prefab instance in.
		 * @return					Instantiated clone of the prefab's scene object hierarchy.
		 */
		B3D_SCRIPT_EXPORT()
		HSceneObject Instantiate(const SPtr<SceneInstance>& sceneInstance) const;

		/**
		 * Returns a version value that gets updated every time the prefab contents update. Can be used for detecting if a prefab instance
		 * is up to date.
		 */
		UUID GetPrefabVersion() const { return mPrefabVersion; }

		bool AllowAsyncLoading() const override { return false; }

		/**
		 * Creates a new prefab from the provided scene object. If the scene object has an existing prefab link it will
		 * be broken. After the prefab is created the scene object will be automatically linked to it.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(Prefab))
		static HPrefab Create(const HSceneObject& sceneObject);

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
		 * @return							Clone of the prefab's scene object hierarchy.
		 */
		HSceneObject Clone(const SPtr<GameObjectCollection>& cloneOwnerCollection) const;

		/**
		 * Instantiates a prefab by creating an instance of the prefab's scene object hierarchy as a brand new scene instance, whose
		 * root is the prefab root.
		 *
		 * @return					Newly created scene instance.
		 */
		SPtr<SceneInstance> InstantiateAsScene() const;

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

		/**
		 * Instantiates a prefab by creating an instance of the prefab's scene object hierarchy. The returned hierarchy
		 * will be parented to world root by default, if the provided instance is non-empty. If the provided scene instance
		 * is empty, new scene instance will be created and prefab's root will be set as the scene instance's root.
		 *
		 * @param	inOutSceneInstance	Scene instance into which to instantiate the prefab instance in. If null, new
		 *								scene instance will be created and output through this parameter.
		 * @return						Instantiated clone of the prefab's scene object hierarchy.
		 */
		HSceneObject InstantiateInternal(SPtr<SceneInstance>& inOutSceneInstance) const;

		void Initialize() override;
		void Destroy() override;

		HSceneObject mRoot;
		UUID mPrefabVersion = UUID::kEmpty;
		UUID mUUID;
		SPtr<GameObjectCollection> mGameObjectCollection; /**< Collection owning the internal hierarchy. */

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class PrefabRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
