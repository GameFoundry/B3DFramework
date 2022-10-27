//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		virtual ~ScriptComponentBase() = default;

		/** Returns the component wrapped by this object. */
		HComponent GetComponent() const { return static_object_cast<Component>(GetNativeHandle()); }

	protected:
		friend class ScriptGameObjectManager;

		/** Destroys the interop object, unless refresh is in progress in which case it is just prepared for re-creation. */
		void Destroy(bool assemblyRefresh);

		/**	Triggered by the script game object manager when the handle this object is referencing is destroyed. */
		virtual void NotifyDestroyedInternal() {}

		/** Checks if the provided game object is destroyed and logs a warning if it is. */
		static bool CheckIfDestroyed(const GameObjectHandleBase& handle);
	};

	/**	Base class for a specific builtin component's interop object. */
	template <class ScriptClass, class CompType, class BaseType = ScriptComponentBase>
	class BS_SCR_BE_EXPORT TScriptComponent : public ScriptObject<ScriptClass, BaseType>
	{
	public:
		/**	Returns a generic handle to the internal wrapped component. */
		HGameObject GetNativeHandle() const { return static_object_cast<GameObject>(mComponent); }

		/**	Sets the internal component this object wraps. */
		void SetNativeHandle(const HGameObject& gameObject) { mComponent = static_object_cast<CompType>(gameObject); }

		/**	Returns a handle to the internal wrapped component. */
		const GameObjectHandle<CompType>& GetHandle() const { return mComponent; }

	protected:
		friend class ScriptGameObjectManager;

		TScriptComponent(MonoObject* instance, const GameObjectHandle<CompType>& component)
			: ScriptObject<ScriptClass, BaseType>(instance), mComponent(component)
		{
			this->SetManagedInstance(instance);
		}

		virtual ~TScriptComponent() {}

		/** @copydoc ScriptObject::_createManagedInstance */
		MonoObject* CreateManagedInstanceInternal(bool construct) override
		{
			MonoObject* managedInstance = ScriptClass::metaData.ScriptClass->CreateInstance(construct);
			this->SetManagedInstance(managedInstance);

			return managedInstance;
		}

		/** @copydoc ScriptObjectBase::_clearManagedInstance */
		void ClearManagedInstanceInternal() override
		{
			this->FreeManagedInstance();
		}

		/**
		 * Triggered by the script game object manager when the native component handle this object point to has been
		 * destroyed.
		 */
		void NotifyDestroyedInternal() override
		{
			this->FreeManagedInstance();
		}

		/**	Called when the managed instance gets finalized by the CLR. */
		void OnManagedInstanceDeletedInternal(bool assemblyRefresh) override
		{
			this->FreeManagedInstance();

			this->Destroy(assemblyRefresh);
		}

		GameObjectHandle<CompType> mComponent;
	};

	/**	Interop class between C++ & CLR for Component. */
	class BS_SCR_BE_EXPORT ScriptComponent : public ScriptObject<ScriptComponent, ScriptComponentBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Component")

	private:
		friend class ScriptGameObjectManager;

		ScriptComponent(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* InternalAddComponent(MonoObject* parentSceneObject, MonoReflectionType* type);
		static MonoObject* InternalGetComponent(MonoObject* parentSceneObject, MonoReflectionType* type);
		static MonoArray* InternalGetComponents(MonoObject* parentSceneObject);
		static MonoArray* InternalGetComponentsPerType(MonoObject* parentSceneObject, MonoReflectionType* type);
		static void InternalRemoveComponent(MonoObject* parentSceneObject, MonoReflectionType* type);
		static MonoObject* InternalGetSceneObject(ScriptComponentBase* nativeInstance);
		static TransformChangedFlags InternalGetNotifyFlags(ScriptComponentBase* nativeInstance);
		static void InternalSetNotifyFlags(ScriptComponentBase* nativeInstance, TransformChangedFlags flags);
		static void InternalDestroy(ScriptComponentBase* nativeInstance, bool immediate);
	};

	/** @} */
} // namespace bs
