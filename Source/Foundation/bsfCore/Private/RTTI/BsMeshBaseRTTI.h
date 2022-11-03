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

	B3D_ALLOW_MEMCPY_SERIALIZATION(SubMesh);

	class MeshBaseRTTI : public RTTIType<MeshBase, Resource, MeshBaseRTTI>
	{
		SubMesh& GetSubMesh(MeshBase* obj, u32 arrayIdx) { return obj->mProperties.mSubMeshes[arrayIdx]; }

		void SetSubMesh(MeshBase* obj, u32 arrayIdx, SubMesh& value) { obj->mProperties.mSubMeshes[arrayIdx] = value; }

		u32 GetNumSubmeshes(MeshBase* obj) { return (u32)obj->mProperties.mSubMeshes.size(); }

		void SetNumSubmeshes(MeshBase* obj, u32 numElements) { obj->mProperties.mSubMeshes.resize(numElements); }

		u32& GetNumVertices(MeshBase* obj) { return obj->mProperties.mNumVertices; }

		void SetNumVertices(MeshBase* obj, u32& value) { obj->mProperties.mNumVertices = value; }

		u32& GetNumIndices(MeshBase* obj) { return obj->mProperties.mNumIndices; }

		void SetNumIndices(MeshBase* obj, u32& value) { obj->mProperties.mNumIndices = value; }

	public:
		MeshBaseRTTI()
		{
			AddPlainField("mNumVertices", 0, &MeshBaseRTTI::GetNumVertices, &MeshBaseRTTI::SetNumVertices);
			AddPlainField("mNumIndices", 1, &MeshBaseRTTI::GetNumIndices, &MeshBaseRTTI::SetNumIndices);

			AddPlainArrayField("mSubMeshes", 2, &MeshBaseRTTI::GetSubMesh, &MeshBaseRTTI::GetNumSubmeshes, &MeshBaseRTTI::SetSubMesh, &MeshBaseRTTI::SetNumSubmeshes);
		}

		SPtr<IReflectable> NewRttiObject()
		{
			B3D_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}

		const String& GetRttiName()
		{
			static String name = "MeshBase";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_MeshBase;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
