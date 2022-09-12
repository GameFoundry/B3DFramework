//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "BsCoreApplication.h"
#include "Mesh/BsMesh.h"
#include "Animation/BsSkeleton.h"
#include "Animation/BsMorphShapes.h"
#include "CoreThread/BsCoreThread.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class MeshRTTI : public RTTIType<Mesh, MeshBase, MeshRTTI>
	{
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR(mVertexDesc, 0)
			BS_RTTI_MEMBER_PLAIN(mIndexType, 1)
			BS_RTTI_MEMBER_PLAIN(mUsage, 2)
			BS_RTTI_MEMBER_REFLPTR(mSkeleton, 4)
			BS_RTTI_MEMBER_REFLPTR(mMorphShapes, 5)
		BS_END_RTTI_MEMBERS

		SPtr<MeshData> GetMeshData(Mesh* obj)
		{
			SPtr<MeshData> meshData = obj->AllocBuffer();

			obj->ReadData(meshData);
			gCoreThread().SubmitAll(true);

			return meshData;
		}

		void SetMeshData(Mesh* obj, SPtr<MeshData> meshData)
		{
			obj->mCPUData = meshData;
		}

	public:
		MeshRTTI()
		{
			addReflectablePtrField("mMeshData", 3, &MeshRTTI::getMeshData, &MeshRTTI::setMeshData);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Mesh* mesh = static_cast<Mesh*>(obj);
			mesh->Initialize();
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return Mesh::CreateEmpty();
		}

		const String& GetRTTIName() override
		{
			static String name = "Mesh";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_Mesh;
		}
	};

	/** @} */
	/** @endcond */
}
