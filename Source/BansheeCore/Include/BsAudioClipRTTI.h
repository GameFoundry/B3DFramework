//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsAudioClip.h"

namespace BansheeEngine
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
		BS_END_RTTI_MEMBERS

		ManagedDataBlock getData(AudioClip* obj)
		{
			ManagedDataBlock dataBlock((UINT8*)obj->getData(), obj->getDataSize());
			return dataBlock;
		}

		void setData(AudioClip* obj, ManagedDataBlock val)
		{
			// TODO: This method isn't needed as this data will be loaded from the data stream
			obj->setData(val.getData(), obj->mNumSamples);
		}

		static UINT8* allocateData(AudioClip* obj, UINT32 numBytes)
		{
			// TODO: This method isn't needed as this data will be loaded from the data stream
			return (UINT8*)bs_alloc(numBytes);
		}

	public:
		AudioClipRTTI()
			:mInitMembers(this)
		{
			addDataBlockField("mData", 6, &AudioClipRTTI::getData, &AudioClipRTTI::setData, 
				0, &AudioClipRTTI::allocateData);
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
			AUDIO_CLIP_DESC desc;
			return AudioClip::_createPtr(nullptr, 0, desc); // Initial values don't matter, they will get overwritten
		}
	};

	/** @} */
	/** @endcond */
}