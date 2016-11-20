//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsModule.h"

namespace bs
{
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
		 * Connects an existing managed ManagedComponent instance with the native ManagedComponent by creating the interop
		 * object. Throws an exception if the interop object already exists.
		 */
		ScriptComponent* createScriptComponent(MonoObject* existingInstance, 
			const GameObjectHandle<ManagedComponent>& component);

		/**
		 * Attempts to find the interop object for the specified managed component. If one cannot be found null is returned.
		 */
		ScriptComponent* getScriptComponent(const GameObjectHandle<ManagedComponent>& component) const;

		/**
		 * Attempts to find the interop object for a managed component with the specified instance ID. If one cannot be
		 * found null is returned.
		 */
		ScriptComponent* getScriptComponent(UINT64 instanceId) const;

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
		void destroyScriptComponent(ScriptComponent* component);

		/** 
		 * Sends OnInitialize/OnEnable events to all components that run only while the game is playing (ones without 
		 * RunInEditor attribute). 
		 */
		void wakeRuntimeComponents();

	private:
		/**
		 * Triggers OnReset methods on all registered managed components.
		 *
		 * @note	Usually this happens after an assembly reload.
		 */
		void sendComponentResetEvents();

		/**	Triggered when the any game object is destroyed. */
		void onGameObjectDestroyed(const HGameObject& go);

		UnorderedMap<UINT64, ScriptComponent*> mScriptComponents;
		UnorderedMap<UINT64, ScriptSceneObject*> mScriptSceneObjects;

		HEvent mOnAssemblyReloadDoneConn;
		HEvent onGameObjectDestroyedConn;
	};

	/** @} */
}