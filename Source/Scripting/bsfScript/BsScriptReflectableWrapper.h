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

	/** Provides a base class for all script object wrappers that wrap an IReflectable object that may be passed as a shared pointer. */
	class ScriptReflectableWrapper : public ScriptObjectWrapper
	{
	public:
		using ScriptObjectWrapper::ScriptObjectWrapper;

	private:
		/** Returns the root base class of the wrapped native object as a shared pointer. */
		virtual SPtr<IReflectable> GetBaseNativeObjectAsShared() const = 0;
	};

	/** Extends TScriptObjectWrapper by providing functionality required for wrapped native types that may be passed along as an IReflectable shared pointer. */
	template<typename NativeType, typename SelfType>
	class TScriptReflectableWrapper : public TScriptObjectWrapper<SelfType, ScriptReflectableWrapper>
	{
	public:
		TScriptReflectableWrapper(const SPtr<NativeType>& nativeObject, MonoObject* scriptObject)
			: TScriptObjectWrapper<SelfType>(nativeObject.get(), scriptObject), mNativeObjectStrongHandle(nativeObject)
		{ }

		/** Returns the wrapped native object as a shared pointer. */
		const SPtr<NativeType>& GetNativeObjectAsShared() const { return mNativeObjectStrongHandle; }

		SPtr<IReflectable> GetBaseNativeObjectAsShared() const override { return GetNativeObjectAsShared(); }
		u32 GetNativeObjectReferenceCount() const override { return (u32)mNativeObjectStrongHandle.use_count(); }

		/**
		 * Creates a new script object and a script object wrapper of @p SelfType, and associates them with the provided native object. Should not be called if @p nativeObject
		 * already has an associated script object.
		 */
		static MonoObject* CreateScriptObjectAndWrapper(const SPtr<IReflectable>& nativeObject)
		{
			MonoObject* const scriptObject = SelfType::CreateScriptObject(false);
			B3DNew<SelfType>(B3DRTTICast<NativeType>(nativeObject), scriptObject);

			return scriptObject;
		}

		/**
		 * Attempts to retrieve an existing associated script object from the provided native object. If one doesn't exist, a new script
		 * object and the associated script wrapper will be created.
		 */
		static MonoObject* GetOrCreateScriptObject(const SPtr<NativeType>& nativeObject)
		{
			if(nativeObject == nullptr)
				return nullptr;

			if(ScriptObjectWrapper* const scriptObjectWrapper = (ScriptObjectWrapper*)nativeObject->GetScriptObjectWrapper())
				return scriptObjectWrapper->GetScriptObject();

			// TODO: Could skip expensive lookup if the type has no derived classes (should be most cases). In that case the code-gen could generate
			// code that calls a streamlined version of this method, with no lookup.
			ScriptWrapperObjectMetaData* metaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(nativeObject->GetTypeId());
			if(B3D_ENSURE(metaData != nullptr))
				return metaData->ReflectableCreateCallback(nativeObject);

			return CreateScriptObjectAndWrapper(nativeObject);
		}

	protected:
		friend class TScriptObjectWrapper<SelfType>;

		/** Initialize RTTI type ID and callback used to create the script object/script object wrapper. */
		static void InitializeAdditionalMetaData(ScriptWrapperObjectMetaData& metaData)
		{
			metaData.TypeId = NativeType::GetRttiStatic()->GetRttiId();
			metaData.ReflectableCreateCallback = &CreateScriptObjectAndWrapper;
		}

		SPtr<NativeType> mNativeObjectStrongHandle;
	};

	/** @} */
} // namespace bs
