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

	/**
	 * Prefab is a saveable hierarchy of scene objects. In general it can serve as any grouping of scene objects
	 * (for example a level) or be used as a form of a template instantiated and reused throughout the scene.
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
		 * will be parented to world root by default.
		 *
		 * @param	sceneInstance	Scene instance into which to instantiate the prefab instance in. If null, prefab will be instantiated
		 *							in a brand new scene instance.
		 * @return					Instantiated clone of the prefab's scene object hierarchy.
		 */
		HSceneObject Instantiate(const SPtr<SceneInstance>& sceneInstance = nullptr) const { return Instantiate(sceneInstance, false); }

		/**
		 * Replaces the contents of this prefab with new contents from the provided object. Object will be automatically
		 * linked to this prefab, and its previous prefab link (if any) will be broken.
		 */
		void Update(const HSceneObject& sceneObject);

		/**
		 * Returns a version value that gets updated every time the prefab contents update. Can be used for detecting if a prefab instance
		 * is up to date.
		 */
		UUID GetPrefabVersion() const { return mPrefabVersion; }

		/**
		 * Determines if the prefab represents a scene or just a generic group of objects. The only difference between the
		 * two is the way root object is handled: scenes are assumed to be saved with the scene root object (which is
		 * hidden), while object group root is a normal scene object (not hidden). This is relevant when when prefabs are
		 * loaded, so the systems knows to append the root object to non-scene prefabs.
		 */
		bool IsScene() const { return mIsScene; }

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Updates any prefab child instances by loading their prefabs and making sure they are up to date. */
		void UpdateChildInstancesInternal() const;

		/**
		 * Returns a reference to the internal prefab hierarchy. Returned hierarchy is not instantiated and cannot be
		 * interacted with in a manner you would with normal scene objects.
		 */
		HSceneObject GetRoot() const { return mRoot; }

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

		/** @} */

	private:
		using CoreObject::Initialize;

		/**	Initializes the internal prefab hierarchy. Must be called druing creation. */
		void Initialize(const HSceneObject& sceneObject);

		/**	Creates an empty and uninitialized prefab. */
		static SPtr<Prefab> CreateEmpty();

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
