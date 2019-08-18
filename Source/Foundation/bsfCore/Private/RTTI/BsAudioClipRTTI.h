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

	class BS_CORE_EXPORT AudioClipRTTI : public RTTIType <AudioClip, Resource, AudioClipRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(readMode, mDesc.readMode, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(format, mDesc.format, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(frequency, mDesc.frequency, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(bitDepth, mDesc.bitDepth, 3)
			BS_RTTI_MEMBER_PLAIN_NAMED(numChannels, mDesc.numChannels, 4)
			BS_RTTI_MEMBER_PLAIN(mNumSamples, 5)
			BS_RTTI_MEMBER_PLAIN(mStreamSize, 7)
			BS_RTTI_MEMBER_PLAIN(mStreamOffset, 8)
			BS_RTTI_MEMBER_PLAIN_NAMED(is3D, mDesc.is3D, 9)
			BS_RTTI_MEMBER_PLAIN(mLength, 10)
		BS_END_RTTI_MEMBERS

		SPtr<DataStream> getData(AudioClip* obj, UINT32& size)
		{
			SPtr<DataStream> stream = obj->getSourceStream(size);
			if (stream != nullptr && stream->isFile())
			{
				BS_LOG(Warning, RTTI, "Saving an AudioClip which uses streaming data. Streaming data might not be "
					"available if saving to the same file.");
			}

			return stream;
		}

		void setData(AudioClip* obj, const SPtr<DataStream>& val, UINT32 size)
		{
			obj->mStreamData = val->clone(); // Making sure that the AudioClip cannot modify the source stream, which is still used by the deserializer
			obj->mStreamSize = size;
			obj->mStreamOffset = (UINT32)val->tell();
		}

	public:
		AudioClipRTTI()
		{
			addDataBlockField("mData", 6, &AudioClipRTTI::getData, &AudioClipRTTI::setData);
		}

		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			AudioClip* clip = static_cast<AudioClip*>(obj);
			clip->initialize();
		}

		const String& getRTTIName() override
		{
			static String name = "AudioClip";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_AudioClip;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return AudioClip::createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}
