//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DOggVorbisEncoder.h"
#include "FileSystem/B3DDataStream.h"
#include "Audio/B3DAudioUtility.h"

using namespace b3d;

// Writes to the internal cached buffer and flushes it if needed
#define WRITE_TO_BUFFER(data, length)                  \
	if((mBufferOffset + length) > kBufferSize)         \
		Flush();                                       \
                                                       \
	if(length > kBufferSize)		                   \
		mWriteCallback(data, length);                  \
	else                                               \
	{                                                  \
		memcpy(mBuffer + mBufferOffset, data, length); \
		mBufferOffset += length;                       \
	}

OggVorbisEncoder::~OggVorbisEncoder()
{
	Close();
}

bool OggVorbisEncoder::Open(std::function<void(u8*, u32)> writeCallback, u32 sampleRate, u32 bitDepth, u32 numChannels)
{
	mNumChannels = numChannels;
	mBitDepth = bitDepth;
	mWriteCallback = writeCallback;
	mClosed = false;

	ogg_stream_init(&mOggState, std::rand());
	vorbis_info_init(&mVorbisInfo);

	// Automatic bitrate management with quality 0.4 (~128 kbps for 44 KHz stereo sound)
	i32 status = vorbis_encode_init_vbr(&mVorbisInfo, numChannels, sampleRate, 0.4f);
	if(status != 0)
	{
		B3D_LOG(Error, LogAudio, "Failed to write Ogg Vorbis file.");
		Close();
		return false;
	}

	vorbis_analysis_init(&mVorbisState, &mVorbisInfo);
	vorbis_block_init(&mVorbisState, &mVorbisBlock);

	// Generate header
	vorbis_comment comment;
	vorbis_comment_init(&comment);

	ogg_packet headerPacket, commentPacket, codePacket;
	status = vorbis_analysis_headerout(&mVorbisState, &comment, &headerPacket, &commentPacket, &codePacket);
	vorbis_comment_clear(&comment);

	if(status != 0)
	{
		B3D_LOG(Error, LogAudio, "Failed to write Ogg Vorbis file.");
		Close();
		return false;
	}

	// Write header
	ogg_stream_packetin(&mOggState, &headerPacket);
	ogg_stream_packetin(&mOggState, &commentPacket);
	ogg_stream_packetin(&mOggState, &codePacket);

	ogg_page page;
	while(ogg_stream_flush(&mOggState, &page) > 0)
	{
		WRITE_TO_BUFFER(page.header, page.header_len);
		WRITE_TO_BUFFER(page.body, page.body_len);
	}

	return true;
}

void OggVorbisEncoder::Write(u8* samples, u32 numSamples)
{
	static const u32 kWriteLength = 1024;

	u32 numFrames = numSamples / mNumChannels;
	while(numFrames > 0)
	{
		u32 numFramesToWrite = std::min(numFrames, kWriteLength);
		float** buffer = vorbis_analysis_buffer(&mVorbisState, numFramesToWrite);

		if(mBitDepth == 8)
		{
			for(u32 i = 0; i < numFramesToWrite; i++)
			{
				for(u32 j = 0; j < mNumChannels; j++)
				{
					i8 sample = *(i8*)samples;
					float encodedSample = sample / 127.0f;
					buffer[j][i] = encodedSample;

					samples++;
				}
			}
		}
		else if(mBitDepth == 16)
		{
			for(u32 i = 0; i < numFramesToWrite; i++)
			{
				for(u32 j = 0; j < mNumChannels; j++)
				{
					i16 sample = *(i16*)samples;
					float encodedSample = sample / 32767.0f;
					buffer[j][i] = encodedSample;

					samples += 2;
				}
			}
		}
		else if(mBitDepth == 24)
		{
			for(u32 i = 0; i < numFramesToWrite; i++)
			{
				for(u32 j = 0; j < mNumChannels; j++)
				{
					i32 sample = AudioUtility::Convert24To32Bits(samples);
					float encodedSample = sample / 2147483647.0f;
					buffer[j][i] = encodedSample;

					samples += 3;
				}
			}
		}
		else if(mBitDepth == 32)
		{
			for(u32 i = 0; i < numFramesToWrite; i++)
			{
				for(u32 j = 0; j < mNumChannels; j++)
				{
					i32 sample = *(i32*)samples;
					float encodedSample = sample / 2147483647.0f;
					buffer[j][i] = encodedSample;

					samples += 4;
				}
			}
		}
		else
			B3D_ASSERT(false);

		// Signal how many frames were written
		vorbis_analysis_wrote(&mVorbisState, numFramesToWrite);
		WriteBlocks();

		numFrames -= numFramesToWrite;
	}
}

void OggVorbisEncoder::WriteBlocks()
{
	while(vorbis_analysis_blockout(&mVorbisState, &mVorbisBlock) == 1)
	{
		// Analyze and determine optimal bitrate
		vorbis_analysis(&mVorbisBlock, nullptr);
		vorbis_bitrate_addblock(&mVorbisBlock);

		// Write block into ogg packets
		ogg_packet packet;
		while(vorbis_bitrate_flushpacket(&mVorbisState, &packet))
		{
			ogg_stream_packetin(&mOggState, &packet);

			// If new page, write it to the internal buffer
			ogg_page page;
			while(ogg_stream_flush(&mOggState, &page) > 0)
			{
				WRITE_TO_BUFFER(page.header, page.header_len);
				WRITE_TO_BUFFER(page.body, page.body_len);
			}
		}
	}
}

void OggVorbisEncoder::Flush()
{
	if(mBufferOffset > 0 && mWriteCallback != nullptr)
		mWriteCallback(mBuffer, mBufferOffset);

	mBufferOffset = 0;
}

void OggVorbisEncoder::Close()
{
	if(mClosed)
		return;

	// Mark end of data and flush any remaining data in the buffers
	vorbis_analysis_wrote(&mVorbisState, 0);
	WriteBlocks();
	Flush();

	ogg_stream_clear(&mOggState);
	vorbis_block_clear(&mVorbisBlock);
	vorbis_dsp_clear(&mVorbisState);
	vorbis_info_clear(&mVorbisInfo);

	mClosed = true;
}

SPtr<MemoryDataStream> OggVorbisEncoder::PCMToOggVorbis(u8* samples, const AudioDataInfo& info, u32& size)
{
	struct EncodedBlock
	{
		u8* Data;
		u32 Size;
	};

	Vector<EncodedBlock> blocks;
	u32 totalEncodedSize = 0;
	auto writeCallback = [&](u8* buffer, u32 size)
	{
		EncodedBlock newBlock;
		newBlock.Data = B3DFrameAllocate(size);
		newBlock.Size = size;

		memcpy(newBlock.Data, buffer, size);
		blocks.push_back(newBlock);
		totalEncodedSize += size;
	};

	B3DMarkAllocatorFrame();

	OggVorbisEncoder writer;
	writer.Open(writeCallback, info.SampleRate, info.BitDepth, info.ChannelCount);

	writer.Write(samples, info.SampleCount);
	writer.Close();

	auto output = B3DMakeShared<MemoryDataStream>(totalEncodedSize);
	u32 offset = 0;
	for(auto& block : blocks)
	{
		memcpy(output->Data() + offset, block.Data, block.Size);
		offset += block.Size;

		B3DFrameFree(block.Data);
	}

	B3DClearAllocatorFrame();

	size = totalEncodedSize;
	return output;
}

#undef WRITE_TO_BUFFER
