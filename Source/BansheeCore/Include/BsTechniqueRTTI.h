//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsTechnique.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT TechniqueRTTI : public RTTIType<Technique, IReflectable, TechniqueRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mRenderAPI, 0)
			BS_RTTI_MEMBER_PLAIN(mRenderer, 1)
			BS_RTTI_MEMBER_REFLPTR_ARRAY(mPasses, 2)
			BS_RTTI_MEMBER_PLAIN_ARRAY(mTags, 3)
		BS_END_RTTI_MEMBERS

	public:
		TechniqueRTTI()
			:mInitMembers(this)
		{ }

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			Technique* technique = static_cast<Technique*>(obj);
			technique->initialize();
		}

		const String& getRTTIName() override
		{
			static String name = "Technique";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Technique;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return Technique::createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}