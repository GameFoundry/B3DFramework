//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsIReflectable.h"
#include "BsSerializedObject.h"

namespace bs
{
	class IRTTIIterator;
	struct RTTIIteratorField;
	struct SerializationContext;

	/** @addtogroup Serialization
	 *  @{
	 */

	/** Helper class for performing SerializedObject <-> IReflectable encoding & decoding. */
	class B3D_UTILITY_EXPORT IntermediateSerializer
	{
	public:
		IntermediateSerializer();

		/** Encodes an IReflectable object into an intermediate representation. */
		SPtr<SerializedObject> Encode(IReflectable* object, SerializedObjectEncodeFlags flags, SerializationContext* context = nullptr);

		/** Decodes an intermediate representation of a serialized object into the actual object. */
		SPtr<IReflectable> Decode(const SerializedObject* serializedObject, SerializationContext* context = nullptr);

		/**
		 * @name Internal
		 * @{
		 */

		/**
		 * Serializes a single field entry from a reflectable object into a SerializedInstance. If a field is an array and
		 * @p arrayIdx is -1 then the entire array will be encoded, otherwise just a single array field will. If the
		 * field is not array the value of @p arrayIdx is not relevant.
		 */
		static SPtr<ISerialized> SerializeField(IReflectable* object, RTTITypeBase* rtti, RTTIField* field, u32 arrayIdx, SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAllocator* alloc); // DEPRECATED

		/**
		 * Serializes a the entire field from the provided reflectable object.
		 *
		 * @param object				Object that contains the field to serialize.
		 * @param rttiType				Type information describing @p object.
		 * @param field					Field to serialize.
		 * @param flags					Flags controlling the serialization process.
		 * @param context				Context that gives more information about the serialization process, and allows the user to store his own.
		 * @param allocator				Allocator to perform temporary allocations with.
		 * @return						Serialized field, or null if the field data is null.
		 */
		static SPtr<ISerialized> SerializeField(IReflectable& object, RTTITypeBase& rttiType, RTTIIteratorField& field, SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAllocator& allocator);

		/**
		 * Serializes a single tuple element from the provided iterator location. 
		 *
		 * @param field					Field from which the element is being serialized.
		 * @param iterator				Iterator pointing to the location of the element to serialize.
		 * @param tupleElementIndex		Tuple index to serialize. e.g. if element is of std::pair<K, V> type, index 0 would represent K and index 1 would represent V. Should be 0 if the iterator element is not a tuple type.
		 * @param flags					Flags controlling the serialization process.
		 * @param context				Context that gives more information about the serialization process, and allows the user to store his own.
		 * @param allocator				Allocator to perform temporary allocations with.
		 * @return						Serialized tuple element, or null if the source element is null.
		 */
		static SPtr<ISerialized> SerializeTupleElement(RTTIIteratorField& field, IRTTIIterator& iterator, u32 tupleElementIndex, SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAllocator& allocator);

		/** @} */
	private:
		friend class BinaryDiff;

		struct ObjectDeserializationData
		{
			ObjectDeserializationData(const SPtr<IReflectable>& object, const SerializedObject* serializedObject)
				: Object(object), SerializedObject(serializedObject)
			{}

			SPtr<IReflectable> Object;
			const SerializedObject* SerializedObject;
			bool IsDeserialized = false;
			bool DeserializationInProgress = false; // Used for error reporting circular references
		};

		/**	Deserializes a single IReflectable object. */
		void DeserializeReflectableObject(const SPtr<IReflectable>& object, const SerializedObject* serializableObject);

		/** Attempts to retrieve a previously deserialized reflectable object from the deserialized object map. If not present, deserializes the object and adds it to the deserializes object map, and returns the new object. */
		SPtr<IReflectable> GetOrDeserializeReflectableObject(const SPtr<SerializedObject>& serializedObject);

		/** Attempts to retrieve a previously deserialized reflectable object. */
		SPtr<IReflectable> GetReflectableObject(const SPtr<SerializedObject>& serializedObject);

		/** Serializes a single IReflectable object. */
		static SPtr<SerializedObject> SerializeReflectableObject(const IReflectable& object, SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAllocator& allocator);

		UnorderedMap<const SerializedObject*, ObjectDeserializationData> mDeserializedObjectMap;
		SerializationContext* mContext = nullptr;
		FrameAllocator* mAllocator = nullptr;
	};

	/** @} */
} // namespace bs
