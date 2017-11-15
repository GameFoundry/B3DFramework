//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	class ScriptRenderable;

	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	/**
	 * Manages all active GameObject interop objects. GameObjects can be created from native code and used in managed code
	 * therefore we need to keep a dictionary or all the native objects we have mapped to managed objects.
	 */
	class BS_SCR_BE_EXPORT ScriptGameObjectManager : public Module<ScriptGameObjectManager>
	{
		/**	Contains information about a single interop object containing a game object. */
		struct ScriptGameObjectEntry
		{
			ScriptGameObjectEntry();
			ScriptGameObjectEntry(ScriptGameObjectBase* instance, bool isComponent);

			ScriptGameObjectBase* instance;
			bool isComponent;
		};

	public:
		ScriptGameObjectManager();
		~ScriptGameObjectManager();

		/**
		 * Attempts to find the interop object for the specified SceneObject. If one cannot be found new one is created and
		 * returned.
		 */
		ScriptSceneObject* getOrCreateScriptSceneObject(const HSceneObject& sceneObject);

		/** Creates a new interop object for the specified SceneObject. Throws an exception if one already exists. */
		ScriptSceneObject* createScriptSceneObject(const HSceneObject& sceneObject);

		/**
		 * Connects an existing managed SceneObject instance with the native SceneObject by creating the interop object.
		 * Throws an exception if the interop object already exists.
		 */
		ScriptSceneObject* createScriptSceneObject(MonoObject* existingInstance, const HSceneObject& sceneObject);

		/**
		 * Connects an existing instance of a ManagedComponent instance with the native ManagedComponent class by creating
		 * the interop object. Throws an exception if the interop object already exists.
		 */
		ScriptManagedComponent* createManagedScriptComponent(MonoObject* existingInstance, const HManagedComponent& component);

		/**
		 * Creates a new interop object that connects a built-in native component with a managed version of that component.
		 */
		ScriptComponentBase* createBuiltinScriptComponent(const HComponent& component);

		/**
		 * Attempts to find the interop object for the specified built-in component. If one cannot be found a new
		 * script interop object is created if @p createNonExisting is enabled, or returns null otherwise.
		 */
		ScriptComponentBase* getBuiltinScriptComponent(const HComponent& component, bool createNonExisting = true);

		/**
		 * Attempts to find the interop object for the specified managed component. If one cannot be found null is returned.
		 */
		ScriptManagedComponent* getManagedScriptComponent(const HManagedComponent& component) const;

		/**
		 * Attempts to find the interop object for a component with the specified instance ID. If one cannot be
		 * found null is returned.
		 */
		ScriptComponentBase* getScriptComponent(UINT64 instanceId) const;

		/** Attempts to find the interop object for the specified SceneObject. If one cannot be found null is returned. */
		ScriptSceneObject* getScriptSceneObject(const HSceneObject& sceneObject) const;

		/**
		 * Attempts to find the interop object for a managed scene object with the specified instance ID. If one cannot be
		 * found null is returned.
		 */
		ScriptSceneObject* getScriptSceneObject(UINT64 instanceId) const;

		/**
		 * Attempts to find the interop object for a GameObject with the specified instance ID. If one cannot be found null
		 * is returned.
		 */
		ScriptGameObjectBase* getScriptGameObject(UINT64 instanceId) const;

		/**	Destroys and unregisters the specified SceneObject interop object. */
		void destroyScriptSceneObject(ScriptSceneObject* sceneObject);

		/**	Destroys and unregisters the specified ManagedComponent interop object. */
		void destroyScriptComponent(ScriptComponentBase* component);

	private:
		/**
		 * Triggers OnReset methods on all registered managed components.
		 *
		 * @note	Usually this happens after an assembly reload.
		 */
		void sendComponentResetEvents();

		/**	Triggered when the any game object is destroyed. */
		void onGameObjectDestroyed(const HGameObject& go);

		UnorderedMap<UINT64, ScriptComponentBase*> mScriptComponents;
		UnorderedMap<UINT64, ScriptSceneObject*> mScriptSceneObjects;

		HEvent mOnAssemblyReloadDoneConn;
		HEvent onGameObjectDestroyedConn;
	};

	/** @} */
}