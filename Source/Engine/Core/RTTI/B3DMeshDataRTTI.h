//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Mesh/B3DMeshData.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "FileSystem/B3DDataStream.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	B3D_ALLOW_MEMCPY_SERIALIZATION(IndexType, TID_IndexType);

	class B3D_EXPORT MeshDataRTTI : public TRTTIType<MeshData, GpuResourceData, MeshDataRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mVertexDescription, 0)
			B3D_RTTI_MEMBER(mIndexType, 1)
			B3D_RTTI_MEMBER(mVertexCount, 2)
			B3D_RTTI_MEMBER(mIndexCount, 3)
		B3D_RTTI_END_MEMBERS

		TShared<DataStream> GetData(MeshData* obj, u32& size)
		{
			size = obj->GetInternalBufferSize();

			return B3DMakeShared<MemoryDataStream>(obj->GetData(), size);
		}

		void SetData(MeshData* obj, const TShared<DataStream>& value, u32 size)
		{
			obj->AllocateInternalBuffer(size);
			value->Read(obj->GetData(), size);
		}

	public:
		MeshDataRTTI()
		{
			AddDataBlockField("data", 4, &MeshDataRTTI::GetData, &MeshDataRTTI::SetData);
		}

		TShared<IReflectable> NewRttiObject()
		{
			return B3DMakeSharedFromExisting<MeshData>(new(B3DAllocate<MeshData>()) MeshData());
		}

		const String& GetRttiName()
		{
			static String name = "MeshData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_MeshData;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
