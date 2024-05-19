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
		SPtr<DataStream> GetCookedData(FPhysXMesh* obj, u32& size)
		{
			size = obj->mCookedDataSize;

			return B3DMakeShared<MemoryDataStream>(obj->mCookedData, obj->mCookedDataSize);
		}

		void SetCookedData(FPhysXMesh* obj, const SPtr<DataStream>& value, u32 size)
		{
			obj->mCookedData = (u8*)B3DAllocate(size);
			obj->mCookedDataSize = size;

			value->Read(obj->mCookedData, size);
		}

	public:
		FPhysXMeshRTTI()
		{
			AddDataBlockField("mCookedData", 0, &FPhysXMeshRTTI::GetCookedData, &FPhysXMeshRTTI::SetCookedData);
		}

		void OnDeserializationEnded(IReflectable* obj, RTTIOperationContext* context)
		{
			FPhysXMesh* mesh = static_cast<FPhysXMesh*>(obj);
			mesh->Initialize();
		}

		const String& GetRttiName()
		{
			static String name = "FPhysXMesh";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_FPhysXMesh;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<FPhysXMesh>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
