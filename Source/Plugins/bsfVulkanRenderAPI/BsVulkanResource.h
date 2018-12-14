//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Allocators/BsStaticAlloc.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Flags that determine how is a resource being accessed by the GPU. */
	enum class VulkanAccessFlag
	{
		None = 0,
		Read = 0x1,
		Write = 0x2
	};

	class VulkanResourceManager;

	typedef Flags<VulkanAccessFlag> VulkanAccessFlags;
	BS_FLAGS_OPERATORS(VulkanAccessFlag);

	/** 
	 * Wraps one or multiple native Vulkan objects. Allows the object usage to be tracked in command buffers, handles
	 * ownership transitions between different queues, and handles delayed object destruction.
	 * 
	 * @note Thread safe
	 */
	class VulkanResource
	{
	public:
		VulkanResource(VulkanResourceManager* owner, bool concurrency);
		virtual ~VulkanResource();

		/** 
		 * Notifies the resource that it is currently bound to a command buffer. Buffer hasn't yet been submitted so the
		 * resource isn't being used on the GPU yet. 
		 * 
		 * Must eventually be followed by a notifyUsed() or notifyUnbound().
		 */
		void notifyBound();

		/** 
		 * Notifies the resource that it is currently being used on the provided command buffer. This means the command
		 * buffer has actually been submitted to the queue and the resource is used by the GPU.
		 * 
		 * A resource can only be used by a single command buffer at a time unless resource concurrency is enabled.
		 * 
		 * Must follow a notifyBound(). Must eventually be followed by a notifyDone().
		 * 
		 * @param[in]	globalQueueIdx	Global index of the queue the resource is being used in.
		 * @param[in]	queueFamily		Family of the queue the resource is being used in.
		 * @param[in]	useFlags		Flags that determine in what way is the resource being used.
		 */
		void notifyUsed(UINT32 globalQueueIdx, UINT32 queueFamily, VulkanAccessFlags useFlags);

		/** 
		 * Notifies the resource that it is no longer used by on the GPU. This makes the resource usable on other command
		 * buffers again.
		 * 
		 * Must follow a notifyUsed().
		 * 
		 * @param[in]	globalQueueIdx	Global index of the queue that finished using the resource.
		 * @param[in]	useFlags		Use flags that specify how was the resource being used.
		 */
		virtual void notifyDone(UINT32 globalQueueIdx, VulkanAccessFlags useFlags);

		/** 
		 * Notifies the resource that it is no longer queued on the command buffer. This is similar to notifyDone(), but
		 * should only be called if resource never got submitted to the GPU (e.g. command buffer was destroyed before
		 * being submitted).
		 * 
		 * Must follow a notifyBound() if notifyUsed() wasn't called.
		 */
		virtual void notifyUnbound();

		/** 
		 * Checks is the resource currently used on a device. 
		 *
		 * @note	Resource usage is only checked at certain points of the program. This means the resource could be
		 *			done on the device but this method may still report true. If you need to know the latest state
		 *			call VulkanCommandBufferManager::refreshStates() before checking for usage.
		 */
		bool isUsed() const { Lock lock(mMutex); return mNumUsedHandles > 0; }

		/** 
		 * Checks is the resource currently bound to any command buffer.
		 *
		 * @note	Resource usage is only checked at certain points of the program. This means the resource could be
		 *			done on the device but this method may still report true. If you need to know the latest state
		 *			call VulkanCommandBufferManager::refreshStates() before checking for usage.
		 */
		bool isBound() const { Lock lock(mMutex); return mNumBoundHandles > 0; }

		/** 
		 * Returns the queue family the resource is currently owned by. Returns -1 if owned by no queue.
		 * 
		 * @note	If resource concurrency is enabled, then this value has no meaning as the resource can be used on
		 *			multiple queue families at once.
		 */
		UINT32 getQueueFamily() const { Lock lock(mMutex); return mQueueFamily; }

		/** 
		 * Returns a mask that has bits set for every queue that the resource is currently used (read or written) by.
		 *
		 * @param[in]	useFlags	Flags for which to check use information (e.g. read only, write only, or both).
		 * @return					Bitmask of which queues is the resource used on. This has the same format as sync mask
		 *							created by CommandSyncMask.
		 */
		UINT32 getUseInfo(VulkanAccessFlags useFlags) const;

		/** Returns on how many command buffers is the buffer currently used on. */
		UINT32 getUseCount() const { return mNumUsedHandles; }

		/** Returns on how many command buffers is the buffer currently bound on. */
		UINT32 getBoundCount() const { return mNumBoundHandles; }

		/** Returns true if the resource is only allowed to be used by a single queue family at once. */
		bool isExclusive() const { Lock lock(mMutex); return mState != State::Shared; }

		/** Returns the device this resource is created on. */
		VulkanDevice& getDevice() const;

		/** 
		 * Destroys the resource and frees its memory. If the resource is currently being used on a device, the
		 * destruction is delayed until the device is done with it.
		 */
		void destroy();

	protected:
		/** Possible states of this object. */
		enum class State
		{
			Normal,
			Shared,
			Destroyed
		};

		static const UINT32 MAX_UNIQUE_QUEUES = BS_MAX_QUEUES_PER_TYPE * GQT_COUNT;

		VulkanResourceManager* mOwner;
		UINT32 mQueueFamily;
		State mState;

		UINT8 mReadUses[MAX_UNIQUE_QUEUES];
		UINT8 mWriteUses[MAX_UNIQUE_QUEUES];
		
		UINT32 mNumUsedHandles;
		UINT32 mNumBoundHandles;

		mutable Mutex mMutex;
	};

	/** 
	 * Creates and destroys annd VulkanResource%s on a single device. 
	 * 
	 * @note Thread safe
	 */
	class VulkanResourceManager
	{
	public:
		VulkanResourceManager(VulkanDevice& device);
		~VulkanResourceManager();

		/** 
		 * Creates a new Vulkan resource of the specified type. User must call VulkanResource::destroy() when done using
		 * the resource. 
		 */
		template<class Type, class... Args>
		Type* create(Args &&...args)
		{
			Type* resource = new (bs_alloc(sizeof(Type))) Type(this, std::forward<Args>(args)...);

#if BS_DEBUG_MODE
			Lock lock(mMutex);
			mResources.insert(resource);
#endif

			return resource;
		}

		/** Returns the device that owns this manager. */
		VulkanDevice& getDevice() const { return mDevice; }

	private:
		friend VulkanResource;

		/** 
		 * Destroys a previously created Vulkan resource. Caller must ensure the resource is not currently being used
		 * on the device.
		 */
		void destroy(VulkanResource* resource);

		VulkanDevice& mDevice;

#if BS_DEBUG_MODE
		UnorderedSet<VulkanResource*> mResources;
		Mutex mMutex;
#endif
	};

	/** @} */
}}