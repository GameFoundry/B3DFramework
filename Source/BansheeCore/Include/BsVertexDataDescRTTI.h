//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsVertexDataDesc.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT VertexDataDescRTTI : public RTTIType<VertexDataDesc, IReflectable, VertexDataDescRTTI>
	{
	private:
		VertexElement& getVertexElementData(VertexDataDesc* obj, UINT32 arrayIdx) { return obj->mVertexElements[arrayIdx]; }
		void setVertexElementData(VertexDataDesc* obj, UINT32 arrayIdx, VertexElement& value) { obj->mVertexElements[arrayIdx] = value; }

		UINT32 getNumVertexElementData(VertexDataDesc* obj) { return (UINT32)obj->mVertexElements.size(); }
		void setNumVertexElementData(VertexDataDesc* obj, UINT32 numElements) { obj->mVertexElements.resize(numElements); }

	public:
		VertexDataDescRTTI()
		{
			addPlainArrayField("mVertexData", 0, &VertexDataDescRTTI::getVertexElementData, 
				&VertexDataDescRTTI::getNumVertexElementData, &VertexDataDescRTTI::setVertexElementData, &VertexDataDescRTTI::setNumVertexElementData);
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr<VertexDataDesc>(new (bs_alloc<VertexDataDesc>()) VertexDataDesc());
		}

		const String& getRTTIName() override
		{
			static String name = "VertexDataDesc";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_VertexDataDesc;
		}
	};

	/** @} */
	/** @endcond */
}