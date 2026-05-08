//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
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

	/** Provides a base class for all script object wrappers that wrap an IReflectable object that may be passed as a shared pointer. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptReflectableWrapper : public ScriptObjectWrapper
	{
		using Super = ScriptObjectWrapper;
	public:
		using ScriptObjectWrapper::ScriptObjectWrapper;

		/** Returns the root base class of the wrapped native object as a shared pointer. */
		const TShared<IReflectable>& GetBaseNativeObjectAsShared() const { return mNativeObjectStrongHandle; }

		/** Checks is the native object alive and valid. */
		bool IsNativeObjectValid() const { return mNativeObjectStrongHandle != nullptr; }

		/**
		 * Attempts to retrieve an existing associated script object from the provided native object. If one doesn't exist, a new script
		 * object and the associated script wrapper will be created.
		 *
		 * Unlike GetOrCreateScriptObject implemented on TScriptReflectableWrapper, this always accepts the object as an IReflectable, and
		 * needs to perform type lookup to get the exact script wrapper type.
		 */
		static MonoObject* GetOrCreateScriptObject(const TShared<IReflectable>& nativeObject);

		/** Returns the script object wrapper associated with the provided script object, and wrapped by a wrapper that owns the provided meta-data. */
		static ScriptReflectableWrapper* GetScriptObjectWrapper(const ScriptTypeMetaData& wrapperMetaData, MonoObject* scriptObject);

	protected:
		void NotifyNativeObjectDestroyed() override
		{
			mNativeObjectStrongHandle = nullptr;
			Super::NotifyNativeObjectDestroyed();
		}

		TShared<IReflectable> mNativeObjectStrongHandle;
	};

	/** Extends TScriptObjectWrapper by providing functionality required for wrapped native types that may be passed along as an IReflectable shared pointer. */
	template<typename NativeType, typename SelfType, typename BaseType = ScriptReflectableWrapper>
	class TScriptReflectableWrapper : public TScriptObjectWrapper<SelfType, BaseType>
	{
	public:
		TScriptReflectableWrapper(const TShared<NativeType>& nativeObject)
			: TScriptObjectWrapper<SelfType, BaseType>(nativeObject.get())
		{
			mNativeObjectStrongHandle = nativeObject;
		}

		/** Returns the wrapped native object as a shared pointer. */
		TShared<NativeType> GetNativeObjectAsShared() const { return std::static_pointer_cast<NativeType>(mNativeObjectStrongHandle); }

		u32 GetNativeObjectReferenceCount() const override { return (u32)mNativeObjectStrongHandle.use_count(); }

		/**
		 * Creates a new script object and a script object wrapper of @p SelfType, and associates them with the provided native object. Should not be called if @p nativeObject
		 * already has an associated script object.
		 */
		static MonoObject* CreateScriptObjectAndWrapper(const TShared<IReflectable>& nativeObject)
		{
			MonoObject* const scriptObject = SelfType::CreateScriptObject(false);
			ScriptObjectWrapper::Create<SelfType>(B3DRTTICast<NativeType>(nativeObject), scriptObject);

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
		static MonoObject* GetOrCreateScriptObject(const TShared<NativeType>& nativeObject)
		{
			if(nativeObject == nullptr)
				return nullptr;

			if(ScriptObjectWrapper* const scriptObjectWrapper = (ScriptObjectWrapper*)nativeObject->GetScriptObjectWrapper())
				return scriptObjectWrapper->GetScriptObject();

			// TODO: Could skip expensive lookup if the type has no derived classes (should be most cases). In that case the code-gen could generate
			// code that calls a streamlined version of this method, with no lookup.
			const ScriptTypeMetaData* metaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(nativeObject->GetTypeId());

			// Meta-data must be present for native object of type NativeType, but derived classes might not have it as they might not be script exported
			B3D_ENSURE(metaData != nullptr || (NativeType::GetRttiStatic()->GetRttiId() != nativeObject->GetTypeId()));
			if(metaData != nullptr)
				return metaData->ReflectableCreateCallback(nativeObject);

			return CreateScriptObjectAndWrapper(nativeObject);
		}

	protected:
		friend class TScriptObjectWrapper<SelfType, BaseType>;
		friend class TScriptTypeDefinition<SelfType>;

		/** Initialize RTTI type ID and callback used to create the script object/script object wrapper. */
		static void InitializeAdditionalMetaData(ScriptTypeMetaData& metaData)
		{
			metaData.TypeId = NativeType::GetRttiStatic()->GetRttiId();
			metaData.ReflectableCreateCallback = &CreateScriptObjectAndWrapper;
			metaData.CreateCallbackType = ScriptWrapperCreateCallbackType::Reflectable;
			metaData.GetScriptExportable = &GetScriptExportable;
		}
	};

	/** @} */
} // namespace b3d
