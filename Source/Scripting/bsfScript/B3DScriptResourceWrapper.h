//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "Resources/B3DResource.h"
#include "Script/B3DIScriptExportable.h"

namespace b3d
{
	/** @addtogroup Script
	 *  @{
	 */

	/** Provides a base class for all script object wrappers that wrap a Resource object that may be passed as a shared pointer. */
	class ScriptResourceWrapper : public ScriptObjectWrapper
	{
	public:
		using ScriptObjectWrapper::ScriptObjectWrapper;

		/** Returns the root base class of the wrapped native object as a shared pointer. */
		TShared<Resource> GetBaseNativeObjectAsShared() const { return mNativeObjectStrongHandle.GetShared(); }

		/** Returns the root base class of the wrapped native object as a handle. */
		const HResource& GetBaseNativeObjectAsHandle() const { return mNativeObjectStrongHandle; }

		/** Checks is the native object alive and valid. */
		bool IsNativeObjectValid() const { return GetBaseNativeObjectAsHandle().IsValid(); }

		/**
		 * Attempts to retrieve an existing associated script object from the provided native object. If one doesn't exist, a new script
		 * object and the associated script wrapper will be created.
		 *
		 * Unlike GetOrCreateScriptObject implemented on TScriptResourceWrapper, this always accepts the object as a Resource, and
		 * needs to perform type lookup to get the exact script wrapper type.
		 */
		static MonoObject* GetOrCreateScriptObject(const HResource& nativeObject);

		/** Returns the script object wrapper associated with the provided script object, and wrapped by a wrapper that owns the provided meta-data. */
		static ScriptResourceWrapper* GetScriptObjectWrapper(const ScriptTypeMetaData& wrapperMetaData, MonoObject* scriptObject);

		/**
		 * Returns a resource reference script object, that is wrapping the provided resource.
		 *
		 * @param	resource	Handle to the resource to retrieve the reference for.
		 * @param	rttiId		Type ID for the resource type to create a reference for.
		 * @return				Script object representing the resource reference.
		 */
		static MonoObject* GetOrCreateResourceReference(const HResource& resource, u32 rttiId);

		/** Maps a RTTI ID to a class representing the specified resource type in script code. Returns null if the ID cannot be mapped to a script resource class. */
		static ::MonoClass* GetResourceScriptClass(u32 rttiId);

		/** Returns a RRef<T> type that can be used for wrapping a resource of the type represented by the provided RTTI ID. */
		static ::MonoClass* GetResourceReferenceScriptClass(u32 rttiId);

	protected:
		HResource mNativeObjectStrongHandle;
	};

	/** Extends TScriptObjectWrapper by providing functionality required for types that may be passed along as a Resource handle. */
	template<typename NativeType, typename SelfType, typename BaseType = ScriptResourceWrapper>
	class TScriptResourceWrapper : public TScriptObjectWrapper<SelfType, BaseType>
	{
	public:
		TScriptResourceWrapper(const TResourceHandle<NativeType>& nativeObject)
			: TScriptObjectWrapper<SelfType, BaseType>(nativeObject.Get())
		{
			this->mNativeObjectStrongHandle = nativeObject;
		}

		/** Returns the wrapped native object as a shared pointer. */
		TShared<NativeType> GetNativeObjectAsShared() const { return std::static_pointer_cast<NativeType>(this->mNativeObjectStrongHandle.GetShared()); }

		/** Returns the wrapped native object as a handle. */
		TResourceHandle<NativeType> GetNativeObjectAsHandle() const { return B3DStaticResourceCast<NativeType>(this->mNativeObjectStrongHandle); }

		u32 GetNativeObjectReferenceCount() const override { return (u32)this->mNativeObjectStrongHandle.GetReferenceCount(); }

		/** Returns a resource reference script object, that is wrapping the provided resource. */
		MonoObject* GetOrCreateResourceReference() const
		{
			return ScriptResourceWrapper::GetOrCreateResourceReference(this->mNativeObjectStrongHandle, NativeType::GetRttiStatic()->GetRttiId());
		}

		bool ShouldPersistScriptReload() const override { return true; }
		ScriptObjectLifetimeTrackingMode GetLifetimeTrackingMode() const override { return ScriptObjectLifetimeTrackingMode::StrongHandleWithExplicitDestroy; }

		/**
		 * Creates a new script object and a script object wrapper of @p SelfType, and associates them with the provided native object. Should not be called if @p nativeObject
		 * already has an associated script object.
		 */
		static MonoObject* CreateScriptObjectAndWrapper(const HResource& nativeObject)
		{
			MonoObject* const scriptObject = SelfType::CreateScriptObject(false);
			ScriptObjectWrapper::Create<SelfType>(B3DStaticResourceCast<NativeType>(nativeObject), scriptObject);

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
		static MonoObject* GetOrCreateScriptObject(const TResourceHandle<NativeType>& nativeObject)
		{
			if(!nativeObject.IsValid())
				return nullptr;

			if(ScriptObjectWrapper* const scriptObjectWrapper = (ScriptObjectWrapper*)nativeObject->GetScriptObjectWrapper())
				return scriptObjectWrapper->GetScriptObject();

			// TODO: Could skip expensive lookup if the type has no derived classes (should be most cases). In that case the code-gen could generate
			// code that calls a streamlined version of this method, with no lookup.
			const ScriptTypeMetaData* metaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(nativeObject->GetTypeId());
			if(B3D_ENSURE(metaData))
				return metaData->ResourceCreateCallback(nativeObject);

			return CreateScriptObjectAndWrapper(nativeObject);
		}

	protected:
		friend class TScriptObjectWrapper<SelfType, BaseType>;
		friend class TScriptTypeDefinition<SelfType>;

		/** Initialize RTTI type ID and callback used to create the script object/script object wrapper. */
		static void InitializeAdditionalMetaData(ScriptTypeMetaData& metaData)
		{
			metaData.TypeId = NativeType::GetRttiStatic()->GetRttiId();
			metaData.ResourceCreateCallback = &CreateScriptObjectAndWrapper;
			metaData.CreateCallbackType = ScriptWrapperCreateCallbackType::Resource;
			metaData.GetScriptExportable = &GetScriptExportable;
		}
	};

	/** @} */

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Resource. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptResource : public TScriptResourceWrapper<Resource, ScriptResource>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Resource")

		ScriptResource(const HResource& nativeObject);

		/** Retrieves the underlying native object cast to the correct type. */
		Resource* GetNativeObject() const;

		static void SetupScriptBindings();

		/** Dummy method to create the script object. Not used as Resources is only used as base class and not created directly. */
		static MonoObject* CreateScriptObject(bool construct)
		{
			return nullptr;
		}

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoString* InternalGetName(ScriptResourceWrapper* self);
		static void InternalGetUuid(ScriptResourceWrapper* self, UUID* uuid);
		static void InternalRelease(ScriptResourceWrapper* self);
	};

	/**	Interop class between C++ & CLR for UUID. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptUUID : public TScriptTypeDefinition<ScriptUUID>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "UUID")

		ScriptUUID();

		/**	Unboxes a boxed managed UUID struct and returns the native version of the structure. */
		static UUID Unbox(MonoObject* obj);

		/**	Boxes a native UUID struct and returns a managed object containing it. */
		static MonoObject* Box(const UUID& value);
	};


	/** @} */
} // namespace b3d
