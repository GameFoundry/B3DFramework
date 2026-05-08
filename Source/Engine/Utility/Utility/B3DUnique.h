//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace b3d
{
	/** @addtogroup Memory
	 *  @{
	 */

	/**
	 * Owning smart pointer with single-owner semantics. The owned object is automatically destroyed when this pointer
	 * goes out of scope or is reassigned. Cannot be copied; ownership is transferred via std::move.
	 *
	 * The default deleter routes through @b3d::Deleter which calls B3DDelete<Type, AllocatorTag>, so the pointer respects
	 * the engine's per-category memory allocators. If the deleter type is empty (the default case) the pointer occupies
	 * exactly sizeof(Type*) thanks to the empty base optimization in CompressedPair.
	 *
	 * Move-converts to TShared2 via the TShared2(TUnique2&&) adopting constructor or via TUnique2::MoveToShared().
	 *
	 * @tparam	Type			Object type referenced by the pointer.
	 * @tparam	AllocatorTag	Memory allocator category used by the default deleter.
	 * @tparam	DeleterType		Functor type invoked to destroy the owned object. Must be invocable with @p Type*.
	 */
	template <typename Type, typename AllocatorTag = DefaultAllocatorTag, typename DeleterType = Deleter<Type, AllocatorTag>>
	class TUnique2
	{
	public:
		using ElementType = Type;
		using PointerType = Type*;
		using AllocatorTagType = AllocatorTag;
		using DeleterTypeAlias = DeleterType;

		constexpr TUnique2() noexcept = default;
		constexpr TUnique2(nullptr_t) noexcept {}

		/** Adopts ownership of @p pointer using the default-constructed deleter. */
		explicit TUnique2(PointerType pointer) noexcept
			: mDeleterAndPointer(DeleterType{}, pointer)
		{ }

		/** Adopts ownership of @p pointer using a copy of the provided deleter. */
		TUnique2(PointerType pointer, const DeleterType& deleter) noexcept
			: mDeleterAndPointer(deleter, pointer)
		{ }

		/** Adopts ownership of @p pointer using the provided deleter (moved). */
		TUnique2(PointerType pointer, DeleterType&& deleter) noexcept
			: mDeleterAndPointer(std::move(deleter), pointer)
		{ }

		// Non-copyable
		TUnique2(const TUnique2&) = delete;
		TUnique2& operator=(const TUnique2&) = delete;

		TUnique2(TUnique2&& other) noexcept
			: mDeleterAndPointer(std::move(other.mDeleterAndPointer.GetFirst()), std::exchange(other.mDeleterAndPointer.GetSecond(), nullptr))
		{ }

		template<typename OtherType, typename OtherAllocatorTag, typename OtherDeleterType,
			std::enable_if_t<std::conjunction_v<
				std::is_convertible<OtherType*, Type*>,
				std::is_convertible<OtherDeleterType, DeleterType>>, int> = 0>
		TUnique2(TUnique2<OtherType, OtherAllocatorTag, OtherDeleterType>&& other) noexcept
			: mDeleterAndPointer(std::move(other.mDeleterAndPointer.GetFirst()), std::exchange(other.mDeleterAndPointer.GetSecond(), nullptr))
		{ }

		/** Adopts the pointer from a std::unique_ptr. The original deleter is replaced with this pointer's deleter. */
		template<typename OtherType, typename OtherDeleter,
			std::enable_if_t<std::conjunction_v<
				std::is_convertible<OtherType*, Type*>,
				std::is_default_constructible<DeleterType>>, int> = 0>
		explicit TUnique2(std::unique_ptr<OtherType, OtherDeleter>&& other) noexcept
			: mDeleterAndPointer(DeleterType{}, other.release())
		{ }

		~TUnique2()
		{
			DeletePointee();
		}

		TUnique2& operator=(TUnique2&& rhs) noexcept
		{
			if(this != &rhs)
			{
				DeletePointee();
				mDeleterAndPointer.GetSecond() = std::exchange(rhs.mDeleterAndPointer.GetSecond(), nullptr);
				mDeleterAndPointer.GetFirst() = std::move(rhs.mDeleterAndPointer.GetFirst());
			}

			return *this;
		}

		template<typename OtherType, typename OtherAllocatorTag, typename OtherDeleterType,
			std::enable_if_t<std::conjunction_v<
				std::is_convertible<OtherType*, Type*>,
				std::is_convertible<OtherDeleterType, DeleterType>>, int> = 0>
		TUnique2& operator=(TUnique2<OtherType, OtherAllocatorTag, OtherDeleterType>&& rhs) noexcept
		{
			DeletePointee();
			mDeleterAndPointer.GetSecond() = std::exchange(rhs.mDeleterAndPointer.GetSecond(), nullptr);
			mDeleterAndPointer.GetFirst() = std::move(rhs.mDeleterAndPointer.GetFirst());
			return *this;
		}

		TUnique2& operator=(nullptr_t) noexcept
		{
			Reset();
			return *this;
		}

		/** Releases ownership of the owned object and returns the raw pointer. The caller becomes responsible for destroying it. */
		PointerType Release() noexcept
		{
			return std::exchange(mDeleterAndPointer.GetSecond(), nullptr);
		}

		/** Replaces the owned object with @p pointer (default null). The previously held object is destroyed. */
		void Reset(PointerType pointer = nullptr) noexcept
		{
			PointerType oldPointer = std::exchange(mDeleterAndPointer.GetSecond(), pointer);
			if(oldPointer != nullptr)
				mDeleterAndPointer.GetFirst()(oldPointer);
		}

		/** Swaps the owned pointer and deleter with another unique pointer. */
		void Swap(TUnique2& other) noexcept
		{
			using std::swap;
			swap(mDeleterAndPointer.GetFirst(), other.mDeleterAndPointer.GetFirst());
			swap(mDeleterAndPointer.GetSecond(), other.mDeleterAndPointer.GetSecond());
		}

		/** Returns the underlying raw pointer without releasing ownership. */
		PointerType Get() const noexcept
		{
			return mDeleterAndPointer.GetSecond();
		}

		/** Returns a reference to the held deleter. */
		DeleterType& GetDeleter() noexcept
		{
			return mDeleterAndPointer.GetFirst();
		}

		/** Returns a reference to the held deleter. */
		const DeleterType& GetDeleter() const noexcept
		{
			return mDeleterAndPointer.GetFirst();
		}

		/** True if the pointer is non-null. */
		explicit operator bool() const noexcept
		{
			return mDeleterAndPointer.GetSecond() != nullptr;
		}

		/** Dereferences the owned object. */
		Type& operator*() const noexcept
		{
			return *mDeleterAndPointer.GetSecond();
		}

		/** Member access on the owned object. */
		PointerType operator->() const noexcept
		{
			return mDeleterAndPointer.GetSecond();
		}

		/**
		 * Releases ownership and constructs a TShared2 adopting the released object. The unique pointer becomes empty after the call.
		 * The held deleter is preserved on the resulting shared pointer's control block.
		 */
		TShared2<Type> MoveToShared() &&
		{
			return TShared2<Type>(std::move(*this));
		}

		template <typename OtherType, typename OtherAllocatorTag, typename OtherDeleterType>
		friend class TUnique2;

		template <typename OtherType, ThreadSafetyPolicy OtherThreadSafety>
		friend class TShared2;

	private:
		void DeletePointee() noexcept
		{
			PointerType pointer = mDeleterAndPointer.GetSecond();
			if(pointer != nullptr)
				mDeleterAndPointer.GetFirst()(pointer);
		}

		CompressedPair<DeleterType, PointerType> mDeleterAndPointer{DeleterType{}, nullptr};
	};

	/** Swap two unique pointers (ADL helper). */
	template <typename Type, typename AllocatorTag, typename DeleterType>
	void Swap(TUnique2<Type, AllocatorTag, DeleterType>& lhs, TUnique2<Type, AllocatorTag, DeleterType>& rhs) noexcept
	{
		lhs.Swap(rhs);
	}

	template <typename T, typename TAllocatorTag, typename TDeleter, typename U, typename UAllocatorTag, typename UDeleter>
	bool operator==(const TUnique2<T, TAllocatorTag, TDeleter>& lhs, const TUnique2<U, UAllocatorTag, UDeleter>& rhs) noexcept
	{
		return lhs.Get() == rhs.Get();
	}

	template <typename T, typename TAllocatorTag, typename TDeleter, typename U, typename UAllocatorTag, typename UDeleter>
	bool operator!=(const TUnique2<T, TAllocatorTag, TDeleter>& lhs, const TUnique2<U, UAllocatorTag, UDeleter>& rhs) noexcept
	{
		return lhs.Get() != rhs.Get();
	}

	template <typename T, typename TAllocatorTag, typename TDeleter, typename U, typename UAllocatorTag, typename UDeleter>
	bool operator<(const TUnique2<T, TAllocatorTag, TDeleter>& lhs, const TUnique2<U, UAllocatorTag, UDeleter>& rhs) noexcept
	{
		return lhs.Get() < rhs.Get();
	}

	template <typename T, typename TAllocatorTag, typename TDeleter, typename U, typename UAllocatorTag, typename UDeleter>
	bool operator<=(const TUnique2<T, TAllocatorTag, TDeleter>& lhs, const TUnique2<U, UAllocatorTag, UDeleter>& rhs) noexcept
	{
		return lhs.Get() <= rhs.Get();
	}

	template <typename T, typename TAllocatorTag, typename TDeleter, typename U, typename UAllocatorTag, typename UDeleter>
	bool operator>(const TUnique2<T, TAllocatorTag, TDeleter>& lhs, const TUnique2<U, UAllocatorTag, UDeleter>& rhs) noexcept
	{
		return lhs.Get() > rhs.Get();
	}

	template <typename T, typename TAllocatorTag, typename TDeleter, typename U, typename UAllocatorTag, typename UDeleter>
	bool operator>=(const TUnique2<T, TAllocatorTag, TDeleter>& lhs, const TUnique2<U, UAllocatorTag, UDeleter>& rhs) noexcept
	{
		return lhs.Get() >= rhs.Get();
	}

	template <typename T, typename TAllocatorTag, typename TDeleter>
	bool operator==(const TUnique2<T, TAllocatorTag, TDeleter>& lhs, nullptr_t) noexcept
	{
		return lhs.Get() == nullptr;
	}

	template <typename T, typename TAllocatorTag, typename TDeleter>
	bool operator==(nullptr_t, const TUnique2<T, TAllocatorTag, TDeleter>& rhs) noexcept
	{
		return nullptr == rhs.Get();
	}

	template <typename T, typename TAllocatorTag, typename TDeleter>
	bool operator!=(const TUnique2<T, TAllocatorTag, TDeleter>& lhs, nullptr_t) noexcept
	{
		return lhs.Get() != nullptr;
	}

	template <typename T, typename TAllocatorTag, typename TDeleter>
	bool operator!=(nullptr_t, const TUnique2<T, TAllocatorTag, TDeleter>& rhs) noexcept
	{
		return nullptr != rhs.Get();
	}

	/** Constructs a TUnique2 pointing to a newly allocated object using the specified allocator tag. */
	template <typename Type, typename AllocatorTag = DefaultAllocatorTag, typename DeleterType = Deleter<Type, AllocatorTag>, typename... Args>
	TUnique2<Type, AllocatorTag, DeleterType> B3DMakeUnique2(Args&&... args)
	{
		return TUnique2<Type, AllocatorTag, DeleterType>(B3DNew<Type, AllocatorTag>(std::forward<Args>(args)...));
	}

	/** Constructs a TUnique2 adopting an already-allocated object. */
	template <typename Type, typename AllocatorTag = DefaultAllocatorTag, typename DeleterType = Deleter<Type, AllocatorTag>>
	TUnique2<Type, AllocatorTag, DeleterType> B3DMakeUniqueFromExisting2(Type* data, DeleterType deleter = DeleterType{})
	{
		return TUnique2<Type, AllocatorTag, DeleterType>(data, std::move(deleter));
	}

	/** @} */
} // namespace b3d

namespace std
{
	template <typename Type, typename AllocatorTag, typename DeleterType>
	struct hash<b3d::TUnique2<Type, AllocatorTag, DeleterType>>
	{
		std::size_t operator()(const b3d::TUnique2<Type, AllocatorTag, DeleterType>& pointer) const noexcept
		{
			return std::hash<typename b3d::TUnique2<Type, AllocatorTag, DeleterType>::PointerType>{}(pointer.Get());
		}
	};
} // namespace std
