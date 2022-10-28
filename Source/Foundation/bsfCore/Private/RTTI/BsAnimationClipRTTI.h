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

	template <>
	struct RTTIPlainType<AnimationEvent>
	{
		enum
		{
			id = TID_AnimationEvent
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const AnimationEvent& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
					constexpr uint8_t VERSION = 0;

					BitLength size = 0;
					size += rtti_write(VERSION, stream);
					size += rtti_write(data.Time, stream);
					size += rtti_write(data.Name, stream);

					return size; });
		}

		static BitLength FromMemory(AnimationEvent& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint8_t version;
			rtti_read(version, stream);
			assert(version == 0);

			rtti_read(data.Time, stream);
			rtti_read(data.Name, stream);

			return size;
		}

		static BitLength GetSize(const AnimationEvent& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint8_t);
			dataSize += rtti_size(data.Time);
			dataSize += rtti_size(data.Name);

			rtti_add_header_size(dataSize, compress);

			return dataSize;
		}
	};

	class BS_CORE_EXPORT AnimationClipRTTI : public RTTIType<AnimationClip, Resource, AnimationClipRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(positionCurves, mCurves->Position, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(rotationCurves, mCurves->Rotation, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(scaleCurves, mCurves->Scale, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(genericCurves, mCurves->Generic, 3)
			BS_RTTI_MEMBER_PLAIN(mIsAdditive, 4)
			BS_RTTI_MEMBER_PLAIN(mLength, 5)
			BS_RTTI_MEMBER_PLAIN(mEvents, 6)
			BS_RTTI_MEMBER_PLAIN(mSampleRate, 7)
			BS_RTTI_MEMBER_PLAIN_NAMED(rootMotionPos, mRootMotion->Position, 8)
			BS_RTTI_MEMBER_PLAIN_NAMED(rootMotionRot, mRootMotion->Rotation, 9)
		BS_END_RTTI_MEMBERS
	public:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context)
		{
			AnimationClip* clip = static_cast<AnimationClip*>(obj);
			clip->Initialize();
		}

		const String& GetRttiName()
		{
			static String name = "AnimationClip";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_AnimationClip;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return AnimationClip::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
