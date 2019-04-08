//************************************ bs::framework - Copyright 2018-2019 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/** Unsafe smart pointer that does not support use across multiple threads, but comes with less overhead. */
	template <class T>
	class USPtr
	{
	public:
		USPtr() = default;

		explicit USPtr(T* ptr)
			: mPtr(ptr)
		{
			add();
		}

		USPtr(const USPtr& ptr) 
			: mPtr(ptr.mPtr), mCounter(ptr.mCounter)
		{
			add();
		}

		template <typename U>
		USPtr(const USPtr<U>& ptr)
			: mPtr(static_cast<T*>(ptr.mPtr)), mCounter(ptr.mCounter)
		{
			add();
		}

		~USPtr()
		{
			release();
		}

		USPtr& operator= (const USPtr& ptr)
		{
			release();

			mPtr = ptr.mPtr;
			mCounter = ptr.mCounter;

			add();

			return *this;
		}

		void reset()
		{
			release();
		}

		void reset(T* ptr)
		{
			assert(ptr == nullptr || (ptr != mPtr));

			release();

			mPtr = ptr;
			mCounter = nullptr;

			add();
		}

		void swap(USPtr& rhs)
		{
			std::swap(mPtr, rhs.mPtr);
			std::swap(mCounter, rhs.mCounter);
		}

		T& operator*() const 
		{
			assert(mPtr != nullptr);
			return *mPtr;
		}

		T* operator->() const
		{
			assert(mPtr != nullptr);
			return mPtr;
		}

		operator bool() const { return mCounter != nullptr && *mCounter > 0; }
		bool unique() const { return mCounter != nullptr && *mCounter == 1; }
		uint32_t useCount() const { return mCounter == nullptr ? 0 : *mCounter; }
		T* get() const { return mPtr; }

	private:
		void add()
		{
			if (mPtr != nullptr)
			{
				if (mCounter == nullptr)
					mCounter = bs_new<uint32_t>(1);
				else
					++(*mCounter);
			}
		}

		void release() 
		{
			if (mCounter != nullptr)
			{
				--(*mCounter);
				if (*mCounter == 0)
				{
					bs_delete(mPtr);
					bs_delete(mCounter);
				}

				mCounter = nullptr;
			}

			mPtr = nullptr;
		}

		T* mPtr = nullptr;
		uint32_t* mCounter = nullptr;
	};

	template<class T, class U> 
	bool operator==(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.get() == rhs.get();
	}

	template<class T, class U> 
	bool operator!=(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.get() != rhs.get();
	}

	template<class T, class U> 
	bool operator<=(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.get() <= rhs.get();
	}

	template<class T, class U> 
	bool operator<(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.get() < rhs.get();
	}

	template<class T, class U> 
	bool operator>=(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.get() >= rhs.get();
	}

	template<class T, class U> 
	bool operator>(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.get() > rhs.get();
	}

	template<class T, class U>
	USPtr<T> static_pointer_cast(const USPtr<U>& ptr)
	{
		return USPtr<T>(ptr);
	}

	/** @} */
}
