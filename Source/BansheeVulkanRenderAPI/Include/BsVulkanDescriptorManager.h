//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanDescriptorLayout.h"

namespace BansheeEngine
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
}

/** @cond STDLIB */
/** @addtogroup Vulkan
 *  @{
 */

namespace std
{
	/**	Hash value generator for VulkanDescriptorLayout. */
	template<>
	struct hash<BansheeEngine::VulkanLayoutKey>
	{
		size_t operator()(const BansheeEngine::VulkanLayoutKey& value) const
		{
			if (value.layout != nullptr)
				return value.layout->getHash();

			return BansheeEngine::VulkanDescriptorLayout::calculateHash(value.bindings, value.numBindings);
		}
	};
}

/** @} */
/** @endcond */

namespace BansheeEngine
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

	protected:
		VulkanDevice& mDevice;

		UnorderedSet<VulkanLayoutKey> mLayouts; 
		Vector<VulkanDescriptorPool*> mPools;
	};

	/** @} */
}