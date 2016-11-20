//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanDescriptorLayout.h"

namespace bs
{
	/** Used as a key in a hash map containing VulkanDescriptorLayout%s. */
	struct VulkanLayoutKey
	{
		VulkanLayoutKey(VkDescriptorSetLayoutBinding* bindings, UINT32 numBindings);

		/** Compares two descriptor layouts. */
		bool operator==(const VulkanLayoutKey& rhs) const;

		UINT32 numBindings;
		VkDescriptorSetLayoutBinding* bindings;

		VulkanDescriptorLayout* layout;
	};

	/** Used as a key in a hash map containing pipeline layouts. */
	struct VulkanPipelineLayoutKey
	{
		VulkanPipelineLayoutKey(VulkanDescriptorLayout** layouts, UINT32 numLayouts);

		/** Compares two pipeline layouts. */
		bool operator==(const VulkanPipelineLayoutKey& rhs) const;

		/** Calculates a has value for the provided descriptor layouts. */
		size_t calculateHash() const;

		UINT32 numLayouts;
		VulkanDescriptorLayout** layouts;
	};
}

/** @cond STDLIB */
/** @addtogroup Vulkan
 *  @{
 */

namespace std
{
	/**	Hash value generator for VulkanLayoutKey. */
	template<>
	struct hash<bs::VulkanLayoutKey>
	{
		size_t operator()(const bs::VulkanLayoutKey& value) const
		{
			if (value.layout != nullptr)
				return value.layout->getHash();

			return bs::VulkanDescriptorLayout::calculateHash(value.bindings, value.numBindings);
		}
	};

	/**	Hash value generator for VulkanPipelineLayoutKey. */
	template<>
	struct hash<bs::VulkanPipelineLayoutKey>
	{
		size_t operator()(const bs::VulkanPipelineLayoutKey& value) const
		{
			return value.calculateHash();
		}
	};
}

/** @} */
/** @endcond */

namespace bs
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
		VulkanDescriptorLayout* getLayout(VkDescriptorSetLayoutBinding* bindings, UINT32 numBindings);

		/** Allocates a new empty descriptor set matching the provided layout. */
		VulkanDescriptorSet* createSet(VulkanDescriptorLayout* layout);

		/** Attempts to find an existing one, or allocates a new pipeline layout based on the provided descriptor layouts. */
		VkPipelineLayout getPipelineLayout(VulkanDescriptorLayout** layouts, UINT32 numLayouts);

	protected:
		VulkanDevice& mDevice;

		UnorderedSet<VulkanLayoutKey> mLayouts; 
		UnorderedMap<VulkanPipelineLayoutKey, VkPipelineLayout> mPipelineLayouts;
		Vector<VulkanDescriptorPool*> mPools;
	};

	/** @} */
}