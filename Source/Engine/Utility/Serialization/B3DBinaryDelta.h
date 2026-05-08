//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "B3DSerializedObject.h"

namespace b3d
{
	struct RTTIOperationContext;
	class IRTTIIterator;
	struct RTTIIteratorField;
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Serialization-Internal
	 *  @{
	 */

	/** Represents an interface RTTI objects need to implement if they want to provide custom delta generation and applying. */
	class B3D_EXPORT IDeltaHandler
	{
	public:
		virtual ~IDeltaHandler() = default;

		/**
		 * Generates differences between the provided original and new object. Each field is compared with a matching field on the other object.
		 * If the field references a container (e.g. an array or a map), then delta is recorded only for entries that don't match (and not the entire container).
		 *
		 * Both provided objects must of the same type. 
		 *
		 * If @p replicableOnly true then only fields with replication enabled will be evaluated and all others will be
		 * ignored.
		 *
		 * Will return null if there is no difference.
		 */
		TShared<SerializedObject> GenerateDelta(const TShared<IReflectable>& original, const TShared<IReflectable>& modified, RTTIOperationContext& context, bool replicableOnly = false);

		/**
		 * Applies a previously generated delta to the provided object. This will essentially transform the
		 * original object the differences were generated for into the modified version.
		 */
		void ApplyDelta(const TShared<IReflectable>& object, const TShared<SerializedObject>& delta, RTTIOperationContext& context);

		/**
		 * @name Internal
		 * @{
		 */

		typedef UnorderedMap<IReflectable*, TShared<SerializedObject>> ObjectMap;

		/**
		 * Recursive version of GenerateDelta(const TShared<IReflectable>&, const TShared<IReflectable>&, bool).
		 *
		 * @see		GenerateDelta(const TShared<IReflectable>&, const TShared<IReflectable>&, SerializationContext*, bool)
		 */
		virtual TShared<SerializedObject> GenerateDeltaRecursive(IReflectable* original, IReflectable* modified, ObjectMap& objectMap, RTTIOperationContext& context, bool replicableOnly) = 0;

		/** @} */

	protected:
		typedef UnorderedMap<TShared<SerializedObject>, TShared<IReflectable>> DeltaObjectMap;

		/** Types of commands that are used when applying difference field values. */
		enum DeltaCommandType
		{
			Diff_Plain = 0x01,
			Diff_Reflectable = 0x02,
			Diff_ReflectablePtr = 0x03,
			Diff_DataBlock = 0x04,
			Diff_ArraySize = 0x05,
			Diff_ObjectStart = 0x06,
			Diff_ObjectEnd = 0x07,
			Diff_SubObjectStart = 0x08,
			Diff_IterableEntryStart = 0x09,
			Diff_IterableEntryEnd = 0x0A,
			Diff_RemoveMapEntry = 0x0B,
			Diff_ArrayFlag = 0x10,
			Diff_MapFlag = 0x20
		};

		/**
		 * A command that is used for delaying writing to an object, it contains all necessary information for setting RTTI
		 * field values on an object.
		 */
		struct DeltaCommand
		{
			RTTIField* Field;
			u32 Type;
			TShared<IReflectable> Object;
			u8* Value;
			TShared<DataStream> StreamValue;
			u32 Size;
			u32 TupleElementIndex = 0;

			union
			{
				u32 ArrayIndex;
				u32 ArraySize;
				RTTIType* RttiType;
				void* MapKey;
			};
		};

		/**
		 * Generates a set of commands that determine operations that need to be performed on @p object in order to apply all the changes from @p delta. Commands
		 * are output in @p inOutDeltaCommands.
		 */
		virtual void GenerateDeltaApplyCommands(const TShared<IReflectable>& object, const TShared<SerializedObject>& delta, FrameAllocator& allocator, DeltaObjectMap& objectMap, FrameVector<DeltaCommand>& inOutDeltaCommands, RTTIOperationContext& context) = 0;

		/** Retrieves the appropriate IDeltaHandler from the provided object and calls the other GenerateDeltaApplyCommands overload. */
		void GenerateDeltaApplyCommands(RTTIType* rtti, const TShared<IReflectable>& object, const TShared<SerializedObject>& delta, FrameAllocator& allocator, DeltaObjectMap& objectMap, FrameVector<DeltaCommand>& inOutDeltaCommands, RTTIOperationContext& context);
	};

	/**
	 * Generates and applies delta (differences) between two objects. This handler is to be used for all native types, while a separate IDeltaHandler is to be provided
	 * for script types.
	 *
	 * Provided object may be any IReflectable object, and special handling is done to also natively support delta between SerializedObject types.
	 */
	class B3D_EXPORT BinaryDeltaHandler : public IDeltaHandler
	{
	protected:
		TShared<SerializedObject> GenerateDeltaRecursive(IReflectable* original, IReflectable* modified, ObjectMap& objectMap, RTTIOperationContext& context, bool replicableOnly) override;
		void GenerateDeltaApplyCommands(const TShared<IReflectable>& object, const TShared<SerializedObject>& delta, FrameAllocator& allocator, DeltaObjectMap& objectMap, FrameVector<DeltaCommand>& inOutDeltaCommands, RTTIOperationContext& context) override;

		/**
		 * Generates delta commands for a single field entry (e.g. a single array or map entry, or the entire field if not a container).
		 *
		 * @param rttiInstance		RTTIType instance for the current object.
		 * @param object			Object that contains the field we're applying the delta to.
		 * @param field				Field to which to apply the delta to.
		 * @param entryDelta		Object containing the delta value to apply.
		 * @param arrayIndex		Optional array index, if the entry we're applying the value to is part of an array. Set to ~0u if not an array.
		 * @param mapKey			Optional map key, if the entry we're applying the value to is part of a map. Set to null if not a map.
		 * @param inOutObjectMap	Map that contains any deserialized objects so far, and into which new deserialized objects will be inserted.
		 * @param outCommands		List of generated commands into which to output the commands.
		 * @param context			Serialization context.
		 * @param allocator			Allocator to perform temporary allocations with.
		 */
		void GenerateDeltaCommandForFieldEntry(RTTIType* rttiInstance, const TShared<IReflectable>& object, RTTIIteratorField& field, const TShared<ISerialized>& entryDelta, u32 arrayIndex, void* mapKey, DeltaObjectMap& inOutObjectMap, FrameVector<DeltaCommand>& outCommands, RTTIOperationContext& context, FrameAllocator& allocator);

		/**
		 * Generates delta commands for a data block field.
		 *
		 * @param field				Field to which to apply the delta to.
		 * @param entryDelta		Object containing the delta value to apply.
		 * @param outCommands		List of generated commands into which to output the commands.
		 */
		void GenerateDeltaCommandForDataBlockField(RTTIField& field, const TShared<ISerialized>& entryDelta, FrameVector<DeltaCommand>& outCommands);
	};

	/** Holds a single tuple element entry in SerializedTupleDelta. */
	struct B3D_EXPORT SerializedTupleEntryDelta : IReflectable
	{
		SerializedTupleEntryDelta() = default;

		u32 Index = 0; /**< Index of the tuple element. */
		TShared<ISerialized> Value; /**< Delta of the tuple element. */

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedTupleEntryDeltaRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Contains a delta between two tuples. Only different tuple elements are stored in the delta. */
	struct B3D_EXPORT SerializedTupleDelta : ISerialized
	{
		SerializedTupleDelta() = default;

		TShared<ISerialized> Clone(bool cloneData = true) override;
		u64 CalculateHash() const override;
		bool Equals(const TShared<ISerialized>& other) const override;

		TShared<ISerialized> Key;
		TInlineArray<SerializedTupleEntryDelta, 2> Values;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedTupleDeltaRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Holds a single array element entry in SerializedArrayDelta. */
	struct B3D_EXPORT SerializedArrayEntryDelta : IReflectable
	{
		SerializedArrayEntryDelta() = default;

		u32 Index = 0;
		TShared<ISerialized> Value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedArrayEntryDeltaRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Contains a delta between two arrays. Only different array elements are stored in the delta. */
	struct B3D_EXPORT SerializedArrayDelta : ISerialized
	{
		SerializedArrayDelta() = default;

		TShared<ISerialized> Clone(bool cloneData = true) override;
		u64 CalculateHash() const override;
		bool Equals(const TShared<ISerialized>& other) const override;

		UnorderedMap<u32, SerializedArrayEntryDelta> Entries;
		u32 ElementCount = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedArrayDeltaRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Holds a single array element entry in SerializedMapDelta. */
	struct B3D_EXPORT SerializedMapEntryDelta : IReflectable
	{
		SerializedMapEntryDelta() = default;

		TShared<ISerialized> Value;
		bool IsRemoved = false; /**< Will be set if the entry doesn't exist in the modified object. */

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedMapEntryDeltaRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Contains a delta between two maps. Only different and removed array elements are stored in the delta. */
	struct B3D_EXPORT SerializedMapDelta : ISerialized
	{
		SerializedMapDelta() = default;

		TShared<ISerialized> Clone(bool cloneData = true) override;
		u64 CalculateHash() const override;
		bool Equals(const TShared<ISerialized>& other) const override;

		UnorderedMap<TShared<ISerialized>, SerializedMapEntryDelta> Entries;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedMapDeltaRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
	/** @} */
} // namespace b3d
