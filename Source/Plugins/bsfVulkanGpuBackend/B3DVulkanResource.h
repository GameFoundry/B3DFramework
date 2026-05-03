//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "Allocators/B3DStaticAlloc.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuResourceManager.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		class VulkanResourceManager;

		/**
		 * Wraps one or multiple native Vulkan objects. Extends IGpuResource with the Vulkan-specific
		 * portion of the lifetime state machine: queue ownership transitions and per-queue read/write
		 * use counters. Aggregate counters and deferred destruction are inherited from IGpuResource.
		 *
		 * @note Thread safe
		 */
		class VulkanResource : public IGpuResource
		{
		public:
			static constexpr u32 kMaximumUniqueQueueCount = B3D_MAX_QUEUES_PER_TYPE * GQT_COUNT;

			VulkanResource(VulkanResourceManager* owner, bool concurrency, const StringView& name);

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
			 * @param	useFlags	Flags for which to check use information (e.g. read only, write only, or both).
			 * @return				Bitmask of which queues is the resource used on.
			 */
			GpuQueueMask GetUseInfo(GpuAccessFlags useFlags) const;

			/** Returns true if the resource is only allowed to be used by a single queue family at once. */
			bool IsExclusive() const
			{
				Lock lock(mMutex);
				return mState != State::Shared;
			}

			/** Returns the device this resource is created on. */
			VulkanGpuDevice& GetDevice() const;

		protected:
			void OnNotifyUsed(GpuQueueId queueId, GpuAccessFlags useFlags) override;
			void OnNotifyDone(GpuQueueId queueId, GpuAccessFlags useFlags) override;

			/** Possible states of this object. */
			enum class State
			{
				Normal,
				Shared
			};

			/**
			 * Typed manager pointer. Shadows IGpuResource::mOwner so that subclasses calling mOwner->GetDevice()
			 * (and similar typed accessors on the manager) see the VulkanResourceManager surface. The base's untyped
			 * pointer drives the deferred-destroy free path inside IGpuResource itself.
			 */
			VulkanResourceManager* mOwner;

			GpuQueueType mOwnedQueueType = GQT_UNKNOWN;
			State mState;

			u8 mReadUses[kMaximumUniqueQueueCount];
			u8 mWriteUses[kMaximumUniqueQueueCount];
		};

		/**
		 * Creates and destroys VulkanResource%s on a single device.
		 *
		 * @note Thread safe
		 */
		class VulkanResourceManager : public GpuResourceManager
		{
		public:
			VulkanResourceManager(VulkanGpuDevice& device);

			/**
			 * Creates a new Vulkan resource of the specified type. User must call VulkanResource::Destroy() when done using
			 * the resource.
			 */
			template <class Type, class... Args>
			Type* Create(Args&&... args)
			{
				Type* resource = new(B3DAllocate(sizeof(Type))) Type(this, std::forward<Args>(args)...);
				RegisterResource(resource);
				return resource;
			}

			/** Returns the device that owns this manager. */
			VulkanGpuDevice& GetDevice() const;
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
