//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Mesh/BsMeshBase.h"
#include "Error/BsException.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	BS_ALLOW_MEMCPY_SERIALIZATION(SubMesh);

	class MeshBaseRTTI : public RTTIType<MeshBase, Resource, MeshBaseRTTI>
	{
		SubMesh& GetSubMesh(MeshBase* obj, UINT32 arrayIdx) { return obj->mProperties.mSubMeshes[arrayIdx]; }
		void SetSubMesh(MeshBase* obj, UINT32 arrayIdx, SubMesh& value) { obj->mProperties.mSubMeshes[arrayIdx] = value; }
		UINT32 GetNumSubmeshes(MeshBase* obj) { return (UINT32)obj->mProperties.mSubMeshes.Size(); }
		void SetNumSubmeshes(MeshBase* obj, UINT32 numElements) { obj->mProperties.mSubMeshes.Resize(numElements); }

		UINT32& GetNumVertices(MeshBase* obj) { return obj->mProperties.mNumVertices; }
		void SetNumVertices(MeshBase* obj, UINT32& value) { obj->mProperties.mNumVertices = value; }

		UINT32& GetNumIndices(MeshBase* obj) { return obj->mProperties.mNumIndices; }
		void SetNumIndices(MeshBase* obj, UINT32& value) { obj->mProperties.mNumIndices = value; }

	public:
		MeshBaseRTTI()
		{
			addPlainField("mNumVertices", 0, &MeshBaseRTTI::getNumVertices, &MeshBaseRTTI::setNumVertices);
			addPlainField("mNumIndices", 1, &MeshBaseRTTI::getNumIndices, &MeshBaseRTTI::setNumIndices);

			addPlainArrayField("mSubMeshes", 2, &MeshBaseRTTI::getSubMesh,
				&MeshBaseRTTI::getNumSubmeshes, &MeshBaseRTTI::setSubMesh, &MeshBaseRTTI::setNumSubmeshes);
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}

		const String& GetRTTIName() override
		{
			static String name = "MeshBase";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_MeshBase;
		}
	};

	/** @} */
	/** @endcond */
}
