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
		SPtr<Resource> mPtr;
		UUID mUUID;
		bool mIsCreated = false;
		std::atomic<std::uint32_t> mRefCount{0};
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
		bool isLoaded(bool checkDependencies = true) const;

		/**
		 * Blocks the current thread until the resource is fully loaded.
		 * 			
		 * @note	Careful not to call this on the thread that does the loading.
		 */
		void blockUntilLoaded(bool waitForDependencies = true) const;

		/**
		 * Releases an internal reference to this resource held by the resources system, if there is one.
		 * 			
		 * @see		Resources::release(ResourceHandleBase&)
		 */
		void release();

		/** Returns the UUID of the resource the handle is referring to. */
		const UUID& getUUID() const { return mData != nullptr ? mData->mUUID : UUID::EMPTY; }

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**	Gets the handle data. For internal use only. */
		const SPtr<ResourceHandleData>& getHandleData() const { return mData; }

		/** @} */
	protected:
		/**	Destroys the resource the handle is pointing to. */
		void destroy();

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
		void setHandleData(const SPtr<Resource>& ptr, const UUID& uuid);

		/**
		 * Clears the created flag and the resource pointer, making the handle invalid until the resource is loaded again
		 * and assigned through setHandleData().
		 */
		void clearHandleData();

		/** Increments the reference count of the handle. Only to be used by Resources for keeping internal references. */
		void addInternalRef();

		/** Decrements the reference count of the handle. Only to be used by Resources for keeping internal references. */
		void removeInternalRef();

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
		void throwIfNotLoaded() const;
	};

	/**
	 * @copydoc	ResourceHandleBase
	 *
	 * Handles differences in reference counting depending if the handle is normal or weak.
	 */
	template <bool WeakHandle>
	class BS_CORE_EXPORT TResourceHandleBase : public ResourceHandleBase
	{ };

	/**	Specialization of TResourceHandleBase for weak handles. Weak handles do no reference counting. */
	template<>
	class BS_CORE_EXPORT TResourceHandleBase<true> : public ResourceHandleBase
	{
	protected:
		void addRef() { };
		void releaseRef() { };

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class WeakResourceHandleRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**	Specialization of TResourceHandleBase for normal (non-weak) handles. */
	template<>
	class BS_CORE_EXPORT TResourceHandleBase<false> : public ResourceHandleBase
	{
	protected:
		void addRef()
		{
			if (mData) 
				mData->mRefCount.fetch_add(1, std::memory_order_relaxed);
		};

		void releaseRef() 
		{ 
			if (mData)
			{
				std::uint32_t refCount = mData->mRefCount.fetch_sub(1, std::memory_order_release);

				if (refCount == 1)
				{
					std::atomic_thread_fence(std::memory_order_acquire);
					destroy();
				}
			}
		};

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class WeakResourceHandleRTTI;
		friend class ResourceHandleRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @copydoc ResourceHandleBase */
	template <typename T, bool WeakHandle>
	class TResourceHandle : public TResourceHandleBase<WeakHandle>
	{
	public:
		TResourceHandle() = default;

		/**	Copy constructor. */
		TResourceHandle(const TResourceHandle& other)
		{
			this->mData = other.getHandleData();
			this->addRef();
		}

		/** Move constructor. */
		TResourceHandle(TResourceHandle&& other) = default;

		~TResourceHandle()
		{
			this->releaseRef();
		}

		/**	Converts a specific handle to generic Resource handle. */
		operator TResourceHandle<Resource, WeakHandle>() const
		{
			TResourceHandle<Resource, WeakHandle> handle;
			handle.setHandleData(this->getHandleData());

			return handle;
		}

		/**
		 * Returns internal resource pointer.
		 *
		 * @note	Throws exception if handle is invalid.
		 */
		T* operator->() const { return get(); }

		/**
		 * Returns internal resource pointer and dereferences it.
		 *
		 * @note	Throws exception if handle is invalid.
		 */
		T& operator*() const { return *get(); }

		/** Clears the handle making it invalid and releases any references held to the resource. */
		TResourceHandle<T, WeakHandle>& operator=(std::nullptr_t ptr)
		{ 	
			this->releaseRef();
			this->mData = nullptr;

			return *this;
		}

		/**	Copy assignment. */
		TResourceHandle<T, WeakHandle>& operator=(const TResourceHandle<T, WeakHandle>& rhs)
		{ 	
			setHandleData(rhs.getHandleData());
			return *this;
		}

		/**	Move assignment. */
		TResourceHandle& operator=(TResourceHandle&& other)
		{
			if(this == &other)
				return *this;

			this->releaseRef();
			this->mData = std::exchange(other.mData, nullptr);

			return *this;
		}

		template<class _Ty>
		struct Bool_struct
		{
			int _Member;
		};

		/**
		 * Allows direct conversion of handle to bool.
		 *
		 * @note	This is needed because we can't directly convert to bool since then we can assign pointer to bool and 
		 *			that's weird.
		 */
		operator int Bool_struct<T>::*() const
		{
			return ((this->mData != nullptr && !this->mData->mUUID.empty()) ? &Bool_struct<T>::_Member : 0);
		}

		/**
		 * Returns internal resource pointer and dereferences it.
		 *
		 * @note	Throws exception if handle is invalid.
		 */
		T* get() const 
		{ 
			this->throwIfNotLoaded();

			return reinterpret_cast<T*>(this->mData->mPtr.get());
		}

		/**
		 * Returns the internal shared pointer to the resource.
		 *
		 * @note	Throws exception if handle is invalid.
		 */
		SPtr<T> getInternalPtr() const
		{ 
			this->throwIfNotLoaded();

			return std::static_pointer_cast<T>(this->mData->mPtr);
		}

		/** Converts a handle into a weak handle. */
		TResourceHandle<T, true> getWeak() const
		{
			TResourceHandle<T, true> handle;
			handle.setHandleData(this->getHandleData());

			return handle;
		}

	protected:
		friend Resources;
		template<class _T, bool _Weak>
		friend class TResourceHandle;
		template<class _Ty1, class _Ty2, bool Weak>
		friend TResourceHandle<_Ty1, Weak> static_resource_cast(const TResourceHandle<_Ty2, Weak>& other);

		/**
		 * Constructs a new valid handle for the provided resource with the provided UUID.
		 *			
		 * @note	Handle will take ownership of the provided resource pointer, so make sure you don't delete it elsewhere.
		 */
		explicit TResourceHandle(T* ptr, const UUID& uuid)
			:TResourceHandleBase<WeakHandle>()
		{
			this->mData = bs_shared_ptr_new<ResourceHandleData>();
			this->addRef();

			this->setHandleData(SPtr<Resource>(ptr), uuid);
		}

		/**
		 * Constructs an invalid handle with the specified UUID. You must call setHandleData() with the actual resource 
		 * pointer to make the handle valid.
		 */
		TResourceHandle(const UUID& uuid)
		{
			this->mData = bs_shared_ptr_new<ResourceHandleData>();
			this->mData->mUUID = uuid;

			this->addRef();
		}

		/**	Constructs a new valid handle for the provided resource with the provided UUID. */
		TResourceHandle(const SPtr<T> ptr, const UUID& uuid)
		{
			this->mData = bs_shared_ptr_new<ResourceHandleData>();
			this->addRef();

			setHandleData(ptr, uuid);
		}

		/**	Replaces the internal handle data pointer, effectively transforming the handle into a different handle. */
		void setHandleData(const SPtr<ResourceHandleData>& data)
		{
			this->releaseRef();
			this->mData = data;
			this->addRef();
		}

		/**	Converts a weak handle into a normal handle. */
		TResourceHandle<T, false> lock() const
		{
			TResourceHandle<Resource, false> handle;
			handle.setHandleData(this->getHandleData());

			return handle;
		}

		using ResourceHandleBase::setHandleData;
	};

	/**	Checks if two handles point to the same resource. */
	template<class _Ty1, bool _Weak1, class _Ty2, bool _Weak2>
	bool operator==(const TResourceHandle<_Ty1, _Weak1>& _Left, const TResourceHandle<_Ty2, _Weak2>& _Right)
	{	
		if(_Left.getHandleData() != nullptr && _Right.getHandleData() != nullptr)
			return _Left.getHandleData()->mPtr == _Right.getHandleData()->mPtr;

		return _Left.getHandleData() == _Right.getHandleData();
	}

	/**	Checks if a handle is null. */
	template<class _Ty1, bool _Weak1, class _Ty2, bool _Weak2>
	bool operator==(const TResourceHandle<_Ty1, _Weak1>& _Left, std::nullptr_t  _Right)
	{	
		return _Left.getHandleData() == nullptr || _Left.getHandleData()->mUUID.empty();
	}

	template<class _Ty1, bool _Weak1, class _Ty2, bool _Weak2>
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
	template<class _Ty1, class _Ty2, bool Weak>
	TResourceHandle<_Ty1, Weak> static_resource_cast(const TResourceHandle<_Ty2, Weak>& other)
	{
		TResourceHandle<_Ty1, Weak> handle;
		handle.setHandleData(other.getHandleData());

		return handle;
	}

	/** @} */
}
