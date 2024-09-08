//*********************************** bs::framework - Copyright 2024 Marko Pintera ***************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"

namespace bs
{
	/** @addtogroup Script
	 *  @{
	 */

	/** Provides a base class for all script object wrappers that wrap a GUIElement object passed as a pointer. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIElementWrapper : public ScriptObjectWrapper
	{
		using Super = ScriptObjectWrapper;
	public:
		using ScriptObjectWrapper::ScriptObjectWrapper;

		/** Checks is the native object alive and valid. */
		bool IsNativeObjectValid() const { return mNativeObject != nullptr; }

	protected:
		void NotifyNativeObjectDestroyed() override
		{
			mNativeObject = nullptr;
			Super::NotifyNativeObjectDestroyed();
		}

		GUIElement* mNativeObject = nullptr;
	};

	/** Extends TScriptObjectWrapper by providing functionality required for types deriving from GUIElement passed along as a pointer. */
	template<typename NativeType, typename SelfType, typename BaseType = ScriptGUIElementWrapper>
	class TScriptGUIElementWrapper : public TScriptObjectWrapper<SelfType, BaseType>
	{
		using Super = TScriptObjectWrapper<SelfType, BaseType>;
	public:
		TScriptGUIElementWrapper(NativeType* nativeObject)
			: TScriptObjectWrapper<SelfType, BaseType>(nativeObject)
		{
			mNativeObject = nativeObject;
		}

		/**
		 * Creates a new script object and a script object wrapper of @p SelfType, and associates them with the provided native object. Should not be called if @p nativeObject
		 * already has an associated script object.
		 */
		static MonoObject* CreateScriptObjectAndWrapper(NativeType* nativeObject)
		{
			MonoObject* const scriptObject = SelfType::CreateScriptObject(false);
			ScriptObjectWrapper::Create<SelfType>(static_cast<NativeType>(nativeObject), scriptObject);

			return scriptObject;
		}

		/**
		 * Attempts to retrieve an existing associated script object from the provided native object. If one doesn't exist, a new script
		 * object and the associated script wrapper will be created.
		 */
		static MonoObject* GetOrCreateScriptObject(NativeType* nativeObject)
		{
			if(nativeObject == nullptr)
				return nullptr;

			if(ScriptObjectWrapper* const scriptObjectWrapper = (ScriptObjectWrapper*)nativeObject->GetScriptObjectWrapper())
				return scriptObjectWrapper->GetScriptObject();

			return CreateScriptObjectAndWrapper(nativeObject);
		}

	protected:
		friend class TScriptObjectWrapper<SelfType, BaseType>;

		static void InitializeAdditionalMetaData(ScriptWrapperObjectMetaData& metaData)
		{ }
	};

	/** @} */
} // namespace bs
