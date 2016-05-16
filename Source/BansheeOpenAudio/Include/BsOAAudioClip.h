//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsOAPrerequisites.h"
#include "BsAudioClip.h"

namespace BansheeEngine
{
	/** @addtogroup OpenAudio
	 *  @{
	 */
	
	class BS_OA_EXPORT OAAudioClip : public AudioClip
	{
	public:
		/** @copydoc AudioClip::getSamples */
		virtual void getSamples(UINT8* samples, UINT32 count, UINT32 offset = 0) const = 0;

	protected:
		/** @copydoc Resource::initialize */
		void initialize() override;

		/** @copydoc AudioClip::getSourceFormatData */
		virtual SPtr<DataStream> getSourceFormatData(UINT32& size) = 0;
	private:
		OAAudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc);
	};

	/** @} */
}