//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Debug/B3DDebug.h"
#include <algorithm>
#include <typeinfo>

namespace b3d
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
			virtual ~DataBase() = default;

			virtual DataBase* Clone() const = 0;
		};

		template <typename ValueType>
		class Data : public DataBase
		{
		public:
			Data(const ValueType& value)
				: Value(value)
			{}

			DataBase* Clone() const override
			{
				return B3DNew<Data>(Data(Value));
			}

			ValueType Value;
		};

	public:
		Any() = default;

		template <typename ValueType>
		Any(const ValueType& value)
			: mData(B3DNew<Data<ValueType>>(value))
		{}

		Any(std::nullptr_t)
			: mData(nullptr)
		{}

		Any(const Any& other)
			: mData(other.mData != nullptr ? other.mData->Clone() : nullptr)
		{}

		~Any()
		{
			if(mData != nullptr)
				B3DDelete(mData);
		}

		/** Swaps the contents of this object with another. */
		Any& Swap(Any& rhs)
		{
			std::swap(mData, rhs.mData);
			return *this;
		}

		template <typename ValueType>
		Any& operator=(const ValueType& rhs)
		{
			Any(rhs).Swap(*this);
			return *this;
		}

		Any& operator=(const Any& rhs)
		{
			Any(rhs).Swap(*this);
			return *this;
		}

		/** Returns true if no type is set. */
		bool Empty() const
		{
			return mData == nullptr;
		}

	private:
		template <typename ValueType>
		friend ValueType* AnyCast(Any*);

		template <typename ValueType>
		friend ValueType* AnyCastUnsafe(Any*);

		DataBase* mData = nullptr;
	};

	/**
	 * Returns a pointer to the internal data of the specified type.
	 *
	 * @note		Will return null if cast fails.
	 */
	template <typename ValueType>
	ValueType* AnyCast(Any* operand)
	{
		if(operand != nullptr)
			return &static_cast<Any::Data<ValueType>*>(operand->mData)->Value;
		else
			return nullptr;
	}

	/**
	 * Returns a const pointer to the internal data of the specified type.
	 *
	 * @note	Will return null if cast fails.
	 */
	template <typename ValueType>
	const ValueType* AnyCast(const Any* operand)
	{
		return AnyCast<ValueType>(const_cast<Any*>(operand));
	}

	/**
	 * Returns a copy of the internal data of the specified type.
	 *
	 * @note	Throws an exception if cast fails.
	 */
	template <typename ValueType>
	ValueType AnyCast(const Any& operand)
	{
		return *AnyCast<ValueType>(const_cast<Any*>(&operand));
	}

	/**
	 * Returns a copy of the internal data of the specified type.
	 *
	 * @note	Throws an exception if cast fails.
	 */
	template <typename ValueType>
	ValueType AnyCast(Any& operand)
	{
		return *AnyCast<ValueType>(&operand);
	}

	/**
	 * Returns a reference to the internal data of the specified type.
	 *
	 * @note	Throws an exception if cast fails.
	 */
	template <typename ValueType>
	const ValueType& AnyCastRef(const Any& operand)
	{
		return *AnyCast<ValueType>(const_cast<Any*>(&operand));
	}

	/**
	 * Returns a reference to the internal data of the specified type.
	 *
	 * @note	Throws an exception if cast fails.
	 */
	template <typename ValueType>
	ValueType& AnyCastRef(Any& operand)
	{
		return *AnyCast<ValueType>(&operand);
	}

	/** Casts a type without performing any kind of checks. */
	template <typename ValueType>
	ValueType* AnyCastUnsafe(Any* operand)
	{
		return &static_cast<Any::Data<ValueType>*>(operand->mData)->value;
	}

	/** Casts a type without performing any kind of checks. */
	template <typename ValueType>
	const ValueType* AnyCastUnsafe(const Any* operand)
	{
		return AnyCastUnsafe<ValueType>(const_cast<Any*>(operand));
	}

	/** @} */
} // namespace b3d
