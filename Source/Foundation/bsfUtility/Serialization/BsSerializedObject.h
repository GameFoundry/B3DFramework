//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsIReflectable.h"

namespace bs
{
	/** @addtogroup Serialization
	 *  @{
	 */

	/** Base class for all data types used in intermediate IReflectable object representation. */
	struct BS_UTILITY_EXPORT SerializedInstance : IReflectable
	{
		virtual ~SerializedInstance() = default;

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

	/** Contains data for a single field in a serialized object. */
	struct BS_UTILITY_EXPORT SerializedEntry : IReflectable
	{
		SerializedEntry() = default;

		UINT32 fieldId = 0;
		SPtr<SerializedInstance> serialized;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedEntryRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Contains a sub-set of fields of a SerializedObject for a single class in a class hierarchy. */
	struct BS_UTILITY_EXPORT SerializedSubObject : IReflectable
	{
		SerializedSubObject() = default;

		UINT32 typeId = 0;
		UnorderedMap<UINT32, SerializedEntry> entries;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedSubObjectRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** 
	 * Represents a serialized version of an IReflectable object. Data for all leaf fields will be serialized into raw
	 * memory but complex objects, their references and fields are available as their own serialized objects and can be
	 * iterated over, viewed, compared or modified. Serialized object can later be decoded back into a IReflectable object.
	 */
	struct BS_UTILITY_EXPORT SerializedObject : SerializedInstance
	{
		/** Returns the RTTI type ID for the most-derived class of this object. */
		UINT32 getRootTypeId() const;

		/** @copydoc SerializedInstance::clone */
		SPtr<SerializedInstance> clone(bool cloneData = true) override;

		/** Decodes the serialized object back into its original IReflectable object form. */
		SPtr<IReflectable> decode() const;

		/** 
		 * Serializes the provided object and returns its SerializedObject representation. 
		 * 
		 * @param[in]	obj			Object to serialize;
		 * @param[in]	shallow		If true then pointers to other IReflectable objects will not be followed. If false the
		 *							entire hierarchy will be serialized.
		 * @return					Serialized version of @p obj.
		 */
		static SPtr<SerializedObject> create(IReflectable& obj, bool shallow = false);

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
		SerializedField() = default;

		~SerializedField()
		{
			if (ownsMemory && value != nullptr)
				bs_free(value);
		}

		/** @copydoc SerializedInstance::clone */
		SPtr<SerializedInstance> clone(bool cloneData = true) override;

		UINT8* value = nullptr;
		UINT32 size = 0;
		bool ownsMemory = false;

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
		SerializedDataBlock() = default;

		/** @copydoc SerializedInstance::clone */
		SPtr<SerializedInstance> clone(bool cloneData = true) override;

		SPtr<DataStream> stream;
		UINT32 offset = 0;
		UINT32 size = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedDataBlockRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** A serialized value representing a single entry in an array. */
	struct BS_UTILITY_EXPORT SerializedArrayEntry : IReflectable
	{
		SerializedArrayEntry() = default;

		UINT32 index = 0;
		SPtr<SerializedInstance> serialized;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SerializedArrayEntryRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** A serialized array containing a list of all its entries. */
	struct BS_UTILITY_EXPORT SerializedArray : SerializedInstance
	{
		SerializedArray() = default;

		/** @copydoc SerializedInstance::clone */
		SPtr<SerializedInstance> clone(bool cloneData = true) override;

		UnorderedMap<UINT32, SerializedArrayEntry> entries;
		UINT32 numElements = 0;

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
