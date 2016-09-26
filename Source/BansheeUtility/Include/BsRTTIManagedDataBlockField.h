//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsRTTIField.h"

namespace BansheeEngine
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	/**
	 * Base class containing common functionality for a managed data block class field. 
	 * 			
	 * @note	
	 * Managed data blocks are just blocks of memory that may, or may not be released automatically when they are no longer 
	 * referenced. They are useful when wanting to return some temporary data only for serialization purposes.
	 */
	struct RTTIManagedDataBlockFieldBase : public RTTIField
	{
		/** Retrieves a managed data block from the specified instance. */
		virtual SPtr<DataStream> getValue(void* object, UINT32& size) = 0;

		/** Sets a managed data block on the specified instance. */
		virtual void setValue(void* object, const SPtr<DataStream>& data, UINT32 size) = 0;
	};

	/** Class containing a managed data block field containing a specific type. */
	template <class DataType, class ObjectType>
	struct RTTIManagedDataBlockField : public RTTIManagedDataBlockFieldBase
	{
		/**
		 * Initializes a field that returns a block of bytes. Can be used for serializing pretty much anything.
		 *
		 * @param[in]	name			Name of the field.
		 * @param[in]	uniqueId		Unique identifier for this field. Although name is also a unique identifier we want a 
		 *								small data type that can be used for efficiently serializing data to disk and similar. 
		 *								It is primarily used for compatibility between different versions of serialized data.
		 * @param[in]	getter  		The getter method for the field. Must be a specific signature: SerializableDataBlock(ObjectType*)
		 * @param[in]	setter  		The setter method for the field. Must be a specific signature: void(ObjectType*, SerializableDataBlock)	
		 * @param[in]	flags			Various flags you can use to specialize how systems handle this field. See RTTIFieldFlag.
		 */
		void initSingle(const String& name, UINT16 uniqueId, Any getter, Any setter, UINT64 flags)
		{
			initAll(getter, setter, nullptr, nullptr, name, uniqueId, false, SerializableFT_DataBlock, flags);
		}

		/** @copydoc RTTIField::getTypeSize */
		UINT32 getTypeSize() override
		{
			return 0; // Data block types don't store size the conventional way
		}

		/** @copydoc RTTIField::hasDynamicSize */
		bool hasDynamicSize() override
		{
			return true;
		}

		/** @copydoc RTTIField::getArraySize */
		UINT32 getArraySize(void* object) override
		{
			BS_EXCEPT(InternalErrorException, 
				"Data block types don't support arrays.");

			return 0;
		}

		/** @copydoc RTTIField::setArraySize */
		void setArraySize(void* object, UINT32 size) override
		{
			BS_EXCEPT(InternalErrorException, 
				"Data block types don't support arrays.");
		}

		/** @copydoc RTTIManagedDataBlockFieldBase::getValue */
		SPtr<DataStream> getValue(void* object, UINT32& size) override
		{
			ObjectType* castObj = static_cast<ObjectType*>(object);
			std::function<SPtr<DataStream>(ObjectType*, UINT32&)> f = any_cast<std::function<SPtr<DataStream>(ObjectType*, UINT32&)>>(valueGetter);

			return f(castObj, size);
		}

		/** @copydoc RTTIManagedDataBlockFieldBase::setValue */
		void setValue(void* object, const SPtr<DataStream>& value, UINT32 size) override
		{
			ObjectType* castObj = static_cast<ObjectType*>(object);
			std::function<void(ObjectType*, const SPtr<DataStream>&, UINT32)> f = 
				any_cast<std::function<void(ObjectType*, const SPtr<DataStream>&, UINT32)>>(valueSetter);

			f(castObj, value, size);
		}
	};

	/** @} */
	/** @} */
}