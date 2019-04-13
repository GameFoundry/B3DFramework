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

	/**
	 * Prefab is a saveable hierarchy of scene objects. In general it can serve as any grouping of scene objects 
	 * (for example a level) or be used as a form of a template instantiated and reused throughout the scene.
	 */
	class BS_CORE_EXPORT Prefab : public Resource
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
		static HPrefab create(const HSceneObject& sceneObject, bool isScene = true);

		/**
		 * Instantiates a prefab by creating an instance of the prefab's scene object hierarchy. The returned hierarchy 
		 * will be parented to world root by default.
		 *			
		 * @return	Instantiated clone of the prefab's scene object hierarchy.
		 */
		HSceneObject instantiate() const { return _instantiate(); }

		/**
		 * Replaces the contents of this prefab with new contents from the provided object. Object will be automatically
		 * linked to this prefab, and its previous prefab link (if any) will be broken.
		 */
		void update(const HSceneObject& sceneObject);

		/**
		 * Returns a hash value that can be used for determining if a prefab changed by comparing it to a previously saved
		 * hash.
		 */
		UINT32 getHash() const { return mHash; }

		/** 
		 * Determines if the prefab represents a scene or just a generic group of objects. The only difference between the
		 * two is the way root object is handled: scenes are assumed to be saved with the scene root object (which is 
		 * hidden), while object group root is a normal scene object (not hidden). This is relevant when when prefabs are
		 * loaded, so the systems knows to append the root object to non-scene prefabs.
		 */
		bool isScene() const { return mIsScene; }

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Updates any prefab child instances by loading their prefabs and making sure they are up to date. */
		void _updateChildInstances() const;

		/**
		 * Returns a reference to the internal prefab hierarchy. Returned hierarchy is not instantiated and cannot be 
		 * interacted with in a manner you would with normal scene objects.
		 */
		HSceneObject _getRoot() const { return mRoot; }

		/**
		 * Creates the clone of the prefab's current hierarchy but doesn't instantiate it.
		 *
		 * @param[in]	preserveUUIDs	If false, each cloned game object will be assigned a brand new UUID. Otherwise
		 *								the UUID of the original game objects will be preserved. Note that two instantiated
		 *								scene objects should never have the same UUID, so if preserving UUID's make sure
		 *								the original is destroyed before instantiating.
		 * @return						Clone of the prefab's scene object hierarchy.
		 */
		HSceneObject _clone(bool preserveUUIDs = false) const;

		/**
		 * Instantiates a prefab by creating an instance of the prefab's scene object hierarchy. The returned hierarchy 
		 * will be parented to world root by default.
		 *			
		 * @param[in]	preserveUUIDs	If false, each cloned game object will be assigned a brand new UUID. Otherwise
		 *								the UUID of the original game objects will be preserved. Note that two instantiated
		 *								scene objects should never have the same UUID, so if preserving UUID's make sure
		 *								the original is destroyed before instantiating.
		 * @return						Instantiated clone of the prefab's scene object hierarchy.
		 */
		HSceneObject _instantiate(bool preserveUUIDs = false) const;

		/** @} */

	private:
		using CoreObject::initialize;

		/**	Initializes the internal prefab hierarchy. Must be called druing creation. */
		void initialize(const HSceneObject& sceneObject);

		/**	Creates an empty and uninitialized prefab. */
		static SPtr<Prefab> createEmpty();

		HSceneObject mRoot;
		UINT32 mHash = 0;
		UUID mUUID;
		bool mIsScene = true;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class PrefabRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}