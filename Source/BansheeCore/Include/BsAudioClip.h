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
	};

	/** 
	 * Audio clip stores audio data in a compressed or uncompressed format. Clips can be provided to audio sources or
	 * or other audio methods to be played.
	 */
	class BS_CORE_EXPORT AudioClip : public Resource
	{
	public:
		static HAudioClip create(UINT32 numSamples, const AUDIO_CLIP_DESC& desc);
		static HAudioClip create(const SPtr<DataStream>& samples, UINT32 numSamples, const AUDIO_CLIP_DESC& desc);

		UINT32 getBitDepth() const { return mDesc.bitDepth; }
		UINT32 getFrequency() const { return mDesc.frequency; }
		UINT32 getNumChannels() const { return mDesc.numChannels; }
		AudioFormat getFormat() const { return mDesc.format; }
		AudioReadMode getReadMode() const { return mDesc.readMode; }
		UINT32 getLength() const { return mNumSamples / mDesc.frequency; }
		UINT32 getNumSamples() const { return mNumSamples; }

		virtual UINT32 getDataSize() const = 0;
		virtual UINT8* getData() const = 0;

		// Note: This will convert internal audio read mode to LoadCompressed (if ogg) or LoadDecompressed (if PCM)
		virtual void setData(UINT8* samples, UINT32 numSamples) = 0;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		static SPtr<AudioClip> _createPtr(const SPtr<DataStream>& samples, UINT32 numSamples, const AUDIO_CLIP_DESC& desc);

		/** @} */
	private:
		AudioClip(const SPtr<DataStream>& samples, UINT32 numSamples, const AUDIO_CLIP_DESC& desc);

	protected:
		AUDIO_CLIP_DESC mDesc;
		UINT32 mNumSamples;
		SPtr<DataStream> mStreamData;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class AudioClipRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}