//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Mesh/BsMeshBase.h"
#include "Error/BsException.h"
#include "Private/RTTI/BsSubMeshRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */


	class MeshBaseRTTI : public TRTTIType<MeshBase, Resource, MeshBaseRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(VertexCount, mProperties.VertexCount, 0)
			B3D_RTTI_MEMBER_NAMED(IndexCount, mProperties.IndexCount, 1)
			B3D_RTTI_MEMBER_CONTAINER_NAMED(SubMeshes, mProperties.SubMeshes, 2)
		B3D_RTTI_END_MEMBERS

	public:

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

		u32 GetRttiId() const override
		{
			return TID_MeshBase;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
