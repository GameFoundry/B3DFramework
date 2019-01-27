//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Utility/BsModule.h"
#include "Wrappers/BsScriptResource.h"

namespace bs
{
	class ScriptRRefBase;

	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	/**
	 * Handles creation and lookup of script interop objects for resources. Since resources can be created in native code
	 * yet used by managed code this manager provides lookups to find managed equivalents of native resources.
	 */
	class BS_SCR_BE_EXPORT ScriptResourceManager : public Module<ScriptResourceManager>
	{
	public:
		ScriptResourceManager();
		~ScriptResourceManager();

		/**
		 * Creates a new interop object for the specified builtin resource.
		 *
		 * @param[in]	resource			Native resource to link to the managed instance.
		 * @param[in]	existingInstance	Existing managed instance. Caller must ensure the managed instance matches the
		 *									native resource type. If not provided new object instance will be created
		 *									internally.
		 * @return							Interop object corresponding to the managed instance.
		 *
		 * @note	Throws an exception if a managed instance for the provided resource already exists.
		 */
		ScriptResourceBase* createBuiltinScriptResource(const HResource& resource, MonoObject* existingInstance = nullptr);

		/**
		 * Creates a new interop object for the specified custom managed resource.
		 *
		 * @param[in]	resource			Native resource to link to the managed instance.
		 * @param[in]	existingInstance	Existing managed instance of the resource.
		 * @return							Interop object corresponding to the managed instance.
		 *
		 * @note	Throws an exception if a managed instance for the provided resource already exists.
		 */
		ScriptManagedResource* createManagedScriptResource(const HManagedResource& resource, MonoObject* existingInstance);

		/**
		 * Attempts to find an existing interop object for the specified resource, and optionally creates a new one if one
		 * cannot be found.
		 * 
		 * @param[in]	resource		Resource to search for.
		 * @param[in]	create			If a resource cannot be found new one will be created when this is true. If false
		 *								and the resource doesn't exist it will be null.
		 * @return						Found or created interop object containing the resource.
		 */
		ScriptResourceBase* getScriptResource(const HResource& resource, bool create = false);

		/**
		 * Attempts to find a resource interop object for a resource with the specified UUID. Returns null if the object
		 * cannot be found.
		 */
		ScriptResourceBase* getScriptResource(const UUID& uuid);

		/**
		 * Attempts to find an existing interop object for the specified resource reference, or creates a new object if one
		 * cannot be found.
		 * 
		 * @param[in]	resource		Resource handle to create the reference wrapper object for.
		 * @param[in]	rrefClass		Class of the managed RRef object to create.
		 */
		ScriptRRefBase* getScriptRRef(const HResource& resource, ::MonoClass* rrefClass);

		/** 
		 * Same as getScriptRRef(const HResource&, MonoClass*) except it automatically deduced the resource class from
		 * the provided template parameter.
		 */
		template<class T>
		ScriptRRefBase* getScriptRRef(const ResourceHandle<T>& resource)
		{
			::MonoClass* rrefClass = ScriptResourceBase::getRRefClass(T::getRTTIStatic()->getRTTIId());
			return getScriptRRef(resource, rrefClass);
		}

		/**
		 * Deletes the provided resource interop objects. All resource interop objects should be deleted using this method.
		 */
		void destroyScriptResource(ScriptResourceBase* resource);

		/**	Throws an exception if the provided UUID already exists in the interop object lookup table. */
		void _throwExceptionIfInvalidOrDuplicate(const UUID& uuid) const;

	private:
		/**	Triggered when the native resource has been unloaded and therefore destroyed. */
		void onResourceDestroyed(const UUID& UUID);

		/** 
		 * Clears all cached RRefs. Should be called before assembly refresh since the refs will no longer be valid
		 * after.
		 */
		void clearRRefs();

		UnorderedMap<UUID, ScriptResourceBase*> mScriptResources;
		UnorderedMap<::MonoClass*, UnorderedMap<UUID, ScriptRRefBase*>> mScriptRRefsPerType;

		HEvent mResourceDestroyedConn;
		HEvent mDomainUnloadedConn;
	};

	/** @} */
}
