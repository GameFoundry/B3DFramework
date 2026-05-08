//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
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

	/** Saveable hierarchy of scene objects that can be instantiated into a SceneInstance. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Localization)) Scene : public Resource
	{
	public:
		Scene();
		~Scene();

		/** Instantiates a scene by creating an instance of the scene object hierarchy. */
		B3D_SCRIPT_EXPORT()
		TShared<SceneInstance> Instantiate() const;

		bool AllowAsyncLoading() const override { return false; }

		/** Creates a new scene from the provided scene object. The provided scene object and all saveable children will be cloned as part of the scene. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(Scene))
		static HScene Create(const HSceneObject& sceneObject);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Returns a reference to the internal scene hierarchy. Returned hierarchy is not instantiated and cannot be
		 * interacted with in a manner you would with normal scene objects.
		 */
		HSceneObject GetRoot() const { return mRoot; }

		/** Returns the game object collection that owns all the game objects in the scene. */
		TShared<GameObjectCollection> GetGameObjectCollection() const { return mGameObjectCollection; }

		/**
		 * Instantiates a scene by creating an instance of the scene's scene object hierarchy. The new hierarchy
		 * be set as the provided scene instance's root.
		 *
		 * @param	inOutSceneInstance	Scene instance into which to instantiate the scene instance in. If null, new
		 *								scene instance will be created and output through this parameter.
		 * @return						Instantiated clone of the scene's scene object hierarchy.
		 */
		HSceneObject Instantiate(TShared<SceneInstance>& inOutSceneInstance) const;

		/** Replaces the contents of this scene with new contents from the provided object. */
		void ReplaceInternalHierarchy(const HSceneObject& sceneObject);

		/** @} */

	private:
		/**	Creates an empty and uninitialized scene. */
		static TShared<Scene> CreateEmpty();

		HSceneObject mRoot;
		UUID mUUID;
		TShared<GameObjectCollection> mGameObjectCollection; /**< Collection owning the internal hierarchy. */

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class SceneRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
