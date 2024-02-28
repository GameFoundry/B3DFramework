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

	/** Wraps a container that can be used for sequentially reading container contents, inserting new elements in the container, and retrieving container element count. */
	template <class ContainerType>
	class RTTIIterator
	{
	public:
		using IteratorType = typename ContainerType::iterator;
		using ElementType = typename ContainerType::value_type;

		RTTIIterator(ContainerType& container)
			: mContainer(&container), mIterator(container.begin())
		{}

		/** Returns true if the iterator points to a valid value. */
		bool IsValid() const { return mIterator != mContainer->end(); }

		/** Resets the iterator to the beginning of the container. */
		void ResetToBeginning() { mIterator = mContainer->begin(); }

		/** Resets the iterator to the end of the container. */
		void ResetToEnd() { mIterator = mContainer->end(); }

		/** Returns the number of elements in the container. */
		u64 GetElementCount() const { return mContainer->size(); }

		/** Assigns (copies) the value at the current iterator location. */
		RTTIIterator& operator=(const ElementType& value)
		{
			mIterator = mContainer->insert(mIterator, value);
			++mIterator;

			return *this;
		}

		/** Assigns (moves) the value at the current iterator location. */
		RTTIIterator& operator=(ElementType&& value)
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
		RTTIIterator& operator++()
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
