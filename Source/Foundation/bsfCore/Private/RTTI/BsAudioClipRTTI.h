//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIPlain.h"
#include "Reflection/BsRTTIType.h"
#include "Audio/BsAudioClip.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT AudioClipRTTI : public RTTIType<AudioClip, Resource, AudioClipRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(readMode, mDesc.ReadMode, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(format, mDesc.Format, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(frequency, mDesc.Frequency, 2)
			B3D_RTTI_MEMBER_PLAIN_NAMED(bitDepth, mDesc.BitDepth, 3)
			B3D_RTTI_MEMBER_PLAIN_NAMED(numChannels, mDesc.NumChannels, 4)
			B3D_RTTI_MEMBER_PLAIN(mNumSamples, 5)
			B3D_RTTI_MEMBER_PLAIN(mStreamSize, 7)
			B3D_RTTI_MEMBER_PLAIN(mStreamOffset, 8)
			B3D_RTTI_MEMBER_PLAIN_NAMED(is3D, mDesc.Is3D, 9)
			B3D_RTTI_MEMBER_PLAIN(mLength, 10)
		B3D_RTTI_END_MEMBERS

		SPtr<DataStream> GetData(AudioClip* obj, u32& size)
		{
			SPtr<DataStream> stream = obj->GetSourceStream(size);
			if(stream != nullptr && stream->IsFile())
			{
				B3D_LOG(Warning, RTTI, "Saving an AudioClip which uses streaming data. Streaming data might not be "
									  "available if saving to the same file.");
			}

			return stream;
		}

		void SetData(AudioClip* obj, const SPtr<DataStream>& val, u32 size)
		{
			obj->mStreamData = val->Clone(); // Making sure that the AudioClip cannot modify the source stream, which is still used by the deserializer
			obj->mStreamSize = size;
			obj->mStreamOffset = (u32)val->Tell();
		}

	public:
		AudioClipRTTI()
		{
			AddDataBlockField("mData", 6, &AudioClipRTTI::GetData, &AudioClipRTTI::SetData);
		}

		void OnDeserializationEnded(IReflectable* obj, RTTIOperationContext* context)
		{
			AudioClip* clip = static_cast<AudioClip*>(obj);
			clip->Initialize();
		}

		const String& GetRttiName()
		{
			static String name = "AudioClip";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_AudioClip;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return AudioClip::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
