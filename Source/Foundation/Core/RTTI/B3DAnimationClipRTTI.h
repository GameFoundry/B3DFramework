//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DStdRTTI.h"
#include "Animation/B3DAnimationClip.h"
#include "RTTI/B3DAnimationCurveRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
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
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
					constexpr uint8_t VERSION = 0;

					BitLength size = 0;
					size += B3DRTTIWrite(VERSION, stream);
					size += B3DRTTIWrite(data.Time, stream);
					size += B3DRTTIWrite(data.Name, stream);

					return size; });
		}

		static BitLength FromMemory(AnimationEvent& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint8_t version;
			B3DRTTIRead(version, stream);
			B3D_ASSERT(version == 0);

			B3DRTTIRead(data.Time, stream);
			B3DRTTIRead(data.Name, stream);

			return size;
		}

		static BitLength GetSize(const AnimationEvent& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint8_t);
			dataSize += B3DRTTISize(data.Time);
			dataSize += B3DRTTISize(data.Name);

			B3DRTTIAddHeaderSize(dataSize, compress);

			return dataSize;
		}
	};

	class B3D_EXPORT AnimationClipRTTI : public TRTTIType<AnimationClip, Resource, AnimationClipRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(positionCurves, mCurves->Position, 0)
			B3D_RTTI_MEMBER_NAMED(rotationCurves, mCurves->Rotation, 1)
			B3D_RTTI_MEMBER_NAMED(scaleCurves, mCurves->Scale, 2)
			B3D_RTTI_MEMBER_NAMED(genericCurves, mCurves->Generic, 3)
			B3D_RTTI_MEMBER(mIsAdditive, 4)
			B3D_RTTI_MEMBER(mLength, 5)
			B3D_RTTI_MEMBER(mEvents, 6)
			B3D_RTTI_MEMBER(mSampleRate, 7)
			B3D_RTTI_MEMBER_NAMED(rootMotionPos, mRootMotion->Position, 8)
			B3D_RTTI_MEMBER_NAMED(rootMotionRot, mRootMotion->Rotation, 9)
		B3D_RTTI_END_MEMBERS
	public:
		void OnOperationEnded(AnimationClip& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}

		const String& GetRttiName()
		{
			static String name = "AnimationClip";
			return name;
		}

		u32 GetRttiId() const override
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
} // namespace b3d
