//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Debug/BsDebug.h"
#include <algorithm>
#include <typeinfo>

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/** Class capable of storing any general type, and safely extracting the proper type from the internal data. */
	class Any
	{
	private:
		class DataBase
		{
		public:
			virtual ~DataBase()
			{ }

			virtual DataBase* clone() const = 0;
		};

		template <typename ValueType>
		class Data : public DataBase
		{
		public:
			Data(const ValueType& value)
				:value(value)
			{ }

			virtual DataBase* clone() const override
			{
				return new Data(value);
			}

			ValueType value;
		};

	public:
		Any() 
			:mData(nullptr)
		{ }

		template <typename ValueType>
		Any(const ValueType& value) 
			:mData(bs_new<Data<ValueType>>(value))
		{ }

		Any(std::nullptr_t)
			:mData(nullptr)
		{ }

		Any(const Any& other) 
			:mData(other.mData != nullptr ? other.mData->clone() : nullptr)
		{ }

		~Any()
		{
			if (mData != nullptr)
				bs_delete(mData);
		}

		/** Swaps the contents of this object with another. */
		Any& swap(Any& rhs)
		{
			std::swap(mData, rhs.mData);
			return *this;
		}

		template <typename ValueType>
		Any& operator= (const ValueType& rhs)
		{
			Any(rhs).swap(*this);
			return *this;
		}

		Any& operator= (const Any& rhs)
		{
			Any(rhs).swap(*this);
			return *this;
		}

		/** Returns true if no type is set. */
		bool empty() const
		{
			return mData == nullptr;
		}

	private:
		template <typename ValueType>
		friend ValueType* any_cast(Any*);

		template <typename ValueType>
		friend ValueType* any_cast_unsafe(Any*);

		DataBase* mData;
	};

	/**
	 * Returns a pointer to the internal data of the specified type.
	 *
	 * @note		Will return null if cast fails.
	 */
	template <typename ValueType>
	ValueType* any_cast(Any* operand)
	{
		if (operand != nullptr)
			return &static_cast<Any::Data<ValueType>*>(operand->mData)->value;
		else
			return nullptr;
	}

	/**
	 * Returns a const pointer to the internal data of the specified type.
	 *
	 * @note	Will return null if cast fails.
	 */
	template <typename ValueType>
	const ValueType* any_cast(const Any* operand)
	{
		return any_cast<ValueType>(const_cast<Any*>(operand));
	}

	/**
	 * Returns a copy of the internal data of the specified type.
	 *
	 * @note	Throws an exception if cast fails.
	 */
	template <typename ValueType>
	ValueType any_cast(const Any& operand)
	{
		return *any_cast<ValueType>(const_cast<Any*>(&operand));
	}

	/**
	 * Returns a copy of the internal data of the specified type.
	 *
	 * @note	Throws an exception if cast fails.
	 */
	template <typename ValueType>
	ValueType any_cast(Any& operand)
	{
		return *any_cast<ValueType>(&operand);
	}

	/**
	 * Returns a reference to the internal data of the specified type.
	 *
	 * @note	Throws an exception if cast fails.
	 */
	template <typename ValueType>
	const ValueType& any_cast_ref(const Any & operand)
	{
		return *any_cast<ValueType>(const_cast<Any*>(&operand));
	}

	/**
	 * Returns a reference to the internal data of the specified type.
	 *
	 * @note	Throws an exception if cast fails.
	 */
	template <typename ValueType>
	ValueType& any_cast_ref(Any& operand)
	{
		return *any_cast<ValueType>(&operand);
	}

	/** Casts a type without performing any kind of checks. */
	template <typename ValueType>
	ValueType* any_cast_unsafe(Any* operand)
	{
		return &static_cast<Any::Data<ValueType>*>(operand->mData)->value;
	}

	/** Casts a type without performing any kind of checks. */
	template <typename ValueType>
	const ValueType* any_cast_unsafe(const Any* operand)
	{
		return any_cast_unsafe<ValueType>(const_cast<Any*>(operand));
	}

	/** @} */
}
