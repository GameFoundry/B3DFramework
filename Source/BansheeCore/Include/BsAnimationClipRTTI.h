//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsAnimationClip.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT AnimationClipRTTI : public RTTIType <AnimationClip, Resource, AnimationClipRTTI>
	{
	private:

	public:
		AnimationClipRTTI()
		{
			
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			AnimationClip* clip = static_cast<AnimationClip*>(obj);
			clip->initialize();
		}

		const String& getRTTIName() override
		{
			static String name = "AnimationClip";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_AnimationClip;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return AnimationClip::createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}