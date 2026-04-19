//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DIReflectable.h"

namespace b3d
{
	struct RTTIOperationContext;
}

namespace b3d
{
	/** @addtogroup Serialization
	 *  @{
	 */

	/** Helper to compare two ISerialized objects for equality, while also checking if they are null. */
	B3D_EXPORT bool Equals(const SPtr<ISerialized>& lhs, const SPtr<ISerialized>& rhs);

	/** Base class for all data types used in intermediate IReflectable object representation. */
	struct B3D_EXPORT ISerialized : IReflectable
	{
		virtual ~ISerialized() = default;

		/**
		 * Performs a deep clone of this object any any potential child objects.
		 *
		 * @param	cloneData	If true the data contained by the objects will be cloned as well, instead of just
		 *						meta-data. If false then both the original and the cloned instances will point to the
		 *						same instances of data. The original will retain data ownership and it will go out of
		 *						scope when the original does.
		 */
		virtual SPtr<ISerialized> Clone(bool cloneData = true) = 0;

		/** Calculates the hash value of the contained data. */
		virtual u64 CalculateHash() const = 0;

		/** Checks if this value matches the other provided value. */
		virtual bool Equals(const SPtr<ISerialized>& other) const = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ISerializedRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};
}

/** @cond STDLIB */

namespace std
{
	/** Hash value generator for SPtr<ISerialized>. */
	template <>
	struct hash<b3d::SPtr<b3d::ISerialized>>
	{
		size_t operator()(const b3d::SPtr<b3d::ISerialized>& value) const
		{
			if(value == nullptr)
				return 0;

			return value->CalculateHash();
		}
	};

	/** Equality operator for SPtr<ISerialized>. */
	template <>
	struct equal_to<b3d::SPtr<b3d::ISerialized>>
	{
		bool operator()(const b3d::SPtr<b3d::ISerialized>& lhs, const b3d::SPtr<b3d::ISerialized>& rhs) const
		{
			return b3d::Equals(lhs, rhs);
		}
	};
} // namespace std

/** @endcond */

namespace b3d
{
	/** Contains data for fields or container entries that are made up of more than one type (e.g. std::pair<K, V>). */
	struct B3D_EXPORT SerializedTuple : ISerialized
	{
		SerializedTuple() = default;

		SPtr<ISerialized> Clone(bool cloneData = true) override;
		u64 CalculateHash() const override;
		bool Equals(const SPtr<ISerialized>& other) const override;

		TInlineArray<SPtr<ISerialized>, 2> Values; /**< One value per type. */

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedTupleRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Contains data for a single field in a serialized object. */
	struct B3D_EXPORT SerializedField : IReflectable
	{
		SerializedField() = default;

		u32 FieldId = 0;
		SPtr<ISerialized> Value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedFieldRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Contains a sub-set of fields of a SerializedObject for a single class in a class hierarchy. */
	struct B3D_EXPORT SerializedSubObject : IReflectable
	{
		SerializedSubObject() = default;

		u32 TypeId = 0;
		UnorderedMap<u32, SerializedField> FieldEntries;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedSubObjectRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Flags used for controlling the serialization process when encoding an IReflectable to a  SerializedObject. */
	enum class SerializedObjectEncodeFlag
	{
		/** Pointers to other IReflectable objects will not be followed and they will not be serialized. */
		Shallow = 1 << 0,

		/** Only fields with the Replicable RTTI flag will be serialized. */
		ReplicableOnly = 1 << 1,

		/**
		 * Lets the system know what the object is being copied as part of delta generation.
		 * Serialization will skip RTTI fields with IgnoreInDeltaCopy flag set.
		 */
		IsDeltaCopy = 1 << 2,
	};

	using SerializedObjectEncodeFlags = Flags<SerializedObjectEncodeFlag>;
	B3D_FLAGS_OPERATORS(SerializedObjectEncodeFlag)

	/**
	 * Represents a serialized version of an IReflectable object. Data for all leaf fields will be serialized into raw
	 * memory but complex objects, their references and fields are available as their own serialized objects and can be
	 * iterated over, viewed, compared or modified. Serialized object can later be decoded back into a IReflectable object.
	 */
	struct B3D_EXPORT SerializedObject : ISerialized
	{
		/** Returns the RTTI type ID for the most-derived class of this object. */
		u32 GetRootTypeId() const;

		SPtr<ISerialized> Clone(bool cloneData = true) override;
		u64 CalculateHash() const override;
		bool Equals(const SPtr<ISerialized>& other) const override;

		/**
		 * Decodes the serialized object back into its original IReflectable object form.
		 *
		 * @param	context	Object that will be passed along to all RTTI type objects through their notify callbacks. Can be used for controlling
		 *					serialization, maintaining state or sharing information between objects during serialization.
		 */
		SPtr<IReflectable> Decode(RTTIOperationContext& context) const;

		/**
		 * Serializes the provided object and returns its SerializedObject representation.
		 *
		 * @param	object	Object to serialize.
		 * @param	flags	Flags used for controlling the serialization process.
		 * @return			Serialized version of @p object.
		 */
		static SPtr<SerializedObject> Create(IReflectable& object, SerializedObjectEncodeFlags flags = SerializedObjectEncodeFlags());

		Vector<SerializedSubObject> SubObjects;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedObjectRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Contains data for a serialized value of a specific field or array entry. */
	struct B3D_EXPORT SerializedPlainData : ISerialized
	{
		SerializedPlainData() = default;

		~SerializedPlainData()
		{
			if(OwnsMemory && Value != nullptr)
				B3DFree(Value);
		}

		SPtr<ISerialized> Clone(bool cloneData = true) override;
		u64 CalculateHash() const override;
		bool Equals(const SPtr<ISerialized>& other) const override;

		u8* Value = nullptr;
		u32 Size = 0;
		bool OwnsMemory = false;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedPlainDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Contains data for a serialized value of a data block field. */
	struct B3D_EXPORT SerializedDataBlock : ISerialized
	{
		SerializedDataBlock() = default;

		SPtr<ISerialized> Clone(bool cloneData = true) override;
		u64 CalculateHash() const override;
		bool Equals(const SPtr<ISerialized>& other) const override;

		SPtr<DataStream> Stream;
		u32 Offset = 0;
		u32 Size = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedDataBlockRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** A serialized array containing a list of all its entries. */
	struct B3D_EXPORT SerializedArray : ISerialized
	{
		SerializedArray() = default;

		SPtr<ISerialized> Clone(bool cloneData = true) override;
		u64 CalculateHash() const override;
		bool Equals(const SPtr<ISerialized>& other) const override;

		TArray<SPtr<ISerialized>> Entries;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedArrayRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** A serialized map containing a list of all its entries. */
	struct B3D_EXPORT SerializedMap : ISerialized
	{
		SerializedMap() = default;

		SPtr<ISerialized> Clone(bool cloneData = true) override;
		u64 CalculateHash() const override;
		bool Equals(const SPtr<ISerialized>& other) const override;

		UnorderedMap<SPtr<ISerialized>, SPtr<ISerialized>> Entries;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedMapRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
