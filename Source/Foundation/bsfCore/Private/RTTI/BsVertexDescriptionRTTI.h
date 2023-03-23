//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "RenderAPI/BsVertexDescription.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	B3D_ALLOW_MEMCPY_SERIALIZATION(VertexElement);

	class B3D_CORE_EXPORT VertexDescriptionRTTI : public RTTIType<VertexDescription, IReflectable, VertexDescriptionRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_ARRAY(mVertexElements, 0)
		B3D_RTTI_END_MEMBERS

	public:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			// TODO - This can be removed after I re-import the builtin assets (after the render backend refactor is done, and the import tool runs again)
			VertexDescription* vertexDescription = static_cast<VertexDescription*>(obj);
			vertexDescription->CalculateOffsets();
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

		u32 GetRttiId()
		{
			return TID_VertexDescription;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
