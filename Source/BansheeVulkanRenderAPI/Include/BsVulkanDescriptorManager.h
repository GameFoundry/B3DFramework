//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"

namespace BansheeEngine
{
	/** Used as a key in a hash map containing VulkanDescriptorLayout%s. */
	struct VulkanLayoutKey
	{
		VulkanLayoutKey(VkDescriptorSetLayoutBinding* bindings, UINT32 numBindings);

		/** Compares two descriptor layouts. */
		bool operator==(const VulkanLayoutKey& rhs) const;

		/** Calculates a hash value for a descriptor layout. */
		size_t calculateHash() const;

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
			return (size_t)value.calculateHash();
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

	protected:
		VulkanDevice& mDevice;

		UnorderedMap<VulkanLayoutKey, UINT32> mSets; // TODO - Just dummy value for now, keep a list of sets here normally
	};

	/** @} */
}