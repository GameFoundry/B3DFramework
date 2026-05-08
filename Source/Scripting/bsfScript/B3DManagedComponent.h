//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Scene/B3DComponent.h"
#include "B3DScriptObjectWrapper.h"

namespace b3d
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/**
	 * Component that internally wraps a Mono component object that can be of user-defined type. Acts as interop
	 * layer between native Component and a Mono user defined component type since managed types cannot simply derive
	 * from Component to implement its functionality.
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedComponent : public Component
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
		 * @param[in]	data		Serialized managed component data that will be used for initializing the new managed instance.
		 */
		void Restore(const RawBackupData& data);

		/**	Triggers the managed OnReset callback. */
		void TriggerOnReset();

	private:
		friend class ScriptManagedComponent;

		/** Sets up script bindings between native and managed class. Must be called after creating the script object wrapper, or after assembly is reloaded. */
		void SetupScriptBindings(const TShared<ManagedObjectInfo>& objectInformation);

		/**
		 * Creates the script object of the correct type.
		 *
		 * @param	outObjectInformation	Information about the component type. Can be null in case the type does no longer exist.
		 * @return							Creates script object of the correct component type, or if type cannot be found, script object of missing type.
		 */
		MonoObject* CreateScriptObject(TShared<ManagedObjectInfo>& outObjectInformation) const;

		typedef void(B3D_THUNKCALL* OnCreatedThunkDef)(MonoObject*, MonoException**);
		typedef void(B3D_THUNKCALL* OnInitializedThunkDef)(MonoObject*, MonoException**);
		typedef void(B3D_THUNKCALL* OnUpdateThunkDef)(MonoObject*, MonoException**);
		typedef void(B3D_THUNKCALL* OnDestroyedThunkDef)(MonoObject*, MonoException**);
		typedef void(B3D_THUNKCALL* OnResetThunkDef)(MonoObject*, MonoException**);
		typedef void(B3D_THUNKCALL* OnEnabledThunkDef)(MonoObject*, MonoException**);
		typedef void(B3D_THUNKCALL* OnDisabledThunkDef)(MonoObject*, MonoException**);
		typedef void(B3D_THUNKCALL* OnTransformChangedThunkDef)(MonoObject*, TransformChangedFlags, MonoException**);

		MonoClass* mManagedClass = nullptr;
		MonoReflectionType* mRuntimeType = nullptr;

		String mNamespace;
		String mTypeName;
		String mFullTypeName;
		bool mRequiresReset = true;

		bool mMissingType = false;
		TShared<ManagedSerializableObject> mSerializedObjectData;
		TShared<ManagedObjectInfo> mObjInfo; // Transient

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

		void Initialize() override;
		void OnCreated() override;
		void OnBeginPlay() override;
		void OnDestroyed() override;
		void OnEnabled() override;
		void OnDisabled() override;
		void OnTransformChanged(TransformChangedFlags flags) override;

	public:
		void Update() override;
		bool TypeEquals(const Component& other) override;
		bool CalculateBounds(Bounds& bounds) override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedComponentRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		ManagedComponent() = default; // Serialization only
	};

	/** @} */
} // namespace b3d
