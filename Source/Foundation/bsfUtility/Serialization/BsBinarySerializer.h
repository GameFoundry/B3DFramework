//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <unordered_map>

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Serialization/BsSerializedObject.h"
#include "Reflection/BsRTTIField.h"

namespace bs
{
	/** @addtogroup Serialization
	 *  @{
	 */

	class IReflectable;
	struct RTTIReflectableFieldBase;
	struct RTTIReflectablePtrFieldBase;

	// TODO - Low priority. I will probably want to extract a generalized Serializer class so we can re-use the code
	// in text or other serializers
	// TODO - Low priority. Encode does a chunk-based encode so that we don't need to know the buffer size in advance,
	// and don't have to use a lot of memory for the buffer. Consider doing something similar for decode.
	// TODO - Low priority. Add a simple encode method that doesn't require a callback, instead it calls the callback internally
	// and creates the buffer internally.
	/**
	 * Encodes all the fields of the provided object into a binary format. Fields are encoded using their unique IDs. 
	 * Encoded data will remain compatible for decoding even if you modify the encoded class, as long as you assign new 
	 * unique field IDs to added/modified fields.
	 * 			
	 * Like for any serializable class, fields are defined in RTTIType that each IReflectable class must be able to return.
	 *
	 * Any data the object or its children are pointing to will also be serialized (unless the pointer isn't registered in 
	 * RTTIType). Upon decoding the pointer addresses will be set to proper values.
	 * 			
	 * @note	
	 * Child elements are guaranteed to be fully deserialized before their parents, except for fields marked with WeakRef flag.
	 */
	class BS_UTILITY_EXPORT BinarySerializer
	{
	public:
		BinarySerializer();

		/**
		 * Encodes all serializable fields provided by @p object into a binary format. Data is written in chunks. Whenever a 
		 * chunk is filled a callback is triggered that gives the user opportunity to expand or empty the buffer (for 
		 * example write the chunk to disk)
		 *
		 * @param[in]	object					Object to encode into binary format.
		 * @param[out]	buffer					Preallocated buffer where the data will be stored.
		 * @param[in]	bufferLength			Length of the buffer, in bytes.
		 * @param[out]	bytesWritten			Length of the data that was actually written to the buffer, in bytes.
		 * @param[in]	flushBufferCallback 	This callback will get called whenever the buffer gets full (Be careful to 
		 *										check the provided @p bytesRead variable, as buffer might not be full 
		 *										completely). User must then either create a new buffer or empty the existing 
		 *										one, and then return it by the callback. If the returned buffer address is 
		 *										NULL, encoding is aborted.
		 * @param[in]	shallow					Determines how to handle referenced objects. If true then references will 
		 *										not be encoded and will be set to null. If false then references will be 
		 *										encoded as well and restored upon decoding.
		 * @param[in]	params					Optional parameters to be passed to the serialization callbacks on the
		 *										objects being serialized.
		 */
		void encode(IReflectable* object, UINT8* buffer, UINT32 bufferLength, UINT32* bytesWritten,
			std::function<UINT8*(UINT8* buffer, UINT32 bytesWritten, UINT32& newBufferSize)> flushBufferCallback,
			bool shallow = false, const UnorderedMap<String, UINT64>& params = UnorderedMap<String, UINT64>());

		/**
		 * Decodes an object from binary data.
		 *
		 * @param[in]	data  		Binary data to decode.
		 * @param[in]	dataLength	Length of the data in bytes.
		 * @param[in]	params		Optional parameters to be passed to the serialization callbacks on the objects being
		 *							serialized.
		 */
		SPtr<IReflectable> decode(const SPtr<DataStream>& data, UINT32 dataLength, 
			const UnorderedMap<String, UINT64>& params = UnorderedMap<String, UINT64>());

		/** @name Internal 
		 *  @{
		 */

		/**
		 * Encodes an object into an intermediate representation.
		 *
		 * @param[in]	object		Object to encode.
		 * @param[in]	shallow		Determines how to handle referenced objects. If true then references will not be encoded
		 *							and will be set to null. If false then references will be encoded as well and restored
		 *							upon decoding.
		 */
		SPtr<SerializedObject> _encodeToIntermediate(IReflectable* object, bool shallow = false);

		/**
		 * Decodes a serialized object into an intermediate representation for easier parsing.
		 *			
		 * @param[in] 	data  		Binary data to decode.
		 * @param[in]	dataLength	Length of the data in bytes.
		 * @param[in]	copyData	Determines should the data be copied or just referenced. If referenced then the returned
		 *							serialized object will be invalid as soon as the original data buffer is destroyed.
		 *							Referencing is faster than copying. If the source data stream is a file stream the data
		 *							will always be copied.
		 *
		 * @note
		 * References to field data will point to the original buffer and will become invalid when it is destroyed.
		 */
		SPtr<SerializedObject> _decodeToIntermediate(const SPtr<DataStream>& data, UINT32 dataLength, bool copyData = false);

		/** Decodes an intermediate representation of a serialized object into the actual object. */
		SPtr<IReflectable> _decodeFromIntermediate(const SPtr<SerializedObject>& serializedObject);

		/** @} */

	private:
		struct ObjectMetaData
		{
			UINT32 objectMeta;
			UINT32 typeId;
		};

		struct ObjectToEncode
		{
			ObjectToEncode(UINT32 _objectId, SPtr<IReflectable> _object)
				:objectId(_objectId), object(_object)
			{ }

			UINT32 objectId;
			SPtr<IReflectable> object;
		};

		struct ObjectToDecode
		{
			ObjectToDecode(const SPtr<IReflectable>& _object, const SPtr<SerializedObject>& serializedObject)
				:object(_object), serializedObject(serializedObject), isDecoded(false), decodeInProgress(false)
			{ }

			SPtr<IReflectable> object;
			SPtr<SerializedObject> serializedObject;
			bool isDecoded;
			bool decodeInProgress; // Used for error reporting circular references
		};

		/** Encodes a single IReflectable object. */
		UINT8* encodeEntry(IReflectable* object, UINT32 objectId, UINT8* buffer, UINT32& bufferLength, UINT32* bytesWritten,
			std::function<UINT8*(UINT8* buffer, UINT32 bytesWritten, UINT32& newBufferSize)> flushBufferCallback, bool shallow);

		/**	Decodes a single IReflectable object. */
		void decodeEntry(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& serializableObject);

		/**	Decodes an object in memory into an intermediate representation for easier parsing. */
		bool decodeEntry(const SPtr<DataStream>& data, UINT32 dataLength, UINT32& bytesRead, SPtr<SerializedObject>& output, 
			bool copyData, bool streamDataBlock);

		/**	Helper method for encoding a complex object and copying its data to a buffer. */
		UINT8* complexTypeToBuffer(IReflectable* object, UINT8* buffer, UINT32& bufferLength, UINT32* bytesWritten,
			std::function<UINT8*(UINT8* buffer, UINT32 bytesWritten, UINT32& newBufferSize)> flushBufferCallback, bool shallow);

		/**	Helper method for encoding a data block to a buffer. */
		UINT8* dataBlockToBuffer(UINT8* data, UINT32 size, UINT8* buffer, UINT32& bufferLength, UINT32* bytesWritten,
			std::function<UINT8*(UINT8* buffer, UINT32 bytesWritten, UINT32& newBufferSize)> flushBufferCallback);

		/**	Finds an existing, or creates a unique unique identifier for the specified object. */
		UINT32 findOrCreatePersistentId(IReflectable* object);

		/**
		 * Finds or creates an id for the provided object and returns it. And it adds the object to a list of objects that 
		 * need to be encoded, if it's not already there.
		 */
		UINT32 registerObjectPtr(SPtr<IReflectable> object);

		/** Encodes data required for representing a serialized field, into 4 bytes. */
		static UINT32 encodeFieldMetaData(UINT16 id, UINT8 size, bool array, 
			SerializableFieldType type, bool hasDynamicSize, bool terminator);

		/** Decode meta field that was encoded using encodeFieldMetaData().*/
		static void decodeFieldMetaData(UINT32 encodedData, UINT16& id, UINT8& size, bool& array, 
			SerializableFieldType& type, bool& hasDynamicSize, bool& terminator);

		/**
		 * Encodes data required for representing an object identifier, into 8 bytes.
		 *
		 * @param[in]	objId	   	Unique ID of the object instance.
		 * @param[in]	objTypeId  	Unique ID of the object type.
		 * @param[in]	isBaseClass	true if this object is base class (that is, just a part of a larger object).
		 *
		 * @note		Id can be a maximum of 30 bits, as two bits are reserved.
		 */
		static ObjectMetaData encodeObjectMetaData(UINT32 objId, UINT32 objTypeId, bool isBaseClass);

		/** Decode meta field that was encoded using encodeObjectMetaData. */
		static void decodeObjectMetaData(ObjectMetaData encodedData, UINT32& objId, UINT32& objTypeId, bool& isBaseClass);

		/** Returns true if the provided encoded meta data represents object meta data. */
		static bool isObjectMetaData(UINT32 encodedData);

		UnorderedMap<void*, UINT32> mObjectAddrToId;
		UINT32 mLastUsedObjectId = 1;
		Vector<ObjectToEncode> mObjectsToEncode;
		UINT32 mTotalBytesWritten;
		FrameAlloc* mAlloc = nullptr;

		UnorderedMap<SPtr<SerializedObject>, ObjectToDecode> mObjectMap;
		UnorderedMap<UINT32, SPtr<SerializedObject>> mInterimObjectMap;

		UnorderedMap<String, UINT64> mParams;

		static constexpr const int META_SIZE = 4; // Meta field size
		static constexpr const int NUM_ELEM_FIELD_SIZE = 4; // Size of the field storing number of array elements
		static constexpr const int COMPLEX_TYPE_FIELD_SIZE = 4; // Size of the field storing the size of a child complex type
		static constexpr const int DATA_BLOCK_TYPE_FIELD_SIZE = 4;
	};

	/** @} */
}
