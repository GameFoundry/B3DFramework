//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsResource.h"

namespace bs
{
	/** @addtogroup Audio
	 *  @{
	 */

	/** Valid internal engine audio formats. */
	enum class BS_SCRIPT_EXPORT(m:Audio) AudioFormat
	{
		PCM, /**< Pulse code modulation audio ("raw" uncompressed audio). */
		VORBIS /**< Vorbis compressed audio. */
	};

	/** Modes that determine how and when is audio data read. */
	enum class BS_SCRIPT_EXPORT(m:Audio) AudioReadMode
	{
		/** Entire audio clip will be loaded and decompressed. Uses most memory but has lowest CPU impact. */
		LoadDecompressed, 
		/**
		 * Entire audio clip will be loaded, but will be decompressed while playing. Uses medium amount of memory and has
		 * the highest CPU impact. 
		 */
		LoadCompressed,
		/** 
		 * Audio will be slowly streamed from the disk, and decompressed if needed. Uses very little memory, and has either
		 * low or high CPU impact depending if the audio is in a compressed format. Since data is streamed from the disk,
		 * read speeds could also be a bottleneck.
		 */
		Stream
	};

	/** Descriptor used for initializing an audio clip. */
	struct AUDIO_CLIP_DESC
	{
		/** Determines how is audio data read. */
		AudioReadMode readMode = AudioReadMode::LoadDecompressed;

		/** Determines in which format is the audio data in. */
		AudioFormat format = AudioFormat::PCM;

		/** Sample rate (frequency) of the audio data. */
		UINT32 frequency = 44100;

		/** Number of bits per sample. Not used for compressed formats. */
		UINT32 bitDepth = 16;
		
		/** Number of channels. Each channel has its own step of samples. */
		UINT32 numChannels = 2;

		/** Determines should the audio clip be played using 3D positioning. Only valid for mono audio. */
		bool is3D = true;

		/** 
		 * Determines should the audio clip keep the original data in memory after creation. For example if the audio data 
		 * is normally compressed, but audio clip uncompresses it on load, the original compressed data will be lost unless
		 * this is enabled. This will cause extra memory to be used, but can be useful in certain circumstances (for example
		 * you might require that data to save the audio clip on disk).
		 *
		 * When loading audio clip directly from disk, this properly is controlled by the ResourceLoadFlag::KeepSourceData.
		 */
		bool keepSourceData = true;
	};

	/** 
	 * Audio clip stores audio data in a compressed or uncompressed format. Clips can be provided to audio sources or
	 * other audio methods to be played.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Audio) AudioClip : public Resource
	{
	public:
		virtual ~AudioClip() = default;

		/** Returns the size of a single sample, in bits. */
		BS_SCRIPT_EXPORT(n:BitDepth,pr:getter)
		UINT32 getBitDepth() const { return mDesc.bitDepth; }
		
		/** Returns how many samples per second is the audio encoded in. */
		BS_SCRIPT_EXPORT(n:SampleRate,pr:getter)
		UINT32 getFrequency() const { return mDesc.frequency; }

		/** Returns the number of channels provided by the clip. */
		BS_SCRIPT_EXPORT(n:NumChannels,pr:getter)
		UINT32 getNumChannels() const { return mDesc.numChannels; }

		/** 
		 * Returns in which format is audio data stored in. 
		 *
		 * @see	AudioFormat
		 */
		BS_SCRIPT_EXPORT(n:Format,pr:getter)
		AudioFormat getFormat() const { return mDesc.format; }

		/** 
		 * Returns how is the audio data read/decoded. 
		 *
		 * @see	AudioReadMode
		 */
		BS_SCRIPT_EXPORT(n:ReadMode,pr:getter)
		AudioReadMode getReadMode() const { return mDesc.readMode; }

		/** Returns the length of the audio clip, in seconds. */
		BS_SCRIPT_EXPORT(n:Duration,pr:getter)
		float getLength() const { return mLength; }

		/** Returns the total number of samples in the clip (includes all channels). */
		BS_SCRIPT_EXPORT(n:NumSamples,pr:getter)
		UINT32 getNumSamples() const { return mNumSamples; }

		/** Determines will the clip be played a spatial 3D sound, or as a normal sound (for example music). */
		BS_SCRIPT_EXPORT(n:Is3D,pr:getter)
		bool is3D() const { return mDesc.is3D; }

		/**
		 * Creates a new AudioClip and populates it with provided samples.
		 *
		 * @param[in]	samples		Data streams containing the samples to load. Data will be read starting from the current
		 *							position in the stream. The samples should be in audio format as specified in the 
		 *							@p desc parameter. Ownership of the data stream is taken by the audio clip and the
		 *							caller must not close it manually.
		 * @param[in]	streamSize	Number of bytes to read from the @p samples stream.
		 * @param[in]	numSamples	Total number of samples (including all channels).
		 * @param[in]	desc		Descriptor containing meta-data for the provided samples.
		 *
		 * @note	If the provided samples are in PCM format, they should be signed integers of provided bit depth.
		 */
		static HAudioClip create(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples,
			const AUDIO_CLIP_DESC& desc);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Creates a new AudioClip without initializing it. Use create() for normal use. */
		static SPtr<AudioClip> _createPtr(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, 
			const AUDIO_CLIP_DESC& desc);

		/** @} */
	protected:
		AudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc);

		/** @copydoc Resource::initialize */
		void initialize() override;

		/** @copydoc Resource::isCompressible */
		bool isCompressible() const override { return false; } // Compression handled on a case by case basis manually by the audio system

		/** Returns original audio data. Only available if @p keepSourceData has been provided on creation. */
		virtual SPtr<DataStream> getSourceStream(UINT32& size) = 0;

	protected:
		AUDIO_CLIP_DESC mDesc;
		UINT32 mNumSamples;
		UINT32 mStreamSize;
		UINT32 mStreamOffset = 0;
		float mLength = 0.0f;
		SPtr<DataStream> mStreamData;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class AudioClipRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

		/** 
		 * Creates an AudioClip with no samples. You must populate its data manually followed by a call to initialize().
		 *
		 * @note	For serialization use only.
		 */
		static SPtr<AudioClip> createEmpty();
	};

	/** @} */
}