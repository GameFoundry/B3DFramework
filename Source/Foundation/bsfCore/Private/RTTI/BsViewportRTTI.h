//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsFlagsRTTI.h"
#include "RTTI/BsColorRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "RenderAPI/BsViewport.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT ViewportRTTI : public RTTIType<Viewport, IReflectable, ViewportRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mNormArea, 0)
			BS_RTTI_MEMBER_PLAIN(mClearColorValue, 1)
			BS_RTTI_MEMBER_PLAIN(mClearDepthValue, 2)
			BS_RTTI_MEMBER_PLAIN(mClearStencilValue, 3)
			BS_RTTI_MEMBER_PLAIN(mClearFlags, 4)
		BS_END_RTTI_MEMBERS
	public:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Viewport* viewport = static_cast<Viewport*>(obj);
			viewport->initialize();
		}

		const String& GetRTTIName() override
		{
			static String name = "Viewport";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_Viewport;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return Viewport::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
}
