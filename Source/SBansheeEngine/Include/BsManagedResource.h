//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsResource.h"

namespace bs
{
	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	struct ResourceBackupData;

	/** Resource that internally wraps a managed resource object that can be of user-defined type. */
	class BS_SCR_BE_EXPORT ManagedResource : public Resource
	{
	public:
		/**	Returns the internal managed resource object. */
		MonoObject* getManagedInstance() const { return mManagedInstance; }

		/**
		 * Serializes the internal managed resource.
		 *
		 * @param[in]	clearExisting	Should the managed resource handle be released. (Will trigger a finalizer if this is
		 *								the last reference to it)
		 * @return						An object containing the serialized resource. You can provide this to restore()
		 *								method to re-create the original resource.
		 */
		ResourceBackupData backup(bool clearExisting = true);

		/**
		 * Restores a resource from previously serialized data.
		 *
		 * @param[in]	instance	New instance of the managed resource. Must be of the valid resource type this object was
		 *							originally created from. Can be null if the type cannot be found (can happen after an
		 *							assembly refresh).
		 * @param[in]	data		Serialized managed resource data that will be used for initializing the new managed
		 *							instance.
		 */
		void restore(MonoObject* instance, const ResourceBackupData& data);

		/**
		 * Creates a new managed resource wrapper from an actual managed resource object. Caller must ensure the provided
		 * instance actually derives from Resource class.
		 */
		static HManagedResource create(MonoObject* managedResource);

		/**
		 * Creates an empty managed resource wrapper pointing to no managed instance. You must call setHandle() before use
		 * manually.
		 */
		static SPtr<ManagedResource> createEmpty();

	private:
		friend class ScriptManagedResource;

		ManagedResource(MonoObject* managedInstance);

		/**
		 * Finalizes construction of the object. Must be called before use or when the managed resource instance changes.
		 *
		 * @param[in]	object		Managed resource instance.
		 * @param[in]	myHandle	Handle to myself.
		 */
		void setHandle(MonoObject* object, const HManagedResource& myHandle);

		/** @copydoc Resource::destroy */
		void destroy() override;

		MonoObject* mManagedInstance;
		uint32_t mManagedHandle;
		WeakResourceHandle<ManagedResource> mMyHandle;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedResourceRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;

	protected:
		ManagedResource(); // Serialization only
	};

	/**	Contains serialized resource data buffer. */
	struct ResourceBackupData
	{
		UINT8* data;
		UINT32 size;
	};

	/** @} */
}