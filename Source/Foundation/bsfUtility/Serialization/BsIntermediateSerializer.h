//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsIReflectable.h"
#include "BsSerializedObject.h"

namespace bs
{
	struct SerializationContext;

	/** @addtogroup Serialization
	 *  @{
	 */

	/** Helper class for performing SerializedObject <-> IReflectable encoding & decoding. */
	class BS_UTILITY_EXPORT IntermediateSerializer
	{
	public:
		IntermediateSerializer();

		/** Encodes an IReflectable object into an intermediate representation. */
		SPtr<SerializedObject> encode(IReflectable* object, SerializedObjectEncodeFlags flags,
			SerializationContext* context = nullptr);

		/** Decodes an intermediate representation of a serialized object into the actual object. */
		SPtr<IReflectable> decode(const SerializedObject* serializedObject,
			SerializationContext* context = nullptr);

		/**
		 * @name Internal
		 * @{
		 */

		/**
		 * Encodes a single field from a reflectable object into a SerializedInstance. If a field is an array and
		 * @p arrayIdx is -1 then the entire array will be encoded, otherwise just a single array field will. If the
		 * field is not array the value of @p arrayIdx is not relevant.
		 */
		static SPtr<SerializedInstance> _encodeField(IReflectable* object, RTTITypeBase* rtti, RTTIField* field, UINT32 arrayIdx,
			SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAlloc* alloc);

		/** @} */
	private:
		friend class BinaryDiff;
		
		struct ObjectToDecode
		{
			ObjectToDecode(const SPtr<IReflectable>& _object, const SerializedObject* serializedObject)
				:object(_object), serializedObject(serializedObject)
			{ }

			SPtr<IReflectable> object;
			const SerializedObject* serializedObject;
			bool isDecoded = false;
			bool decodeInProgress = false; // Used for error reporting circular references
		};

		/**	Decodes a single IReflectable object. */
		void decodeEntry(const SPtr<IReflectable>& object, const SerializedObject* serializableObject);

		/** Encodes a single IReflectable object. */
		static SPtr<SerializedObject> encodeEntry(IReflectable* object, SerializedObjectEncodeFlags flags,
			SerializationContext* context, FrameAlloc* alloc);

		UnorderedMap<const SerializedObject*, ObjectToDecode> mObjectMap;
		SerializationContext* mContext = nullptr;
		FrameAlloc* mAlloc = nullptr;
	};

	/** @} */
}
