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
		SubMesh& getSubMesh(MeshBase* obj, UINT32 arrayIdx) { return obj->mProperties.mSubMeshes[arrayIdx]; }
		void setSubMesh(MeshBase* obj, UINT32 arrayIdx, SubMesh& value) { obj->mProperties.mSubMeshes[arrayIdx] = value; }
		UINT32 getNumSubmeshes(MeshBase* obj) { return (UINT32)obj->mProperties.mSubMeshes.size(); }
		void setNumSubmeshes(MeshBase* obj, UINT32 numElements) { obj->mProperties.mSubMeshes.resize(numElements); }

		UINT32& getNumVertices(MeshBase* obj) { return obj->mProperties.mNumVertices; }
		void setNumVertices(MeshBase* obj, UINT32& value) { obj->mProperties.mNumVertices = value; }

		UINT32& getNumIndices(MeshBase* obj) { return obj->mProperties.mNumIndices; }
		void setNumIndices(MeshBase* obj, UINT32& value) { obj->mProperties.mNumIndices = value; }

	public:
		MeshBaseRTTI()
		{
			addPlainField("mNumVertices", 0, &MeshBaseRTTI::getNumVertices, &MeshBaseRTTI::setNumVertices);
			addPlainField("mNumIndices", 1, &MeshBaseRTTI::getNumIndices, &MeshBaseRTTI::setNumIndices);

			addPlainArrayField("mSubMeshes", 2, &MeshBaseRTTI::getSubMesh,
				&MeshBaseRTTI::getNumSubmeshes, &MeshBaseRTTI::setSubMesh, &MeshBaseRTTI::setNumSubmeshes);
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}

		const String& getRTTIName() override
		{
			static String name = "MeshBase";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_MeshBase;
		}
	};

	/** @} */
	/** @endcond */
}
