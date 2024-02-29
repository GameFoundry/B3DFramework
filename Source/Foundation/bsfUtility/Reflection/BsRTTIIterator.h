//************************************ bs::framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	/** Interface for a RTTI iterator. */
	class IRTTIIterator
	{
	public:
		virtual ~IRTTIIterator() = default;

		/** Returns true if the iterator points to a valid value. */
		virtual bool IsValid() const = 0;

		/** Resets the iterator to the beginning of the container. */
		virtual void ResetToBeginning() = 0;

		/** Resets the iterator to the end of the container. */
		virtual void ResetToEnd() = 0;

		/** Returns the number of elements in the container. */
		virtual u64 GetElementCount() const = 0;

		/** Assigns the value at the current iterator location. */
		virtual void SetValue(const void* value) = 0;

		/** Returns the current value of the iterator. */
		virtual const void* GetValue() const = 0;

		/** Increment operator. */
		virtual void Increment() = 0;
	};

	/** Wraps a container that can be used for sequentially reading container contents, inserting new elements in the container, and retrieving container element count. */
	template <class ContainerType>
	class TRTTIIterator : public IRTTIIterator
	{
	public:
		using IteratorType = typename ContainerType::iterator;
		using ElementType = typename ContainerType::value_type;

		TRTTIIterator(ContainerType& container)
			: mContainer(&container), mIterator(container.begin())
		{}

		bool IsValid() const override { return mIterator != mContainer->end(); }
		void ResetToBeginning() override { mIterator = mContainer->begin(); }
		void ResetToEnd() override { mIterator = mContainer->end(); }
		u64 GetElementCount() const override { return mContainer->size(); }
		void SetValue(const void* value) override { operator=(*static_cast<const ElementType*>(value)); }
		const void* GetValue() const override { return &(*mIterator); }
		void Increment() override { operator++(); }

		/** Assigns (copies) the value at the current iterator location. */
		TRTTIIterator& operator=(const ElementType& value)
		{
			mIterator = mContainer->insert(mIterator, value);
			++mIterator;

			return *this;
		}

		/** Assigns (moves) the value at the current iterator location. */
		TRTTIIterator& operator=(ElementType&& value)
		{
			mIterator = mContainer->insert(mIterator, std::move(value));
			++mIterator;

			return *this;
		}

		/** Returns the current value of the iterator. */
		ElementType& operator*()
		{
			return *mIterator;
		}

		/** Pre-increment operator. */
		TRTTIIterator& operator++()
		{
			++mIterator;
			return *this;
		}

	protected:
		ContainerType* mContainer = nullptr;
		IteratorType mIterator;
	};

	/** @} */
	/** @} */
} // namespace bs
