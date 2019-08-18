//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "RenderAPI/BsVertexBuffer.h"
#include "Managers/BsHardwareBufferManager.h"
#include "RenderAPI/BsVertexDataDesc.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	BS_ALLOW_MEMCPY_SERIALIZATION(VertexElement);
	
	class VertexDeclarationRTTI : public RTTIType<VertexDeclaration, IReflectable, VertexDeclarationRTTI>
	{
	private:
		VertexElement& getElement(VertexDeclaration* obj, UINT32 idx)
		{
			Vector<VertexElement>& elemList = obj->mProperties.mElementList;

			auto iter = elemList.begin();
			for(UINT32 i = 0; i < idx; i++)
				++iter;

			return *iter;
		}

		void setElement(VertexDeclaration* obj, UINT32 idx, VertexElement& data)
		{
			Vector<VertexElement>& elemList = obj->mProperties.mElementList;

			auto iter = elemList.begin();
			for(UINT32 i = 0; i < idx; i++)
				++iter;

			*iter = data;
		}

		UINT32 getElementArraySize(VertexDeclaration* obj)
		{
			Vector<VertexElement>& elemList = obj->mProperties.mElementList;

			return (UINT32)elemList.size();
		}

		void setElementArraySize(VertexDeclaration* obj, UINT32 size)
		{
			Vector<VertexElement>& elemList = obj->mProperties.mElementList;

			for (size_t i = elemList.size(); i < size; i++)
				elemList.push_back(VertexElement());
		}

	public:
		VertexDeclarationRTTI()
		{
			addPlainArrayField("mElementList", 0, &VertexDeclarationRTTI::getElement, &VertexDeclarationRTTI::getElementArraySize,
				&VertexDeclarationRTTI::setElement, &VertexDeclarationRTTI::setElementArraySize);
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return HardwareBufferManager::instance().createVertexDeclaration(VertexDataDesc::create());
		}

		const String& getRTTIName() override
		{
			static String name = "VertexDeclaration";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_VertexDeclaration;
		}
	};

	/** @} */
	/** @endcond */
}
