//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsResource.h"

namespace BansheeEngine
{
	/** @addtogroup Audio
	 *  @{
	 */

	/** Valid internal engine audio formats. */
	enum class AudioFormat
	{
		PCM, /**< Pulse code modulation audio ("raw" uncompressed audio). */
		VORBIS /**< Vorbis compressed audio. */
	};

	/** Modes that determine how and when is audio data read. */
	enum class AudioReadMode
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
	};

	/** 
	 * Audio clip stores audio data in a compressed or uncompressed format. Clips can be provided to audio sources or
	 * or other audio methods to be played.
	 */
	class BS_CORE_EXPORT AudioClip : public Resource
	{
	public:
		virtual ~AudioClip() { }

		UINT32 getBitDepth() const { return mDesc.bitDepth; }
		UINT32 getFrequency() const { return mDesc.frequency; }
		UINT32 getNumChannels() const { return mDesc.numChannels; }
		AudioFormat getFormat() const { return mDesc.format; }
		AudioReadMode getReadMode() const { return mDesc.readMode; }
		float getLength() const { return mLength; }
		UINT32 getNumSamples() const { return mNumSamples; }
		bool is3D() const { return mDesc.is3D; }

		/** 
		 * Returns audio samples in PCM format, channel data interleaved. Sample read pointer is advanced by the read 
		 * amount. Only available if the audio data has been created with AudioReadMode::Stream, 
		 * AudioReadMode::LoadCompressed (and the format is compressed), or if @p keepSourceData was enabled on creation.
		 *
		 * @param[in]	samples		Previously allocated buffer to contain the samples.
		 * @param[in]	count		Number of samples to read (should be a multiple of number of channels).
		 *
		 * @note	Implementation must be thread safe as this will get called from audio streaming thread.
		 */
		virtual void getSamples(UINT8* samples, UINT32 count) const = 0;

		/**
		 * Moves the read location from which the getSamples method retrieves samples. Only available if the audio data
		 * has been created with AudioReadMode::Stream, AudioReadMode::LoadCompressed (and the format is compressed), 
		 * or if @p keepSourceData was enabled on creation.
		 *
		 * @param[in]	offset	Offset in number of samples at which to start reading (should be a multiple of number
		 *						of channels).
		 *
		 * @note	Implementation must be thread safe as this will get called from audio streaming thread.
		 */
		virtual void seekSamples(UINT32 offset) = 0;

		static HAudioClip create(UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc);
		static HAudioClip create(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples,
			const AUDIO_CLIP_DESC& desc); // Note that ownership of stream is taken by the AudioClip

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		static SPtr<AudioClip> _createPtr(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, 
			const AUDIO_CLIP_DESC& desc);

		/** @} */
	protected:
		AudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc);

		/** @copydoc Resource::initialize */
		void initialize() override;

		/** Returns original audio data. Only available if @p keepSourceData has been provided on creation. */
		virtual SPtr<DataStream> getSourceStream(UINT32& size) = 0;

	protected:
		AUDIO_CLIP_DESC mDesc;
		UINT32 mNumSamples;
		UINT32 mStreamSize;
		UINT32 mStreamOffset;
		float mLength;
		bool mKeepSourceData;
		SPtr<DataStream> mStreamData;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class AudioClipRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

		static SPtr<AudioClip> createEmpty();
	};

	/** @} */
}