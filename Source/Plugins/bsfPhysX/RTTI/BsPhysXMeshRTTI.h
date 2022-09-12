//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "BsPhysXMesh.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-PhysX
	 *  @{
	 */

	class FPhysXMeshRTTI : public RTTIType<FPhysXMesh, FPhysicsMesh, FPhysXMeshRTTI>
	{
	private:
		SPtr<DataStream> GetCookedData(FPhysXMesh* obj, UINT32& size)
		{
			size = obj->mCookedDataSize;

			return bs_shared_ptr_new<MemoryDataStream>(obj->mCookedData, obj->mCookedDataSize);
		}

		void SetCookedData(FPhysXMesh* obj, const SPtr<DataStream>& value, UINT32 size)
		{
			obj->mCookedData = (UINT8*)bs_alloc(size);
			obj->mCookedDataSize = size;

			value->Read(obj->mCookedData, size);
		}

	public:
		FPhysXMeshRTTI()
		{
			addDataBlockField("mCookedData", 0, &FPhysXMeshRTTI::getCookedData, &FPhysXMeshRTTI::setCookedData);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			FPhysXMesh* mesh = static_cast<FPhysXMesh*>(obj);
			mesh->Initialize();
		}

		const String& GetRTTIName() override
		{
			static String name = "FPhysXMesh";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_FPhysXMesh;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return bs_shared_ptr_new<FPhysXMesh>();
		}
	};

	/** @} */
	/** @endcond */
}
