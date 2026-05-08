//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Resources/B3DResource.h"

namespace b3d
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	struct ResourceBackupData;

	/** Resource that internally wraps a managed resource object that can be of user-defined type. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedResource : public Resource
	{
	public:
		ManagedResource();

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

		/** Creates an empty managed resource without calling Initialize(), as a resource handle. */
		static HManagedResource CreateUninitialized();

		/** Creates an empty managed resource without calling Initialize(), as a shared pointer. */
		static TShared<ManagedResource> CreateUninitializedAsShared();

	private:
		friend class ScriptManagedResource;

		void Initialize() override;

		/** Sets up script bindings between native and managed class. Must be called after creating the script object wrapper, or after assembly is reloaded. */
		void SetupScriptBindings(const TShared<ManagedObjectInfo>& objectInformation);

		/**
		 * Creates the script object of the correct type.
		 *
		 * @param	outObjectInformation	Information about the resource type. Can be null in case the type does no longer exist.
		 * @return							Creates script object of the correct resource type, or if type cannot be found, script object of missing type.
		 */
		MonoObject* CreateScriptObject(TShared<ManagedObjectInfo>& outObjectInformation) const;

		bool mMissingType = false;
		TShared<ManagedSerializableObject> mSerializedObjectData;
		TShared<ManagedObjectInfo> mObjectInformation; // Transient

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedResourceRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/**	Contains serialized resource data buffer. */
	struct ResourceBackupData
	{
		u8* Data;
		u32 Size;
	};

	/** @} */
} // namespace b3d
