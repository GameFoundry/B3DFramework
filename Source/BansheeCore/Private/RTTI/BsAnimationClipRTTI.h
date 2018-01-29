//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Animation/BsAnimationClip.h"
#include "Private/RTTI/BsAnimationCurveRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	template<>
	struct RTTIPlainType<AnimationEvent>
	{
		enum { id = TID_AnimationEvent }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static void toMemory(const AnimationEvent& data, char* memory)
		{
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			UINT8 version = 0;
			memory = rttiWriteElem(version, memory, size);
			memory = rttiWriteElem(data.time, memory, size);
			memory = rttiWriteElem(data.name, memory, size);

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(AnimationEvent& data, char* memory)
		{
			UINT32 size = 0;
			memory = rttiReadElem(size, memory);

			UINT8 version;
			memory = rttiReadElem(version, memory);
			assert(version == 0);

			memory = rttiReadElem(data.time, memory);
			memory = rttiReadElem(data.name, memory);

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const AnimationEvent& data)
		{
			UINT64 dataSize = sizeof(UINT8) + sizeof(UINT32);
			dataSize += rttiGetElemSize(data.time);
			dataSize += rttiGetElemSize(data.name);

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}
	};

	class BS_CORE_EXPORT AnimationClipRTTI : public RTTIType <AnimationClip, Resource, AnimationClipRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(positionCurves, mCurves->position, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(rotationCurves, mCurves->rotation, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(scaleCurves, mCurves->scale, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(genericCurves, mCurves->generic, 3)
			BS_RTTI_MEMBER_PLAIN(mIsAdditive, 4)
			BS_RTTI_MEMBER_PLAIN(mLength, 5)
			BS_RTTI_MEMBER_PLAIN(mEvents, 6)
			BS_RTTI_MEMBER_PLAIN(mSampleRate, 7)
			BS_RTTI_MEMBER_PLAIN_NAMED(rootMotionPos, mRootMotion->position, 8)
			BS_RTTI_MEMBER_PLAIN_NAMED(rootMotionRot, mRootMotion->rotation, 9)
		BS_END_RTTI_MEMBERS
	public:
		AnimationClipRTTI()
			:mInitMembers(this)
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