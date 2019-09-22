//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsStdRTTI.h"
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
		static BitLength toMemory(const AnimationEvent& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
				{
					constexpr uint8_t VERSION = 0;

					BitLength size = 0;
					size += rtti_write(VERSION, stream);
					size += rtti_write(data.time, stream);
					size += rtti_write(data.name, stream);

					return size;
				});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength fromMemory(AnimationEvent& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);
			
			uint8_t version;
			rtti_read(version, stream);
			assert(version == 0);

			rtti_read(data.time, stream);
			rtti_read(data.name, stream);

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength getSize(const AnimationEvent& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint8_t);
			dataSize += rtti_size(data.time);
			dataSize += rtti_size(data.name);

			rtti_add_header_size(dataSize, compress);
			
			return dataSize;
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
		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
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
