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
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(VertexCount, mProperties.VertexCount, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(IndexCount, mProperties.IndexCount, 1)
			B3D_RTTI_MEMBER_PLAIN_ARRAY_NAMED(SubMeshes, mProperties.SubMeshes, 2)
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

		u32 GetRttiId()
		{
			return TID_MeshBase;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
