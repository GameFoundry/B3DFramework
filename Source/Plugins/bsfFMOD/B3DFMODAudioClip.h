//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DFMODPrerequisites.h"
#include "Audio/B3DAudioClip.h"
#include "B3DOggVorbisDecoder.h"
#include <fmod.hpp>

namespace b3d
{
	/** @addtogroup FMOD
	 *  @{
	 */

	/** Contains data used for decompressing an Ogg Vorbis stream. */
	struct FMODOggDecompressorData
	{
		u32 ReadPos = 0;
		OggVorbisDecoder VorbisReader;
		const FMODAudioClip* Clip = nullptr;
	};

	/** FMOD implementation of an AudioClip. */
	class FMODAudioClip : public AudioClip
	{
	public:
		FMODAudioClip(const SPtr<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation);
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
		void Initialize() override;
		SPtr<DataStream> GetSourceStream(u32& outSize) override;

		FMOD::Sound* mSound = nullptr;

		// These streams exist to save original audio data in case it's needed later (usually for saving with the editor, or
		// manual data manipulation). In normal usage (in-game) these will be null so no memory is wasted.
		SPtr<DataStream> mSourceStreamData;
		u32 mSourceStreamSize = 0;
	};

	/** @} */
} // namespace b3d
