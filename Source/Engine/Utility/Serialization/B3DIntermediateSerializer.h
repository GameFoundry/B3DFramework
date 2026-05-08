//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DIReflectable.h"
#include "B3DSerializedObject.h"

namespace b3d
{
	class IRTTIIterator;
	struct RTTIIteratorField;
	struct RTTIOperationContext;

	/** @addtogroup Serialization
	 *  @{
	 */

	/** Helper class for performing SerializedObject <-> IReflectable encoding & decoding. */
	class B3D_EXPORT IntermediateSerializer
	{
	public:
		IntermediateSerializer(FrameAllocator* allocator, RTTIOperationContext& context);

		/** Encodes an IReflectable object into an intermediate representation. */
		TShared<SerializedObject> Encode(IReflectable* object, SerializedObjectEncodeFlags flags);

		/** Decodes an intermediate representation of a serialized object into the actual object. */
		TShared<IReflectable> Decode(const SerializedObject* serializedObject);

		/**
		 * @name Internal
		 * @{
		 */

		/**
		 * Serializes an entire field from the provided reflectable object. Field must support RTTI iterators.
		 *
		 * @param	object				Object that contains the field to serialize.
		 * @param	rttiInstance		Type information describing @p object.
		 * @param	field				Field to serialize.
		 * @param	flags				Flags controlling the serialization process.
		 * @return						Serialized field, or null if the field data is null.
		 */
		TShared<ISerialized> SerializeIterableField(IReflectable& object, RTTIType& rttiInstance, RTTIIteratorField& field, SerializedObjectEncodeFlags flags);

		/** Serializes a data block field entry from a reflectable object into a SerializedInstance. */
		TShared<ISerialized> SerializeDataBlockField(IReflectable* object, RTTIType* rtti, RTTIField* field, SerializedObjectEncodeFlags flags);

		/**
		 * Serializes an element at the provided iterator location.
		 *
		 * @param	object				Object that contains the field to serialize.
		 * @param	rttiInstance		Type information describing @p object.
		 * @param	field				Field from which the element is being serialized.
		 * @param	iterator			Iterator pointing to the location of the element to serialize.
		 * @param	flags				Flags controlling the serialization process.
		 * @return						Serialized element, or null if the source element is null.
		 */
		TShared<ISerialized> SerializeElement(IReflectable& object, RTTIType& rttiInstance, RTTIIteratorField& field, IRTTIIterator& iterator, SerializedObjectEncodeFlags flags);

		/**
		 * Serializes a single tuple element from the provided iterator location.
		 *
		 * @param	object				Object that contains the field to serialize.
		 * @param	rttiInstance		Type information describing @p object.
		 * @param	field				Field from which the element is being serialized.
		 * @param	iterator			Iterator pointing to the location of the element to serialize.
		 * @param	tupleElementIndex	Tuple index to serialize. e.g. if element is of std::pair<K, V> type, index 0 would represent K and index 1 would represent V. Should be 0 if the iterator element is not a tuple type.
		 * @param	flags				Flags controlling the serialization process.
		 * @return						Serialized tuple element, or null if the source element is null.
		 */
		TShared<ISerialized> SerializeTupleElement(IReflectable& object, RTTIType& rttiInstance, RTTIIteratorField& field, IRTTIIterator& iterator, u32 tupleElementIndex, SerializedObjectEncodeFlags flags);

		/** Deserializes a single field, array or map entry from the provided field, at the provided iterator location. */
		void DeserializeElement(RTTIType& rttiInstance, const TShared<IReflectable>& object, RTTIIteratorField& field, const TShared<IRTTIIterator>& iterator, const TShared<ISerialized>& entry);

		/** Similar to DeserializeElement overload accepting an iterator, except accepts a pre-allocated output field value in which to write to. */
		void DeserializeElement(RTTIIteratorField& field, void* outFieldValue, const TShared<ISerialized>& entry);

		/** Decodes @p entry and writes it into @p outFieldValue at the specified tuple element index. */
		void DeserializeTupleElement(RTTIIteratorField& field, void* outFieldValue, u32 tupleElementIndex, const TShared<ISerialized>& entry);

		/** @} */
	private:
		friend class BinaryDeltaHandler;

		struct ObjectDeserializationData
		{
			ObjectDeserializationData(const TShared<IReflectable>& object, const SerializedObject* serializedObject)
				: Object(object), SerializedObject(serializedObject)
			{}

			TShared<IReflectable> Object;
			const SerializedObject* SerializedObject;
			bool IsDeserialized = false;
			bool DeserializationInProgress = false; // Used for error reporting circular references
		};

		/**	Deserializes a single IReflectable object. */
		void DeserializeReflectableObject(const TShared<IReflectable>& object, const SerializedObject* serializableObject);

		/** Attempts to retrieve a previously deserialized reflectable object from the deserialized object map. If not present, deserializes the object and adds it to the deserializes object map, and returns the new object. */
		TShared<IReflectable> GetOrDeserializeReflectableObject(const TShared<SerializedObject>& serializedObject);

		/** Attempts to retrieve a previously deserialized reflectable object. */
		TShared<IReflectable> GetReflectableObject(const TShared<SerializedObject>& serializedObject);

		/** Serializes a single IReflectable object. */
		TShared<SerializedObject> SerializeReflectableObject(const IReflectable& object, SerializedObjectEncodeFlags flags);

		/** Attempts to retrieve a previously serialized object for the provided reflectable object, or if not found, serializes the object. */
		TShared<SerializedObject> GetOrSerializeReflectableObject(const IReflectable& object, SerializedObjectEncodeFlags flags);

		UnorderedMap<const SerializedObject*, ObjectDeserializationData> mDeserializedObjectMap;
		UnorderedMap<const IReflectable*, TShared<SerializedObject>> mSerializedObjectMap;
		RTTIOperationContext& mContext;
		FrameAllocator* mAllocator = nullptr;
	};

	/** @} */
} // namespace b3d
