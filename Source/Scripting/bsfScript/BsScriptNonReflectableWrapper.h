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

	/** Extends TScriptObjectWrapper by providing functionality required for types not deriving from IReflectable that may be passed along as a shared pointer. */
	template<typename NativeType, typename SelfType, typename BaseType = ScriptObjectWrapper>
	class TScriptNonReflectableWrapper : public TScriptObjectWrapper<SelfType, BaseType>
	{
		using Super = TScriptObjectWrapper<SelfType, BaseType>;
	public:
		TScriptNonReflectableWrapper(const SPtr<NativeType>& nativeObject)
			: TScriptObjectWrapper<SelfType, BaseType>(nativeObject.get()), mNativeObjectStrongHandle(nativeObject)
		{ }

		/** Returns the wrapped native object as a shared pointer. */
		const SPtr<NativeType>& GetNativeObjectAsShared() const { return mNativeObjectStrongHandle; }

		SPtr<NativeType> GetBaseNativeObjectAsShared() const { return GetNativeObjectAsShared(); }

		/** Checks is the native object alive and valid. */
		bool IsNativeObjectValid() const { return mNativeObjectStrongHandle != nullptr; }

		/**
		 * Creates a new script object and a script object wrapper of @p SelfType, and associates them with the provided native object. Should not be called if @p nativeObject
		 * already has an associated script object.
		 */
		static MonoObject* CreateScriptObjectAndWrapper(const SPtr<NativeType>& nativeObject)
		{
			MonoObject* const scriptObject = SelfType::CreateScriptObject(false);
			ScriptObjectWrapper::Create<SelfType>(std::static_pointer_cast<NativeType>(nativeObject), scriptObject);

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

			return CreateScriptObjectAndWrapper(nativeObject);
		}

	protected:
		void NotifyNativeObjectDestroyed() override
		{
			mNativeObjectStrongHandle = nullptr;
			Super::NotifyNativeObjectDestroyed();
		}

		SPtr<NativeType> mNativeObjectStrongHandle;
	};

	/** @} */
} // namespace bs
