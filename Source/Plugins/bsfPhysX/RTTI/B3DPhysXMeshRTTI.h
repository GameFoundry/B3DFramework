//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "BsPhysXMesh.h"
#include "FileSystem/BsDataStream.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-PhysX
	 *  @{
	 */

	class PhysXMeshRTTI : public TRTTIType<PhysXMesh, IPhysicsMeshImplementation, PhysXMeshRTTI>
	{
	private:
		SPtr<DataStream> GetCookedData(PhysXMesh* obj, u32& size)
		{
			size = obj->mCookedDataSize;

			return B3DMakeShared<MemoryDataStream>(obj->mCookedData, obj->mCookedDataSize);
		}

		void SetCookedData(PhysXMesh* obj, const SPtr<DataStream>& value, u32 size)
		{
			obj->mCookedData = (u8*)B3DAllocate(size);
			obj->mCookedDataSize = size;

			value->Read(obj->mCookedData, size);
		}

	public:
		PhysXMeshRTTI()
		{
			AddDataBlockField("mCookedData", 0, &PhysXMeshRTTI::GetCookedData, &PhysXMeshRTTI::SetCookedData);
		}

		void OnOperationEnded(PhysXMesh& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "PhysXMesh";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_PhysXMesh;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<PhysXMesh>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
