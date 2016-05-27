//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsFMODPrerequisites.h"
#include "BsAudioClip.h"

namespace BansheeEngine
{
	/** @addtogroup FMOD
	 *  @{
	 */
	
	/** FMOD implementation of an AudioClip. */
	class BS_FMOD_EXPORT FMODAudioClip : public AudioClip
	{
	public:
		FMODAudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc);
		virtual ~FMODAudioClip();

		/** @copydoc AudioClip::getSamples */
		void getSamples(UINT8* samples, UINT32 offset, UINT32 count) const override;

	protected:
		/** @copydoc Resource::initialize */
		void initialize() override;

		/** @copydoc AudioClip::getSourceFormatData */
		SPtr<DataStream> getSourceStream(UINT32& size) override;
	};

	/** @} */
}