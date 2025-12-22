//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "Allocators/B3DStaticAlloc.h"
#include "RenderAPI/B3DGpuDevice.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		class VulkanResourceManager;

		/**
		 * Wraps one or multiple native Vulkan objects. Allows the object usage to be tracked in command buffers, handles
		 * ownership transitions between different queues, and handles delayed object destruction.
		 *
		 * @note Thread safe
		 */
		class VulkanResource
		{
		public:
			static constexpr u32 kMaximumUniqueQueueCount = B3D_MAX_QUEUES_PER_TYPE * GQT_COUNT;

			VulkanResource(VulkanResourceManager* owner, bool concurrency, const StringView& name);
			virtual ~VulkanResource();

			/** Sets a name of the resource, primarily used for debugging. */
			void SetDebugName(const StringView& name)
			{
#if B3D_BUILD_TYPE_DEVELOPMENT
				mDebugName = name;
#endif
			}

			/**
			 * Notifies the resource that it is currently bound to a command buffer. Buffer hasn't yet been submitted so the
			 * resource isn't being used on the GPU yet.
			 *
			 * Must eventually be followed by a NotifyUsed() or NotifyUnbound().
			 */
			void NotifyBound();

			/**
			 * Notifies the resource that it is currently being used on the provided command buffer. This means the command
			 * buffer has actually been submitted to the queue and the resource is used by the GPU.
			 *
			 * A resource can only be used by a single command buffer at a time unless resource concurrency is enabled.
			 *
			 * Must follow a NotifyBound(). Must eventually be followed by a NotifyDone().
			 *
			 * @param[in]	queueId			ID of the queue the resource is being used in.
			 * @param[in]	useFlags		Flags that determine in what way is the resource being used.
			 */
			void NotifyUsed(GpuQueueId queueId, GpuAccessFlags useFlags);

			/**
			 * Notifies the resource that it is no longer used by on the GPU. This makes the resource usable on other command
			 * buffers again.
			 *
			 * Must follow a NotifyUsed().
			 *
			 * @param[in]	queueId			ID of the queue the resource is being used in.
			 * @param[in]	useFlags		Use flags that specify how was the resource being used.
			 */
			virtual void NotifyDone(GpuQueueId queueId, GpuAccessFlags useFlags);

			/**
			 * Notifies the resource that it is no longer queued on the command buffer. This is similar to notifyDone(), but
			 * should only be called if resource never got submitted to the GPU (e.g. command buffer was destroyed before
			 * being submitted).
			 *
			 * Must follow a NotifyBound() if NotifyUsed() wasn't called.
			 */
			virtual void NotifyUnbound();

			/**
			 * Checks is the resource currently used on a device.
			 *
			 * @note	Resource usage is only checked at certain points of the program. This means the resource could be
			 *			done on the device but this method may still report true.
			 */
			bool IsUsed() const
			{
				Lock lock(mMutex);
				return mUsedCount > 0;
			}

			/**
			 * Checks is the resource currently bound to any command buffer.
			 *
			 * @note	Resource usage is only checked at certain points of the program. This means the resource could be
			 *			done on the device but this method may still report true.
			 */
			bool IsBound() const
			{
				Lock lock(mMutex);
				return mBountCount > 0;
			}

			/** Checks has the resource been destroyed. */
			bool IsDestroyed() const
			{
				Lock lock(mMutex);
				return mState == State::Destroyed;
			}

			/**
			 * Returns the queue usage the resource is currently owned by. Returns -1 if owned by no queue.
			 *
			 * @note	If resource concurrency is enabled, then this value has no meaning as the resource can be used on
			 *			multiple queue families at once.
			 */
			GpuQueueType GetOwnedQueueType() const
			{
				Lock lock(mMutex);
				return mOwnedQueueType;
			}

			/**
			 * Returns a mask that has bits set for every queue that the resource is currently used (read or written) by.
			 *
			 * @param[in]	useFlags	Flags for which to check use information (e.g. read only, write only, or both).
			 * @return					Bitmask of which queues is the resource used on. 
			 */
			GpuQueueMask GetUseInfo(GpuAccessFlags useFlags) const;

			/** Returns on how many command buffers is the buffer currently used on. */
			u32 GetUseCount() const { return mUsedCount; }

			/** Returns on how many command buffers is the buffer currently bound on. */
			u32 GetBoundCount() const { return mBountCount; }

			/** Returns true if the resource is only allowed to be used by a single queue family at once. */
			bool IsExclusive() const
			{
				Lock lock(mMutex);
				return mState != State::Shared;
			}

			/** Returns the device this resource is created on. */
			VulkanGpuDevice& GetDevice() const;

			/**
			 * Destroys the resource and frees its memory. If the resource is currently being used on a device, the
			 * destruction is delayed until the device is done with it.
			 */
			virtual void Destroy();

		protected:
			/** Possible states of this object. */
			enum class State
			{
				Normal,
				Shared,
				Destroyed
			};

			VulkanResourceManager* mOwner;
			GpuQueueType mOwnedQueueType = GQT_UNKNOWN;
			State mState;
			String mDebugName;

			u8 mReadUses[kMaximumUniqueQueueCount];
			u8 mWriteUses[kMaximumUniqueQueueCount];

			u32 mUsedCount;
			u32 mBountCount;

			// TODO - Work on getting rid of this mutex
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
			VulkanResourceManager(VulkanGpuDevice& device);
			~VulkanResourceManager();

			/**
			 * Creates a new Vulkan resource of the specified type. User must call VulkanResource::Destroy() when done using
			 * the resource.
			 */
			template <class Type, class... Args>
			Type* Create(Args&&... args)
			{
				Type* resource = new(B3DAllocate(sizeof(Type))) Type(this, std::forward<Args>(args)...);

#if B3D_DEBUG
				Lock lock(mMutex);
				mResources.insert(resource);
#endif

				return resource;
			}

			/** Returns the device that owns this manager. */
			VulkanGpuDevice& GetDevice() const { return mDevice; }

		private:
			friend VulkanResource;

			/**
			 * Destroys a previously created Vulkan resource. Caller must ensure the resource is not currently being used
			 * on the device.
			 */
			void Destroy(VulkanResource* resource);

			VulkanGpuDevice& mDevice;

#if B3D_DEBUG
			UnorderedSet<VulkanResource*> mResources;
			Mutex mMutex;
#endif
		};

		/** Determines on which pipeline and how is a resource being accessed. Together with read/write flags allows the caller to uniquely determine Vulkan pipeline and access masks from this enum. */
		enum class VulkanAccessStageFlag
		{
			None							= 0,		//				 Pipeline stage						|				Access (read)					|				Access (write)
			DrawIndirect					= 1 << 0,	// VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT				| VK_ACCESS_INDIRECT_COMMAND_READ_BIT			| N/A
			VertexInputAttributes			= 1 << 1,	// VK_PIPELINE_STAGE_VERTEX_INPUT_BIT				| VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT			| N/A
			VertexInputIndices				= 1 << 2,	// VK_PIPELINE_STAGE_VERTEX_INPUT_BIT				| VK_ACCESS_INDEX_READ_BIT						| N/A
			VertexShaderNonUniform			= 1 << 3,	// VK_PIPELINE_STAGE_VERTEX_SHADER_BIT				| VK_ACCESS_SHADER_READ_BIT						| VK_ACCESS_SHADER_WRITE_BIT
			FragmentShaderNonUniform		= 1 << 4,	// VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT			| VK_ACCESS_SHADER_READ_BIT						| VK_ACCESS_SHADER_WRITE_BIT
			ComputeShaderNonUniform			= 1 << 5,	// VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT				| VK_ACCESS_SHADER_READ_BIT						| VK_ACCESS_SHADER_WRITE_BIT
			VertexShaderUniform				= 1 << 6,	// VK_PIPELINE_STAGE_VERTEX_SHADER_BIT				| VK_ACCESS_UNIFORM_READ_BIT					| N/A
			FragmentShaderUniform			= 1 << 7,	// VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT			| VK_ACCESS_UNIFORM_READ_BIT					| N/A
			ComputeShaderUniform			= 1 << 8,	// VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT				| VK_ACCESS_UNIFORM_READ_BIT					| N/A
			EarlyFragmentTests				= 1 << 9,	// VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT		| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT	| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
			LateFragmentTests				= 1 << 10,	// VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT		| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT	| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
			ColorAttachment					= 1 << 11,	// VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT	| VK_ACCESS_COLOR_ATTACHMENT_READ_BIT			| VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			Transfer						= 1 << 12,	// VK_PIPELINE_STAGE_TRANSFER_BIT					| VK_ACCESS_TRANSFER_READ_BIT					| VK_ACCESS_TRANSFER_WRITE_BIT
			Host							= 1 << 13,	// VK_PIPELINE_STAGE_HOST_BIT						| VK_ACCESS_HOST_READ_BIT						| VK_ACCESS_HOST_WRITE_BIT

			AllShader = VertexShaderNonUniform | FragmentShaderNonUniform | ComputeShaderNonUniform | VertexShaderUniform | FragmentShaderUniform | ComputeShaderUniform,
			All = AllShader | DrawIndirect | VertexInputAttributes | VertexInputIndices | EarlyFragmentTests | LateFragmentTests | ColorAttachment | Transfer | Host
		};

		typedef Flags<VulkanAccessStageFlag> VulkanAccessStageFlags;
		B3D_FLAGS_OPERATORS(VulkanAccessStageFlag)

		/** @} */
	} // namespace render
} // namespace b3d
