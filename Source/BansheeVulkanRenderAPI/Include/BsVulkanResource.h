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
	 */
	class VulkanResource
	{
	public:
		VulkanResource(VulkanResourceManager* owner, bool concurrency);
		virtual ~VulkanResource();

		/** 
		 * Notifies the resource that it is currently being used on the provided command buffer. This means the command
		 * buffer has actually been submitted to the queue and the resource is used by the GPU.
		 * 
		 * A resource can only be used by a single command buffer at a time.
		 */
		virtual void notifyUsed(VulkanCmdBuffer* buffer, VulkanUseFlags flags);

		/** 
		 * Notifies the resource that it is no longer used by on the GPU. This makes the resource usable on other command
		 * buffers again.
		 */
		virtual void notifyDone(VulkanCmdBuffer* buffer);

		/** 
		 * Checks is the resource currently used on a device. 
		 *
		 * @note	Resource usage is only checked at certain points of the program. This means the resource could be
		 *			done on the device but this method may still report true. If you need to know the latest state
		 *			call VulkanCommandBufferManager::refreshStates() before checking for usage.
		 */
		bool isUsed() const { return mNumHandles > 0; }

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

		/** Information about use of this resource on a specific command buffer. */
		struct UseHandle
		{
			VulkanCmdBuffer* buffer;
			VulkanUseFlags flags;
		};

		VulkanResourceManager* mOwner;
		UINT32 mQueueFamily;
		State mState;
		
		UseHandle* mHandles;
		UINT32 mNumHandles;
		UINT32 mHandleCapacity;
		
		static const UINT32 INITIAL_HANDLE_CAPACITY = 2;
		StaticAlloc<sizeof(UseHandle) * INITIAL_HANDLE_CAPACITY> mAlloc;
	};

	/** Creates and destroys annd VulkanResource%s on a single device. */
	class VulkanResourceManager
	{
	public:
		~VulkanResourceManager();

		/** 
		 * Creates a new Vulkan resource of the specified type. User must call VulkanResource::destroy() when done using
		 * the resource. 
		 * 
		 * @param[in]	concurrency		If true, the resource is allowed to be used on multiple queue types at once.
		 */
		template<class Type, class... Args>
		VulkanResource* create(bool concurrency, Args &&...args)
		{
			VulkanResource* resource = new (bs_alloc(sizeof(Type))) Type(this, concurrency, std::forward<Args>(args)...);

#if BS_DEBUG_MODE
			mResources.insert(resource);
#endif

			return resource;
		}

	private:
		friend VulkanResource;

		/** 
		 * Destroys a previously created Vulkan resource. Caller must ensure the resource is not currently being used
		 * on the device.
		 */
		void destroy(VulkanResource* resource);

#if BS_DEBUG_MODE
		UnorderedSet<VulkanResource*> mResources;
#endif
	};

	/** @} */
}
