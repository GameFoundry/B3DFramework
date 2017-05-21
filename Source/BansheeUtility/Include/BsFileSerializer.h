//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Serialization
	 *  @{
	 */

	// TODO - Low priority. Eventually I'll want to generalize BinarySerializer to Serializer class, then I can make this class accept
	// a generic Serializer interface so it may write both binary, plain-text or some other form of data.

	/** Encodes the provided object to the specified file using the RTTI system. */
	
	class BS_UTILITY_EXPORT FileEncoder
	{
		public:
		FileEncoder(const Path& fileLocation);
		~FileEncoder();

		/**
		 * Parses the provided object, serializes all of its data as specified by its RTTIType and saves the serialized 
		 * data to the provided file location.
		 *
		 * @param[in]	object		Object to encode.
		 * @param[in]	params		Optional parameters to be passed to the serialization callbacks on the objects being
		 *							serialized.
		 */
		void encode(IReflectable* object, const UnorderedMap<String, UINT64>& params = UnorderedMap<String, UINT64>());

	private:
		/** Called by the binary serializer whenever the buffer gets full. */
		UINT8* flushBuffer(UINT8* bufferStart, UINT32 bytesWritten, UINT32& newBufferSize);

		std::ofstream mOutputStream;
		UINT8* mWriteBuffer;

		static const UINT32 WRITE_BUFFER_SIZE = 2048;
	};

	/** Decodes objects from the specified file using the RTTI system. */
	class BS_UTILITY_EXPORT FileDecoder
	{
	public:
		FileDecoder(const Path& fileLocation);

		/**	
		 * Deserializes an IReflectable object by reading the binary data at the provided file location. 
		 *
		 * @param[in]	params		Optional parameters to be passed to the serialization callbacks on the objects being
		 *							serialized.
		 */
		SPtr<IReflectable> decode(const UnorderedMap<String, UINT64>& params = UnorderedMap<String, UINT64>());

		/** Skips over than object in the file. Calling decode() will decode the next object. */
		void skip();

	private:
		SPtr<DataStream> mInputStream;
	};

	/** @} */
}