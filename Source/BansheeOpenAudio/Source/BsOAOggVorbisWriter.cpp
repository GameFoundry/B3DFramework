#include "BsOAOggVorbisWriter.h"

namespace BansheeEngine
{
// Writes to the internal cached buffer and flushes it if needed
#define WRITE_TO_BUFFER(data, length)					\
	if ((mBufferOffset + length) > BUFFER_SIZE)			\
		flush();										\
														\
	if(length > BUFFER_SIZE)							\
		mWriteCallback(data, length);					\
	else												\
	{													\
		memcpy(mBuffer + mBufferOffset, data, length);	\
		mBufferOffset += length;						\
	}

	OAOggVorbisWriter::OAOggVorbisWriter()
		:mBufferOffset(0), mNumChannels(0), mBitDepth(0)
	{ }

	OAOggVorbisWriter::~OAOggVorbisWriter()
	{
		close();
	}

	bool OAOggVorbisWriter::open(std::function<void(UINT8*, UINT32)> writeCallback, UINT32 sampleRate, UINT32 bitDepth, 
		UINT32 numChannels)
	{
		mNumChannels = numChannels;
		mBitDepth = bitDepth;
		mWriteCallback = writeCallback;

		ogg_stream_init(&mOggState, std::rand());
		vorbis_info_init(&mVorbisInfo);

		// Automatic bitrate management with quality 0.4 (~128 kbps for 44 KHz stereo sound)
		INT32 status = vorbis_encode_init_vbr(&mVorbisInfo, numChannels, sampleRate, 0.4f);
		if (status < 0)
		{
			LOGERR("Failed to write Ogg Vorbis file.");
			close();
			return false;
		}

		vorbis_analysis_init(&mVorbisState, &mVorbisInfo);

		// Generate header
		vorbis_comment comment;
		vorbis_comment_init(&comment);

		ogg_packet headerPacket, commentPacket, codePacket;
		status = vorbis_analysis_headerout(&mVorbisState, &comment, &headerPacket, &commentPacket, &codePacket);
		vorbis_comment_clear(&comment);

		if (status < 0)
		{
			LOGERR("Failed to write Ogg Vorbis file.");
			close();
			return false;
		}

		// Write header
		ogg_stream_packetin(&mOggState, &headerPacket);
		ogg_stream_packetin(&mOggState, &commentPacket);
		ogg_stream_packetin(&mOggState, &codePacket);

		ogg_page page;
		while (ogg_stream_flush(&mOggState, &page) > 0)
		{
			WRITE_TO_BUFFER(page.header, page.header_len);
			WRITE_TO_BUFFER(page.body, page.body_len);
		}

		return true;
	}

	void OAOggVorbisWriter::write(UINT8* samples, UINT32 numSamples)
	{
		// Allocate new buffer and write PCM data to it
		UINT32 numFrames = numSamples / mNumChannels;
		float** buffer = vorbis_analysis_buffer(&mVorbisState, numFrames);

		if (mBitDepth == 8)
		{
			for (UINT32 i = 0; i < numFrames; i++)
			{
				for (UINT32 j = 0; j < mNumChannels; j++)
				{
					INT8 sample = *(INT8*)samples;
					float encodedSample = sample / 127.0f;
					buffer[j][i] = encodedSample;

					samples++;
				}
			}
		}
		else if (mBitDepth == 16)
		{
			for (UINT32 i = 0; i < numFrames; i++)
			{
				for (UINT32 j = 0; j < mNumChannels; j++)
				{
					INT16 sample = *(INT16*)samples;
					float encodedSample = sample / 32767.0f;
					buffer[j][i] = encodedSample;

					samples++;
				}
			}
		}
		else if (mBitDepth == 24)
		{
			for (UINT32 i = 0; i < numFrames; i++)
			{
				for (UINT32 j = 0; j < mNumChannels; j++)
				{
					INT32 sample = (*(INT32*)samples) & 0x00FFFFFF;
					float encodedSample = sample / 8388607.0f;
					buffer[j][i] = encodedSample;

					samples++;
				}
			}
		}
		else if (mBitDepth == 32)
		{
			for (UINT32 i = 0; i < numFrames; i++)
			{
				for (UINT32 j = 0; j < mNumChannels; j++)
				{
					INT32 sample = *(INT32*)samples;
					float encodedSample = sample / 2147483647.0f;
					buffer[j][i] = encodedSample;

					samples++;
				}
			}
		}
		else
			assert(false);

		// Signal how many frames were written
		vorbis_analysis_wrote(&mVorbisState, numFrames);
		writeBlocks();
	}

	void OAOggVorbisWriter::writeBlocks()
	{
		// Create a new block
		vorbis_block block;
		vorbis_block_init(&mVorbisState, &block);
		while (vorbis_analysis_blockout(&mVorbisState, &block) == 1)
		{
			// Analyze and determine optimal bitrate
			vorbis_analysis(&block, nullptr);
			vorbis_bitrate_addblock(&block);

			// Write block into ogg packets
			ogg_packet packet;
			while (vorbis_bitrate_flushpacket(&mVorbisState, &packet))
			{
				ogg_stream_packetin(&mOggState, &packet);

				// If new page, write it to the internal buffer
				ogg_page page;
				while (ogg_stream_flush(&mOggState, &page) > 0)
				{
					WRITE_TO_BUFFER(page.header, page.header_len);
					WRITE_TO_BUFFER(page.body, page.body_len);
				}
			}
		}

		// Clear the allocated block
		vorbis_block_clear(&block);
	}

	void OAOggVorbisWriter::flush()
	{
		if (mBufferOffset > 0 && mWriteCallback != nullptr)
			mWriteCallback(mBuffer, mBufferOffset);

		mBufferOffset = 0;
	}

	void OAOggVorbisWriter::close()
	{
		// Mark end of data and flush any remaining data in the buffers
		vorbis_analysis_wrote(&mVorbisState, 0);
		writeBlocks();
		flush();

		ogg_stream_clear(&mOggState);
		vorbis_dsp_clear(&mVorbisState);
		vorbis_info_clear(&mVorbisInfo);
	}

#undef WRITE_TO_BUFFER
}