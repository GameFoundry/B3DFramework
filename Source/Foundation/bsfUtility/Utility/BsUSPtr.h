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
		USPtr(nullptr_t) { }

		explicit USPtr(T* ptr)
			: MPtr(ptr)
		{
			add();
		}

		USPtr(const USPtr& ptr) 
			: MPtr(ptr.mPtr), mCounter(ptr.mCounter)
		{
			add();
		}

		template <typename U>
		USPtr(const USPtr<U>& ptr)
			: MPtr(static_cast<T*>(ptr.mPtr)), mCounter(ptr.mCounter)
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

		void Reset()
		{
			release();
		}

		void Reset(T* ptr)
		{
			assert(ptr == nullptr || (ptr != mPtr));

			release();

			mPtr = ptr;
			mCounter = nullptr;

			add();
		}

		void Swap(USPtr& rhs)
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

		operator Bool() const { return mCounter != nullptr && *mCounter > 0; }
		bool Unique() const { return mCounter != nullptr && *mCounter == 1; }
		uint32_t UseCount() const { return mCounter == nullptr ? 0 : *mCounter; }
		T* get() const { return mPtr; }

	private:
		template<class U>
		friend class USPtr;

		void Add()
		{
			if (mPtr != nullptr)
			{
				if (mCounter == nullptr)
					mCounter = bs_new<uint32_t>(1);
				else
					++(*mCounter);
			}
		}

		void Release()
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
		return lhs.Get() == rhs.get();
	}

	template<class T, class U> 
	bool operator!=(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.Get() != rhs.get();
	}

	template<class T, class U> 
	bool operator<=(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.Get() <= rhs.get();
	}

	template<class T, class U> 
	bool operator<(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.Get() < rhs.get();
	}

	template<class T, class U> 
	bool operator>=(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.Get() >= rhs.get();
	}

	template<class T, class U> 
	bool operator>(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.Get() > rhs.get();
	}

	template<class T> 
	bool operator==(nullptr_t, const USPtr<T>& rhs)
	{
		return nullptr == rhs.Get();
	}

	template<class T> 
	bool operator==(const USPtr<T>& lhs, nullptr_t)
	{
		return lhs.Get() == nullptr;
	}

	template<class T> 
	bool operator!=(nullptr_t, const USPtr<T>& rhs)
	{
		return nullptr != rhs.Get();
	}

	template<class T> 
	bool operator!=(const USPtr<T>& lhs, nullptr_t)
	{
		return lhs.Get() != nullptr;
	}

	/** Cast an unsafe shared pointer from one type to another. */
	template<class T, class U>
	USPtr<T> static_pointer_cast(const USPtr<U>& ptr)
	{
		return USPtr<T>(ptr);
	}

	/** Create a new unsafe shared pointer using a custom allocator category. */
	template<typename Type, typename... Args>
	USPtr<Type> bs_ushared_ptr_new(Args &&... args)
	{
		// Note: Ideally we merge the pointer and internal USPtr counter allocation in a single allocation

		return USPtr<Type>(bs_new<Type>(std::forward<Args>(args)...));
	}

	/** Create a new unsafe shared pointer from a previously constructed object. */
	template<typename Type>
	USPtr<Type> bs_ushared_ptr(Type* data)
	{
		return USPtr<Type>(data);
	}

	/** @} */
}
