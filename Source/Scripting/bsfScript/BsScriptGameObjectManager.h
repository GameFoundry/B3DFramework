//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	class ScriptRenderable;

	/** @addtogroup bsfScript
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
			ScriptGameObjectEntry() = default;
			ScriptGameObjectEntry(ScriptGameObjectBase* instance, bool isComponent);

			ScriptGameObjectBase* Instance = nullptr;
			bool IsComponent = false;
		};

	public:
		ScriptGameObjectManager();
		~ScriptGameObjectManager();

		/**
		 * Attempts to find the interop object for the specified SceneObject. If one cannot be found new one is created and
		 * returned.
		 */
		ScriptSceneObject* GetOrCreateScriptSceneObject(const HSceneObject& sceneObject);

		/** Creates a new interop object for the specified SceneObject. Throws an exception if one already exists. */
		ScriptSceneObject* CreateScriptSceneObject(const HSceneObject& sceneObject);

		/**
		 * Connects an existing managed SceneObject instance with the native SceneObject by creating the interop object.
		 * Throws an exception if the interop object already exists.
		 */
		ScriptSceneObject* CreateScriptSceneObject(MonoObject* existingInstance, const HSceneObject& sceneObject);

		/**
		 * Connects an existing instance of a ManagedComponent instance with the native ManagedComponent class by creating
		 * the interop object. Throws an exception if the interop object already exists.
		 */
		ScriptManagedComponent* CreateManagedScriptComponent(MonoObject* existingInstance, const HManagedComponent& component);

		/**
		 * Creates a new interop object that connects a built-in native component with a managed version of that component.
		 */
		ScriptComponentBase* CreateBuiltinScriptComponent(const HComponent& component);

		/**
		 * Attempts to find the interop object for the specified built-in component. If one cannot be found a new
		 * script interop object is created if @p createNonExisting is enabled, or returns null otherwise.
		 */
		ScriptComponentBase* GetBuiltinScriptComponent(const HComponent& component, bool createNonExisting = true);

		/**
		 * Attempts to find the interop object for the specified managed component. If one cannot be found null is returned.
		 */
		ScriptManagedComponent* GetManagedScriptComponent(const HManagedComponent& component) const;

		/**
		 * Attempts to find the interop object for a component with the specified instance ID. If one cannot be
		 * found null is returned.
		 */
		ScriptComponentBase* GetScriptComponent(u64 instanceId) const;

		/** Attempts to find the interop object for the specified SceneObject. If one cannot be found null is returned. */
		ScriptSceneObject* GetScriptSceneObject(const HSceneObject& sceneObject) const;

		/**
		 * Attempts to find the interop object for a managed scene object with the specified instance ID. If one cannot be
		 * found null is returned.
		 */
		ScriptSceneObject* GetScriptSceneObject(u64 instanceId) const;

		/**
		 * Attempts to find the interop object for a GameObject with the specified instance ID. If one cannot be found null
		 * is returned.
		 */
		ScriptGameObjectBase* GetScriptGameObject(u64 instanceId) const;

		/**	Destroys and unregisters the specified SceneObject interop object. */
		void DestroyScriptSceneObject(ScriptSceneObject* sceneObject);

		/**	Destroys and unregisters the specified ManagedComponent interop object. */
		void DestroyScriptComponent(ScriptComponentBase* component);

	private:
		/**
		 * Triggers OnReset methods on all registered managed components.
		 *
		 * @note	Usually this happens after an assembly reload.
		 */
		void SendComponentResetEvents();

		/**	Triggered when the any game object is destroyed. */
		void OnGameObjectDestroyed(const HGameObject& go);

		UnorderedMap<u64, ScriptComponentBase*> mScriptComponents;
		UnorderedMap<u64, ScriptSceneObject*> mScriptSceneObjects;

		HEvent mOnAssemblyReloadDoneConn;
		HEvent onGameObjectDestroyedConn;
	};

	/** @} */
} // namespace bs
