//********************************* B3D Framework - Copyright 2024 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "Script/B3DIScriptExportable.h"
#include "Serialization/B3DScriptAssemblyManager.h"

namespace b3d
{
	/** @addtogroup Script
	 *  @{
	 */

	/** Provides a base class for all script object wrappers that wrap a GameObject object that may be passed as a shared pointer. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGameObjectWrapper : public ScriptObjectWrapper
	{
	public:
		using ScriptObjectWrapper::ScriptObjectWrapper;

		/** Returns the root base class of the wrapped native object as a shared pointer. */
		SPtr<GameObject> GetBaseNativeObjectAsShared() const { return mNativeObjectStrongHandle.GetShared(); }

		/** Returns the root base class of the wrapped native object as a handle. */
		const HGameObject& GetBaseNativeObjectAsHandle() const { return mNativeObjectStrongHandle; }

		/** Checks is the native object alive and valid. */
		bool IsNativeObjectValid() const { return GetBaseNativeObjectAsHandle().IsValid(); }

		/**
		 * Attempts to retrieve an existing associated script object from the provided native object. If one doesn't exist, a new script
		 * object and the associated script wrapper will be created.
		 *
		 * Unlike GetOrCreateScriptObject implemented on TScriptGameObjectWrapper, this always accepts the object as a GameObject, and
		 * needs to perform type lookup to get the exact script wrapper type.
		 */
		static MonoObject* GetOrCreateScriptObject(const HGameObject& nativeObject);

		/** Returns the script object wrapper associated with the provided script object, and wrapped by a wrapper that owns the provided meta-data. */
		static ScriptGameObjectWrapper* GetScriptObjectWrapper(const ScriptTypeMetaData& wrapperMetaData, MonoObject* scriptObject);

	protected:
		HGameObject mNativeObjectStrongHandle;
	};

	/** Extends TScriptObjectWrapper by providing functionality required for types that may be passed along as a GameObject handle. */
	template<typename NativeType, typename SelfType, typename BaseType = ScriptGameObjectWrapper>
	class TScriptGameObjectWrapper : public TScriptObjectWrapper<SelfType, BaseType>
	{
	public:
		TScriptGameObjectWrapper(const TGameObjectHandle<NativeType>& nativeObject)
			: TScriptObjectWrapper<SelfType, BaseType>(nativeObject.Get())
		{
			mNativeObjectStrongHandle = nativeObject;
		}

		bool ShouldPersistScriptReload() const override { return true; }
		ScriptObjectLifetimeTrackingMode GetLifetimeTrackingMode() const override { return ScriptObjectLifetimeTrackingMode::StrongHandleWithExplicitDestroy; }

		/** Returns the wrapped native object as a shared pointer. */
		SPtr<NativeType> GetNativeObjectAsShared() const { return std::static_pointer_cast<NativeType>(mNativeObjectStrongHandle.GetShared()); }

		/** Returns the wrapped native object as a handle. */
		TGameObjectHandle<NativeType> GetNativeObjectAsHandle() const { return B3DStaticGameObjectCast<NativeType>(mNativeObjectStrongHandle); }

		/**
		 * Creates a new script object and a script object wrapper of @p SelfType, and associates them with the provided native object. Should not be called if @p nativeObject
		 * already has an associated script object.
		 */
		static MonoObject* CreateScriptObjectAndWrapper(const HGameObject& nativeObject)
		{
			MonoObject* const scriptObject = SelfType::CreateScriptObject(false);
			ScriptObjectWrapper::Create<SelfType>(B3DStaticGameObjectCast<NativeType>(nativeObject), scriptObject);

			return scriptObject;
		}

		/** Casts the reflectable object to script exportable. */
		static IScriptExportable* GetScriptExportable(IReflectable* nativeObject)
		{
			return (IScriptExportable*)(NativeType*)nativeObject;
		}

		/**
		 * Attempts to retrieve an existing associated script object from the provided native object. If one doesn't exist, a new script
		 * object and the associated script wrapper will be created.
		 */
		static MonoObject* GetOrCreateScriptObject(const TGameObjectHandle<NativeType>& nativeObject)
		{
			if(!nativeObject.IsValid())
				return nullptr;

			if(ScriptObjectWrapper* const scriptObjectWrapper = (ScriptObjectWrapper*)nativeObject->GetScriptObjectWrapper())
				return scriptObjectWrapper->GetScriptObject();

			// TODO: Could skip expensive lookup if the type has no derived classes (should be most cases). In that case the code-gen could generate
			// code that calls a streamlined version of this method, with no lookup.
			const ScriptTypeMetaData* metaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(nativeObject->GetTypeId());
			if(B3D_ENSURE(metaData))
				return metaData->GameObjectCreateCallback(nativeObject);

			return CreateScriptObjectAndWrapper(nativeObject);
		}

	protected:
		friend class TScriptObjectWrapper<SelfType, BaseType>;
		friend class TScriptTypeDefinition<SelfType>;

		/** Initialize RTTI type ID and callback used to create the script object/script object wrapper. */
		static void InitializeAdditionalMetaData(ScriptTypeMetaData& metaData)
		{
			metaData.TypeId = NativeType::GetRttiStatic()->GetRttiId();
			metaData.GameObjectCreateCallback = &CreateScriptObjectAndWrapper;
			metaData.CreateCallbackType = ScriptWrapperCreateCallbackType::GameObject;
			metaData.GetScriptExportable = &GetScriptExportable;
		}
	};

	/**	Interop class between C++ & CLR for GameObject. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGameObject : public TScriptGameObjectWrapper<GameObject, ScriptGameObject>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GameObject")

		ScriptGameObject(const HGameObject& nativeObject);

		static void SetupScriptBindings();

		/** Dummy method to create the script object. Not used as game objects are always just base classes, not created directly. */
		static MonoObject* CreateScriptObject(bool construct)
		{
			return nullptr;
		}

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void Internal_GetId(ScriptGameObject* nativeInstance, UUID* outId);
		static bool Internal_IsDestroyed(ScriptGameObject* nativeInstance);
	};

	/** @} */
} // namespace b3d
