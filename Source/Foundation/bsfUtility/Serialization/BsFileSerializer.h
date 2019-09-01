//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Serialization
	 *  @{
	 */

	struct SerializationContext;

	/** Encodes the provided object to the specified file using the RTTI system. */
	class BS_UTILITY_EXPORT FileEncoder
	{
	public:
		FileEncoder(const Path& fileLocation);

		/**
		 * Parses the provided object, serializes all of its data as specified by its RTTIType and saves the serialized
		 * data to the provided file location.
		 *
		 * @param[in]	object		Object to encode.
		 * @param[in]	context		Optional object that will be passed along to all serialized objects through
		 *							their serialization callbacks. Can be used for controlling serialization,
		 *							maintaining state or sharing information between objects during
		 *							serialization.
		 */
		void encode(IReflectable* object, SerializationContext* context = nullptr);

	private:
		SPtr<DataStream> mOutputStream;
	};

	/** Decodes objects from the specified file using the RTTI system. */
	class BS_UTILITY_EXPORT FileDecoder
	{
	public:
		FileDecoder(const Path& fileLocation);

		/**	
		 * Deserializes an IReflectable object by reading the binary data at the provided file location.
		 *
		 * @param[in]	context		Optional object that will be passed along to all deserialized objects through
		 *							their deserialization callbacks. Can be used for controlling deserialization,
		 *							maintaining state or sharing information between objects during
		 *							deserialization.
		 */
		SPtr<IReflectable> decode(SerializationContext* context = nullptr);

		/** Gets the size in bytes of the next object in the file. Returns 0 if no next object. */
		UINT32 getSize() const;

		/** Skips over than object in the file. Calling decode() will decode the next object. */
		void skip();

	private:
		SPtr<DataStream> mInputStream;
	};

	/** @} */
}
