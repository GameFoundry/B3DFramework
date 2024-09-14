//*********************************** bs::framework - Copyright 2024 Marko Pintera ***************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptTypeMetaData.h"
#include "BsMonoManager.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "Script/BsIScriptObjectWrapper.h"
#include "BsScriptTypeDefinition.h"

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

	/** Determines how is script object lifetime tracked, and when should the native object be destroyed. */
	enum class ScriptObjectLifetimeTrackingMode
	{
		/**
		 * Script object wrapper will hold a strong handle onto the script object. Garbage collection will
		 * trigger at certain time intervals and check if the native object is referenced only by the script
		 * object wrapper. If so, the strong handle will transition to a weak handle and the native object
		 * will be destroyed when the script object gets deleted.
		 *
		 * This should be used by objects that can be referenced by both native and script code, and are
		 * released when their reference count drops to 0.
		 */
		StrongHandleWithGarbageCollection,

		/**
		 * Script object wrapper will hold a strong handle onto the script object. Strong handle will be freed
		 * when the native object is destroyed.
		 *
		 * This should be used for objects that can be referenced by both native and script code, and have an
		 * explicit Destroy() method.
		 */
		StrongHandleWithExplicitDestroy,

		/**
		 * Script object wrapper will hold a weak handle onto the script object. Native object will be freed
		 * when the script object gets deleted.
		 *
		 * This should be used for objects that are referenced from script code only (e.g. objects passed into script code by value).
		 */
		WeakHandle
	};

	/**
	 * Extends IScriptObjectWrapper by keeping a strong reference to the script object, and releasing it as needed.
	 * As well as providing an interface for script reload functionality.. See IScriptObjectWrapper.
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptObjectWrapper : public IScriptObjectWrapper
	{
	public:
		ScriptObjectWrapper(IScriptExportable* nativeObject);
		virtual ~ScriptObjectWrapper();

		MonoObject* GetScriptObject() const;

		/** Returns the number of strong references on the underlying native object. */
		virtual u32 GetNativeObjectReferenceCount() const
		{
			// Default to 1, as with no reference tracking we assume the native resource is destroyed explicitly (i.e. Destroy() method), and will notify the wrapper when that happens.
			return 1;
		}

		/** Used by derived classes to connect callbacks to native object events. */
		virtual void RegisterEvents() { }

		void NotifyNativeObjectDestroyed() override;

		/**
		 * @name Script object lifetime tracking
		 * @{
		 */

		/** Determines how is script object lifetime tracked, and when should the native object be destroyed. See ScriptObjectLifetimeTrackingMode. */
		virtual ScriptObjectLifetimeTrackingMode GetLifetimeTrackingMode() const { return ScriptObjectLifetimeTrackingMode::StrongHandleWithGarbageCollection; }

		/**
		 * Changes the internal script object handle from strong to weak. If the handle is already weak does nothing. Only relevant
		 * if lifetime tracking mode is using garbage collection.
		 */
		virtual void TransitionToWeakHandle();

		/**
		 * Changes the internal script object handle from weak to strong. If the handle is already strong does nothing. Only relevant
		 * if lifetime tracking mode is using garbage collection.
		 */
		virtual void TransitionToStrongHandle();

		/** Called when the script system is notified that the script object has been destroyed. */
		virtual void NotifyScriptObjectDestroyed(bool isDestroyedDueToScriptReload);

		/** @} */

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
		virtual void ReleaseStrongHandlesBeforeScriptReload() { ReleaseScriptObjectHandle(); }

		/**
		 * Called on all script object wrappers after script assemblies have been reloaded. This needs to recreate the internal script object using the new assemblies,
		 * as the old one will have been destroyed during the reload. Only relevant for script objects that persist script reload (i.e. ShouldPersistScriptReload() returns true).
		 */
		virtual void RecreateScriptObjectAfterScriptReload() { }

		/**
		 * Called on all script object wrappers after script objects have been created in RecreateScriptObjectAfterScriptReload(). Allows you to restore data backed up
		 * in BackupDataBeforeScriptReload() call to the newly created script object. Only relevant for script objects that persist script reload (i.e. ShouldPersistScriptReload() returns true).
		 */
		virtual void RestoreDataAfterScriptReload(const ScriptObjectReloadPersistentData& data) { }

		/**
		 * Called on all script object wrappers as the final step in script reload, after RestoreDataAfterScriptReload(). Allows you to perform actions that require
		 * the entire scripting world to be fully recreated.
		 */
		virtual void NotifyScriptReloadFinished() { }

		/** @} */

		/**
		 * Creates a script wrapper object of the specified type and initializes it. All wrappers should be created using this method rather
		 * than manually creating them.
		 */
		template<typename ScriptWrapperType, typename NativeType>
		static ScriptWrapperType* Create(NativeType&& nativeObject, MonoObject* scriptObject)
		{
			ScriptWrapperType* const scriptWrapper = B3DNew<ScriptWrapperType>(std::forward<NativeType>(nativeObject));
			scriptWrapper->BindToScriptObject(scriptObject);

			return scriptWrapper;
		}

	protected:
		/** Creates a new handle to the provided script object. Previous handle must be released. */
		void CreateScriptObjectHandle(MonoObject* scriptObject);

		/** Releases the currently held script object strong handle, if any. */ 
		void ReleaseScriptObjectHandle();

		u32 mScriptObjectHandle = ~0u;
		bool mRequiresStrongHandle = false;
	};

	template <typename SelfType, typename BaseType>
	class TScriptObjectWrapper;

	/** Ensures that ScriptObjectWrapper types are initialized on application load. */
	template <typename SelfType, typename BaseType>
	struct InitializeScriptObjectWrapperOnLoadTime
	{
	public:
		InitializeScriptObjectWrapperOnLoadTime()
		{
			TScriptObjectWrapper<SelfType, BaseType>::InitializeMetaDataAtLoadTime();
		}

		void MakeSureIAmInstantiated() {}
	};

	/**
	 * Provides common functionality required by specializations of ScriptObjectWrapper, including a meta-data object to store information about the type, ability to
	 * bind the script object wrapper to the script object, and retrieve the wrapper from the script object
	 *
	 * @tparam SelfType		Type that is deriving from TScriptObjectWrapper.
	 * @tparam BaseType		Type that TScriptObjectWrapper should inherit from. This type must be ScriptObjectWrapper, or a type deriving from it.
	 */
	template <typename SelfType, typename BaseType = ScriptObjectWrapper>
	class TScriptObjectWrapper : public BaseType, public TScriptTypeDefinition<SelfType>
	{
	public:
		TScriptObjectWrapper(IScriptExportable* nativeObject)
			: BaseType(nativeObject), TScriptTypeDefinition<SelfType>()
		{ }

		virtual ~TScriptObjectWrapper() = default;

		void RecreateScriptObjectAfterScriptReload() override
		{
			MonoObject* const scriptObject = SelfType::CreateScriptObject(true);

			if(B3D_ENSURE(scriptObject != nullptr))
				BindToScriptObject(scriptObject);
		}

		/** Returns the script object wrapper associated with the provided script object. */
		static SelfType* GetScriptObjectWrapper(MonoObject* scriptObject)
		{
			SelfType* scriptObjectWrapper = nullptr;

			if(sInteropMetaData.ScriptObjectWrapperPointerField != nullptr && scriptObject != nullptr)
				sInteropMetaData.ScriptObjectWrapperPointerField->Get(scriptObject, &scriptObjectWrapper);

			return scriptObjectWrapper;
		}

	protected:
		friend class ScriptObjectWrapper;

		/** Stores a pointer to itself in the script object. This ensures that calls to GetScriptObjectWrapper() can return the script object wrapper associated with the script object. */
		void BindToScriptObject(MonoObject* scriptObject)
		{
			SelfType* self = (SelfType*)(BaseType*)this; // Needed due to multiple inheritance. Safe since SelfType must point to an class derived from this one.

			self->CreateScriptObjectHandle(scriptObject);

			if(sInteropMetaData.ScriptObjectWrapperPointerField != nullptr)
				sInteropMetaData.ScriptObjectWrapperPointerField->Set(scriptObject, &self);

			if(sInteropMetaData.IsUsingNewScriptObjectManagerField != nullptr)
			{
				i32 value = 1;
				sInteropMetaData.IsUsingNewScriptObjectManagerField->Set(scriptObject, &value);
			}
		}
	};

	/**	Specialized version of TScriptObjectWrapper that should be used for types that are never going to be explicitly instantiated (e.g. singletons, static-only classes and base classes). */
	template <typename SelfType>
	class TNonInstantiableScriptObjectWrapper : public TScriptObjectWrapper<SelfType>
	{
	public:
		TNonInstantiableScriptObjectWrapper()
			:TScriptObjectWrapper<SelfType>(nullptr)
		{ }

		/** Dummy method to create the script object. Not needed for non-instantiable types. */
		static MonoObject* CreateScriptObject(bool construct)
		{
			return nullptr;
		}
	};

	/** Implements default methods required by script object wrapper implementations. */
#define B3D_SCRIPT_TYPE_DEFINITION(Assembly, Namespace, Name) \
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

	/**	Script object wrapper for ScriptObject. (Script prefix used as standard for script object wrappers, wrapping ScriptObject. Therefore ScriptScriptObject.) */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptScriptObject : public TNonInstantiableScriptObjectWrapper<ScriptScriptObject>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ScriptObject")

		static void SetupScriptBindings();
	private:
		ScriptScriptObject();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void Internal_ScriptObjectFinalizerCalled(ScriptObjectWrapper* scriptObjectWrapper);
	};

	/** @} */
} // namespace bs
