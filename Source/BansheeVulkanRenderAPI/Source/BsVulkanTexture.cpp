//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanTexture.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanUtility.h"
#include "BsCoreThread.h"
#include "BsRenderStats.h"
#include "BsMath.h"

namespace bs
{
	VulkanImage::VulkanImage(VulkanResourceManager* owner, VkImage image, VkDeviceMemory memory, VkImageLayout layout,
							 const TextureProperties& props)
		: VulkanResource(owner, false), mImage(image), mMemory(memory), mLayout(layout)
	{
		mImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		mImageViewCI.pNext = nullptr;
		mImageViewCI.flags = 0;
		mImageViewCI.image = image;
		mImageViewCI.format = VulkanUtility::getPixelFormat(props.getFormat());
		mImageViewCI.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};

		switch (props.getTextureType())
		{
		case TEX_TYPE_1D:
			mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_1D;
			break;
		default:
		case TEX_TYPE_2D:
			mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			break;
		case TEX_TYPE_3D:
			mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_3D;
			break;
		case TEX_TYPE_CUBE_MAP:
			mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			break;
		}

		if ((props.getUsage() & TU_DEPTHSTENCIL) != 0)
			mImageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		else
			mImageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		TextureSurface completeSurface(0, props.getNumMipmaps() + 1, 0, props.getNumArraySlices());
		mMainView = createView(completeSurface);
	}

	VulkanImage::~VulkanImage()
	{
		VulkanDevice& device = mOwner->getDevice();
		VkDevice vkDevice = device.getLogical();

		vkDestroyImageView(vkDevice, mMainView, gVulkanAllocator);

		for(auto& entry : mImageInfos)
			vkDestroyImageView(vkDevice, entry.view, gVulkanAllocator);

		vkDestroyImage(vkDevice, mImage, gVulkanAllocator);
		device.freeMemory(mMemory);
	}

	VkImageView VulkanImage::getView(const TextureSurface& surface) const
	{
		for(auto& entry : mImageInfos)
		{
			if (surface.mipLevel == entry.surface.mipLevel &&
				surface.numMipLevels == entry.surface.numMipLevels &&
				surface.arraySlice == entry.surface.arraySlice &&
				surface.numArraySlices == entry.surface.numArraySlices)
			{
				return entry.view;
			}
		}

		ImageViewInfo info;
		info.surface = surface;
		info.view = createView(surface);

		mImageInfos.push_back(info);

		return info.view;
	}

	VkImageView VulkanImage::createView(const TextureSurface& surface) const
	{
		VkImageViewType oldViewType = mImageViewCI.viewType;

		if(surface.numArraySlices > 1)
		{
			switch (oldViewType)
			{
			case VK_IMAGE_VIEW_TYPE_1D:
				mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
				break;
			case VK_IMAGE_VIEW_TYPE_2D:
				mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
				break;
			case VK_IMAGE_VIEW_TYPE_CUBE:
				mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
				break;
			default:
				break;
			}
		}

		mImageViewCI.subresourceRange.baseMipLevel = surface.mipLevel;
		mImageViewCI.subresourceRange.levelCount = surface.numMipLevels;
		mImageViewCI.subresourceRange.baseArrayLayer = surface.arraySlice;
		mImageViewCI.subresourceRange.layerCount = surface.numArraySlices;

		VkImageView view;
		VkResult result = vkCreateImageView(mOwner->getDevice().getLogical(), &mImageViewCI, gVulkanAllocator, &view);
		assert(result == VK_SUCCESS);

		mImageViewCI.viewType = oldViewType;
		return view;
	}

	VulkanTextureCore::VulkanTextureCore(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData,
		GpuDeviceFlags deviceMask)
		: TextureCore(desc, initialData, deviceMask), mImages(), mDeviceMask(deviceMask)
	{
		
	}

	VulkanTextureCore::~VulkanTextureCore()
	{ 
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mImages[i] == nullptr)
				return;

			mImages[i]->destroy();
		}

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Texture);
	}

	void VulkanTextureCore::initialize()
	{
		THROW_IF_NOT_CORE_THREAD;

		const TextureProperties& props = mProperties;

		mImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		mImageCI.pNext = nullptr;
		mImageCI.flags = 0;

		TextureType texType = props.getTextureType();
		switch(texType)
		{
		case TEX_TYPE_1D:
			mImageCI.imageType = VK_IMAGE_TYPE_1D;
			break;
		case TEX_TYPE_2D:
			mImageCI.imageType = VK_IMAGE_TYPE_2D;
			break;
		case TEX_TYPE_3D:
			mImageCI.imageType = VK_IMAGE_TYPE_3D;
			break;
		case TEX_TYPE_CUBE_MAP:
			mImageCI.imageType = VK_IMAGE_TYPE_2D;
			mImageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			break;
		}

		int usage = props.getUsage();
		if((usage & TU_RENDERTARGET) != 0)
			mImageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		else if((usage & TU_DEPTHSTENCIL) != 0)
			mImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else if((usage & TU_LOADSTORE) != 0)
			mImageCI.usage = VK_IMAGE_USAGE_STORAGE_BIT;
		else
			mImageCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		if ((usage & TU_CPUREADABLE) != 0)
			mImageCI.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		mImageCI.format = VulkanUtility::getPixelFormat(props.getFormat());
		mImageCI.extent = { props.getWidth(), props.getHeight(), props.getDepth() };
		mImageCI.mipLevels = props.getNumMipmaps() + 1;
		mImageCI.arrayLayers = props.getNumFaces();
		mImageCI.samples = VulkanUtility::getSampleFlags(props.getNumSamples());
		mImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		mImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		mImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		mImageCI.queueFamilyIndexCount = 0;
		mImageCI.pQueueFamilyIndices = nullptr;

		bool isReadable = (usage & GBU_READABLE) != 0 || BS_EDITOR_BUILD;

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());
		VulkanDevice* devices[BS_MAX_DEVICES];
		VulkanUtility::getDevices(rapi, mDeviceMask, devices);

		// Allocate buffers per-device
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] == nullptr)
				continue;

			mImages[i] = createImage(*devices[i], false, isReadable);
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Texture);
		TextureCore::initialize();
	}

	VulkanImage* VulkanTextureCore::createImage(VulkanDevice& device, bool staging, bool readable)
	{
		VkBufferUsageFlags usage = mImageCI.usage;
		if (staging)
		{
			mImageCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			// Staging buffers are used as a destination for reads
			if (readable)
				mImageCI.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}
		else if (readable) // Non-staging readable
			mImageCI.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		bool directlyMappable = (getProperties().getUsage() & TU_DYNAMIC) != 0;
		VkMemoryPropertyFlags flags = (directlyMappable || staging) ?
			(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) : // Note: Try using cached memory
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VkDevice vkDevice = device.getLogical();

		VkImage image;
		VkResult result = vkCreateImage(vkDevice, &mImageCI, gVulkanAllocator, &image);
		assert(result == VK_SUCCESS);

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(vkDevice, image, &memReqs);

		VkDeviceMemory memory = device.allocateMemory(memReqs, flags);
		result = vkBindImageMemory(vkDevice, image, memory, 0);
		assert(result == VK_SUCCESS);

		mImageCI.usage = usage; // Restore original usage
		return device.getResourceManager().create<VulkanImage>(image, memory, VK_IMAGE_LAYOUT_UNDEFINED, getProperties());
	}

	VkImageView VulkanTextureCore::getView(UINT32 deviceIdx) const
	{
		if (mImages[deviceIdx] == nullptr)
			return VK_NULL_HANDLE;

		return mImages[deviceIdx]->getView();
	}

	VkImageView VulkanTextureCore::getView(UINT32 deviceIdx, const TextureSurface& surface) const
	{
		if (mImages[deviceIdx] == nullptr)
			return VK_NULL_HANDLE;

		return mImages[deviceIdx]->getView(surface);
	}

	void VulkanTextureCore::copyImpl(UINT32 srcFace, UINT32 srcMipLevel, UINT32 destFace, UINT32 destMipLevel,
									 const SPtr<TextureCore>& target, UINT32 queueIdx)
	{
		// TODO - Handle resolve here as well
	}

	PixelData VulkanTextureCore::lockImpl(GpuLockOptions options, UINT32 mipLevel, UINT32 face, UINT32 deviceIdx,
										  UINT32 queueIdx)
	{
		PixelData lockedArea(1, 1, 1, mProperties.getFormat());
	
		// TODO - Increment read/write stat (do this for other buffers as well)

		return lockedArea;
	}

	void VulkanTextureCore::unlockImpl()
	{
		
	}

	void VulkanTextureCore::readDataImpl(PixelData& dest, UINT32 mipLevel, UINT32 face, UINT32 deviceIdx, UINT32 queueIdx)
	{
		if (mProperties.getNumSamples() > 1)
		{
			LOGERR("Multisampled textures cannot be accessed from the CPU directly.");
			return;
		}

		PixelData myData = lock(GBL_READ_ONLY, mipLevel, face, deviceIdx, queueIdx);

#if BS_DEBUG_MODE
		if (dest.getConsecutiveSize() != myData.getConsecutiveSize())
		{
			unlock();
			BS_EXCEPT(InternalErrorException, "Buffer sizes don't match");
		}
#endif

		PixelUtil::bulkPixelConversion(myData, dest);

		unlock();

		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_Texture);
	}

	void VulkanTextureCore::writeDataImpl(const PixelData& src, UINT32 mipLevel, UINT32 face, bool discardWholeBuffer,
									  UINT32 queueIdx)
	{
		if (mProperties.getNumSamples() > 1)
		{
			LOGERR("Multisampled textures cannot be accessed from the CPU directly.");
			return;
		}

		PixelFormat format = mProperties.getFormat();

		mipLevel = Math::clamp(mipLevel, (UINT32)mipLevel, mProperties.getNumMipmaps());
		face = Math::clamp(face, (UINT32)0, mProperties.getNumFaces() - 1);

		if (face > 0 && mProperties.getTextureType() == TEX_TYPE_3D)
		{
			LOGERR("3D texture arrays are not supported.");
			return;
		}

		// Write to every device
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mImages[i] == nullptr)
				continue;

			PixelData myData = lock(discardWholeBuffer ? GBL_WRITE_ONLY_DISCARD : GBL_WRITE_ONLY, mipLevel, face, i,
									queueIdx);
			PixelUtil::bulkPixelConversion(src, myData);
			unlock();
		}

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_Texture);
	}
}