//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	struct ComponentBackupData;

	/**
	 * Component that internally wraps a Mono component object that can be of user-defined type. Acts as interop
	 * layer between native Component and a Mono user defined component type since managed types cannot simply derive
	 * from Component to implement its functionality.
	 */
	class BS_SCR_BE_EXPORT ManagedComponent : public Component
	{
	public:
		~ManagedComponent();

		/**	Returns managed component object instance. */
		MonoObject* getManagedInstance() const;

		/**	Returns managed class of the component. */
		MonoClass* getClass() const { return mManagedClass; }

		/**	Returns managed type of the component. */
		MonoReflectionType* getRuntimeType() const { return mRuntimeType; }

		/**	Returns namespace of the managed component. */
		const String& getManagedNamespace() const { return mNamespace; }

		/**	Returns type name of the managed component. */
		const String& getManagedTypeName() const { return mTypeName; }

		/**	Returns namespace and type name of the component in format "namespace.typename". */
		const String& getManagedFullTypeName() const { return mFullTypeName; }

		/**
		 * Serializes the internal managed component.
		 *
		 * @param[in]	clearExisting	Should the managed component handle be released. (Will trigger a finalizer if this
		 *								is the last reference to it)
		 * @return						An object containing the serialized component. You can provide this to restore()
		 *								method to re-create the original component.
		 */
		ComponentBackupData backup(bool clearExisting = true);

		/**
		 * Restores a component from previously serialized data.
		 *
		 * @param[in]	instance	New instance of the managed component. Must be of the valid component type or of 
		 *							BansheeEngine.MissingComponent type if the original type is missing.
		 * @param[in]	data		Serialized managed component data that will be used for initializing the new managed
		 *							instance.
		 * @param[in]	missingType	Is the component's type missing (can happen after assembly reload). If true then the
		 *							serialized data will be stored internally until later date when user perhaps restores
		 *							the type with another refresh. @p instance must be null if this is true.
		 */
		void restore(MonoObject* instance, const ComponentBackupData& data, bool missingType);

		/**	Triggers the managed OnReset callback. */
		void triggerOnReset();

	private:
		/**
		 * Finalizes construction of the object. Must be called before use or when the managed component instance changes.
		 *
		 * @param[in]	object	Managed component instance.
		 */
		void initialize(MonoObject* object);

		typedef void(BS_THUNKCALL *OnCreatedThunkDef) (MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL *OnInitializedThunkDef) (MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL *OnUpdateThunkDef) (MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL *OnDestroyedThunkDef) (MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL *OnResetThunkDef) (MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL *OnEnabledThunkDef) (MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL *OnDisabledThunkDef) (MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL *OnTransformChangedThunkDef) (MonoObject*, TransformChangedFlags, MonoException**);

		MonoClass* mManagedClass = nullptr;
		MonoReflectionType* mRuntimeType = nullptr;
		uint32_t mGCHandle = 0;

		String mNamespace;
		String mTypeName;
		String mFullTypeName;
		bool mRequiresReset = true;

		bool mMissingType = false;
		SPtr<ManagedSerializableObject> mSerializedObjectData;
		SPtr<ManagedSerializableObjectInfo> mObjInfo; // Transient

		OnCreatedThunkDef mOnCreatedThunk = nullptr;
		OnInitializedThunkDef mOnInitializedThunk = nullptr;
		OnUpdateThunkDef mOnUpdateThunk = nullptr;
		OnResetThunkDef mOnResetThunk = nullptr;
		OnDestroyedThunkDef mOnDestroyThunk = nullptr;
		OnDestroyedThunkDef mOnDisabledThunk = nullptr;
		OnDestroyedThunkDef mOnEnabledThunk = nullptr;
		OnTransformChangedThunkDef mOnTransformChangedThunk = nullptr;
		MonoMethod* mCalculateBoundsMethod = nullptr;

		/************************************************************************/
		/* 							COMPONENT OVERRIDES                    		*/
		/************************************************************************/

	protected:
		friend class SceneObject;
		friend class ScriptComponent;

		ManagedComponent(const HSceneObject& parent, MonoReflectionType* runtimeType);

		/** @copydoc Component::_instantiate */
		void _instantiate() override;

		/** @copydoc Component::onCreated */
		void onCreated() override;

		/** @copydoc Component::onInitialized */
		void onInitialized() override;

		/** @copydoc Component::onDestroyed */
		void onDestroyed() override;

		/** @copydoc Component::onEnabled  */
		void onEnabled() override;

		/** @copydoc Component::onDisabled  */
		void onDisabled() override;

		/** @copydoc Component::onTransformChanged  */
		void onTransformChanged(TransformChangedFlags flags) override;

	public:
		/** @copydoc Component::update */
		void update() override;

		/** @copydoc Component::typeEquals */
		bool typeEquals(const Component& other) override;

		/** @copydoc Component::calculateBounds */
		bool calculateBounds(Bounds& bounds) override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedComponentRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		ManagedComponent(); // Serialization only
	};

	/**	Contains serialized component data buffer. */
	struct ComponentBackupData
	{
		UINT8* data;
		UINT32 size;
	};

	/** @} */
}