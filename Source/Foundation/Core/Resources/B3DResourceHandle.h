//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Reflection/B3DIReflectable.h"
#include "Threading/B3DSignal.h"
#include "Utility/B3DUUID.h"

namespace b3d
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**	Data that is shared between all resource handles. */
	struct B3D_EXPORT ResourceHandleData
	{
		ResourceHandleData() = default;

		ResourceHandleData(const UUID& id)
			: Id(id)
		{ }

		ResourceHandleData(const SPtr<Resource>& object, const UUID& id)
			: Object(object), Id(id), IsCreated(true)
		{ }

		/** Increments the strong reference count. As long as strong reference count is non-zero the handle will keep the managed resource alive. */
		void IncrementStrongReferenceCount()
		{
			StrongReferenceCount.fetch_add(1, std::memory_order_relaxed);
		}

		/**
		 * Decrements the strong reference count. If the strong reference count reaches zero the managed resource will be destroyed. Additionally
		 * if there are no weak resource handles alive either, resource handle data will also be destroyed. 
		 */
		void DecrementStrongReferenceCount()
		{
			if(StrongReferenceCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
			{
				DestroyManagedResource();
				DecrementWeakReferenceCount();
			}
		}

		/** Increments the weak reference count. This keeps the resource handle data alive, but not the managed resource itself. */
		void IncrementWeakReferenceCount()
		{
			WeakReferenceCount.fetch_add(1, std::memory_order_relaxed);
		}

		/**
		 * Decrements the weak reference count. If this was the last weak reference and there are no strong references either, handle data
		 * will be destroyed.
		 */
		void DecrementWeakReferenceCount()
		{
			if(WeakReferenceCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
				DestroySelf();
		}

		/** Increments the strong reference count, but only if it is not already at zero. Returns true if incremented. */
		bool IncrementStrongReferenceCountIfNonZero()
		{
			std::uint32_t referenceCount = StrongReferenceCount.load(std::memory_order_acquire);
			while(referenceCount != 0)
			{
				if(StrongReferenceCount.compare_exchange_weak(referenceCount, referenceCount + 1, std::memory_order_release, std::memory_order_relaxed))
					return true;
			}

			return false;
		}

		/**	Destroys the resource the handle is pointing to. */
		void DestroyManagedResource();

		/** Destroys the handle data. */
		void DestroySelf();

		SPtr<Resource> Object;
		UUID Id;
		bool IsCreated = false;
		std::atomic<std::uint32_t> StrongReferenceCount{ 1 }; /**< References keeping the resource alive (strong handles). */
		std::atomic<std::uint32_t> WeakReferenceCount{ 1 }; /**< References keeping the resource handle data alive (weak handles + 1 if any strong handle is alive). */
	};

	/**
	 * Represents a handle to a resource. Handles are similar to a smart pointers, but they have two advantages:
	 *	- When loading a resource asynchronously you can be immediately returned the handle that you may use throughout
	 *    the engine. The handle will be made valid as soon as the resource is loaded.
	 *	- Handles can be serialized and deserialized, therefore saving/restoring references to their original resource.
	 */
	class B3D_EXPORT ResourceHandle : public IReflectable
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

		/** Checks if the resource handle is not null and resource is loaded. */
		bool IsValid() const { return IsLoaded(false); }

		/**
		 * Blocks the current thread until the resource is fully loaded.
		 *
		 * @note	Careful not to call this on the thread that does the loading.
		 */
		void BlockUntilLoaded(bool waitForDependencies = true) const;

		/** Forces the resource to be unloaded and destroyed, regardless of reference count. */
		void Destroy() const;

		/**
		 * Releases an internal reference to this resource held by the resources system, if there is one.
		 *
		 * @see		Resources::ReleaseInternalReference(ResourceHandle&)
		 */
		void ReleaseInternalReference();

		/** Returns the number of strong references on the resource pointed by the handle. */
		u32 GetReferenceCount() const { return mData != nullptr ? mData->StrongReferenceCount.load(std::memory_order_relaxed) : 0; }

		/** Returns the UUID of the resource the handle is referring to. */
		const UUID& GetId() const { return mData != nullptr ? mData->Id : UUID::kEmpty; }

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**	Gets the handle data. For internal use only. */
		ResourceHandleData* GetHandleData() const { return mData; }

		/** @} */
	protected:
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
		void AssociateResourceWithHandle(const SPtr<Resource>& resource, const UUID& resourceId);

		/** Increments the strong reference count. As long as strong reference count is non-zero the handle will keep the managed resource alive. */
		void IncrementStrongReferenceCount() const
		{
			if(mData != nullptr)
				mData->IncrementStrongReferenceCount();
		}

		/**
		 * Decrements the strong reference count. If the strong reference count reaches zero the managed resource will be destroyed. Additionally
		 * if there are no weak resource handles alive either, resource handle data will also be destroyed. 
		 */
		void DecrementStrongReferenceCount() const
		{
			if(mData != nullptr)
				mData->DecrementStrongReferenceCount();
		}

		/** Increments the weak reference count. This keeps the resource handle data alive, but not the managed resource itself. */
		void IncrementWeakReferenceCount() const
		{
			if(mData != nullptr)
				mData->IncrementWeakReferenceCount();
		}

		/**
		 * Decrements the weak reference count. If this was the last weak reference and there are no strong references either, handle data
		 * will be destroyed.
		 */
		void DecrementWeakReferenceCount() const
		{
			if(mData != nullptr)
				mData->DecrementWeakReferenceCount();
		}

		/**
		 * Notification sent by the resource system when the resource is done with the loading process. This will trigger
		 * even if the load fails.
		 */
		void NotifyLoadComplete();

		/**
		 * @note
		 * All handles to the same source must share this same handle data. Otherwise things like counting number of
		 * references or replacing pointed to resource become impossible without additional logic.
		 */
		ResourceHandleData* mData = nullptr;

	private:
		friend class Resources;

		static Signal mResourceCreatedCondition;
		static Mutex mResourceCreatedMutex;

	protected:
		void ThrowIfNotLoaded() const;
	};

	/**	Implementation of ResourceHandle for weak handles. Weak handles do no reference counting. */
	class B3D_EXPORT WeakResourceHandle : public ResourceHandle
	{
	protected:
		void IncrementReferenceCount() const { IncrementWeakReferenceCount(); }
		void DecrementReferenceCount() const { DecrementWeakReferenceCount(); }

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class WeakResourceHandleRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Implementation of ResourceHandle for strong (non-weak) handles. */
	class B3D_EXPORT StrongResourceHandle : public ResourceHandle
	{
	protected:
		void IncrementReferenceCount() const { IncrementStrongReferenceCount(); }
		void DecrementReferenceCount() const { DecrementStrongReferenceCount(); }

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class WeakResourceHandleRTTI;
		friend class StrongResourceHandleRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @copydoc ResourceHandle */
	template <typename ResourceType, bool IsWeakHandle = false>
	class TResourceHandle : public std::conditional_t<IsWeakHandle, WeakResourceHandle, StrongResourceHandle>
	{
	public:
		TResourceHandle() = default;

		TResourceHandle(std::nullptr_t) {}

		/**	Copy constructor. */
		TResourceHandle(const TResourceHandle& other)
		{
			other.IncrementReferenceCount();
			this->mData = other.GetHandleData();
		}

		/**	Copy constructor allowing conversion from derived to base type. */
		template<class DerivedResourceType, std::enable_if_t<std::disjunction_v<std::is_same<ResourceType, Resource>, std::is_base_of<ResourceType, DerivedResourceType>>, int> = 0>
		TResourceHandle(const TResourceHandle<DerivedResourceType, IsWeakHandle>& other)
		{
			// Above enable_if purposefully ignores is_base_of<> check if base is Resource. This is because is_base_of<> requires a fully defined type, requiring excessive #includes.

			other.IncrementReferenceCount();
			this->mData = other.GetHandleData();
		}

		/** Move constructor. */
		TResourceHandle(TResourceHandle&& other) noexcept
		{
			this->mData = std::exchange(other.mData, nullptr);
		}

		~TResourceHandle()
		{
			this->DecrementReferenceCount();
		}

		/**	Converts a specific handle to generic Resource handle. */
		operator TResourceHandle<Resource, IsWeakHandle>()
		{
			return TResourceHandle<Resource, IsWeakHandle>(*this);
		}

		/**	Converts a specific handle to Resource handle of the resource's base class. */
		template<class BaseResourceType, std::enable_if_t<std::conjunction_v<std::negation<std::is_same<BaseResourceType, Resource>>, std::is_base_of<BaseResourceType, ResourceType>>, int> = 0>
		operator TResourceHandle<BaseResourceType, IsWeakHandle>()
		{
			// Above enable_if purposefully ignores is_base_of<> check if base is Resource. This is because is_base_of<> requires a fully defined type, requiring excessive #includes.

			return TResourceHandle<BaseResourceType, IsWeakHandle>(*this);
		}

		/** Swaps the contents of this handle with another. */
		void Swap(TResourceHandle& other)
		{
			std::swap(this->mData, other.mData);
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
		TResourceHandle& operator=(std::nullptr_t rhs)
		{
			TResourceHandle(rhs).Swap(*this);
			return *this;
		}

		/**	Copy assignment. */
		TResourceHandle& operator=(const TResourceHandle& rhs)
		{
			TResourceHandle(rhs).Swap(*this);
			return *this;
		}

		/**	Move assignment. */
		TResourceHandle& operator=(TResourceHandle&& rhs) noexcept
		{
			TResourceHandle(std::move(rhs)).Swap(*this);
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
		template <bool IsWeakHandleAlias = IsWeakHandle, std::enable_if_t<!IsWeakHandleAlias, int> = 0>
		TResourceHandle<ResourceType, true> GetWeak() const
		{
			return TResourceHandle<ResourceType, true>(*this);
		}

		/**	Converts a weak handle into a normal handle. */
		template <bool IsWeakHandleAlias = IsWeakHandle, std::enable_if_t<IsWeakHandleAlias, int> = 0>
		TResourceHandle<ResourceType, false> Lock() const
		{
			return TResourceHandle<ResourceType, false>(*this);
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
		 * Constructs an invalid handle with the specified Id. You must call AssociateResourceWithHandle() with the actual resource
		 * pointer to make the handle valid.
		 */
		TResourceHandle(const UUID& resourceId)
		{
			this->mData = B3DNew<ResourceHandleData>(resourceId);
		}

		/**	Constructs a new valid handle for the provided resource with the provided ID. */
		TResourceHandle(const SPtr<ResourceType> object, const UUID& resourceId)
		{
			this->mData = B3DNew<ResourceHandleData>(object, resourceId);
		}

		/**	Constructs a new handle from existing handle data. */
		TResourceHandle(ResourceHandleData* handleData)
		{
			if(handleData != nullptr)
			{
				if constexpr(IsWeakHandle)
					handleData->IncrementWeakReferenceCount();
				else
					handleData->IncrementStrongReferenceCount();
			}

			this->mData = handleData;
		}

		/**	Copy constructor allowing weak -> strong handle change. */
		template <bool IsWeakHandleAlias = IsWeakHandle, std::enable_if_t<!IsWeakHandleAlias, int> = 0>
		TResourceHandle(const TResourceHandle<ResourceType, true>& other)
		{
			if(other.mData != nullptr && other.mData->IncrementStrongReferenceCountIfNonZero())
				this->mData = other.GetHandleData();
		}

		/**	Copy constructor allowing strong -> weak handle change. */
		template <bool IsWeakHandleAlias = IsWeakHandle, std::enable_if_t<IsWeakHandleAlias, int> = 0>
		TResourceHandle(const TResourceHandle<ResourceType, false>& other)
		{
			other.IncrementWeakReferenceCount();
			this->mData = other.GetHandleData();
		}

		/** Copy constructor allowing conversion from base to derived type, for casts. */
		template<class DerivedResourceType, std::enable_if_t<!std::disjunction_v<std::is_same<ResourceType, Resource>, std::is_base_of<ResourceType, DerivedResourceType>>, int> = 0>
		TResourceHandle(const TResourceHandle<DerivedResourceType, IsWeakHandle>& other)
		{
			// Above enable_if purposefully ignores is_base_of<> check if base is Resource. This is because is_base_of<> requires a fully defined type, requiring excessive #includes.

			other.IncrementReferenceCount();
			this->mData = other.GetHandleData();
		}

		using ResourceHandle::AssociateResourceWithHandle;
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

	/**
	 * @copydoc ResourceHandle
	 *
	 * Weak handles don't prevent the resource from being unloaded.
	 */
	template <typename T>
	using TWeakResourceHandle = TResourceHandle<T, true>;

	/**	Casts one resource handle to another. */
	template <class ResourceTypeLhs, class ResourceTypeRhs, bool IsWeakHandleLhs, bool IsWeakHandleRhs>
	TResourceHandle<ResourceTypeLhs, IsWeakHandleLhs> B3DStaticResourceCast(const TResourceHandle<ResourceTypeRhs, IsWeakHandleRhs>& other)
	{
		return TResourceHandle<ResourceTypeLhs, IsWeakHandleLhs>(other);
	}

	/**	Casts one resource handle to another. */
	template <class ResourceTypeLhs, class ResourceTypeRhs, bool IsWeakHandleRhs>
	TResourceHandle<ResourceTypeLhs, false> B3DStaticResourceCast(const TResourceHandle<ResourceTypeRhs, IsWeakHandleRhs>& other)
	{
		return TResourceHandle<ResourceTypeLhs, false>(other);
	}

	/** @} */
} // namespace b3d

/** @cond STDLIB */

namespace std
{
/** Hash value generator for TResourceHandle<T>. */
template<class T>
struct hash<b3d::TResourceHandle<T>>
{
	size_t operator()(const b3d::TResourceHandle<T>& value) const
	{
		size_t hash = 0;
		b3d::B3DCombineHash(hash, value.GetHandleData());

		return hash;
	}
};
} // namespace std

/** @endcond */
