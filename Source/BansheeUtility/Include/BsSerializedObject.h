//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsIReflectable.h"

namespace BansheeEngine
{
	/** @addtogroup Serialization
	 *  @{
	 */

	/** Base class for intermediate representations of objects that are being decoded with BinarySerializer. */
	struct BS_UTILITY_EXPORT SerializedInstance : IReflectable
	{
		virtual ~SerializedInstance() { }

		/**
		 * Performs a deep clone of this object any any potential child objects.
		 * 			
		 * @param[in]	cloneData	If true the data contained by the objects will be cloned as well, instead of just 
		 *							meta-data. If false then both the original and the cloned instances will point to the 
		 *							same instances of data. The original will retain data ownership and it will go out of 
		 *							scope when the original does.
		 */
		virtual SPtr<SerializedInstance> clone(bool cloneData = true) = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedInstanceRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** An intermediate serialized data for a single field in an object. */
	struct BS_UTILITY_EXPORT SerializedEntry : IReflectable
	{
		SerializedEntry()
			:fieldId(0), serialized(nullptr)
		{ }

		UINT32 fieldId;
		SPtr<SerializedInstance> serialized;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedEntryRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** A serialized value representing a single entry in an array. */
	struct BS_UTILITY_EXPORT SerializedArrayEntry : IReflectable
	{
		SerializedArrayEntry()
			:index(0), serialized(nullptr)
		{ }

		UINT32 index;
		SPtr<SerializedInstance> serialized;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedArrayEntryRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * A serialized portion of an object belonging to a specific class in a class hierarchy. Consists of multiple entries,
	 * one for each field.
	 */
	struct BS_UTILITY_EXPORT SerializedSubObject : IReflectable
	{
		SerializedSubObject()
			:typeId(0)
		{ }

		UINT32 typeId;
		UnorderedMap<UINT32, SerializedEntry> entries;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedSubObjectRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** A serialized object consisting of multiple sub-objects, one for each inherited class. */
	struct BS_UTILITY_EXPORT SerializedObject : SerializedInstance
	{
		/** Returns the RTTI type ID for the most-derived class of this object. */
		UINT32 getRootTypeId() const;

		/** @copydoc SerializedInstance::clone */
		SPtr<SerializedInstance> clone(bool cloneData = true) override;

		Vector<SerializedSubObject> subObjects;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedObjectRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Contains data for a serialized value of a specific field or array entry. */
	struct BS_UTILITY_EXPORT SerializedField : SerializedInstance
	{
		SerializedField()
			:value(nullptr), size(0), ownsMemory(false)
		{

		}

		~SerializedField()
		{
			if (ownsMemory && value != nullptr)
				bs_free(value);
		}

		/** @copydoc SerializedInstance::clone */
		SPtr<SerializedInstance> clone(bool cloneData = true) override;

		UINT8* value;
		UINT32 size;
		bool ownsMemory;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedFieldRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Contains data for a serialized value of a data block field. */
	struct BS_UTILITY_EXPORT SerializedDataBlock : SerializedInstance
	{
		SerializedDataBlock()
			:offset(0), size(0)
		{

		}

		/** @copydoc SerializedInstance::clone */
		SPtr<SerializedInstance> clone(bool cloneData = true) override;

		SPtr<DataStream> stream;
		UINT32 offset;
		UINT32 size;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedDataBlockRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** A serialized array containing a list of all its entries. */
	struct BS_UTILITY_EXPORT SerializedArray : SerializedInstance
	{
		SerializedArray()
			:numElements(0)
		{ }

		/** @copydoc SerializedInstance::clone */
		SPtr<SerializedInstance> clone(bool cloneData = true) override;

		UnorderedMap<UINT32, SerializedArrayEntry> entries;
		UINT32 numElements;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedArrayRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}