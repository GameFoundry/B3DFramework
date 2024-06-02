//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Reflection/BsIReflectable.h"
#include "Threading/BsSignal.h"
#include "Utility/BsUUID.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**	Data that is shared between all resource handles. */
	struct B3D_CORE_EXPORT ResourceHandleData
	{
		SPtr<Resource> Object;
		UUID Id;
		bool IsCreated = false;
		std::atomic<std::uint32_t> ReferenceCount{ 0 };
	};

	/**
	 * Represents a handle to a resource. Handles are similar to a smart pointers, but they have two advantages:
	 *	- When loading a resource asynchronously you can be immediately returned the handle that you may use throughout
	 *    the engine. The handle will be made valid as soon as the resource is loaded.
	 *	- Handles can be serialized and deserialized, therefore saving/restoring references to their original resource.
	 */
	class B3D_CORE_EXPORT ResourceHandleBase : public IReflectable
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
		 * @see		Resources::ReleaseInternalReference(ResourceHandleBase&)
		 */
		void ReleaseInternalReference();

		/** Returns the UUID of the resource the handle is referring to. */
		const UUID& GetId() const { return mData != nullptr ? mData->Id : UUID::kEmpty; }

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
		void IncrementInternalReferenceCount();

		/** Decrements the reference count of the handle. Only to be used by Resources for keeping internal references. */
		void DecrementInternalReferenceCount();

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
	class B3D_CORE_EXPORT TResourceHandleBase : public ResourceHandleBase
	{};

	/**	Specialization of TResourceHandleBase for weak handles. Weak handles do no reference counting. */
	template <>
	class B3D_CORE_EXPORT TResourceHandleBase<true> : public ResourceHandleBase
	{
	protected:
		void IncrementReferenceCount(){}
		void DecrementReferenceCount(){}

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class WeakResourceHandleRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Specialization of TResourceHandleBase for normal (non-weak) handles. */
	template <>
	class B3D_CORE_EXPORT TResourceHandleBase<false> : public ResourceHandleBase
	{
	protected:
		void IncrementReferenceCount()
		{
			if(mData)
				mData->ReferenceCount.fetch_add(1, std::memory_order_relaxed);
		}

		void DecrementReferenceCount()
		{
			if(mData)
			{
				std::uint32_t refCount = mData->ReferenceCount.fetch_sub(1, std::memory_order_release);

				if(refCount == 1)
				{
					std::atomic_thread_fence(std::memory_order_acquire);
					Destroy();
				}
			}
		}

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class WeakResourceHandleRTTI;
		friend class ResourceHandleRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @copydoc ResourceHandleBase */
	template <typename ResourceType, bool WeakHandle>
	class TResourceHandle : public TResourceHandleBase<WeakHandle>
	{
	public:
		TResourceHandle() = default;

		TResourceHandle(std::nullptr_t) {}

		/**	Copy constructor. */
		TResourceHandle(const TResourceHandle& other)
		{
			this->mData = other.GetHandleData();
			this->IncrementReferenceCount();
		}

		/** Move constructor. */
		TResourceHandle(TResourceHandle&& other) = default;

		~TResourceHandle()
		{
			this->DecrementReferenceCount();
		}

		/**	Converts a specific handle to generic Resource handle. */
		operator TResourceHandle<Resource, WeakHandle>() const
		{
			TResourceHandle<Resource, WeakHandle> handle;
			handle.SetHandleData(this->GetHandleData());

			return handle;
		}

		/**	Converts a specific handle to Resource handle of the resource's base class. */
		template<class BaseResourceType, std::enable_if_t<std::is_base_of_v<BaseResourceType, ResourceType>, int> = 0>
		operator TResourceHandle<BaseResourceType, WeakHandle>() const
		{
			TResourceHandle<BaseResourceType, WeakHandle> handle;
			handle.SetHandleData(this->GetHandleData());

			return handle;
		}

		/**
		 * Returns internal resource pointer.
		 *
		 * @note	Throws exception if handle is invalid.
		 */
		ResourceType* operator->() const { return Get(); }

		/**
		 * Returns internal resource pointer and dereferences it.
		 *
		 * @note	Throws exception if handle is invalid.
		 */
		ResourceType& operator*() const { return *Get(); }

		/** Clears the handle making it invalid and releases any references held to the resource. */
		TResourceHandle<ResourceType, WeakHandle>& operator=(std::nullptr_t rhs)
		{
			this->DecrementReferenceCount();
			this->mData = nullptr;

			return *this;
		}

		/**	Copy assignment. */
		TResourceHandle<ResourceType, WeakHandle>& operator=(const TResourceHandle<ResourceType, WeakHandle>& rhs)
		{
			SetHandleData(rhs.GetHandleData());
			return *this;
		}

		/**	Move assignment. */
		TResourceHandle& operator=(TResourceHandle&& other)
		{
			if(this == &other)
				return *this;

			this->DecrementReferenceCount();
			this->mData = std::exchange(other.mData, nullptr);

			return *this;
		}

		template <class _Ty>
		struct Bool_struct
		{
			int Member;
		};

		/** Allows direct conversion of handle to bool. */
		operator int Bool_struct<ResourceType>::*() const
		{
			return ((this->mData != nullptr && !this->mData->Id.Empty()) ? &Bool_struct<ResourceType>::Member : 0);
		}

		/**
		 * Returns internal resource pointer and dereferences it.
		 *
		 * @note	Throws exception if handle is invalid.
		 */
		ResourceType* Get() const
		{
			this->ThrowIfNotLoaded();

			return reinterpret_cast<ResourceType*>(this->mData->Object.get());
		}

		/**
		 * Returns the internal shared pointer to the resource.
		 *
		 * @note	Throws exception if handle is invalid.
		 */
		SPtr<ResourceType> GetShared() const
		{
			this->ThrowIfNotLoaded();

			return std::static_pointer_cast<ResourceType>(this->mData->Object);
		}

		/** Converts a handle into a weak handle. */
		TResourceHandle<ResourceType, true> GetWeak() const
		{
			TResourceHandle<ResourceType, true> handle;
			handle.SetHandleData(this->GetHandleData());

			return handle;
		}

		/**	Converts a weak handle into a normal handle. */
		TResourceHandle<ResourceType, false> Lock() const
		{
			TResourceHandle<ResourceType, false> handle;
			handle.SetHandleData(this->GetHandleData());

			return handle;
		}

	protected:
		friend Resources;

		template <class ResourceTypeOther, bool IsWeakHandleOther>
		friend class TResourceHandle;

		template <class ResourceTypeLhs, class ResourceTypeRhs, bool IsWeakHandleLhs, bool IsWeakHandleRhs>
		friend TResourceHandle<ResourceTypeLhs, IsWeakHandleLhs> B3DStaticResourceCast(const TResourceHandle<ResourceTypeRhs, IsWeakHandleRhs>& other);

		template <class ResourceTypeLhs, class ResourceTypeRhs, bool IsWeakHandleRhs>
		friend TResourceHandle<ResourceTypeLhs, false> B3DStaticResourceCast(const TResourceHandle<ResourceTypeRhs, IsWeakHandleRhs>& other);

		/**
		 * Constructs a new valid handle for the provided resource with the provided UUID.
		 *
		 * @note	Handle will take ownership of the provided resource pointer, so make sure you don't delete it elsewhere.
		 */
		explicit TResourceHandle(ResourceType* object, const UUID& uuid)
			: TResourceHandleBase<WeakHandle>()
		{
			this->mData = B3DMakeShared<ResourceHandleData>();
			this->IncrementReferenceCount();

			this->SetHandleData(SPtr<Resource>(object), uuid);
			this->mIsCreated = true;
		}

		/**
		 * Constructs an invalid handle with the specified UUID. You must call setHandleData() with the actual resource
		 * pointer to make the handle valid.
		 */
		TResourceHandle(const UUID& uuid)
		{
			this->mData = B3DMakeShared<ResourceHandleData>();
			this->mData->Id = uuid;

			this->IncrementReferenceCount();
		}

		/**	Constructs a new valid handle for the provided resource with the provided UUID. */
		TResourceHandle(const SPtr<ResourceType> object, const UUID& uuid)
		{
			this->mData = B3DMakeShared<ResourceHandleData>();
			this->IncrementReferenceCount();

			this->SetHandleData(object, uuid);
			this->mData->IsCreated = true;
		}

		/**	Replaces the internal handle data pointer, effectively transforming the handle into a different handle. */
		void SetHandleData(const SPtr<ResourceHandleData>& data)
		{
			this->DecrementReferenceCount();
			this->mData = data;
			this->IncrementReferenceCount();
		}

		using ResourceHandleBase::SetHandleData;
	};

	/**	Checks if two handles point to the same resource. */
	template <class ResourceTypeLhs, bool IsWeakHandleLhs, class ResourceTypeRhs, bool IsWeakHandleRhs>
	bool operator==(const TResourceHandle<ResourceTypeLhs, IsWeakHandleLhs>& lhs, const TResourceHandle<ResourceTypeRhs, IsWeakHandleRhs>& rhs)
	{
		if(lhs.GetHandleData() != nullptr && rhs.GetHandleData() != nullptr)
			return lhs.GetHandleData()->Object == rhs.GetHandleData()->Object;

		return lhs.GetHandleData() == rhs.GetHandleData();
	}

	/**	Checks if a handle is null. */
	template <class ResourceType, bool IsWeakHandle>
	bool operator==(const TResourceHandle<ResourceType, IsWeakHandle>& lhs, std::nullptr_t rhs)
	{
		return lhs.GetHandleData() == nullptr || lhs.GetHandleData()->Id.Empty();
	}

	template <class ResourceTypeLhs, bool IsWeakHandleLhs, class ResourceTypeRhs, bool IsWeakHandleRhs>
	bool operator!=(const TResourceHandle<ResourceTypeLhs, IsWeakHandleLhs>& lhs, const TResourceHandle<ResourceTypeRhs, IsWeakHandleRhs>& rhs)
	{
		return (!(lhs == rhs));
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
	template <class ResourceTypeLhs, class ResourceTypeRhs, bool IsWeakHandleLhs, bool IsWeakHandleRhs>
	TResourceHandle<ResourceTypeLhs, IsWeakHandleLhs> B3DStaticResourceCast(const TResourceHandle<ResourceTypeRhs, IsWeakHandleRhs>& other)
	{
		TResourceHandle<ResourceTypeLhs, IsWeakHandleLhs> handle;
		handle.SetHandleData(other.GetHandleData());

		return handle;
	}

	/**	Casts one resource handle to another. */
	template <class ResourceTypeLhs, class ResourceTypeRhs, bool IsWeakHandle>
	TResourceHandle<ResourceTypeLhs, false> B3DStaticResourceCast(const TResourceHandle<ResourceTypeRhs, IsWeakHandle>& other)
	{
		TResourceHandle<ResourceTypeLhs, false> handle;
		handle.SetHandleData(other.GetHandleData());

		return handle;
	}

	/** @} */
} // namespace bs
