//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Reflection/B3DRTTIType.h"
#include "Audio/B3DAudioClip.h"
#include "FileSystem/B3DDataStream.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT AudioClipRTTI : public TRTTIType<AudioClip, Resource, AudioClipRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(readMode, mInformation.ReadMode, 0)
			B3D_RTTI_MEMBER_NAMED(format, mInformation.Format, 1)
			B3D_RTTI_MEMBER_NAMED(frequency, mInformation.Frequency, 2)
			B3D_RTTI_MEMBER_NAMED(bitDepth, mInformation.BitDepth, 3)
			B3D_RTTI_MEMBER_NAMED(numChannels, mInformation.ChannelCount, 4)
			B3D_RTTI_MEMBER(mSampleCount, 5)
			B3D_RTTI_MEMBER(mStreamSize, 7)
			B3D_RTTI_MEMBER(mStreamOffset, 8)
			B3D_RTTI_MEMBER_NAMED(is3D, mInformation.Is3D, 9)
			B3D_RTTI_MEMBER(mLength, 10)
		B3D_RTTI_END_MEMBERS

		TShared<DataStream> GetData(AudioClip* obj, u32& size)
		{
			TShared<DataStream> stream = obj->GetSourceStream(size);
			if(stream == nullptr)
				B3D_LOG(Error, LogRTTI, "Cannot serialize audio clip '{0}'. It doesn't have its source audio data, which clips loaded from a package don't keep.", obj->GetName());

			return stream;
		}

		void SetData(AudioClip* obj, const TShared<DataStream>& val, u32 size)
		{
			// Making sure that the AudioClip cannot modify the source stream, which is still used by the deserializer
			if(val->IsFile())
			{
				obj->mStreamData = val->Clone();
				obj->mStreamOffset = (u32)val->Tell();
			}
			else
			{
				const TShared<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>(size);
				val->Read(stream->Data(), size);

				obj->mStreamData = stream;
				obj->mStreamOffset = 0;
			}

			obj->mStreamSize = size;
		}

	public:
		AudioClipRTTI()
		{
			AddDataBlockField("mData", 6, &AudioClipRTTI::GetData, &AudioClipRTTI::SetData);
		}

		void OnOperationEnded(AudioClip& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "AudioClip";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_AudioClip;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return AudioClip::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
