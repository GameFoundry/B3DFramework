//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsRTTIType.h"
#include "BsPhysXMesh.h"
#include "BsDataStream.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-PhysX
	 *  @{
	 */

	class FPhysXMeshRTTI : public RTTIType<FPhysXMesh, FPhysicsMesh, FPhysXMeshRTTI>
	{
	private:
		SPtr<DataStream> getCookedData(FPhysXMesh* obj, UINT32& size)
		{
			size = obj->mCookedDataSize;

			return bs_shared_ptr_new<MemoryDataStream>(obj->mCookedData, obj->mCookedDataSize, false);
		}

		void setCookedData(FPhysXMesh* obj, const SPtr<DataStream>& value, UINT32 size)
		{
			obj->mCookedData = (UINT8*)bs_alloc(size);
			obj->mCookedDataSize = size;

			value->read(obj->mCookedData, size);
		}

	public:
		FPhysXMeshRTTI()
		{
			addDataBlockField("mCookedData", 0, &FPhysXMeshRTTI::getCookedData, &FPhysXMeshRTTI::setCookedData, 0);
		}

		/** @copydoc IReflectable::onDeserializationEnded */
		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			FPhysXMesh* mesh = static_cast<FPhysXMesh*>(obj);
			mesh->initialize();
		}

		const String& getRTTIName() override
		{
			static String name = "FPhysXMesh";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_FPhysXMesh;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<FPhysXMesh>();
		}
	};

	/** @} */
	/** @endcond */
}