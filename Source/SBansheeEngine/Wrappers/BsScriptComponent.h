//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptGameObject.h"
#include "BsScriptObject.h"
#include "BsMonoUtil.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Base class for all Component interop classes. */
	class BS_SCR_BE_EXPORT ScriptComponentBase : public ScriptGameObjectBase
	{
	public:
		ScriptComponentBase(MonoObject* instance);
		virtual ~ScriptComponentBase() { }

		/** Returns the component wrapped by this object. */
		HComponent getComponent() const { return static_object_cast<Component>(getNativeHandle()); }

	protected:
		friend class ScriptGameObjectManager;

		/** Destroys the interop object, unless refresh is in progress in which case it is just prepared for re-creation. */
		void destroy(bool assemblyRefresh);

		/**	Triggered by the script game object manager when the handle this object is referencing is destroyed. */
		virtual void _notifyDestroyed() { }

		/** Checks if the provided game object is destroyed and logs a warning if it is. */
		static bool checkIfDestroyed(const GameObjectHandleBase& handle);
	};

	/**	Base class for a specific builtin component's interop object. */
	template<class ScriptClass, class CompType, class BaseType = ScriptComponentBase>
	class BS_SCR_BE_EXPORT TScriptComponent : public ScriptObject <ScriptClass, BaseType>
	{
	public:
		/**	Returns a generic handle to the internal wrapped component. */
		HGameObject getNativeHandle() const override { return mComponent; }

		/**	Sets the internal component this object wraps. */
		void setNativeHandle(const HGameObject& gameObject) override { mComponent = static_object_cast<CompType>(gameObject); }

		/**	Returns a handle to the internal wrapped component. */
		const GameObjectHandle<CompType>& getHandle() const { return mComponent; }

	protected:
		friend class ScriptGameObjectManager;

		TScriptComponent(MonoObject* instance, const GameObjectHandle<CompType>& component)
			:ScriptObject<ScriptClass, BaseType>(instance), mComponent(component)
		{
			this->setManagedInstance(instance);
		}

		virtual ~TScriptComponent() {}

		/** @copydoc ScriptObject::_createManagedInstance */
		MonoObject* _createManagedInstance(bool construct) override
		{
			MonoObject* managedInstance = ScriptClass::metaData.scriptClass->createInstance(construct);
			this->setManagedInstance(managedInstance);

			return managedInstance;
		}

		/** @copydoc ScriptObjectBase::_clearManagedInstance */
		void _clearManagedInstance() override
		{
			this->freeManagedInstance();
		}

		/**
		 * Triggered by the script game object manager when the native component handle this object point to has been
		 * destroyed.
		 */
		void _notifyDestroyed() override
		{
			this->freeManagedInstance();
		}

		/**	Called when the managed instance gets finalized by the CLR. */
		void _onManagedInstanceDeleted(bool assemblyRefresh) override
		{
			this->freeManagedInstance();

			this->destroy(assemblyRefresh);
		}

		GameObjectHandle<CompType> mComponent;
	};

	/**	Interop class between C++ & CLR for Component. */
	class BS_SCR_BE_EXPORT ScriptComponent : public ScriptObject<ScriptComponent, ScriptComponentBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Component")

	private:
		friend class ScriptGameObjectManager;

		ScriptComponent(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_addComponent(MonoObject* parentSceneObject, MonoReflectionType* type);
		static MonoObject* internal_getComponent(MonoObject* parentSceneObject, MonoReflectionType* type);
		static MonoArray* internal_getComponents(MonoObject* parentSceneObject);
		static MonoArray* internal_getComponentsPerType(MonoObject* parentSceneObject, MonoReflectionType* type);
		static void internal_removeComponent(MonoObject* parentSceneObject, MonoReflectionType* type);
		static MonoObject* internal_getSceneObject(ScriptComponentBase* nativeInstance);
		static TransformChangedFlags internal_getNotifyFlags(ScriptComponentBase* nativeInstance);
		static void internal_setNotifyFlags(ScriptComponentBase* nativeInstance, TransformChangedFlags flags);
		static void internal_destroy(ScriptComponentBase* nativeInstance, bool immediate);
	};

	/** @} */
}