//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Reflection/BsIReflectable.h"
#include "Utility/BsUUID.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**	Data that is shared between all resource handles. */
	struct BS_CORE_EXPORT ResourceHandleData
	{
		SPtr<Resource> MPtr;
		UUID MUuid;
		bool MIsCreated = false;
		std::atomic<std::uint32_t> MRefCount{ 0 };
	};

	/**
	 * Represents a handle to a resource. Handles are similar to a smart pointers, but they have two advantages:
	 *	- When loading a resource asynchronously you can be immediately returned the handle that you may use throughout
	 *    the engine. The handle will be made valid as soon as the resource is loaded.
	 *	- Handles can be serialized and deserialized, therefore saving/restoring references to their original resource.
	 */
	class BS_CORE_EXPORT ResourceHandleBase : public IReflectable
	{
	public:
		/**
		 * Checks if the resource is loaded. Until resource is loaded this handle is invalid and you may not get the
		 * internal resource from it.
		 *
		 * @param[in]	checkDependencies	If true, and if resource has any dependencies, this method will also check if
		 *									they are loaded.
		 */
		bool IsLoaded(bool checkDependencies = true) const;

		/**
		 * Blocks the current thread until the resource is fully loaded.
		 *
		 * @note	Careful not to call this on the thread that does the loading.
		 */
		void BlockUntilLoaded(bool waitForDependencies = true) const;

		/**
		 * Releases an internal reference to this resource held by the resources system, if there is one.
		 *
		 * @see		Resources::release(ResourceHandleBase&)
		 */
		void Release();

		/** Returns the UUID of the resource the handle is referring to. */
		const UUID& GetUuid() const { return mData != nullptr ? mData->MUuid : UUID::kEmpty; }

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**	Gets the handle data. For internal use only. */
		const SPtr<ResourceHandleData>& GetHandleData() const { return mData; }

		/** @} */
	protected:
		/**	Destroys the resource the handle is pointing to. */
		void Destroy();

		/**
		 * Sets the created flag to true and assigns the resource pointer. Called by the constructors, or if you
		 * constructed just using a UUID, then you need to call this manually before you can access the resource from
		 * this handle.
		 *
		 * @note
		 * This is needed because two part construction is required due to  multithreaded nature of resource loading.
		 * @note
		 * Internal method.
		 */
		void SetHandleData(const SPtr<Resource>& ptr, const UUID& uuid);

		/**
		 * Clears the created flag and the resource pointer, making the handle invalid until the resource is loaded again
		 * and assigned through setHandleData().
		 */
		void ClearHandleData();

		/** Increments the reference count of the handle. Only to be used by Resources for keeping internal references. */
		void AddInternalRef();

		/** Decrements the reference count of the handle. Only to be used by Resources for keeping internal references. */
		void RemoveInternalRef();

		/**
		 * Notification sent by the resource system when the resource is done with the loading process. This will trigger
		 * even if the load fails.
		 */
		void NotifyLoadComplete();

		/**
		 * @note
		 * All handles to the same source must share this same handle data. Otherwise things like counting number of
		 * references or replacing pointed to resource become impossible without additional logic. */
		SPtr<ResourceHandleData> mData;

	private:
		friend class Resources;

		static Signal mResourceCreatedCondition;
		static Mutex mResourceCreatedMutex;

	protected:
		void ThrowIfNotLoaded() const;
	};

	/**
	 * @copydoc	ResourceHandleBase
	 *
	 * Handles differences in reference counting depending if the handle is normal or weak.
	 */
	template <bool WeakHandle>
	class BS_CORE_EXPORT TResourceHandleBase : public ResourceHandleBase
	{};

	/**	Specialization of TResourceHandleBase for weak handles. Weak handles do no reference counting. */
	template <>
	class BS_CORE_EXPORT TResourceHandleBase<true> : public ResourceHandleBase
	{
	protected:
		void AddRef(){};
		void ReleaseRef(){};

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class WeakResourceHandleRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**	Specialization of TResourceHandleBase for normal (non-weak) handles. */
	template <>
	class BS_CORE_EXPORT TResourceHandleBase<false> : public ResourceHandleBase
	{
	protected:
		void AddRef()
		{
			if(mData)
				mData->MRefCount.fetch_add(1, std::memory_order_relaxed);
		};

		void ReleaseRef()
		{
			if(mData)
			{
				std::uint32_t refCount = mData->MRefCount.fetch_sub(1, std::memory_order_release);

				if(refCount == 1)
				{
					std::atomic_thread_fence(std::memory_order_acquire);
					Destroy();
				}
			}
		};

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class WeakResourceHandleRTTI;
		friend class ResourceHandleRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @copydoc ResourceHandleBase */
	template <typename T, bool WeakHandle>
	class TResourceHandle : public TResourceHandleBase<WeakHandle>
	{
	public:
		TResourceHandle() = default;

		TResourceHandle(std::nullptr_t) {}

		/**	Copy constructor. */
		TResourceHandle(const TResourceHandle& other)
		{
			this->mData = other.GetHandleData();
			this->AddRef();
		}

		/** Move constructor. */
		TResourceHandle(TResourceHandle&& other) = default;

		~TResourceHandle()
		{
			this->ReleaseRef();
		}

		/**	Converts a specific handle to generic Resource handle. */
		operator TResourceHandle<Resource, WeakHandle>() const
		{
			TResourceHandle<Resource, WeakHandle> handle;
			handle.SetHandleData(this->GetHandleData());

			return handle;
		}

		/**
		 * Returns internal resource pointer.
		 *
		 * @note	Throws exception if handle is invalid.
		 */
		T* operator->() const { return Get(); }

		/**
		 * Returns internal resource pointer and dereferences it.
		 *
		 * @note	Throws exception if handle is invalid.
		 */
		T& operator*() const { return *Get(); }

		/** Clears the handle making it invalid and releases any references held to the resource. */
		TResourceHandle<T, WeakHandle>& operator=(std::nullptr_t ptr)
		{
			this->ReleaseRef();
			this->mData = nullptr;

			return *this;
		}

		/**	Copy assignment. */
		TResourceHandle<T, WeakHandle>& operator=(const TResourceHandle<T, WeakHandle>& rhs)
		{
			SetHandleData(rhs.GetHandleData());
			return *this;
		}

		/**	Move assignment. */
		TResourceHandle& operator=(TResourceHandle&& other)
		{
			if(this == &other)
				return *this;

			this->ReleaseRef();
			this->mData = std::exchange(other.mData, nullptr);

			return *this;
		}

		template <class _Ty>
		struct Bool_struct
		{
			int Member;
		};

		/**
		 * Allows direct conversion of handle to bool.
		 *
		 * @note	This is needed because we can't directly convert to bool since then we can assign pointer to bool and
		 *			that's weird.
		 */
		operator int Bool_struct<T>::*() const
		{
			return ((this->mData != nullptr && !this->mData->MUuid.Empty()) ? &Bool_struct<T>::Member : 0);
		}

		/**
		 * Returns internal resource pointer and dereferences it.
		 *
		 * @note	Throws exception if handle is invalid.
		 */
		T* Get() const
		{
			this->ThrowIfNotLoaded();

			return reinterpret_cast<T*>(this->mData->MPtr.get());
		}

		/**
		 * Returns the internal shared pointer to the resource.
		 *
		 * @note	Throws exception if handle is invalid.
		 */
		SPtr<T> GetInternalPtr() const
		{
			this->ThrowIfNotLoaded();

			return std::static_pointer_cast<T>(this->mData->MPtr);
		}

		/** Converts a handle into a weak handle. */
		TResourceHandle<T, true> GetWeak() const
		{
			TResourceHandle<T, true> handle;
			handle.SetHandleData(this->GetHandleData());

			return handle;
		}

	protected:
		friend Resources;
		template <class _T, bool _Weak>
		friend class TResourceHandle;
		template <class _Ty1, class _Ty2, bool _Weak2, bool _Weak1>
		friend TResourceHandle<_Ty1, _Weak1> static_resource_cast(const TResourceHandle<_Ty2, _Weak2>& other);
		template <class _Ty1, class _Ty2, bool _Weak2>
		friend TResourceHandle<_Ty1, false> static_resource_cast(const TResourceHandle<_Ty2, _Weak2>& other);

		/**
		 * Constructs a new valid handle for the provided resource with the provided UUID.
		 *
		 * @note	Handle will take ownership of the provided resource pointer, so make sure you don't delete it elsewhere.
		 */
		explicit TResourceHandle(T* ptr, const UUID& uuid)
			: TResourceHandleBase<WeakHandle>()
		{
			this->mData = B3DMakeShared<ResourceHandleData>();
			this->AddRef();

			this->SetHandleData(SPtr<Resource>(ptr), uuid);
			this->mIsCreated = true;
		}

		/**
		 * Constructs an invalid handle with the specified UUID. You must call setHandleData() with the actual resource
		 * pointer to make the handle valid.
		 */
		TResourceHandle(const UUID& uuid)
		{
			this->mData = B3DMakeShared<ResourceHandleData>();
			this->mData->MUuid = uuid;

			this->AddRef();
		}

		/**	Constructs a new valid handle for the provided resource with the provided UUID. */
		TResourceHandle(const SPtr<T> ptr, const UUID& uuid)
		{
			this->mData = B3DMakeShared<ResourceHandleData>();
			this->AddRef();

			this->SetHandleData(ptr, uuid);
			this->mData->MIsCreated = true;
		}

		/**	Replaces the internal handle data pointer, effectively transforming the handle into a different handle. */
		void SetHandleData(const SPtr<ResourceHandleData>& data)
		{
			this->ReleaseRef();
			this->mData = data;
			this->AddRef();
		}

		/**	Converts a weak handle into a normal handle. */
		TResourceHandle<T, false> Lock() const
		{
			TResourceHandle<Resource, false> handle;
			handle.SetHandleData(this->GetHandleData());

			return handle;
		}

		using ResourceHandleBase::SetHandleData;
	};

	/**	Checks if two handles point to the same resource. */
	template <class _Ty1, bool _Weak1, class _Ty2, bool _Weak2>
	bool operator==(const TResourceHandle<_Ty1, _Weak1>& _Left, const TResourceHandle<_Ty2, _Weak2>& _Right)
	{
		if(_Left.GetHandleData() != nullptr && _Right.GetHandleData() != nullptr)
			return _Left.GetHandleData()->MPtr == _Right.GetHandleData()->MPtr;

		return _Left.GetHandleData() == _Right.GetHandleData();
	}

	/**	Checks if a handle is null. */
	template <class _Ty1, bool _Weak1, class _Ty2, bool _Weak2>
	bool operator==(const TResourceHandle<_Ty1, _Weak1>& _Left, std::nullptr_t _Right)
	{
		return _Left.GetHandleData() == nullptr || _Left.GetHandleData()->mUUID.empty();
	}

	template <class _Ty1, bool _Weak1, class _Ty2, bool _Weak2>
	bool operator!=(const TResourceHandle<_Ty1, _Weak1>& _Left, const TResourceHandle<_Ty2, _Weak2>& _Right)
	{
		return (!(_Left == _Right));
	}

	/** @} */

	/** @addtogroup Resources
	 *  @{
	 */

	/** @copydoc ResourceHandleBase */
	template <typename T>
	using ResourceHandle = TResourceHandle<T, false>;

	/**
	 * @copydoc ResourceHandleBase
	 *
	 * Weak handles don't prevent the resource from being unloaded.
	 */
	template <typename T>
	using WeakResourceHandle = TResourceHandle<T, true>;

	/**	Casts one resource handle to another. */
	template <class _Ty1, class _Ty2, bool _Weak2, bool _Weak1>
	TResourceHandle<_Ty1, _Weak1> static_resource_cast(const TResourceHandle<_Ty2, _Weak2>& other)
	{
		TResourceHandle<_Ty1, _Weak1> handle;
		handle.SetHandleData(other.GetHandleData());

		return handle;
	}

	/**	Casts one resource handle to another. */
	template <class _Ty1, class _Ty2, bool _Weak2>
	TResourceHandle<_Ty1, false> static_resource_cast(const TResourceHandle<_Ty2, _Weak2>& other)
	{
		TResourceHandle<_Ty1, false> handle;
		handle.SetHandleData(other.GetHandleData());

		return handle;
	}

	/** @} */
} // namespace bs
