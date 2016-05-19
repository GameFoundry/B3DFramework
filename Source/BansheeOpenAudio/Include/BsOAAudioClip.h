//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsOAPrerequisites.h"
#include "BsAudioClip.h"
#include "BsOAOggVorbisReader.h"

namespace BansheeEngine
{
	/** @addtogroup OpenAudio
	 *  @{
	 */
	
	class BS_OA_EXPORT OAAudioClip : public AudioClip
	{
	public:
		virtual ~OAAudioClip();

		/** @copydoc AudioClip::getSamples */
		void getSamples(UINT8* samples, UINT32 count) const override;

		/** @copydoc AudioClip::seekSamples */
		void seekSamples(UINT32 offset) override;

		/** @name Internal
		 *  @{
		 */

		/** Returns the internal OpenAL buffer. Only valid if the audio clip was created without AudioReadMode::Stream. */
		UINT32 _getOpenALBuffer() const { return mBufferId; }

		/** @} */
	protected:
		/** @copydoc Resource::initialize */
		void initialize() override;

		/** @copydoc AudioClip::getSourceFormatData */
		SPtr<DataStream> getSourceStream(UINT32& size) override;
	private:
		OAAudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc);

		mutable Mutex mMutex;
		mutable OAOggVorbisReader mVorbisReader;
		bool mNeedsDecompression;
		UINT32 mBufferId;

		// These streams exist to save original audio data in case it's needed later (usually for saving with the editor, or
		// manual data manipulation). In normal usage (in-game) these will be null so no memory is wasted.
		SPtr<DataStream> mSourceStreamData;
		UINT32 mSourceStreamSize;
	};

	/** @} */
}