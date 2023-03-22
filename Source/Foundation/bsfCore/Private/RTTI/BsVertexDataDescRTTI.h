//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "RenderAPI/BsVertexDataDesc.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT VertexDataDescRTTI : public RTTIType<VertexDataDesc, IReflectable, VertexDataDescRTTI>
	{
	private:
		VertexElement& GetVertexElementData(VertexDataDesc* obj, u32 arrayIdx) { return obj->mVertexElements[arrayIdx]; }

		void SetVertexElementData(VertexDataDesc* obj, u32 arrayIdx, VertexElement& value) { obj->mVertexElements[arrayIdx] = value; }

		u32 GetNumVertexElementData(VertexDataDesc* obj) { return (u32)obj->mVertexElements.size(); }

		void SetNumVertexElementData(VertexDataDesc* obj, u32 numElements) { obj->mVertexElements.resize(numElements); }

	public:
		VertexDataDescRTTI()
		{
			AddPlainArrayField("mVertexData", 0, &VertexDataDescRTTI::GetVertexElementData, &VertexDataDescRTTI::GetNumVertexElementData, &VertexDataDescRTTI::SetVertexElementData, &VertexDataDescRTTI::SetNumVertexElementData);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			VertexDataDesc* vertexDescription = static_cast<VertexDataDesc*>(obj);
			vertexDescription->CalculateOffsets();
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeSharedFromExisting<VertexDataDesc>(new(B3DAllocate<VertexDataDesc>()) VertexDataDesc());
		}

		const String& GetRttiName()
		{
			static String name = "VertexDataDesc";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_VertexDataDesc;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
