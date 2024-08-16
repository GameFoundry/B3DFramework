//********************************* bs::framework - Copyright 2024 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptMeta.h"
#include "BsMonoManager.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "Script/BsIScriptObjectWrapper.h"
#include "Serialization/BsScriptAssemblyManager.h"

namespace bs
{
	/** @addtogroup Script
	 *  @{
	 */

	/** Checks if the class @p T has a GetShared() method that accepts no parameters. */
	template <typename T, typename = void>
	struct B3DHasGetShared : std::false_type {};

	template <typename T>
	struct B3DHasGetShared<T, std::void_t<decltype(std::declval<T>().GetShared())>> : std::true_type {};

	/** Checks if the class @p T has a GetHandle() method that accepts no parameters. */
	template <typename T, typename = void>
	struct B3DHasGetHandle : std::false_type {};

	template <typename T>
	struct B3DHasGetHandle<T, std::void_t<decltype(std::declval<T>().GetHandle())>> : std::true_type {};

	/** Structure to persist object data during script reload. Objects will store their data before reload happens, and then restore the data after it happens. */
	struct ScriptObjectReloadPersistentData
	{
		ScriptObjectReloadPersistentData() {}

		explicit ScriptObjectReloadPersistentData(const Any& data)
			: Data(data)
		{}

		Any Data; // TODO - Don't use Any
	};

	/**
	 * Extends IScriptObjectWrapper by keeping a strong reference to the script object, and releasing it as needed.
	 * As well as providing an interface for script reload functionality.. See IScriptObjectWrapper.
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptObjectWrapper : public IScriptObjectWrapper
	{
	public:
		ScriptObjectWrapper(IScriptExportable* nativeObject, MonoObject* scriptObject);
		virtual ~ScriptObjectWrapper();

		MonoObject* GetScriptObject() const;

		/** Called when the script system is notified that the script object has been destroyed. */
		virtual void NotifyScriptObjectDestroyed(bool isDestroyedDueToScriptReload);

		void NotifyNativeObjectDestroyed() override;

		/**
		 * @name Script reload
		 * @{
		 */

		/**
		 * If true, the object will be given an opportunity to back up its data before a script reload operation, and its script object will be automatically
		 * recreated once script reload ends, and given an opportunity to restore the backed up data. If false, the script object and its wrapper will
		 * be destroyed on script reload.
		 */
		virtual bool ShouldPersistScriptReload() const { return false; }

		/**
		 * Called on all script object wrappers when script reload is about to happen. Allows the script object to back up its current state
		 * so it may be restored after reload completes. Only relevant for script objects that persist script reload (i.e. ShouldPersistScriptReload() returns true).
		 */
		virtual Optional<ScriptObjectReloadPersistentData> BackupDataBeforeScriptReload() { return {}; }

		/**
		 * Called on all script object wrappers when script reload is about to happen, after BackupDataBeforeScriptReload() is called. Allows the wrapper to
		 * release any explicit strong handles it may be holding, so the object gets released correctly.
		 */
		virtual void ReleaseStrongHandlesBeforeScriptReload();

		/**
		 * Called after script reload completes. This needs to recreate the internal script object, as the old one will have been destroyed during the reload.
		 * Only relevant for script objects that persist script reload (i.e. ShouldPersistScriptReload() returns true).
		 */
		virtual void RecreateScriptObjectAfterScriptReload() { }

		/**
		 * Called as the final step after script reload completes. Allows you to restore data backed up in BackupDataBeforeScriptReload() call to the
		 * newly created script object. Only relevant for script objects that persist script reload (i.e. ShouldPersistScriptReload() returns true).
		 */
		virtual void RestoreDataAfterScriptReload(const ScriptObjectReloadPersistentData& data) { }

		/** @} */

	protected:
		u32 mStrongScriptObjectHandle = ~0u;
	};

	template <typename SelfType>
	class TScriptObjectWrapper;

	/** Ensures that ScriptObjectWrapper types are initialized on application load. */
	template <typename SelfType>
	struct InitializeScriptObjectWrapperOnLoadTime
	{
	public:
		InitializeScriptObjectWrapperOnLoadTime()
		{
			TScriptObjectWrapper<SelfType>::InitializeMetaDataAtLoadTime();
		}

		void MakeSureIAmInstantiated() {}
	};

	/**
	 * Provides common functionality required by specializations of ScriptObjectWrapper, including a meta-data object to store information about the type, ability to
	 * bind the script object wrapper to the script object, and retrieve the wrapper from the script object
	 */
	template <typename SelfType>
	class TScriptObjectWrapper : public ScriptObjectWrapper 
	{
	public:
		TScriptObjectWrapper(IScriptExportable* nativeObject, MonoObject* scriptObject)
			: ScriptObjectWrapper(nativeObject, scriptObject)
		{
			sInitializeOnLoadTime.MakeSureIAmInstantiated();
			BindSelfToScriptObject(scriptObject);
		}

		virtual ~TScriptObjectWrapper() = default;

		void RecreateScriptObjectAfterScriptReload() override
		{
			MonoObject* const scriptObject = SelfType::CreateScriptObject(true);
			BindSelfToScriptObject(scriptObject);
		}

		/** Returns the script object wrapper associated with the provided script object. */
		static SelfType* GetScriptObjectWrapper(MonoObject* scriptObject)
		{
			SelfType* scriptObjectWrapper = nullptr;

			if(sInteropMetaData.ScriptObjectWrapperPointerField != nullptr && scriptObject != nullptr)
				sInteropMetaData.ScriptObjectWrapperPointerField->Get(scriptObject, &scriptObjectWrapper);

			return scriptObjectWrapper;
		}

		/** Returns the meta-data storing information about the script exported type. */
		static const ScriptWrapperObjectMetaData* GetMetaData() { return &sInteropMetaData; }

		/**
		 * Takes care of initializing the meta-data when the application first load. The meta-data will be registered with a global manager that will ensure
		 * it is kept up-to-date after operations such as assembly (re)load.
		 */
		static void InitializeMetaDataAtLoadTime()
		{
			// Need to delay init of sInteropMetaData since it's also a static, and we can't guarantee the order
			// (if it gets initialized after this, it will just overwrite the data)
			ScriptWrapperObjectMetaData localMetaData = ScriptWrapperObjectMetaData(SelfType::GetAssemblyName(), SelfType::GetNamespace(), SelfType::GetTypeName(), &SelfType::SetupScriptBindings);

			SelfType::InitializeAdditionalMetaData(localMetaData);
			MonoManager::RegisterScriptType(&sInteropMetaData, localMetaData);
		}

	protected:
		/** Stores a pointer to itself in the script object. This ensures that calls to GetScriptObjectWrapper() can return the script object wrapper associated with the script object. */
		void BindSelfToScriptObject(MonoObject* scriptObject)
		{
			SelfType* self = (SelfType*)(ScriptObjectWrapper*)this; // Needed due to multiple inheritance. Safe since SelfType must point to an class derived from this one.

			if(sInteropMetaData.ScriptObjectWrapperPointerField != nullptr)
				sInteropMetaData.ScriptObjectWrapperPointerField->Set(scriptObject, &self);

			if(sInteropMetaData.IsUsingNewScriptObjectManagerField != nullptr)
			{
				i32 value = 1;
				sInteropMetaData.IsUsingNewScriptObjectManagerField->Set(scriptObject, &value);
			}
		}

		static ScriptWrapperObjectMetaData sInteropMetaData;
		static InitializeScriptObjectWrapperOnLoadTime<SelfType> sInitializeOnLoadTime;
	};

	template <typename SelfType>
	InitializeScriptObjectWrapperOnLoadTime<SelfType> TScriptObjectWrapper<SelfType>::sInitializeOnLoadTime;

	template <typename SelfType>
	ScriptWrapperObjectMetaData TScriptObjectWrapper<SelfType>::sInteropMetaData;

	/**	Specialized version of TScriptObjectWrapper that should be used for types that are never going to be explicitly instantiated (e.g. singletons, static-only classes and base classes). */
	template <typename SelfType>
	class TNonInstantiableScriptObjectWrapper : public TScriptObjectWrapper<SelfType>
	{
	public:
		TNonInstantiableScriptObjectWrapper(MonoObject* scriptObject)
			:TScriptObjectWrapper<SelfType>(nullptr, scriptObject)
		{ }

		/** Dummy method to create the script object. Not needed for non-instantiable types. */
		static MonoObject* CreateScriptObject(bool construct)
		{
			return nullptr;
		}

	protected:
		friend class TScriptObjectWrapper<SelfType>;

		/** Dummy method to initialize additional meta-data. Not needed for non-instantiable types. */
		static void InitializeAdditionalMetaData(ScriptWrapperObjectMetaData& metaData)
		{
			// Do nothing
		}
	};

	/** Extends TScriptObjectWrapper by providing functionality required for types that may be passed along as an IReflectable shared pointer. */
	template<typename NativeType, typename SelfType>
	class TScriptReflectableWrapper : public TScriptObjectWrapper<SelfType> // TODO - Move to own file
	{
	public:
		TScriptReflectableWrapper(const SPtr<NativeType>& nativeObject, MonoObject* scriptObject)
			: TScriptObjectWrapper<SelfType>(nativeObject.get(), scriptObject), mNativeObjectStrongHandle(nativeObject)
		{ }

		/** Returns the wrapped native object as a shared pointer. */
		const SPtr<NativeType>& GetNativeObjectAsShared() const { return mNativeObjectStrongHandle; }

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

	/** Extends TScriptObjectWrapper by providing functionality required for types that may be passed along as a GameObject handle. */
	template<typename NativeType, typename SelfType>
	class TScriptGameObjectWrapper : public TScriptObjectWrapper<SelfType> // TODO - Move to own file
	{
	public:
		TScriptGameObjectWrapper(const GameObjectHandle<NativeType>& nativeObject, MonoObject* scriptObject)
			: TScriptObjectWrapper<SelfType>(nativeObject.Get(), scriptObject), mNativeObjectStrongHandle(nativeObject)
		{ }

		/** Returns the wrapped native object as a shared pointer. */
		const SPtr<NativeType>& GetNativeObjectAsShared() const { return mNativeObjectStrongHandle.GetShared(); }

		/** Returns the wrapped native object as a handle. */
		const GameObjectHandle<NativeType>& GetNativeObjectAsHandle() const { return mNativeObjectStrongHandle; }

		/**
		 * Creates a new script object and a script object wrapper of @p SelfType, and associates them with the provided native object. Should not be called if @p nativeObject
		 * already has an associated script object.
		 */
		static MonoObject* CreateScriptObjectAndWrapper(const HGameObject& nativeObject)
		{
			MonoObject* const scriptObject = SelfType::CreateScriptObject(false);
			B3DNew<SelfType>(B3DStaticGameObjectCast<NativeType>(nativeObject), scriptObject);

			return scriptObject;
		}

		/**
		 * Attempts to retrieve an existing associated script object from the provided native object. If one doesn't exist, a new script
		 * object and the associated script wrapper will be created.
		 */
		static MonoObject* GetOrCreateScriptObject(const GameObjectHandle<NativeType>& nativeObject)
		{
			if(!nativeObject.IsValid())
				return nullptr;

			if(ScriptObjectWrapper* const scriptObjectWrapper = (ScriptObjectWrapper*)nativeObject->GetScriptObjectWrapper())
				return scriptObjectWrapper->GetScriptObject();

			// TODO: Could skip expensive lookup if the type has no derived classes (should be most cases). In that case the code-gen could generate
			// code that calls a streamlined version of this method, with no lookup.
			ScriptWrapperObjectMetaData* metaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(nativeObject->GetTypeId());
			if(!B3D_ENSURE(metaData))
				return metaData->GameObjectCreateCallback(nativeObject);

			return CreateScriptObjectAndWrapper(nativeObject);
		}

	protected:
		friend class TScriptObjectWrapper<SelfType>;

		/** Initialize RTTI type ID and callback used to create the script object/script object wrapper. */
		static void InitializeAdditionalMetaData(ScriptWrapperObjectMetaData& metaData)
		{
			metaData.TypeId = NativeType::GetRttiStatic()->GetRttiId();
			metaData.GameObjectCreateCallback = &CreateScriptObjectAndWrapper;
		}

		GameObjectHandle<NativeType> mNativeObjectStrongHandle;
	};

	/** Extends TScriptObjectWrapper by providing functionality required for types that may be passed along as a Resource handle. */
	template<typename NativeType, typename SelfType>
	class TScriptResourceWrapper : public TScriptObjectWrapper<SelfType> // TODO - Move to own file
	{
	public:
		TScriptResourceWrapper(const TResourceHandle<NativeType>& nativeObject, MonoObject* scriptObject)
			: TScriptObjectWrapper<SelfType>(nativeObject.Get(), scriptObject), mNativeObjectStrongHandle(nativeObject)
		{ }

		/** Returns the wrapped native object as a shared pointer. */
		const SPtr<NativeType>& GetNativeObjectAsShared() const { return mNativeObjectStrongHandle.GetShared(); }

		/** Returns the wrapped native object as a handle. */
		const TResourceHandle<NativeType>& GetNativeObjectAsHandle() const { return mNativeObjectStrongHandle; }

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

	// TODO - Add wrapper for GUI elements and non-reflectable classes

	/** Implements default methods required by script object wrapper implementations. */
#define B3D_SCRIPT_OBJECT_WRAPPER(Assembly, Namespace, Name) \
	static const char* GetAssemblyName()      \
	{                                         \
		return Assembly;                      \
	}                                         \
	static const char* GetNamespace()         \
	{                                         \
		return Namespace;                     \
	}                                         \
	static const char* GetTypeName()          \
	{                                         \
		return Name;                          \
	}                                         \
	static void SetupScriptBindings();

	/**	Script object wrapper for ScriptObject. (Script prefix used as standard for script object wrappers, wrapping ScriptObject. Therefore ScriptScriptObject.) */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptScriptObject : public TNonInstantiableScriptObjectWrapper<ScriptScriptObject>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ScriptObject")

	private:
		ScriptScriptObject(MonoObject* scriptObject);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void Internal_ScriptObjectFinalizerCalled(ScriptObjectWrapper* scriptObjectWrapper);
	};

	/** @} */
} // namespace bs
