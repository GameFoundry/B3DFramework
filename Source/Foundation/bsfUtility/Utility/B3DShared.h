//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace b3d
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * Stores two types, where the first type is guaranteed to have zero size if the type is empty. This is utilizing optimization for
	 * empty types (https://en.cppreference.com/w/cpp/language/ebo), i.e. if the FirstType is an empty type, only takes up size for SecondType.
	 */
	template <class FirstType, class SecondType, bool = std::is_empty_v<FirstType> && !std::is_final_v<FirstType>>
	class CompressedPair final : private FirstType
	{
	public:
		template <class FirstArgumentType, class... SecondArgumentType>
		constexpr CompressedPair(FirstArgumentType&& FirstArgument, SecondArgumentType&&... SecondArgument)
			: FirstType(std::forward<FirstArgumentType>(FirstArgument)), Second(std::forward<SecondArgumentType>(SecondArgument)...)
		{ }

		constexpr FirstType& GetFirst()
		{
			return *this;
		}

		constexpr const SecondType& GetSecond() const
		{
			return Second;
		}

	private:
		SecondType Second;
	};

	template <class FirstType, class SecondType>
	class CompressedPair<FirstType, SecondType, false> final
	{
	public:
		template <class FirstArgumentType, class... SecondArgumentType>
		constexpr CompressedPair(FirstArgumentType&& FirstArgument, SecondArgumentType&&... SecondArgument)
			: First(std::forward<FirstArgumentType>(FirstArgument)), Second(std::forward<SecondArgumentType>(SecondArgument)...)
		{ }

		constexpr FirstType& GetFirst() { return First; }
		constexpr const FirstType& GetFirst() const { return First; }
		constexpr SecondType& GetSecond() { return Second; }
		constexpr const SecondType& GetSecond() const { return Second; }

	private:
		FirstType First;
		SecondType Second;
	};

	/**
	 * Common based class for TShared/TWeak control blocks. Control blocks maintains a reference count of the shared pointer and will release
	 * the object after the reference count reaches zero.
	 */
	template<ThreadSafetyPolicy ThreadSafety>
	struct TSharedControlBlock
	{
	protected:
		constexpr TSharedControlBlock() = default;

	public:
		// Non-copyable
		TSharedControlBlock(const TSharedControlBlock&) = delete;
		TSharedControlBlock& operator=(TSharedControlBlock&) = delete;

		virtual ~TSharedControlBlock() = default;

		/** Returns the number of currently held strong references. */
		u32 GetStrongReferenceCount() const
		{
			if constexpr(ThreadSafety == ThreadSafe)
				return StrongReferenceCount.load(std::memory_order_relaxed);
			else
				return StrongReferenceCount;
		}

		/** Increments the strong reference count. As long as strong reference count is non-zero the owned object will be kept alive. */
		void IncrementStrongReferenceCount()
		{
			if constexpr(ThreadSafety == ThreadSafe)
				StrongReferenceCount.fetch_add(1, std::memory_order_relaxed);
			else
				++StrongReferenceCount;
		}

		/**
		 * Decrements the strong reference count. If the strong reference count reaches zero the owned object will be destroyed. Additionally
		 * if there are no weak resource handles alive either, control block data will be destroyed.
		 */
		void DecrementStrongReferenceCount()
		{
			if constexpr(ThreadSafety == ThreadSafe)
			{
				if(StrongReferenceCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
				{
					DestroyOwnedObject();
					DecrementWeakReferenceCount();
				}
			}
			else
			{
				if(--StrongReferenceCount == 0)
				{
					DestroyOwnedObject();
					DecrementWeakReferenceCount();
				}
			}
		}

		/** Increments the weak reference count. This keeps the control block data alive, but not the owned object itself. */
		void IncrementWeakReferenceCount()
		{
			if constexpr(ThreadSafety == ThreadSafe)
				WeakReferenceCount.fetch_add(1, std::memory_order_relaxed);
			else
				++WeakReferenceCount;
		}

		/**
		 * Decrements the weak reference count. If this was the last weak reference and there are no strong references either, control block data
		 * will be destroyed.
		 */
		void DecrementWeakReferenceCount()
		{
			if constexpr(ThreadSafety == ThreadSafe)
			{
				if(WeakReferenceCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
					DestroySelf();
			}
			else
			{
				if(--WeakReferenceCount == 0)
					DestroySelf();
			}
		}

		/** Increments the strong reference count, but only if it is not already at zero. Returns true if incremented. */
		bool IncrementStrongReferenceCountIfNonZero()
		{
			if constexpr(ThreadSafety == ThreadSafe)
			{
				std::uint32_t referenceCount = StrongReferenceCount.load(std::memory_order_acquire);
				while(referenceCount != 0)
				{
					if(StrongReferenceCount.compare_exchange_weak(referenceCount, referenceCount + 1, std::memory_order_release, std::memory_order_relaxed))
						return true;
				}

				return false;
			}
			else
			{
				if(StrongReferenceCount == 0)
					return false;

				++StrongReferenceCount;
				return true;
			}
		}

		/**	Destroys the object the control block is pointing to. */
		virtual void DestroyOwnedObject() = 0;

		/** Destroys the control block. */
		virtual void DestroySelf() = 0;

	private:
		using CounterType = std::conditional_t<ThreadSafety == ThreadSafe, std::atomic<u32>, u32>;

		CounterType StrongReferenceCount{ 1 }; /**< References keeping the object alive (strong handles). */
		CounterType WeakReferenceCount{ 1 }; /**< References keeping the control block data alive (weak handles + 1 if any strong handle is alive). */
	};

	/** Shared control block that deletes the references object using the default deleter. */
	template<typename ObjectType, ThreadSafetyPolicy ThreadSafety>
	struct TSharedControlBlockWithDefaultDeleter : TSharedControlBlock<ThreadSafety>
	{
		TSharedControlBlockWithDefaultDeleter(ObjectType* object)
			:mObject(object)
		{ }

		~TSharedControlBlockWithDefaultDeleter() override { }

		void DestroyOwnedObject() override
		{
			B3DDelete(mObject);
		}

		void DestroySelf() override
		{
			B3DDelete(this);
		}

	private:
		ObjectType* mObject;
	};

	/** Shared control block that deletes the references object using a user provided deleter. */
	template<typename ObjectType, typename DeleterType, ThreadSafetyPolicy ThreadSafety>
	struct TSharedControlBlockWithCustomDeleter : TSharedControlBlock<ThreadSafety>
	{
		TSharedControlBlockWithCustomDeleter(ObjectType* object, DeleterType deleter)
			:mDeleterAndObject(std::move(deleter), object)
		{ }

		~TSharedControlBlockWithCustomDeleter() override { }

		void DestroyOwnedObject() override
		{
			mDeleterAndObject.GetFirst()(mDeleterAndObject.GetSecond());
		}

		void DestroySelf() override
		{
			B3DDelete(this);
		}

	private:
		CompressedPair<DeleterType, ObjectType*> mDeleterAndObject;
	};

	/** Shared control block that stores the referenced object within the control block itself. */
	template<typename ObjectType, ThreadSafetyPolicy ThreadSafety>
	struct TSharedControlBlockWithObject : TSharedControlBlock<ThreadSafety>
	{
		template<typename... ArgumentType>
		explicit TSharedControlBlockWithObject(ArgumentType&&... argument)
		{
			new ((void*)&Object) ObjectType(std::forward<ArgumentType>(argument)...);
		}

		~TSharedControlBlockWithObject() override { }

		void DestroyOwnedObject() override
		{
			Object.~ObjectType();
		}

		void DestroySelf() override
		{
			B3DDelete(this);
		}

		union
		{
			ObjectType Object;
		};
	};

	template <typename Type, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	class TShared;

	template <typename Type, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	class TWeak;

	template <class Type, class = void>
	struct TSupportsSharedFromThis : std::false_type {};

	template <class Type>
	struct TSupportsSharedFromThis<Type, std::void_t<typename Type::SharedFromThisType>>
		: std::is_convertible<std::remove_cv_t<Type>*, typename Type::SharedFromThisType*>::type {
	};

	/** Base class class for TShared/TWeak. */
	template <typename Type, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	class TSharedCommon
	{
	public:
		using ElementType = Type;

		/** Returns the number of strong references held by the pointer. */
		u32 GetReferenceCount() const
		{
			if(mControlBlock != nullptr)
				return mControlBlock->GetStrongReferenceCount();

			return 0;
		}

		/** Returns the underlying object. */
		Type* Get() const { return mOwnedObject; }

	protected:
		~TSharedCommon() = default;

		/** Constructs a new shared pointer referencing the provided object and control block. */
		template<typename OtherType>
		void Construct(OtherType* pointer, TSharedControlBlock<ThreadSafe>* controlBlock)
		{
			mOwnedObject = pointer;
			mControlBlock = controlBlock;

			if constexpr(TSupportsSharedFromThis<OtherType>::value)
			{
				if(pointer != nullptr && pointer->mWeakThis.IsExpired())
				{
					pointer->mWeakThis = TShared<std::remove_cv_t<OtherType>>(*this, const_cast<std::remove_cv_t<OtherType>*>(pointer));
				}
			}
		}

		/** Constructs a new shared pointer by moving another shared pointer. */
		template<typename OtherType>
		void MoveConstructFrom(TSharedCommon<OtherType, ThreadSafety>&& other)
		{
			mOwnedObject = std::exchange(other.mOwnedObject, nullptr);
			mControlBlock = std::exchange(other.mControlBlock, nullptr);
		}

		/** Constructs a new shared pointer by copying another shared pointer. */
		template<typename OtherType>
		void CopyConstructFrom(const TSharedCommon<OtherType, ThreadSafety>& other)
		{
			other.IncrementStrongReferenceCount();

			mOwnedObject = other.mOwnedObject;
			mControlBlock = other.mControlBlock;
		}

		/** Constructs a new shared pointer by moving a shared pointer of another type. Used for casts. */
		template<typename OtherType>
		void AliasMoveConstructFrom(TSharedCommon<OtherType, ThreadSafety>&& other, Type* object)
		{
			mOwnedObject = object;
			other.mOwnedObject = nullptr;

			mControlBlock = std::exchange(other.mControlBlock, nullptr);
		}

		/** Constructs a new shared pointer by copying a shared pointer of another type. Used for casts. */
		template<typename OtherType>
		void AliasCopyConstructFrom(const TSharedCommon<OtherType, ThreadSafety>& other, Type* object)
		{
			other.IncrementStrongReferenceCount();

			mOwnedObject = object;
			mControlBlock = other.mControlBlock;
		}

		/** Constructs a new shared pointer from a weak pointer, if the weak pointer is still valid. */
		template<typename OtherType>
		bool ConstructFromWeak(const TSharedCommon<OtherType, ThreadSafety>& other)
		{
			if(other.mControlBlock != nullptr && other.mControlBlock->IncrementStrongReferenceCountIfNonZero())
			{
				mOwnedObject = other.mOwnedObject;
				mControlBlock = other.mControlBlock;

				return true;
			}

			return false;
		}

		/** Constructs a new weak pointer from a shared pointer. */
		template<typename OtherType>
		void ConstructWeakFrom(const TSharedCommon<OtherType, ThreadSafety>& other)
		{
			if(other.mControlBlock != nullptr)
			{
				mOwnedObject = other.mOwnedObject;
				mControlBlock = other.mControlBlock;
				mControlBlock->IncrementWeakReferenceCount();
			}
			else
			{
				B3D_ASSERT(mControlBlock == nullptr && mOwnedObject == nullptr);
			}

			other.IncrementStrongReferenceCount();

			mOwnedObject = other.mOwnedObject;
			mControlBlock = other.mControlBlock;
		}

		/** Constructs a new weak pointer from a shared pointer, if the shared pointer is still valid. */
		template<typename OtherType>
		void ConstructWeakFromIfNotExpired(const TSharedCommon<OtherType>& other)
		{
			if(other.mControlBlock)
			{
				mControlBlock = other.mControlBlock;
				mControlBlock->IncrementWeakReferenceCount();

				// Keep the object alive while we copy it
				if(mControlBlock->IncrementStrongReferenceCountIfNonZero())
				{
					mOwnedObject = other.mOwnedObject;
					mControlBlock->DecrementStrongReferenceCount();
				}
				else
				{
					B3D_ASSERT(mOwnedObject == nullptr);
				}
			}
			else
			{
				B3D_ASSERT(mOwnedObject == nullptr && mControlBlock == nullptr);
			}
		}

		/** Constructs a new weak pointer from a shared pointer, if the shared pointer is still valid. */
		template<typename OtherType>
		void MoveWeakFromIfNotExpired(TSharedCommon<OtherType>&& other)
		{
			mControlBlock = std::exchange(other.mControlBlock, nullptr);

			// Keep the object alive while we copy it
			if(mControlBlock != nullptr && mControlBlock->IncrementStrongReferenceCountIfNonZero())
			{
				mOwnedObject = other.mOwnedObject;
				mControlBlock->DecrementStrongReferenceCount();
			}
			else
			{
				B3D_ASSERT(mOwnedObject == nullptr);
			}

			other.mOwnedObject = nullptr;
		}

		/** Increments the strong reference count. */
		void IncrementStrongReferenceCount() const
		{
			if(mControlBlock != nullptr)
				mControlBlock->IncrementStrongReferenceCount();
		}

		/** Decrements the strong reference count, and releases the object if count is zero. */
		void DecrementStrongReferenceCount() const
		{
			if(mControlBlock != nullptr)
				mControlBlock->DecrementStrongReferenceCount();
		}

		/** Increments the weak reference count. */
		void IncrementWeakReferenceCount() const
		{
			if(mControlBlock != nullptr)
				mControlBlock->IncrementWeakReferenceCount();
		}

		/** Decrements the weak reference count, and releases the control block if the count is zero. */
		void DecrementWeakReferenceCount() const
		{
			if(mControlBlock != nullptr)
				mControlBlock->DecrementWeakReferenceCount();
		}

		void Swap(TSharedCommon& other)
		{
			std::swap(mOwnedObject, other.mOwnedObject);
			std::swap(mControlBlock, other.mControlBlock);
		}


		Type* mOwnedObject = nullptr;
		TSharedControlBlock<ThreadSafety>* mControlBlock = nullptr;
	};

	/**
	 * Reference counted object pointer that will keep the underlying object alive as long as the reference count is above 0. 
	 *
	 * @tparam	Type			Object type to reference by the pointer.
	 * @tparam	ThreadSafety	If ThreadSafe, shared pointer can be safely accessed from multiple threads, otherwise it is only safe to access from a single thread.
	 *							Non-thread safe version is faster.
	 */
	template <typename Type, ThreadSafetyPolicy ThreadSafety>
	class TShared : public TSharedCommon<Type, ThreadSafety>
	{
	public:
		constexpr TShared() = default;
		constexpr TShared(nullptr_t) {}

		template<typename OtherType, std::enable_if_t<std::is_convertible_v<OtherType, Type>, int> = 0>
		explicit TShared(OtherType* pointer)
		{
			this->Construct(pointer, B3DNew<TSharedControlBlockWithDefaultDeleter<OtherType, ThreadSafety>>(pointer));
		}

		template<typename OtherType, typename DeleterType, std::enable_if_t<std::is_convertible_v<OtherType, Type>, int> = 0>
		explicit TShared(OtherType* pointer, DeleterType deleter)
		{
			this->Construct(pointer, B3DNew<TSharedControlBlockWithCustomDeleter<Type, DeleterType, ThreadSafety>>(pointer, std::move(deleter)));
		}

		template<typename OtherType>
		TShared(const TShared<OtherType>& other, Type* object)
		{
			AliasCopyConstructFrom(other, object);
		}

		template<typename OtherType>
		TShared(const TShared<OtherType>&& other, Type* object)
		{
			AliasMoveConstructFrom(std::move(other), object);
		}

		TShared(const TShared& other)
		{
			this->CopyConstructFrom(other);
		}

		template<typename OtherType, std::enable_if_t<std::is_convertible_v<OtherType, Type>, int> = 0>
		TShared(const TShared<OtherType>& other)
		{
			CopyConstructFrom(other);
		}

		TShared(TShared&& other)
		{
			this->MoveConstructFrom(std::move(other));
		}

		template<typename OtherType, std::enable_if_t<std::is_convertible_v<OtherType, Type>, int> = 0>
		TShared(TShared<OtherType>&& other)
		{
			MoveConstructFrom(std::move(other));
		}

		template<typename OtherType, std::enable_if_t<std::is_convertible_v<OtherType, Type>, int> = 0>
		TShared(const TWeak<OtherType>& other)
		{
			B3D_ENSURE(this->ConstructFromWeak(other));
		}

		template <typename OtherType, typename DeleterType,
        std::enable_if_t<std::conjunction_v<std::is_convertible<OtherType, Type>,
                        std::is_convertible<typename std::unique_ptr<OtherType, DeleterType>::pointer, Type*>>, int> = 0>
		TShared(std::unique_ptr<OtherType, DeleterType>&& other)
		{
			using PointerType = typename std::unique_ptr<OtherType, DeleterType>::pointer;

			const PointerType pointer = other.get();
			if (pointer)
			{
				this->Construct(pointer, B3DNew<TSharedControlBlockWithDefaultDeleter<OtherType, ThreadSafety>>(pointer));
				other.release();
			}
		}

		~TShared()
		{
			this->DecrementStrongReferenceCount();
		}

		TShared& operator=(const TShared& rhs)
		{
			TShared(rhs).Swap(*this);
			return *this;
		}

		template<typename OtherType, std::enable_if_t<std::is_convertible_v<OtherType, Type>, int> = 0>
		TShared& operator=(const TShared<OtherType>& rhs)
		{
			TShared(rhs).Swap(*this);
			return *this;
		}

		TShared& operator=(TShared&& rhs)
		{
			TShared(std::move(rhs)).Swap(*this);
			return *this;
		}

		template<typename OtherType, std::enable_if_t<std::is_convertible_v<OtherType, Type>, int> = 0>
		TShared& operator=(TShared<OtherType>&& rhs)
		{
			TShared(std::move(rhs)).Swap(*this);
			return *this;
		}

		template<typename OtherType, typename DeleterType, std::enable_if_t<std::conjunction_v<std::is_convertible<OtherType, Type>, std::is_convertible<typename std::unique_ptr<OtherType, DeleterType>::pointer, Type*>>, int> = 0>
		TShared& operator=(std::unique_ptr<OtherType, DeleterType>&& rhs)
		{
			TShared(std::move(rhs)).Swap(*this);
			return *this;
		}

		/** Swaps the shared pointer with another. */
		void Swap(TShared& other)
		{
			TSharedCommon<Type, ThreadSafety>::Swap(other);
		}

		/** Clears the strong reference and the pointed object. */
		void Reset()
		{
			TShared().Swap(*this);
		}

		/** Clears the strong reference and existing pointed object, and assigns a new pointed object. */
		template<typename OtherType, std::enable_if_t<std::is_convertible_v<OtherType, Type>, int> = 0>
		void Reset(OtherType* other)
		{
			TShared(other).Swap(*this);
		}

		/** Clears the strong reference and existing pointed object, and assigns a new pointed object and deleter. */
		template<typename OtherType, typename DeleterType, std::enable_if_t<std::is_convertible_v<OtherType, Type>, int> = 0>
		void Reset(OtherType* other, DeleterType deleter)
		{
			TShared(other, deleter).Swap(*this);
		}

		/** Accesses the underlying object as a reference */
		template <class OtherType = Type>
		OtherType& operator*() const
		{
			return *this->mOwnedObject;
		}

		/** Accesses the underlying object as a pointer */
		template <class OtherType = Type>
		OtherType* operator->() const
		{
			return this->mOwnedObject;
		}

		/** True if the pointed object is still alive. */
		explicit operator bool() const 
		{
			return this->mOwnedObject != nullptr;
		}

		template <typename Type2, typename... ArgumentType, ThreadSafetyPolicy ThreadSafety2>
		friend TShared<Type2, ThreadSafety2> B3DMakeShared2(ArgumentType&&... argument);
	};

	template <class T, class U, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	bool operator==(const TShared<T, ThreadSafety>& lhs, const TShared<U, ThreadSafety>& rhs)
	{
		return lhs.Get() == rhs.Get();
	}

	template <class T, class U, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	bool operator!=(const TShared<T, ThreadSafety>& lhs, const TShared<U, ThreadSafety>& rhs)
	{
		return lhs.Get() != rhs.Get();
	}

	template <class T, class U, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	bool operator<=(const TShared<T, ThreadSafety>& lhs, const TShared<U, ThreadSafety>& rhs)
	{
		return lhs.Get() <= rhs.Get();
	}

	template <class T, class U, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	bool operator<(const TShared<T, ThreadSafety>& lhs, const TShared<U, ThreadSafety>& rhs)
	{
		return lhs.Get() < rhs.Get();
	}

	template <class T, class U, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	bool operator>=(const TShared<T, ThreadSafety>& lhs, const TShared<U, ThreadSafety>& rhs)
	{
		return lhs.Get() >= rhs.Get();
	}

	template <class T, class U, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	bool operator>(const TShared<T, ThreadSafety>& lhs, const TShared<U, ThreadSafety>& rhs)
	{
		return lhs.Get() > rhs.Get();
	}

	template <class T, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	bool operator==(nullptr_t, const TShared<T, ThreadSafety>& rhs)
	{
		return nullptr == rhs.Get();
	}

	template <class T, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	bool operator==(const TShared<T, ThreadSafety>& lhs, nullptr_t)
	{
		return lhs.Get() == nullptr;
	}

	template <class T, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	bool operator!=(nullptr_t, const TShared<T, ThreadSafety>& rhs)
	{
		return nullptr != rhs.Get();
	}

	template <class T, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	bool operator!=(const TShared<T, ThreadSafety>& lhs, nullptr_t)
	{
		return lhs.Get() != nullptr;
	}

	/** Cast a shared pointer from one type to another. */
	template <class T, class U, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	TShared<T, ThreadSafety> B3DStaticPointerCast(const TShared<U, ThreadSafety>& other)
	{
		const auto object = static_cast<typename TShared<T>::ElementType*>(other.Get());
		return TShared<T, ThreadSafety>(other, object);
	}

	/** Cast a shared pointer from one type to another. */
	template <class T, class U, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	TShared<T, ThreadSafety> B3DStaticPointerCast(TShared<U, ThreadSafety>&& other)
	{
		const auto object = static_cast<typename TShared<T>::ElementType*>(other.Get());
		return TShared<T, ThreadSafety>(std::move(other), object);
	}

	/** Constructs a shared pointer where the referenced object and the shader pointer control block are both allocated via a single memory allocation. */
	template <typename Type, typename... ArgumentType, ThreadSafetyPolicy ThreadSafety = ThreadSafe>
	TShared<Type, ThreadSafety> B3DMakeShared2(ArgumentType&&... argument)
	{
		TSharedControlBlockWithObject<Type, ThreadSafety>* controlBlock = B3DNew<TSharedControlBlockWithObject<Type, ThreadSafety>>(std::forward<ArgumentType>(argument)...);

		TShared<Type> shared;
		shared.Construct(&controlBlock->Object, controlBlock);
		return shared;
	}

	/**
	 * References an object owned by a TShared. Unlike TShared, this type of pointer will not keep the object alive, and the referenced object may or may not
	 * still be live. Weak pointer must be converted to a strong pointer (TShared) before use.
	 *
	 * @tparam	Type			Object type to reference by the pointer.
	 * @tparam	ThreadSafety	If ThreadSafe, shared pointer can be safely accessed from multiple threads, otherwise it is only safe to access from a single thread.
	 *							Non-thread safe version is faster.
	 */
	template <typename Type, ThreadSafetyPolicy ThreadSafety>
	class TWeak : public TSharedCommon<Type, ThreadSafety>
	{
		template<typename OtherType, typename = const OtherType*>
		static constexpr bool DisallowExpiredConversions = true;

		template<typename OtherType>
		static constexpr bool DisallowExpiredConversions<OtherType, decltype(static_cast<const OtherType*>(static_cast<Type*>(nullptr)))> = true;
	public:
		constexpr TWeak() = default;

		explicit TWeak(const TWeak& other)
		{
			this->ConstructWeakFrom(other);
		}

		template<typename OtherType, std::enable_if_t<std::is_convertible_v<OtherType, Type>, int> = 0>
		explicit TWeak(const TShared<OtherType>& other)
		{
			this->ConstructWeakFrom(other);
		}

		template<typename OtherType, std::enable_if_t<std::is_convertible_v<OtherType, Type>, int> = 0>
		explicit TWeak(const TWeak<OtherType>& other)
		{
			if constexpr(DisallowExpiredConversions<>)
				this->ConstructWeakFromIfNotExpired(other);
			else
				this->ConstructWeakFrom(other);
		}

		template<typename OtherType, std::enable_if_t<std::is_convertible_v<OtherType, Type>, int> = 0>
		explicit TWeak(TWeak<OtherType>&& other)
		{
			if constexpr(DisallowExpiredConversions<>)
				this->MoveWeakFromIfNotExpired(std::move(other));
			else
				this->MoveConstructFrom(std::move(other));
		}

		~TWeak()
		{
			this->DecrementWeakReferenceCount();
		}

		TWeak& operator=(const TWeak& rhs)
		{
			TWeak(rhs).Swap(*this);
			return *this;
		}

		template<typename OtherType, std::enable_if_t<std::is_convertible_v<OtherType, Type>, int> = 0>
		TWeak& operator=(const TWeak<OtherType>& rhs)
		{
			TWeak(rhs).Swap(*this);
			return *this;
		}

		TWeak& operator=(TWeak&& rhs)
		{
			TWeak(std::move(rhs)).Swap(*this);
			return *this;
		}

		template<typename OtherType, std::enable_if_t<std::is_convertible_v<OtherType, Type>, int> = 0>
		TWeak& operator=(TWeak<OtherType>&& rhs)
		{
			TWeak(std::move(rhs)).Swap(*this);
			return *this;
		}

		template<typename OtherType, std::enable_if_t<std::is_convertible_v<OtherType, Type>, int> = 0>
		TWeak& operator=(const TShared<OtherType>&& rhs)
		{
			TWeak(rhs).Swap(*this);
			return *this;
		}

		/** Swaps the shared pointer with another. */
		void Swap(TWeak& other)
		{
			TSharedCommon<Type, ThreadSafety>::Swap(other);
		}

		/** Clears the strong reference and the pointed object. */
		void Reset()
		{
			TWeak().Swap(*this);
		}

		/** Returns true if the underlying object is still valid. */
		bool IsExpired() const
		{
			return this->GetReferenceCount() == 0;
		}

		/** Converts a weak pointer to a strong pointer. */
		TShared<Type> Pin() const
		{
			TShared<Type> shared;
			shared.ConstructFromWeak(*this);
			return shared;
		}
	};

	/** Interface that allows a shared pointer to be retrieved from the this pointer. */
	template <typename Type>
	class ISharedFromThis
	{
	public:
		using SharedFromThisType = ISharedFromThis;

		TShared<Type> GetSharedFromThis() { return TShared<Type>(mWeakThis); }
		TShared<const Type> GetSharedFromThis() const { return TShared<const Type>(mWeakThis); }

		TWeak<Type> GetWeakFromThis() { return mWeakThis; }
		TWeak<const Type> GetWeakFromThis() const { return mWeakThis; }

	protected:
		constexpr ISharedFromThis() = default;
		ISharedFromThis(const ISharedFromThis&) = default;

		ISharedFromThis& operator=(const ISharedFromThis&) noexcept { return *this; }
		~ISharedFromThis() = default;

	private:
		template <typename OtherType, ThreadSafetyPolicy ThreadPolicy>
		friend class TShared;

		mutable TWeak<Type> mWeakThis;
	};

	/** @} */
} // namespace b3d
