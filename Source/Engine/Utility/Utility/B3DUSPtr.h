//************************************ B3D Framework - Copyright 2018-2019 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
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

		USPtr(nullptr_t) {}

		explicit USPtr(T* ptr)
			: mPtr(ptr)
		{
			Add();
		}

		USPtr(const USPtr& ptr)
			: mPtr(ptr.mPtr), mCounter(ptr.mCounter)
		{
			Add();
		}

		template <typename U>
		USPtr(const USPtr<U>& ptr)
			: mPtr(static_cast<T*>(ptr.mPtr)), mCounter(ptr.mCounter)
		{
			Add();
		}

		~USPtr()
		{
			Release();
		}

		USPtr& operator=(const USPtr& ptr)
		{
			Release();

			mPtr = ptr.mPtr;
			mCounter = ptr.mCounter;

			Add();

			return *this;
		}

		void reset() // NOLINT
		{
			Release();
		}

		void reset(T* ptr) // NOLINT
		{
			B3D_ASSERT(ptr == nullptr || (ptr != mPtr));

			Release();

			mPtr = ptr;
			mCounter = nullptr;

			Add();
		}

		void swap(USPtr& rhs) // NOLINT
		{
			std::swap(mPtr, rhs.mPtr);
			std::swap(mCounter, rhs.mCounter);
		}

		T& operator*() const
		{
			B3D_ASSERT(mPtr != nullptr);
			return *mPtr;
		}

		T* operator->() const
		{
			B3D_ASSERT(mPtr != nullptr);
			return mPtr;
		}

		operator bool() const { return mCounter != nullptr && *mCounter > 0; }

		bool unique() const { return mCounter != nullptr && *mCounter == 1; } // NOLINT

		uint32_t use_count() const { return mCounter == nullptr ? 0 : *mCounter; } // NOLINT

		T* get() const { return mPtr; } // NOLINT

	private:
		template <class U>
		friend class USPtr;

		void Add()
		{
			if(mPtr != nullptr)
			{
				if(mCounter == nullptr)
					mCounter = B3DNew<uint32_t>(1);
				else
					++(*mCounter);
			}
		}

		void Release()
		{
			if(mCounter != nullptr)
			{
				--(*mCounter);
				if(*mCounter == 0)
				{
					B3DDelete(mPtr);
					B3DDelete(mCounter);
				}

				mCounter = nullptr;
			}

			mPtr = nullptr;
		}

		T* mPtr = nullptr;
		uint32_t* mCounter = nullptr;
	};

	template <class T, class U>
	bool operator==(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.get() == rhs.get();
	}

	template <class T, class U>
	bool operator!=(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.get() != rhs.get();
	}

	template <class T, class U>
	bool operator<=(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.get() <= rhs.get();
	}

	template <class T, class U>
	bool operator<(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.get() < rhs.get();
	}

	template <class T, class U>
	bool operator>=(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.get() >= rhs.get();
	}

	template <class T, class U>
	bool operator>(const USPtr<T>& lhs, const USPtr<U>& rhs)
	{
		return lhs.get() > rhs.get();
	}

	template <class T>
	bool operator==(nullptr_t, const USPtr<T>& rhs)
	{
		return nullptr == rhs.get();
	}

	template <class T>
	bool operator==(const USPtr<T>& lhs, nullptr_t)
	{
		return lhs.get() == nullptr;
	}

	template <class T>
	bool operator!=(nullptr_t, const USPtr<T>& rhs)
	{
		return nullptr != rhs.get();
	}

	template <class T>
	bool operator!=(const USPtr<T>& lhs, nullptr_t)
	{
		return lhs.get() != nullptr;
	}

	/** Cast an unsafe shared pointer from one type to another. */
	template <class T, class U>
	USPtr<T> StaticPointerCast(const USPtr<U>& ptr)
	{
		return USPtr<T>(ptr);
	}

	/** Create a new unsafe shared pointer using a custom allocator category. */
	template <typename Type, typename... Args>
	USPtr<Type> B3DMakeUnsafeShared(Args&&... args)
	{
		// Note: Ideally we merge the pointer and internal USPtr counter allocation in a single allocation

		return USPtr<Type>(B3DNew<Type>(std::forward<Args>(args)...));
	}

	/** Create a new unsafe shared pointer from a previously constructed object. */
	template <typename Type>
	USPtr<Type> B3DMakeUnsafeSharedFromExisting(Type* data)
	{
		return USPtr<Type>(data);
	}

	// Checks is the provided type an unsafe shared pointer
	template <typename T>
	struct B3DIsUnsafeSharedPointer : std::false_type {};

	template <typename T>
	struct B3DIsUnsafeSharedPointer<USPtr<T>> : std::true_type {};

	/** @} */
} // namespace b3d
