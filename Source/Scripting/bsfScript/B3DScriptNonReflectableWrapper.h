//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"

namespace b3d
{
	/** @addtogroup Script
	 *  @{
	 */

	/** Provides a root base class to use for TScriptNonReflectableWrapper implementations. Ensures all derived types have a common base class that holds the native object of the root type in the class hierarchy. */
	template<typename NativeType>
	class B3D_SCRIPT_INTEROP_EXPORT TScriptNonReflectableWrapperBase : public ScriptObjectWrapper
	{
		using Super = ScriptObjectWrapper;
	public:
		using ScriptObjectWrapper::ScriptObjectWrapper;

		/** Returns the root base class of the wrapped native object as a shared pointer. */
		const SPtr<NativeType>& GetBaseNativeObjectAsShared() const { return mNativeObjectStrongHandle; }

		/** Checks is the native object alive and valid. */
		bool IsNativeObjectValid() const { return mNativeObjectStrongHandle != nullptr; }

	protected:
		void NotifyNativeObjectDestroyed() override
		{
			mNativeObjectStrongHandle = nullptr;
			Super::NotifyNativeObjectDestroyed();
		}

		SPtr<NativeType> mNativeObjectStrongHandle;
	};

	/** Extends TScriptObjectWrapper by providing functionality required for types not deriving from IReflectable that may be passed along as a shared pointer. */
	template<typename NativeType, typename SelfType, typename BaseType = TScriptNonReflectableWrapperBase<NativeType>>
	class TScriptNonReflectableWrapper : public TScriptObjectWrapper<SelfType, BaseType>
	{
		using Super = TScriptObjectWrapper<SelfType, BaseType>;
	public:
		TScriptNonReflectableWrapper(const SPtr<NativeType>& nativeObject)
			: TScriptObjectWrapper<SelfType, BaseType>(nativeObject.get())
		{
			mNativeObjectStrongHandle = nativeObject;
		}

		/** Returns the wrapped native object as a shared pointer. */
		SPtr<NativeType> GetNativeObjectAsShared() const { return std::static_pointer_cast<NativeType>(mNativeObjectStrongHandle); }

		u32 GetNativeObjectReferenceCount() const override { return (u32)mNativeObjectStrongHandle.use_count(); }

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
	};

	/** @} */
} // namespace b3d
