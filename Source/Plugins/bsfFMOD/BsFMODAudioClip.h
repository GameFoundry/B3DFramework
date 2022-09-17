//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsFMODPrerequisites.h"
#include "Audio/BsAudioClip.h"
#include "BsOggVorbisDecoder.h"
#include <fmod.hpp>

namespace bs
{
	/** @addtogroup FMOD
	 *  @{
	 */
	
	/** Contains data used for decompressing an Ogg Vorbis stream. */
	struct FMODOggDecompressorData
	{
		UINT32 readPos = 0;
		OggVorbisDecoder vorbisReader;
		const FMODAudioClip* clip = nullptr;
	};

	/** FMOD implementation of an AudioClip. */
	class FMODAudioClip : public AudioClip
	{
	public:
		FMODAudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc);
		virtual ~FMODAudioClip();

		/**
		 * Creates a new streaming sound. Only valid if the clip was created with AudioReadMode::Stream. Caller is
		 * responsible for releasing the sound. Make sure to call releaseStreamingSound() when done.
		 */
		FMOD::Sound* CreateStreamingSound() const;

		/** Releases any resources with a streaming sound (created with createStreamingSound()). */
		static void ReleaseStreamingSound(FMOD::Sound* sound);

		/** Returns FMOD sound representing this clip. Only valid for non-streaming clips. */
		FMOD::Sound* GetSound() const { return mSound; }

		/**
		 * Checks whether the audio clip requires a streaming sound retrieved via createStreamingSound(), or can the
		 * basic sound retrieved via getSound() be used.
		 */
		bool RequiresStreaming() const;

	protected:
		/** @copydoc Resource::initialize */
		void Initialize() override;

		/** @copydoc AudioClip::getSourceStream */
		SPtr<DataStream> GetSourceStream(UINT32& size) ;

		FMOD::Sound* mSound = nullptr;

		// These streams exist to save original audio data in case it's needed later (usually for saving with the editor, or
		// manual data manipulation). In normal usage (in-game) these will be null so no memory is wasted.
		SPtr<DataStream> mSourceStreamData;
		UINT32 mSourceStreamSize = 0;
	};

	/** @} */
}
