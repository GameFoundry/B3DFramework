//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFLACDecoder.h"
#include "BsDataStream.h"

namespace BansheeEngine
{
	FLAC__StreamDecoderReadStatus streamRead(const FLAC__StreamDecoder*, FLAC__byte buffer[], size_t* bytes, void* clientData)
	{
		FLACDecoderData* data = (FLACDecoderData*)(clientData);

		INT64 count = (INT64)data->stream->read(buffer, *bytes);
		if (count > 0)
		{
			*bytes = (size_t)count;
			return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
		}
		
		if (count == 0)
			return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
		
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
	}

	FLAC__StreamDecoderSeekStatus streamSeek(const FLAC__StreamDecoder*, FLAC__uint64 absoluteByteOffset, void* clientData)
	{
		FLACDecoderData* data = (FLACDecoderData*)(clientData);

		data->stream->seek(data->streamOffset + absoluteByteOffset);
		INT64 position = (INT64)(data->stream->tell() - data->streamOffset);
		if (position >= 0)
			return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
		else
			return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
	}

	FLAC__StreamDecoderTellStatus streamTell(const FLAC__StreamDecoder*, FLAC__uint64* absoluteByteOffset, void* clientData)
	{
		FLACDecoderData* data = (FLACDecoderData*)(clientData);

		INT64 position = (INT64)(data->stream->tell() - data->streamOffset);
		if (position >= 0)
		{
			*absoluteByteOffset = position;
			return FLAC__STREAM_DECODER_TELL_STATUS_OK;
		}
		else
		{
			return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
		}
	}

	FLAC__StreamDecoderLengthStatus streamLength(const FLAC__StreamDecoder*, FLAC__uint64* streamLength, void* clientData)
	{
		FLACDecoderData* data = (FLACDecoderData*)(clientData);

		*streamLength = data->stream->size() - data->streamOffset;
		return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
	}

	FLAC__bool streamEof(const FLAC__StreamDecoder*, void* clientData)
	{
		FLACDecoderData* data = (FLACDecoderData*)(clientData);

		return data->stream->eof();
	}

	FLAC__StreamDecoderWriteStatus streamWrite(const FLAC__StreamDecoder*, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* clientData)
	{
		FLACDecoderData* data = (FLACDecoderData*)(clientData);

		if (!data->output) // Seek
			return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;

		UINT32 bytesPerSample = data->info.bitDepth / 8;
		
		// If we received more data than we need, store it in the overflow buffer
		UINT32 frameSamples = frame->header.blocksize * frame->header.channels;
		if (data->samplesToRead < frameSamples)
		{
			UINT32 numExtraSamples = frameSamples - data->samplesToRead;
			UINT32 extraBytes = numExtraSamples * bytesPerSample;
			data->overflow.reserve(extraBytes);
		}

		assert(bytesPerSample <= 4);
		for (UINT32 i = 0; i < frame->header.blocksize; i++)
		{
			for (UINT32 j = 0; j < frame->header.channels; j++)
			{
				if (data->samplesToRead > 0)
				{
					memcpy(data->output, &buffer[j][i], bytesPerSample);
					
					data->output += bytesPerSample;
					data->samplesToRead--;
				}
				else
				{
					UINT8 sample[4];
					memcpy(sample, &buffer[j][i], bytesPerSample);

					for(UINT32 k = 0; k < bytesPerSample; k++)
						data->overflow.push_back(sample[k]);
				}
			}
		}

		return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
	}

	void streamMetadata(const FLAC__StreamDecoder*, const FLAC__StreamMetadata* meta, void* clientData)
	{
		FLACDecoderData* data = (FLACDecoderData*)(clientData);

		if (meta->type == FLAC__METADATA_TYPE_STREAMINFO)
		{
			data->info.numSamples = (UINT32)meta->data.stream_info.total_samples * meta->data.stream_info.channels;
			data->info.sampleRate = meta->data.stream_info.sample_rate;
			data->info.numChannels = meta->data.stream_info.channels;
			data->info.bitDepth = meta->data.stream_info.bits_per_sample;
		}
	}

	void streamError(const FLAC__StreamDecoder*, FLAC__StreamDecoderErrorStatus, void* clientData)
	{
		FLACDecoderData* data = (FLACDecoderData*)(clientData);
		data->error = true;
	}

	FLACDecoder::FLACDecoder()
		:mDecoder(nullptr)
	{ }

	FLACDecoder::~FLACDecoder()
	{
		close();
	}

	bool FLACDecoder::isValid(const SPtr<DataStream>& stream, UINT32 offset)
	{
		stream->seek(offset);

		FLAC__StreamDecoder* decoder = FLAC__stream_decoder_new();
		if (!decoder)
			return false;

		FLACDecoderData data;
		data.stream = stream;
		mData.streamOffset = offset;
		FLAC__stream_decoder_init_stream(decoder, &streamRead, &streamSeek, &streamTell, &streamLength, &streamEof, 
			&streamWrite, nullptr, &streamError, &data);

		bool valid = FLAC__stream_decoder_process_until_end_of_metadata(decoder) != 0;

		FLAC__stream_decoder_finish(decoder);
		FLAC__stream_decoder_delete(decoder);

		return valid && !data.error;
	}

	bool FLACDecoder::open(const SPtr<DataStream>& stream, AudioDataInfo& info, UINT32 offset)
	{
		if (stream == nullptr)
			return false;

		stream->seek(offset);

		mDecoder = FLAC__stream_decoder_new();
		if (mDecoder == nullptr)
		{
			LOGERR("Failed to open a FLAC file.");
			return false;
		}

		mData.stream = stream;
		mData.streamOffset = offset;
		FLAC__stream_decoder_init_stream(mDecoder, &streamRead, &streamSeek, &streamTell, &streamLength, &streamEof,
			&streamWrite, &streamMetadata, &streamError, &mData);

		if (!FLAC__stream_decoder_process_until_end_of_metadata(mDecoder))
		{
			close();
			LOGERR("Failed to open a FLAC file.");
			return false;
		}

		info = mData.info;

		return true;
	}

	void FLACDecoder::seek(UINT32 offset)
	{
		mData.output = nullptr;
		mData.samplesToRead = 0;
		mData.overflow.clear();

		FLAC__stream_decoder_seek_absolute(mDecoder, offset);
	}

	UINT32 FLACDecoder::read(UINT8* samples, UINT32 numSamples)
	{
		UINT32 overflowSize = (UINT32)mData.overflow.size();
		UINT32 overflowNumSamples = 0;
		
		UINT32 bytesPerSample = mData.info.bitDepth / 8;
		if (overflowSize > 0)
		{
			UINT32 sampleSize = numSamples * bytesPerSample;
			if (overflowSize > sampleSize)
			{
				std::copy(mData.overflow.begin(), mData.overflow.begin() + sampleSize, samples);
				mData.overflow.erase(mData.overflow.begin(), mData.overflow.begin() + sampleSize);

				return numSamples;
			}
			else
				std::copy(mData.overflow.begin(), mData.overflow.end(), samples);

			overflowNumSamples = overflowSize / bytesPerSample;
		}

		mData.output = samples + overflowSize;
		mData.samplesToRead = numSamples - overflowNumSamples;
		mData.overflow.clear();

		while (mData.samplesToRead > 0)
		{
			if (!FLAC__stream_decoder_process_single(mDecoder))
				break;

			if (FLAC__stream_decoder_get_state(mDecoder) == FLAC__STREAM_DECODER_END_OF_STREAM)
				break;
		}

		return numSamples - mData.samplesToRead;
	}

	void FLACDecoder::close()
	{
		if (mDecoder != nullptr)
		{
			FLAC__stream_decoder_finish(mDecoder);
			FLAC__stream_decoder_delete(mDecoder);
			mDecoder = nullptr;
		}
	}
}