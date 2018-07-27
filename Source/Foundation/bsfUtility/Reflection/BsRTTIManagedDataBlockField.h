//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIField.h"

namespace bs
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
		virtual SPtr<DataStream> getValue(RTTITypeBase* rtti, void* object, UINT32& size) = 0;

		/** Sets a managed data block on the specified instance. */
		virtual void setValue(RTTITypeBase* rtti, void* object, const SPtr<DataStream>& data, UINT32 size) = 0;
	};

	/** Class containing a managed data block field containing a specific type. */
	template <class InterfaceType, class DataType, class ObjectType>
	struct RTTIManagedDataBlockField : public RTTIManagedDataBlockFieldBase
	{
		typedef SPtr<DataStream> (InterfaceType::*GetterType)(ObjectType*, UINT32&);
		typedef void (InterfaceType::*SetterType)(ObjectType*, const SPtr<DataStream>&, UINT32);

		/**
		 * Initializes a field that returns a block of bytes. Can be used for serializing pretty much anything.
		 *
		 * @param[in]	name			Name of the field.
		 * @param[in]	uniqueId		Unique identifier for this field. Although name is also a unique identifier we want a 
		 *								small data type that can be used for efficiently serializing data to disk and similar. 
		 *								It is primarily used for compatibility between different versions of serialized data.
		 * @param[in]	getter  		The getter method for the field.
		 * @param[in]	setter  		The setter method for the field.
		 * @param[in]	flags			Various flags you can use to specialize how systems handle this field. See RTTIFieldFlag.
		 */
		void initSingle(const String& name, UINT16 uniqueId, GetterType getter, SetterType setter, UINT64 flags)
		{
			this->getter = getter;
			this->setter = setter;

			init(name, uniqueId, false, SerializableFT_DataBlock, flags);
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
		UINT32 getArraySize(RTTITypeBase* rtti, void* object) override
		{
			BS_EXCEPT(InternalErrorException, 
				"Data block types don't support arrays.");

			return 0;
		}

		/** @copydoc RTTIField::setArraySize */
		void setArraySize(RTTITypeBase* rtti, void* object, UINT32 size) override
		{
			BS_EXCEPT(InternalErrorException, 
				"Data block types don't support arrays.");
		}

		/** @copydoc RTTIManagedDataBlockFieldBase::getValue */
		SPtr<DataStream> getValue(RTTITypeBase* rtti, void* object, UINT32& size) override
		{
			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObj = static_cast<ObjectType*>(object);

			return (rttiObject->*getter)(castObj, size);
		}

		/** @copydoc RTTIManagedDataBlockFieldBase::setValue */
		void setValue(RTTITypeBase* rtti, void* object, const SPtr<DataStream>& value, UINT32 size) override
		{
			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObj = static_cast<ObjectType*>(object);

			(rttiObject->*setter)(castObj, value, size);
		}

	private:
		GetterType getter;
		SetterType setter;
	};

	/** @} */
	/** @} */
}