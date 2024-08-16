//********************************* bs::framework - Copyright 2024 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"

namespace bs
{
	/** @addtogroup Script
	 *  @{
	 */

	/** Extends TScriptObjectWrapper by providing functionality required for types that may be passed along as a Resource handle. */
	template<typename NativeType, typename SelfType>
	class TScriptResourceWrapper : public TScriptObjectWrapper<SelfType>
	{
	public:
		TScriptResourceWrapper(const TResourceHandle<NativeType>& nativeObject, MonoObject* scriptObject)
			: TScriptObjectWrapper<SelfType>(nativeObject.Get(), scriptObject), mNativeObjectStrongHandle(nativeObject)
		{ }

		/** Returns the wrapped native object as a shared pointer. */
		const SPtr<NativeType>& GetNativeObjectAsShared() const { return mNativeObjectStrongHandle.GetShared(); }

		/** Returns the wrapped native object as a handle. */
		const TResourceHandle<NativeType>& GetNativeObjectAsHandle() const { return mNativeObjectStrongHandle; }

		u32 GetNativeObjectReferenceCount() const override
		{
			return (u32)mNativeObjectStrongHandle.GetReferenceCount();
		}

		/**
		 * Creates a new script object and a script object wrapper of @p SelfType, and associates them with the provided native object. Should not be called if @p nativeObject
		 * already has an associated script object.
		 */
		static MonoObject* CreateScriptObjectAndWrapper(const HResource& nativeObject)
		{
			MonoObject* const scriptObject = SelfType::CreateScriptObject(false);
			B3DNew<SelfType>(B3DStaticResourceCast<NativeType>(nativeObject), scriptObject);

			return scriptObject;
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
			ScriptWrapperObjectMetaData* metaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(nativeObject->GetTypeId());
			if(!B3D_ENSURE(metaData))
				return metaData->ResourceCreateCallback(nativeObject);

			return CreateScriptObjectAndWrapper(nativeObject);
		}

	protected:
		friend class TScriptObjectWrapper<SelfType>;

		/** Initialize RTTI type ID and callback used to create the script object/script object wrapper. */
		static void InitializeAdditionalMetaData(ScriptWrapperObjectMetaData& metaData)
		{
			metaData.TypeId = NativeType::GetRttiStatic()->GetRttiId();
			metaData.ResourceCreateCallback = &CreateScriptObjectAndWrapper;
		}

		TResourceHandle<NativeType> mNativeObjectStrongHandle;
	};

	/** @} */
} // namespace bs
