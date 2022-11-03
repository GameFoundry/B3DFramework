//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Mesh/BsMeshData.h"
#include "RenderAPI/BsVertexDeclaration.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	B3D_ALLOW_MEMCPY_SERIALIZATION(IndexType);

	class B3D_CORE_EXPORT MeshDataRTTI : public RTTIType<MeshData, GpuResourceData, MeshDataRTTI>
	{
	private:
		SPtr<VertexDataDesc> GetVertexData(MeshData* obj) { return obj->mVertexData; }

		void SetVertexData(MeshData* obj, SPtr<VertexDataDesc> value) { obj->mVertexData = value; }

		IndexType& GetIndexType(MeshData* obj) { return obj->mIndexType; }

		void SetIndexType(MeshData* obj, IndexType& value) { obj->mIndexType = value; }

		u32& GetNumVertices(MeshData* obj) { return obj->mNumVertices; }

		void SetNumVertices(MeshData* obj, u32& value) { obj->mNumVertices = value; }

		u32& GetNumIndices(MeshData* obj) { return obj->mNumIndices; }

		void SetNumIndices(MeshData* obj, u32& value) { obj->mNumIndices = value; }

		SPtr<DataStream> GetData(MeshData* obj, u32& size)
		{
			size = obj->GetInternalBufferSize();

			return B3DMakeShared<MemoryDataStream>(obj->GetData(), size);
		}

		void SetData(MeshData* obj, const SPtr<DataStream>& value, u32 size)
		{
			obj->AllocateInternalBuffer(size);
			value->Read(obj->GetData(), size);
		}

	public:
		MeshDataRTTI()
		{
			AddReflectablePtrField("mVertexData", 0, &MeshDataRTTI::GetVertexData, &MeshDataRTTI::SetVertexData);

			AddPlainField("mIndexType", 1, &MeshDataRTTI::GetIndexType, &MeshDataRTTI::SetIndexType);
			AddPlainField("mNumVertices", 2, &MeshDataRTTI::GetNumVertices, &MeshDataRTTI::SetNumVertices);
			AddPlainField("mNumIndices", 3, &MeshDataRTTI::GetNumIndices, &MeshDataRTTI::SetNumIndices);

			AddDataBlockField("data", 4, &MeshDataRTTI::GetData, &MeshDataRTTI::SetData);
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeSharedFromExisting<MeshData>(new(B3DAllocate<MeshData>()) MeshData());
		}

		const String& GetRttiName()
		{
			static String name = "MeshData";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_MeshData;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
