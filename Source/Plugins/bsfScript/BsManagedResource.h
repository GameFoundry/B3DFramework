//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Resources/BsResource.h"

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
		MonoObject* getManagedInstance() const;

		/**
		 * Serializes the internal managed resource.
		 *
		 * @return						An object containing the serialized resource. You can provide this to restore()
		 *								method to re-create the original resource.
		 */
		ResourceBackupData backup();

		/**
		 * Restores a resource from previously serialized data.
		 *
		 * @param[in]	data		Serialized managed resource data that will be used for initializing the new managed
		 *							instance.
		 */
		void restore(const ResourceBackupData& data);

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

		WeakResourceHandle<ManagedResource> mMyHandle;
		ScriptManagedResource* mOwner = nullptr;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedResourceRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

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