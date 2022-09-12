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

	BS_ALLOW_MEMCPY_SERIALIZATION(IndexType);

	class BS_CORE_EXPORT MeshDataRTTI : public RTTIType<MeshData, GpuResourceData, MeshDataRTTI>
	{
	private:
		SPtr<VertexDataDesc> GetVertexData(MeshData* obj) { return obj->mVertexData; }
		void SetVertexData(MeshData* obj, SPtr<VertexDataDesc> value) { obj->mVertexData = value; }

		IndexType& GetIndexType(MeshData* obj) { return obj->mIndexType; }
		void SetIndexType(MeshData* obj, IndexType& value) { obj->mIndexType = value; }

		UINT32& GetNumVertices(MeshData* obj) { return obj->mNumVertices; }
		void SetNumVertices(MeshData* obj, UINT32& value) { obj->mNumVertices = value; }

		UINT32& GetNumIndices(MeshData* obj) { return obj->mNumIndices; }
		void SetNumIndices(MeshData* obj, UINT32& value) { obj->mNumIndices = value; }

		SPtr<DataStream> GetData(MeshData* obj, UINT32& size)
		{
			size = obj->GetInternalBufferSize();

			return bs_shared_ptr_new<MemoryDataStream>(obj->GetData(), size);
		}

		void SetData(MeshData* obj, const SPtr<DataStream>& value, UINT32 size)
		{
			obj->AllocateInternalBuffer(size);
			value->Read(obj->getData(), size);
		}

	public:
		MeshDataRTTI()
		{
			addReflectablePtrField("mVertexData", 0, &MeshDataRTTI::getVertexData, &MeshDataRTTI::setVertexData);

			addPlainField("mIndexType", 1, &MeshDataRTTI::getIndexType, &MeshDataRTTI::setIndexType);
			addPlainField("mNumVertices", 2, &MeshDataRTTI::getNumVertices, &MeshDataRTTI::setNumVertices);
			addPlainField("mNumIndices", 3, &MeshDataRTTI::getNumIndices, &MeshDataRTTI::setNumIndices);

			addDataBlockField("data", 4, &MeshDataRTTI::getData, &MeshDataRTTI::setData);
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return bs_shared_ptr<MeshData>(new (bs_alloc<MeshData>()) MeshData());
		}

		const String& GetRTTIName() override
		{
			static String name = "MeshData";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_MeshData;
		}
	};

	/** @} */
	/** @endcond */
}
