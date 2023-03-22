//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Managers/BsHardwareBufferManager.h"
#include "RenderAPI/BsVertexDataDesc.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	B3D_ALLOW_MEMCPY_SERIALIZATION(VertexElement);

	class VertexDeclarationRTTI : public RTTIType<VertexDeclaration, IReflectable, VertexDeclarationRTTI>
	{
	private:
		VertexElement& GetElement(VertexDeclaration* obj, u32 idx)
		{
			Vector<VertexElement>& elemList = obj->mProperties.mElementList;

			auto iter = elemList.begin();
			for(u32 i = 0; i < idx; i++)
				++iter;

			return *iter;
		}

		void SetElement(VertexDeclaration* obj, u32 idx, VertexElement& data)
		{
			Vector<VertexElement>& elemList = obj->mProperties.mElementList;

			auto iter = elemList.begin();
			for(u32 i = 0; i < idx; i++)
				++iter;

			*iter = data;
		}

		u32 GetElementArraySize(VertexDeclaration* obj)
		{
			Vector<VertexElement>& elemList = obj->mProperties.mElementList;

			return (u32)elemList.size();
		}

		void SetElementArraySize(VertexDeclaration* obj, u32 size)
		{
			Vector<VertexElement>& elemList = obj->mProperties.mElementList;

			for(size_t i = elemList.size(); i < size; i++)
				elemList.push_back(VertexElement());
		}

	public:
		VertexDeclarationRTTI()
		{
			AddPlainArrayField("mElementList", 0, &VertexDeclarationRTTI::GetElement, &VertexDeclarationRTTI::GetElementArraySize, &VertexDeclarationRTTI::SetElement, &VertexDeclarationRTTI::SetElementArraySize);
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return HardwareBufferManager::Instance().CreateVertexDeclaration(B3DMakeShared<VertexDataDesc>(SmallVector<VertexElement, 8>()));
		}

		const String& GetRttiName()
		{
			static String name = "VertexDeclaration";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_VertexDeclaration;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
