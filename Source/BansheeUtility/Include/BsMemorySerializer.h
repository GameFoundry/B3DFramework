//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"

namespace BansheeEngine
{
	/** @addtogroup Serialization
	 *  @{
	 */

	/**	Encodes/decodes an IReflectable object from/to memory. */
	class BS_UTILITY_EXPORT MemorySerializer
	{
		struct BufferPiece
		{
			UINT8* buffer;
			UINT32 size;
		};

	public:
		MemorySerializer();
		~MemorySerializer();

		/**
		 * Parses the provided object, serializes all of its data as specified by its RTTIType and returns the data in the 
		 * form of raw memory.
		 *
		 * @param[in]	object			Object to encode.
		 * @param[in]	bytesWritten	Output value containing the total number of bytes it took to encode the object.
		 * @param[in]	allocator		Determines how is memory allocated. If not specified the default allocator is used.
		 * @param[in]	shallow			Determines how to handle referenced objects. If true then references will not be 
		 *								encoded and will be set to null. If false then references will be encoded as well 
		 *								and restored upon decoding.
		 * @param[in]	params			Optional parameters to be passed to the serialization callbacks on the objects being
		 *								serialized.
		 *
		 * @return						A buffer containing the encoded object. It is up to the user to release the buffer 
		 *								memory when no longer needed.
		 */
		UINT8* encode(IReflectable* object, UINT32& bytesWritten, std::function<void*(UINT32)> allocator = nullptr, 
			bool shallow = false, const UnorderedMap<String, UINT64>& params = UnorderedMap<String, UINT64>());

		/** 
		 * Deserializes an IReflectable object by reading the binary data from the provided memory location. 
		 *
		 * @param[in]	buffer		Previously allocated buffer to store the data in.
		 * @param[in]	bufferSize	Size of the @p buffer in bytes.
		 * @param[in]	params		Optional parameters to be passed to the serialization callbacks on the objects being
		 *							serialized.
		 */
		SPtr<IReflectable> decode(UINT8* buffer, UINT32 bufferSize, 
			const UnorderedMap<String, UINT64>& params = UnorderedMap<String, UINT64>());

	private:
		Vector<BufferPiece> mBufferPieces;

		/** Called by the binary serializer whenever the buffer gets full. */
		UINT8* flushBuffer(UINT8* bufferStart, UINT32 bytesWritten, UINT32& newBufferSize);

		/************************************************************************/
		/* 								CONSTANTS	                     		*/
		/************************************************************************/
	private:
		static const UINT32 WRITE_BUFFER_SIZE = 16384;
	};

	/** @} */
}