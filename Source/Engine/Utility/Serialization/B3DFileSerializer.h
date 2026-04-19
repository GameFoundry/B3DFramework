//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIType.h"

namespace b3d
{
	/** @addtogroup Serialization
	 *  @{
	 */

	struct RTTIOperationContext;

	/** Encodes the provided object to the specified file using the RTTI system. */
	class B3D_EXPORT FileEncoder
	{
	public:
		FileEncoder(const Path& fileLocation);
		FileEncoder(const SPtr<DataStream>& stream);

		/**
		 * Parses the provided object, serializes all of its data as specified by its RTTIType and saves the serialized
		 * data to the provided file location.
		 *
		 * @param	object		Object to encode.
		 * @param	context		Optional object that will be passed along to all serialized objects through
		 *						their operation notify methods . Can be used for controlling serialization,
		 *						maintaining state or sharing information between objects during
		 *						serialization.
		 */
		void Encode(IReflectable* object, RTTIOperationContext& context);

		/** Overload of Encode(IReflectable*, RTTIOperationContext&) that uses a default-constructed context. */
		void Encode(IReflectable* object);

	private:
		SPtr<DataStream> mOutputStream;
	};

	/** Decodes objects from the specified file using the RTTI system. */
	class B3D_EXPORT FileDecoder
	{
	public:
		FileDecoder(const Path& fileLocation);
		FileDecoder(const SPtr<DataStream>& stream);

		/**
		 * Deserializes an IReflectable object by reading the binary data at the provided file location.
		 *
		 * @param	context		Optional object that will be passed along to all deserialized objects through
		 *						their deserialization callbacks. Can be used for controlling deserialization,
		 *						maintaining state or sharing information between objects during
		 *						deserialization.
		 */
		SPtr<IReflectable> Decode(RTTIOperationContext& context);

		/** Deserializes an IReflectable object by reading the binary data at the provided file location. */
		SPtr<IReflectable> Decode();

		/** Gets the size in bytes of the next object in the file. Returns 0 if no next object. */
		u32 GetSize() const;

		/** Skips over than object in the file. Calling decode() will decode the next object. */
		void Skip();

	private:
		SPtr<DataStream> mInputStream;
	};

	/** @} */
} // namespace b3d
