//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Scene/BsComponent.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/**
	 * Component that internally wraps a Mono component object that can be of user-defined type. Acts as interop
	 * layer between native Component and a Mono user defined component type since managed types cannot simply derive
	 * from Component to implement its functionality.
	 */
	class BS_SCR_BE_EXPORT ManagedComponent : public Component
	{
	public:
		~ManagedComponent() = default;

		/**	Returns managed component object instance. */
		MonoObject* GetManagedInstance() const;

		/**	Returns managed class of the component. */
		MonoClass* GetClass() const { return mManagedClass; }

		/**	Returns managed type of the component. */
		MonoReflectionType* GetRuntimeType() const { return mRuntimeType; }

		/**	Returns namespace of the managed component. */
		const String& GetManagedNamespace() const { return mNamespace; }

		/**	Returns type name of the managed component. */
		const String& GetManagedTypeName() const { return mTypeName; }

		/**	Returns namespace and type name of the component in format "namespace.typename". */
		const String& GetManagedFullTypeName() const { return mFullTypeName; }

		/**
		 * Serializes the internal managed component.
		 *
		 * @param[in]	clearExisting	Should the managed component handle be released. (Will trigger a finalizer if this
		 *								is the last reference to it)
		 * @return						An object containing the serialized component. You can provide this to restore()
		 *								method to re-create the original component.
		 */
		RawBackupData Backup(bool clearExisting = true);

		/**
		 * Restores a component from previously serialized data.
		 *
		 * @param[in]	data		Serialized managed component data that will be used for initializing the new managed
		 *							instance.
		 * @param[in]	missingType	Is the component's type missing (can happen after assembly reload). If true then the
		 *							serialized data will be stored internally until later date when user perhaps restores
		 *							the type with another refresh. @p instance must be null if this is true.
		 */
		void Restore(const RawBackupData& data, bool missingType);

		/**	Triggers the managed OnReset callback. */
		void TriggerOnReset();

	private:
		friend class ScriptManagedComponent;

		/**
		 * Finalizes construction of the object. Must be called before use or when the managed component instance changes.
		 *
		 * @param[in]	owner		Script class that handles interop between the native and managed code for this
		 *							component.
		 */
		void Initialize(ScriptManagedComponent* owner);

		typedef void(BS_THUNKCALL* OnCreatedThunkDef)(MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL* OnInitializedThunkDef)(MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL* OnUpdateThunkDef)(MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL* OnDestroyedThunkDef)(MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL* OnResetThunkDef)(MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL* OnEnabledThunkDef)(MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL* OnDisabledThunkDef)(MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL* OnTransformChangedThunkDef)(MonoObject*, TransformChangedFlags, MonoException**);

		MonoClass* mManagedClass = nullptr;
		MonoReflectionType* mRuntimeType = nullptr;
		ScriptManagedComponent* mOwner = nullptr;

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
		OnDisabledThunkDef mOnDisabledThunk = nullptr;
		OnEnabledThunkDef mOnEnabledThunk = nullptr;
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
		void InstantiateInternal() override;

		/** @copydoc Component::onCreated */
		void OnCreated();

		/** @copydoc Component::onInitialized */
		void OnInitialized();

		/** @copydoc Component::onDestroyed */
		void OnDestroyed();

		/** @copydoc Component::onEnabled  */
		void OnEnabled();

		/** @copydoc Component::onDisabled  */
		void OnDisabled();

		/** @copydoc Component::onTransformChanged  */
		void OnTransformChanged(TransformChangedFlags flags) override;

	public:
		/** @copydoc Component::update */
		void Update() override;

		/** @copydoc Component::typeEquals */
		bool TypeEquals(const Component& other) override;

		/** @copydoc Component::calculateBounds */
		bool CalculateBounds(Bounds& bounds) override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedComponentRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		ManagedComponent() = default; // Serialization only
	};

	/** @} */
} // namespace bs
