//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanDescriptorLayout.h"

namespace b3d
{
	namespace render
	{
		/** Used as a key in a hash map containing VulkanDescriptorLayout%s. */
		struct VulkanLayoutKey
		{
			VulkanLayoutKey(TArrayView<VkDescriptorSetLayoutBinding> bindings);

			/** Compares two descriptor layouts. */
			bool operator==(const VulkanLayoutKey& rhs) const;

			TArrayView<VkDescriptorSetLayoutBinding> Bindings;
			VulkanDescriptorLayout* Layout = nullptr;
		};

		/** Used as a key in a hash map containing pipeline layouts. */
		struct VulkanPipelineLayoutKey
		{
			VulkanPipelineLayoutKey(VulkanDescriptorLayout** layouts, u32 numLayouts);

			/** Compares two pipeline layouts. */
			bool operator==(const VulkanPipelineLayoutKey& rhs) const;

			/** Calculates a has value for the provided descriptor layouts. */
			size_t CalculateHash() const;

			u32 NumLayouts;
			VulkanDescriptorLayout** Layouts;
		};
	} // namespace render
} // namespace b3d

/** @cond STDLIB */
/** @addtogroup Vulkan
 *  @{
 */

namespace std
{
	/**	Hash value generator for VulkanLayoutKey. */
	template <>
	struct hash<b3d::render::VulkanLayoutKey>
	{
		size_t operator()(const b3d::render::VulkanLayoutKey& value) const
		{
			if(value.Layout != nullptr)
				return value.Layout->GetHash();

			return b3d::render::VulkanDescriptorLayout::CalculateHash(value.Bindings);
		}
	};

	/**	Hash value generator for VulkanPipelineLayoutKey. */
	template <>
	struct hash<b3d::render::VulkanPipelineLayoutKey>
	{
		size_t operator()(const b3d::render::VulkanPipelineLayoutKey& value) const
		{
			return value.CalculateHash();
		}
	};
} // namespace std

/** @} */
/** @endcond */

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Manages allocation of descriptor layouts and sets for a single Vulkan device. */
		class VulkanDescriptorManager
		{
		public:
			VulkanDescriptorManager(VulkanGpuDevice& device);
			~VulkanDescriptorManager();

			/** Attempts to find an existing one, or allocates a new descriptor set layout from the provided set of bindings. */
			VulkanDescriptorLayout* GetLayout(TArrayView<VkDescriptorSetLayoutBinding> bindings);

			/** Attempts to find an existing one, or allocates a new pipeline layout based on the provided descriptor layouts. */
			VkPipelineLayout GetPipelineLayout(VulkanDescriptorLayout** layouts, u32 bindingCount);

		protected:
			VulkanGpuDevice& mDevice;

			UnorderedSet<VulkanLayoutKey> mLayouts;
			UnorderedMap<VulkanPipelineLayoutKey, VkPipelineLayout> mPipelineLayouts;
		};

		/** @} */
	} // namespace render
} // namespace b3d
