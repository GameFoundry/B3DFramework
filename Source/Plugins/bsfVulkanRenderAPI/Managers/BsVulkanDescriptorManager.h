//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanDescriptorLayout.h"

namespace bs
{
	namespace ct
	{
		/** Used as a key in a hash map containing VulkanDescriptorLayout%s. */
		struct VulkanLayoutKey
		{
			VulkanLayoutKey(VkDescriptorSetLayoutBinding* bindings, u32 numBindings);

			/** Compares two descriptor layouts. */
			bool operator==(const VulkanLayoutKey& rhs) const;

			u32 NumBindings;
			VkDescriptorSetLayoutBinding* Bindings;

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
	} // namespace ct
} // namespace bs

/** @cond STDLIB */
/** @addtogroup Vulkan
 *  @{
 */

namespace std
{
	/**	Hash value generator for VulkanLayoutKey. */
	template <>
	struct hash<bs::ct::VulkanLayoutKey>
	{
		size_t operator()(const bs::ct::VulkanLayoutKey& value) const
		{
			if(value.Layout != nullptr)
				return value.Layout->GetHash();

			return bs::ct::VulkanDescriptorLayout::CalculateHash(value.Bindings, value.NumBindings);
		}
	};

	/**	Hash value generator for VulkanPipelineLayoutKey. */
	template <>
	struct hash<bs::ct::VulkanPipelineLayoutKey>
	{
		size_t operator()(const bs::ct::VulkanPipelineLayoutKey& value) const
		{
			return value.CalculateHash();
		}
	};
} // namespace std

/** @} */
/** @endcond */

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Manages allocation of descriptor layouts and sets for a single Vulkan device. */
		class VulkanDescriptorManager
		{
		public:
			VulkanDescriptorManager(VulkanDevice& device);
			~VulkanDescriptorManager();

			/** Attempts to find an existing one, or allocates a new descriptor set layout from the provided set of bindings. */
			VulkanDescriptorLayout* GetLayout(VkDescriptorSetLayoutBinding* bindings, u32 numBindings);

			/** Allocates a new empty descriptor set matching the provided layout. */
			VulkanDescriptorSet* CreateSet(VulkanDescriptorLayout* layout);

			/** Attempts to find an existing one, or allocates a new pipeline layout based on the provided descriptor layouts. */
			VkPipelineLayout GetPipelineLayout(VulkanDescriptorLayout** layouts, u32 numLayouts);

		protected:
			VulkanDevice& mDevice;

			UnorderedSet<VulkanLayoutKey> mLayouts;
			UnorderedMap<VulkanPipelineLayoutKey, VkPipelineLayout> mPipelineLayouts;
			Vector<VulkanDescriptorPool*> mPools;
		};

		/** @} */
	} // namespace ct
} // namespace bs
