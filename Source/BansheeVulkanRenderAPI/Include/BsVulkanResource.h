//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsStaticAlloc.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Flags that determine how is a resource being used by the GPU. */
	enum class VulkanUseFlag
	{
		None = 0,
		Read = 0x1,
		Write = 0x2
	};

	class VulkanResourceManager;

	typedef Flags<VulkanUseFlag> VulkanUseFlags;
	BS_FLAGS_OPERATORS(VulkanUseFlag);

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
		 */
		void notifyUsed(VulkanCmdBuffer* buffer, VulkanUseFlags useFlags);

		/** 
		 * Notifies the resource that it is no longer used by on the GPU. This makes the resource usable on other command
		 * buffers again.
		 * 
		 * Must follow a notifyUsed().
		 */
		void notifyDone();

		/** 
		 * Notifies the resource that it is no longer queued on the command buffer. This is similar to notifyDone(), but
		 * should only be called if resource never got submitted to the GPU (e.g. command buffer was destroyed before
		 * being submitted).
		 * 
		 * Must follow a notifyBound() if notifyUsed() wasn't called.
		 */
		void notifyUnbound();

		/** 
		 * Checks is the resource currently used on a device. 
		 *
		 * @note	Resource usage is only checked at certain points of the program. This means the resource could be
		 *			done on the device but this method may still report true. If you need to know the latest state
		 *			call VulkanCommandBufferManager::refreshStates() before checking for usage.
		 */
		bool isUsed() const { Lock(mMutex); return mNumUsedHandles > 0; }

		/** 
		 * Checks is the resource currently bound to any command buffer.
		 *
		 * @note	Resource usage is only checked at certain points of the program. This means the resource could be
		 *			done on the device but this method may still report true. If you need to know the latest state
		 *			call VulkanCommandBufferManager::refreshStates() before checking for usage.
		 */
		bool isBound() const { Lock(mMutex); return mNumBoundHandles > 0; }

		/** 
		 * Returns the queue family the resource is currently owned by. Returns -1 if owned by no queue.
		 * 
		 * @note	If resource concurrency is enabled, then this value has no meaning as the resource can be used on
		 *			multiple queue families at once.
		 */
		UINT32 getQueueFamily() const { Lock(mMutex); return mQueueFamily; }

		/** Returns true if the resource is only allowed to be used by a single queue family at once. */
		bool isExclusive() const { Lock(mMutex); return mState != State::Shared; }

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

		VulkanResourceManager* mOwner;
		UINT32 mQueueFamily;
		State mState;
		VulkanUseFlags mUseFlags;
		
		UINT32 mNumUsedHandles;
		UINT32 mNumBoundHandles;

		Mutex mMutex;
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
}
