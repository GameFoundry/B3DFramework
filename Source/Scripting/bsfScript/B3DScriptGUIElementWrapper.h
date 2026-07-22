//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "Serialization/B3DScriptAssemblyManager.h"

namespace b3d
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

		ScriptObjectLifetimeTrackingMode GetLifetimeTrackingMode() const override { return ScriptObjectLifetimeTrackingMode::StrongHandleWithExplicitDestroy; } 

		/** Checks is the native object alive and valid. */
		bool IsNativeObjectValid() const { return mNativeObject != nullptr; }

		/** Returns the native object that is being wrapped. */
		GUIElement* GetNativeObject() const { return mNativeObject; }

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
			this->mNativeObject = nativeObject;
		}

		/**
		 * Creates a new script object and a script object wrapper of @p SelfType, and associates them with the provided native object. Should not be called if @p nativeObject
		 * already has an associated script object.
		 */
		static MonoObject* CreateScriptObjectAndWrapper(GUIElement* nativeObject)
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
		static MonoObject* GetOrCreateScriptObject(NativeType* nativeObject)
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
				return metaData->GUIElementCreateCallback(nativeObject);

			return CreateScriptObjectAndWrapper(nativeObject);
		}

	protected:
		friend class TScriptObjectWrapper<SelfType, BaseType>;
		friend class TScriptTypeDefinition<SelfType>;

		/** Initialize RTTI type ID and callback used to create the script object/script object wrapper. */
		static void InitializeAdditionalMetaData(ScriptTypeMetaData& metaData)
		{
			metaData.TypeId = NativeType::GetRttiStatic()->GetRttiId();
			metaData.GUIElementCreateCallback = &CreateScriptObjectAndWrapper;
			metaData.CreateCallbackType = ScriptWrapperCreateCallbackType::GUIElement;
			metaData.GetScriptExportable = &GetScriptExportable;
		}
	};

	/** @} */
} // namespace b3d
