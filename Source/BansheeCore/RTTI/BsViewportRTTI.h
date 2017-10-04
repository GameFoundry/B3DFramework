//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
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
		ViewportRTTI()
			:mInitMembers(this)
		{ }

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			Viewport* viewport = static_cast<Viewport*>(obj);
			viewport->initialize();
		}

		const String& getRTTIName() override
		{
			static String name = "Viewport";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Viewport;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return Viewport::createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}