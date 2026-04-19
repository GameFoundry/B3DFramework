//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RenderAPI/B3DVertexDescription.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	B3D_ALLOW_MEMCPY_SERIALIZATION(VertexElement, TID_VertexElement);

	class B3D_EXPORT VertexDescriptionRTTI : public TRTTIType<VertexDescription, IReflectable, VertexDescriptionRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(mVertexElements, 0)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(VertexDescription& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				// TODO - This can be removed after I re-import the builtin assets (after the render backend refactor is done, and the import tool runs again)
				object.CalculateOffsets();
			}
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeSharedFromExisting<VertexDescription>(new(B3DAllocate<VertexDescription>()) VertexDescription());
		}

		const String& GetRttiName()
		{
			static String name = "VertexDescription";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_VertexDescription;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
