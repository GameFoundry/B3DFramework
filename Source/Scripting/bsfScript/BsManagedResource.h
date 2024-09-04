//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Resources/BsResource.h"

namespace bs
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	struct ResourceBackupData;

	/** Resource that internally wraps a managed resource object that can be of user-defined type. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedResource : public Resource
	{
	public:
		/**	Returns the internal managed resource object. */
		MonoObject* GetManagedInstance() const;

		/**
		 * Serializes the internal managed resource.
		 *
		 * @return						An object containing the serialized resource. You can provide this to restore()
		 *								method to re-create the original resource.
		 */
		ResourceBackupData Backup();

		/**
		 * Restores a resource from previously serialized data.
		 *
		 * @param[in]	data		Serialized managed resource data that will be used for initializing the new managed
		 *							instance.
		 */
		void Restore(const ResourceBackupData& data);

		/**
		 * Creates a new managed resource wrapper from an actual managed resource object. Caller must ensure the provided
		 * instance actually derives from Resource class.
		 */
		static HManagedResource Create(MonoObject* managedResource);

		/**
		 * Creates an empty managed resource wrapper pointing to no managed instance. You must call setHandle() before use
		 * manually.
		 */
		static SPtr<ManagedResource> CreateEmpty();

	private:
		friend class ScriptManagedResource;

		ManagedResource(MonoObject* managedInstance);

		/**
		 * Binds the managed resource to the currently assigned script object. This involves setting up bindings and resolving
		 * the exact managed resource class.
		 */
		void BindToScriptObject(const SPtr<ManagedSerializableObjectInfo>& objectInformation);

		/**
		 * Creates the script object of the correct type.
		 *
		 * @param	outObjectInfo		Information about the resource type. Can be null in case the type does no longer exist.
		 * @return						Creates script object of the correct resource type, or if type cannot be found, script object of missing type.
		 */
		MonoObject* CreateScriptObject(SPtr<ManagedSerializableObjectInfo>& outObjectInfo) const;

		/**
		 * Finalizes construction of the object. Must be called before use or when the managed resource instance changes.
		 *
		 * @param[in]	object		Managed resource instance.
		 * @param[in]	myHandle	Handle to myself.
		 */
		void SetHandle(MonoObject* object, const HManagedResource& myHandle);

		void Destroy() override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedResourceRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		ManagedResource(); // Serialization only
	};

	/**	Contains serialized resource data buffer. */
	struct ResourceBackupData
	{
		u8* Data;
		u32 Size;
	};

	/** @} */
} // namespace bs
