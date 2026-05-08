//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DResource.h"

namespace b3d
{
	/** @addtogroup Audio
	 *  @{
	 */

	/** Valid internal engine audio formats. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Audio)) AudioFormat
	{
		PCM, /**< Pulse code modulation audio ("raw" uncompressed audio). */
		VORBIS /**< Vorbis compressed audio. */
	};

	/** Modes that determine how and when is audio data read. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Audio)) AudioReadMode
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
	struct AudioClipCreateInformation
	{
		/** Determines how is audio data read. */
		AudioReadMode ReadMode = AudioReadMode::LoadDecompressed;

		/** Determines in which format is the audio data in. */
		AudioFormat Format = AudioFormat::PCM;

		/** Sample rate (frequency) of the audio data. */
		u32 Frequency = 44100;

		/** Number of bits per sample. Not used for compressed formats. */
		u32 BitDepth = 16;

		/** Number of channels. Each channel has its own step of samples. */
		u32 ChannelCount = 2;

		/** Determines should the audio clip be played using 3D positioning. Only valid for mono audio. */
		bool Is3D = true;

		/**
		 * Determines should the audio clip keep the original data in memory after creation. For example if the audio data
		 * is normally compressed, but audio clip uncompresses it on load, the original compressed data will be lost unless
		 * this is enabled. This will cause extra memory to be used, but can be useful in certain circumstances (for example
		 * you might require that data to save the audio clip on disk).
		 *
		 * When loading audio clip directly from disk, this properly is controlled by the ResourceLoadFlag::KeepSourceData.
		 */
		bool KeepSourceData = true;
	};

	/**
	 * Audio clip stores audio data in a compressed or uncompressed format. Clips can be provided to audio sources or
	 * other audio methods to be played.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Audio)) AudioClip : public Resource
	{
	public:
		virtual ~AudioClip() = default;

		/** Returns the size of a single sample, in bits. */
		B3D_SCRIPT_EXPORT(ExportName(BitDepth), Property(Getter))
		u32 GetBitDepth() const { return mInformation.BitDepth; }

		/** Returns how many samples per second is the audio encoded in. */
		B3D_SCRIPT_EXPORT(ExportName(SampleRate), Property(Getter))
		u32 GetFrequency() const { return mInformation.Frequency; }

		/** Returns the number of channels provided by the clip. */
		B3D_SCRIPT_EXPORT(ExportName(NumChannels), Property(Getter))
		u32 GetChannelCount() const { return mInformation.ChannelCount; }

		/**
		 * Returns in which format is audio data stored in.
		 *
		 * @see	AudioFormat
		 */
		B3D_SCRIPT_EXPORT(ExportName(Format), Property(Getter))
		AudioFormat GetFormat() const { return mInformation.Format; }

		/**
		 * Returns how is the audio data read/decoded.
		 *
		 * @see	AudioReadMode
		 */
		B3D_SCRIPT_EXPORT(ExportName(ReadMode), Property(Getter))
		AudioReadMode GetReadMode() const { return mInformation.ReadMode; }

		/** Returns the length of the audio clip, in seconds. */
		B3D_SCRIPT_EXPORT(ExportName(Duration), Property(Getter))
		float GetLength() const { return mLength; }

		/** Returns the total number of samples in the clip (includes all channels). */
		B3D_SCRIPT_EXPORT(ExportName(NumSamples), Property(Getter))
		u32 GetSampleCount() const { return mSampleCount; }

		/** Determines will the clip be played a spatial 3D sound, or as a normal sound (for example music). */
		B3D_SCRIPT_EXPORT(ExportName(Is3D), Property(Getter))
		bool Is3D() const { return mInformation.Is3D; }

		/**
		 * Creates a new AudioClip and populates it with provided samples.
		 *
		 * @param	samples				Data streams containing the samples to load. Data will be read starting from the current
		 *								position in the stream. The samples should be in audio format as specified in the
		 *								@p createInformation parameter. Ownership of the data stream is taken by the audio clip and the
		 *								caller must not close it manually.
		 * @param	streamSize			Number of bytes to read from the @p samples stream.
		 * @param	sampleCount			Total number of samples (including all channels).
		 * @param	createInformation	Descriptor containing meta-data for the provided samples.
		 *
		 * @note	If the provided samples are in PCM format, they should be signed integers of provided bit depth.
		 */
		static HAudioClip Create(const TShared<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Creates a new AudioClip without initializing it. Use Create() for normal use. */
		static TShared<AudioClip> CreateShared(const TShared<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation);

		/** @} */
	protected:
		AudioClip(const TShared<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation);

		void Initialize() override;
		bool IsCompressible() const override { return false; } // Compression handled on a case by case basis manually by the audio system

		/**
		 * Returns original audio data. Only available if @p keepSourceData has been provided on creation.
		 *
		 * @param	outSize	Size of the returned stream data, in bytes.
		 * @return			Stream containing the original audio data.
		 */
		virtual TShared<DataStream> GetSourceStream(u32& outSize) = 0;

	protected:
		AudioClipCreateInformation mInformation;
		u32 mSampleCount;
		u32 mStreamSize;
		u32 mStreamOffset = 0;
		float mLength = 0.0f;
		TShared<DataStream> mStreamData;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class AudioClipRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

		/**
		 * Creates an AudioClip with no samples. You must populate its data manually followed by a call to Initialize().
		 *
		 * @note	For serialization use only.
		 */
		static TShared<AudioClip> CreateEmpty();
	};

	/** @} */
} // namespace b3d
