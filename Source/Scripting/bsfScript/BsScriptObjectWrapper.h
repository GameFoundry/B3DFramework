//********************************* bs::framework - Copyright 2024 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptMeta.h"
#include "BsMonoManager.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "Script/BsIScriptObjectWrapper.h"

namespace bs
{
	/** @addtogroup Script
	 *  @{
	 */

	// TODO - Doc
	struct ScriptObjectReloadPersistentData
	{
		ScriptObjectReloadPersistentData() {}

		explicit ScriptObjectReloadPersistentData(const Any& data)
			: Data(data)
		{}

		Any Data; // TODO - Don't use Any
	};

	// TODO - Doc
	template<typename NativeObjectType>
	class ScriptExportedNativeObjectStorage
	{ };

	// TODO - Doc
	template<typename NativeObjectType>
	class ScriptExportedNativeObjectStorage<SPtr<NativeObjectType>>
	{
		ScriptExportedNativeObjectStorage(SPtr<NativeObjectType> nativeObject)
			:mNativeObject(std::move(nativeObject))
		{ }

		// TODO - Doc
		NativeObjectType* Get() const { return mNativeObject.get(); }

		// TODO - Doc
		const SPtr<NativeObjectType>& GetShared() const { return mNativeObject; }

		// TODO - Doc
		void Clear() { mNativeObject = nullptr; }

		// TODO - Doc
		static NativeObjectType* GetRawPointer(const SPtr<NativeObjectType>& object) { return object.get(); }
	private:
		SPtr<NativeObjectType> mNativeObject;
	};

	// TODO - Doc
	template<typename NativeObjectType>
	class ScriptExportedNativeObjectStorage<TResourceHandle<NativeObjectType>>
	{
		ScriptExportedNativeObjectStorage(TResourceHandle<NativeObjectType> nativeObject)
			:mNativeObject(std::move(nativeObject))
		{ }

		// TODO - Doc
		NativeObjectType* Get() const { return mNativeObject.Get(); }

		// TODO - Doc
		const SPtr<NativeObjectType>& GetShared() const { return mNativeObject.GetShared(); }

		// TODO - Doc
		const TResourceHandle<NativeObjectType>& GetHandle() const { return mNativeObject; }

		// TODO - Doc
		void Clear() { mNativeObject = nullptr; }

		// TODO - Doc
		static NativeObjectType* GetRawPointer(const TResourceHandle<NativeObjectType>& object) { return object.Get(); }
	private:
		TResourceHandle<NativeObjectType> mNativeObject;
	};

	// TODO - Doc
	template<typename NativeObjectType>
	class ScriptExportedNativeObjectStorage<GameObjectHandle<NativeObjectType>>
	{
		ScriptExportedNativeObjectStorage(GameObjectHandle<NativeObjectType> nativeObject)
			:mNativeObject(std::move(nativeObject))
		{ }

		// TODO - Doc
		NativeObjectType* Get() const { return mNativeObject.Get(); }

		// TODO - Doc
		const SPtr<NativeObjectType>& GetShared() const { return mNativeObject.GetShared(); }

		// TODO - Doc
		const GameObjectHandle<NativeObjectType>& GetHandle() const { return mNativeObject; }

		// TODO - Doc
		void Clear() { mNativeObject = nullptr; }

		// TODO - Doc
		static NativeObjectType* GetRawPointer(const GameObjectHandle<NativeObjectType>& object) { return object.Get(); }
	private:
		GameObjectHandle<NativeObjectType> mNativeObject;
	};

	// TODO - Doc
	class B3D_SCRIPT_INTEROP_EXPORT ScriptObjectWrapper : public IScriptObjectWrapper
	{
	public:
		ScriptObjectWrapper(IScriptExportable* nativeObject, MonoObject* scriptObject);
		virtual ~ScriptObjectWrapper();

		MonoObject* GetScriptObject() const;

		/** Called when the script system is notified that the script object has been destroyed. */
		virtual void NotifyScriptObjectDestroyed(bool isDestroyedDueToScriptReload);

		/**
		 * If this method returns true, then the we will keep a strong reference to the script object as long as the native object is alive. Note this is only safe for
		 * native objects that are explicitly destroyed, as the wrapper itself holds a strong reference to the native object and the object would never be freed if we only
		 * rely on reference counting. Note when explicitly destroying the native object, you need to manually call NotifyNativeObjectDestroyed() from the destroy method.
		 *
		 * // TODO - The above approach might not work for e.g. resources. I might need to add a boolean to IScriptExportable that lets the object know if its being referenced
		 * // from script or not. Then at certain intervals we can iterate over all script object wrappers and check objects that have their reference count at 1 with the
		 * // script reference flag set. In such situation we can release the script object strong handle.
		 * // - Actually this approach can be extended in the general case and we can keep strong handles in all cases, and don't need this method at all
		 * // - And I don't need a special flag, I can just check if the wrapper is assigned to IScriptExportable
		 * // - TODO - What when the object gets referenced from native code again? e.g. a weak resource handle gets converted into a strong one. Then the
		 * //   script object might get killed, but the native object is still alive. But in that case we might be okay to just re-create the script object?
		 */
		virtual bool ShouldKeepStrongReferenceToScriptObject() const { return false; }

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

	template <typename NativeTypeContainer, typename SelfType, typename ScriptWrapperObjectBaseClass>
	class TScriptObjectWrapper;

	// TODO - Doc
	template <typename NativeTypeContainer, typename SelfType, typename ScriptWrapperObjectBaseClass>
	struct InitializeScriptObjectWrapperOnLoadTime
	{
	public:
		InitializeScriptObjectWrapperOnLoadTime()
		{
			TScriptObjectWrapper<NativeTypeContainer, SelfType, ScriptWrapperObjectBaseClass>::InitializeMetaDataAtLoadTime();
		}

		void MakeSureIAmInstantiated() {}
	};

	/**	Template version of ScriptObjectBase populates the object meta-data on library load. */
	template <typename NativeTypeContainerType, typename SelfType, typename ScriptWrapperObjectBaseClass = ScriptObjectWrapper>
	class TScriptObjectWrapper : public ScriptWrapperObjectBaseClass 
	{
	public:
		/** Constructor for wrappers that are expected to be instantiated (non-abstract types). */
		template<typename Condition = NativeTypeContainerType, std::enable_if_t<!std::is_same_v<Condition, nullptr_t>, int> = 0>
		TScriptObjectWrapper(NativeTypeContainerType nativeObject, MonoObject* scriptObject)
			: ScriptWrapperObjectBaseClass(ScriptExportedNativeObjectStorage<NativeTypeContainerType>::GetRawPointer(nativeObject), scriptObject), mNativeObjectStorage(std::move(nativeObject))
		{
			sInitializeOnLoadTime.MakeSureIAmInstantiated();

			SelfType* self = (SelfType*)(ScriptWrapperObjectBaseClass*)this; // Needed due to multiple inheritance. Safe since SelfType must point to an class derived from this one.

			if(sInteropMetaData.ScriptObjectWrapperPointerField != nullptr)
				sInteropMetaData.ScriptObjectWrapperPointerField->Set(scriptObject, &self);

			if(sInteropMetaData.IsUsingNewScriptObjectManagerField != nullptr)
			{
				i32 value = 1;
				sInteropMetaData.IsUsingNewScriptObjectManagerField->Set(scriptObject, &value);
			}
		}

		/** Constructors for wrappers that won't be instantiated (abstract types). Those wrappers only provide interop bindings and don't do lifetime tracking. */
		template<typename Condition = NativeTypeContainerType, std::enable_if_t<std::is_same_v<Condition, nullptr_t>, int> = 0>
		TScriptObjectWrapper(MonoObject* scriptObject)
			: ScriptWrapperObjectBaseClass(nullptr, scriptObject)
		{
			sInitializeOnLoadTime.MakeSureIAmInstantiated();

			SelfType* self = (SelfType*)(ScriptWrapperObjectBaseClass*)this; // Needed due to multiple inheritance. Safe since SelfType must point to an class derived from this one.

			if(sInteropMetaData.ScriptObjectWrapperPointerField != nullptr)
				sInteropMetaData.ScriptObjectWrapperPointerField->Set(scriptObject, &self);

			if(sInteropMetaData.IsUsingNewScriptObjectManagerField != nullptr)
			{
				i32 value = 1;
				sInteropMetaData.IsUsingNewScriptObjectManagerField->Set(scriptObject, &value);
			}
		}

		virtual ~TScriptObjectWrapper() = default;

		/** Returns the storage object that is responsible for holding a strong reference to the native object. */
		const ScriptExportedNativeObjectStorage<NativeTypeContainerType>& GetNativeObjectStorage() const { return mNativeObjectStorage; }

		// TODO - Doc
		virtual MonoObject* CreateScriptObject(bool construct)
		{
			return sInteropMetaData.ScriptClass->CreateInstance(construct);
		}

		void RecreateScriptObjectAfterScriptReload() override
		{
			MonoObject* const scriptObject = CreateScriptObject(true);

			SelfType* self = (SelfType*)(ScriptWrapperObjectBaseClass*)this; // Needed due to multiple inheritance. Safe since SelfType must point to an class derived from this one.

			if(sInteropMetaData.ScriptObjectWrapperPointerField != nullptr)
				sInteropMetaData.ScriptObjectWrapperPointerField->Set(scriptObject, &self);

			if(sInteropMetaData.IsUsingNewScriptObjectManagerField != nullptr)
			{
				i32 value = 1;
				sInteropMetaData.IsUsingNewScriptObjectManagerField->Set(scriptObject, &value);
			}
		}

		// TODO - Doc
		template<typename Condition = NativeTypeContainerType, std::enable_if_t<!std::is_same_v<Condition, nullptr_t>, int> = 0>
		static MonoObject* GetOrCreateScriptObjectWrapper(NativeTypeContainerType nativeObject)
		{
			if(ScriptObjectWrapper* const scriptObjectWrapper = (ScriptObjectWrapper*)nativeObject->GetScriptObjectWrapper())
				return scriptObjectWrapper->GetScriptObject();

			MonoObject* const scriptObject = SelfType::sInteropMetaData.ScriptClass->CreateInstance(false);
			B3DNew<SelfType>(nativeObject, scriptObject);

			return scriptObject;
		}

		/** Returns the script object wrapper associated with the provided script object. */
		static SelfType* GetScriptObjectWrapper(MonoObject* scriptObject)
		{
			SelfType* scriptObjectWrapper = nullptr;

			if(sInteropMetaData.ScriptObjectWrapperPointerField != nullptr && scriptObject != nullptr)
				sInteropMetaData.ScriptObjectWrapperPointerField->Get(scriptObject, &scriptObjectWrapper);

			return scriptObjectWrapper;
		}

		// TODO - Doc
		static const ScriptMeta* GetMetaData() { return &sInteropMetaData; }

		// TODO - Doc
		static void InitializeMetaDataAtLoadTime()
		{
			// Need to delay init of sInteropMetaData since it's also a static, and we can't guarantee the order
			// (if it gets initialized after this, it will just overwrite the data)
			ScriptMeta localMetaData = ScriptMeta(SelfType::GetAssemblyName(), SelfType::GetNamespace(), SelfType::GetTypeName(), &SelfType::SetupScriptBindings);

			MonoManager::RegisterScriptType(&sInteropMetaData, localMetaData);
		}

		// TODO - When object is finalized, ScriptObjectManager needs to notify ScriptObjectWrapper

	protected:
		static ScriptMeta sInteropMetaData;
		static InitializeScriptObjectWrapperOnLoadTime<NativeTypeContainerType, SelfType, ScriptWrapperObjectBaseClass> sInitializeOnLoadTime;

		/**
		 * Holds a strong reference to the native object and allows the object to be retrieved as shared pointer or a handle. Redundant raw pointer to this
		 * same object is also kept in IScriptObjectWrapper::mNativeObject.
		 */
		ScriptExportedNativeObjectStorage<NativeTypeContainerType> mNativeObjectStorage;
	};

	template <typename NativeTypeContainerType, typename SelfType, typename ScriptWrapperObjectBaseClass>
	InitializeScriptObjectWrapperOnLoadTime<NativeTypeContainerType, SelfType, ScriptWrapperObjectBaseClass> TScriptObjectWrapper<NativeTypeContainerType, SelfType, ScriptWrapperObjectBaseClass>::sInitializeOnLoadTime;

	template <typename NativeTypeContainerType, typename SelfType, typename ScriptWrapperObjectBaseClass>
	ScriptMeta TScriptObjectWrapper<NativeTypeContainerType, SelfType, ScriptWrapperObjectBaseClass>::sInteropMetaData;

	/**	Specialized version of TScriptObjectWrapper that should be used for types that are never going to be explicitly instantiated (e.g. singletons, static-only classes and base classes). */
	template <typename SelfType, typename ScriptWrapperObjectBaseClass = ScriptObjectWrapper>
	class TNonInstantiableScriptObjectWrapper : public TScriptObjectWrapper<nullptr_t, SelfType, ScriptWrapperObjectBaseClass>
	{
	public:
		TNonInstantiableScriptObjectWrapper(MonoObject* scriptObject)
			:TScriptObjectWrapper<nullptr_t, SelfType, ScriptWrapperObjectBaseClass>(scriptObject)
		{ }
	};


	// TODO - Doc
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
