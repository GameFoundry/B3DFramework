//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanTexture.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanUtility.h"
#include "BsVulkanCommandBufferManager.h"
#include "BsVulkanHardwareBuffer.h"
#include "BsCoreThread.h"
#include "BsRenderStats.h"
#include "BsMath.h"

namespace bs
{
	VULKAN_IMAGE_DESC createDesc(VkImage image, VkDeviceMemory memory, VkImageLayout layout, const TextureProperties& props)
	{
		VULKAN_IMAGE_DESC desc;
		desc.image = image;
		desc.memory = memory;
		desc.type = props.getTextureType();
		desc.format = VulkanUtility::getPixelFormat(props.getFormat(), props.isHardwareGammaEnabled());
		desc.numFaces = props.getNumFaces();
		desc.numMipLevels = props.getNumMipmaps() + 1;
		desc.isDepthStencil = (props.getUsage() & TU_DEPTHSTENCIL) != 0;
		desc.isStorage = (props.getUsage() & TU_LOADSTORE) != 0;
		desc.layout = layout;

		return desc;
	}

	VulkanImage::VulkanImage(VulkanResourceManager* owner, VkImage image, VkDeviceMemory memory, VkImageLayout layout,
							 const TextureProperties& props, bool ownsImage)
		: VulkanImage(owner, createDesc(image, memory, layout, props), ownsImage)
	{ }

	VulkanImage::VulkanImage(VulkanResourceManager* owner, const VULKAN_IMAGE_DESC& desc, bool ownsImage)
		: VulkanResource(owner, false), mImage(desc.image), mMemory(desc.memory), mLayout(desc.layout)
		, mFramebufferMainView(VK_NULL_HANDLE), mOwnsImage(ownsImage), mNumFaces(desc.numFaces)
		, mNumMipLevels(desc.numMipLevels), mIsStorage(desc.isStorage)
	{
		mImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		mImageViewCI.pNext = nullptr;
		mImageViewCI.flags = 0;
		mImageViewCI.image = desc.image;
		mImageViewCI.format = desc.format;
		mImageViewCI.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};

		switch (desc.type)
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

		mIsDepthStencil = desc.isDepthStencil;

		TextureSurface completeSurface(0, desc.numMipLevels, 0, desc.numFaces);
		if (mIsDepthStencil)
		{
			mFramebufferMainView = createView(completeSurface, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
			mMainView = createView(completeSurface, VK_IMAGE_ASPECT_DEPTH_BIT);
		}
		else
			mMainView = createView(completeSurface, VK_IMAGE_ASPECT_COLOR_BIT);

		ImageViewInfo mainViewInfo;
		mainViewInfo.surface = completeSurface;
		mainViewInfo.framebuffer = false;
		mainViewInfo.view = mMainView;

		mImageInfos.push_back(mainViewInfo);

		if (mFramebufferMainView != VK_NULL_HANDLE)
		{
			ImageViewInfo fbMainViewInfo;
			fbMainViewInfo.surface = completeSurface;
			fbMainViewInfo.framebuffer = true;
			fbMainViewInfo.view = mFramebufferMainView;

			mImageInfos.push_back(fbMainViewInfo);
		}

		UINT32 numSubresources = mNumFaces * mNumMipLevels;
		mSubresources = (VulkanImageSubresource**)bs_alloc(sizeof(VulkanImageSubresource*) * numSubresources);
		for (UINT32 i = 0; i < numSubresources; i++)
			mSubresources[i] = owner->create<VulkanImageSubresource>();
	}

	VulkanImage::~VulkanImage()
	{
		VulkanDevice& device = mOwner->getDevice();
		VkDevice vkDevice = device.getLogical();

		UINT32 numSubresources = mNumFaces * mNumMipLevels;
		for (UINT32 i = 0; i < numSubresources; i++)
		{
			assert(!mSubresources[i]->isBound()); // Image beeing freed but its subresources are still bound somewhere

			mSubresources[i]->destroy();
		}

		for(auto& entry : mImageInfos)
			vkDestroyImageView(vkDevice, entry.view, gVulkanAllocator);

		if (mOwnsImage)
		{
			vkDestroyImage(vkDevice, mImage, gVulkanAllocator);
			device.freeMemory(mMemory);
		}
	}

	VkImageView VulkanImage::getView(bool framebuffer) const
	{
		if(framebuffer && mIsDepthStencil)
			return mFramebufferMainView;

		return mMainView;
	}

	VkImageView VulkanImage::getView(const TextureSurface& surface, bool framebuffer) const
	{
		for(auto& entry : mImageInfos)
		{
			if (surface.mipLevel == entry.surface.mipLevel &&
				surface.numMipLevels == entry.surface.numMipLevels &&
				surface.arraySlice == entry.surface.arraySlice &&
				surface.numArraySlices == entry.surface.numArraySlices)
			{
				if(!mIsDepthStencil)
					return entry.view;
				else
				{
					if (framebuffer == entry.framebuffer)
						return entry.view;
				}
			}
		}

		ImageViewInfo info;
		info.surface = surface;
		info.framebuffer = framebuffer;

		if (mIsDepthStencil)
		{
			if(framebuffer)
				info.view = createView(surface, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
			else
				info.view = createView(surface, VK_IMAGE_ASPECT_DEPTH_BIT);
		}
		else
			info.view = createView(surface, VK_IMAGE_ASPECT_COLOR_BIT);

		mImageInfos.push_back(info);

		return info.view;
	}

	VkImageView VulkanImage::createView(const TextureSurface& surface, VkImageAspectFlags aspectMask) const
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
			case VK_IMAGE_VIEW_TYPE_3D:
				mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
				break;
			case VK_IMAGE_VIEW_TYPE_CUBE:
				mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
				break;
			default:
				break;
			}
		}

		mImageViewCI.subresourceRange.aspectMask = aspectMask;
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

	VkImageSubresourceRange VulkanImage::getRange() const
	{
		VkImageSubresourceRange range;
		range.baseArrayLayer = 0;
		range.layerCount = mNumFaces;
		range.baseMipLevel = 0;
		range.levelCount = mNumMipLevels;

		if (mIsDepthStencil)
			range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		else
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		return range;
	}

	VulkanImageSubresource* VulkanImage::getSubresource(UINT32 face, UINT32 mipLevel)
	{
		return mSubresources[face * mNumMipLevels + mipLevel];
	}

	void VulkanImage::map(UINT32 face, UINT32 mipLevel, PixelData& output) const
	{
		VulkanDevice& device = mOwner->getDevice();

		VkImageSubresource range;
		range.mipLevel = mipLevel;
		range.arrayLayer = face;

		if (mImageViewCI.subresourceRange.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT)
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		else // Depth stencil, but we only map depth
			range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		VkSubresourceLayout layout;
		vkGetImageSubresourceLayout(device.getLogical(), mImage, &range, &layout);

		assert(layout.size == output.getSize());
		output.setRowPitch((UINT32)layout.rowPitch);
		output.setSlicePitch((UINT32)layout.depthPitch);

		UINT8* data;
		VkResult result = vkMapMemory(device.getLogical(), mMemory, layout.offset, layout.size, 0, (void**)&data);
		assert(result == VK_SUCCESS);

		output.setExternalBuffer(data);
	}

	UINT8* VulkanImage::map(UINT32 offset, UINT32 size) const
	{
		VulkanDevice& device = mOwner->getDevice();

		UINT8* data;
		VkResult result = vkMapMemory(device.getLogical(), mMemory, offset, size, 0, (void**)&data);
		assert(result == VK_SUCCESS);

		return data;
	}

	void VulkanImage::unmap()
	{
		VulkanDevice& device = mOwner->getDevice();

		vkUnmapMemory(device.getLogical(), mMemory);
	}

	void VulkanImage::copy(VulkanTransferBuffer* cb, VulkanBuffer* destination, const VkExtent3D& extent,
						   const VkImageSubresourceLayers& range, VkImageLayout layout)
	{
		VkBufferImageCopy region;
		region.bufferRowLength = destination->getRowPitch();
		region.bufferImageHeight = destination->getSliceHeight();
		region.bufferOffset = 0;
		region.imageOffset.x = 0;
		region.imageOffset.y = 0;
		region.imageOffset.z = 0;
		region.imageExtent = extent;
		region.imageSubresource = range;

		vkCmdCopyImageToBuffer(cb->getCB()->getHandle(), mImage, layout, destination->getHandle(), 1, &region);
	}

	VkAccessFlags VulkanImage::getAccessFlags(VkImageLayout layout)
	{
		VkAccessFlags accessFlags;

		switch (layout)
		{
		case VK_IMAGE_LAYOUT_GENERAL: // Only used for render targets that are also read by shaders, or for storage textures
			if (mIsStorage)
				accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			else
				accessFlags = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			accessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			accessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			accessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			accessFlags = VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_UNDEFINED:
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			accessFlags = 0;
			break;
		default:
			accessFlags = 0;
			LOGWRN("Unsupported source layout for Vulkan image.");
			break;
		}

		return accessFlags;
	}

	VulkanImageSubresource::VulkanImageSubresource(VulkanResourceManager* owner)
		:VulkanResource(owner, false)
	{ }

	VulkanTextureCore::VulkanTextureCore(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData,
		GpuDeviceFlags deviceMask)
		: TextureCore(desc, initialData, deviceMask), mImages(), mDeviceMask(deviceMask), mStagingBuffer(nullptr)
		, mMappedDeviceIdx(-1), mMappedGlobalQueueIdx(-1), mMappedMip(0), mMappedFace(0), mMappedRowPitch(false)
		, mMappedSlicePitch(false), mMappedLockOptions(GBL_WRITE_ONLY), mDirectlyMappable(false), mSupportsGPUWrites(false)
		, mIsMapped(false)
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

		assert(mStagingBuffer == nullptr);

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

		// Note: I force rendertarget and depthstencil types to be readable in shader. Depending on performance impact
		// it might be beneficial to allow the user to enable this explicitly only when needed.
		
		int usage = props.getUsage();
		if ((usage & TU_RENDERTARGET) != 0)
		{
			mImageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			mSupportsGPUWrites = true;
		}
		else if ((usage & TU_DEPTHSTENCIL) != 0)
		{
			mImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			mSupportsGPUWrites = true;
		}
		else if ((usage & TU_LOADSTORE) != 0)
		{
			mImageCI.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			mSupportsGPUWrites = true;
		}
		else
		{
			mImageCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		}

		mImageCI.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
		if ((usage & TU_DYNAMIC) != 0) // Attempt to use linear tiling for dynamic textures, so we can directly map and modify them
		{
			// Only support 2D textures, with one sample and one mip level, only used for shader reads
			// (Optionally check vkGetPhysicalDeviceFormatProperties & vkGetPhysicalDeviceImageFormatProperties for
			// additional supported configs, but right now there doesn't seem to be any additional support)
			if(texType == TEX_TYPE_2D && props.getNumSamples() <= 1 && props.getNumMipmaps() == 0 && 
				props.getNumFaces() == 1 && (mImageCI.usage & VK_IMAGE_USAGE_SAMPLED_BIT) != 0)
			{
				// Also, only support normal textures, not render targets or storage textures
				if (!mSupportsGPUWrites)
				{
					mDirectlyMappable = true;
					tiling = VK_IMAGE_TILING_LINEAR;
					layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
				}
			}
		}

		mImageCI.format = VulkanUtility::getPixelFormat(props.getFormat(), props.isHardwareGammaEnabled());
		mImageCI.extent = { props.getWidth(), props.getHeight(), props.getDepth() };
		mImageCI.mipLevels = props.getNumMipmaps() + 1;
		mImageCI.arrayLayers = props.getNumFaces();
		mImageCI.samples = VulkanUtility::getSampleFlags(props.getNumSamples());
		mImageCI.tiling = tiling;
		mImageCI.initialLayout = layout;
		mImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		mImageCI.queueFamilyIndexCount = 0;
		mImageCI.pQueueFamilyIndices = nullptr;

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());
		VulkanDevice* devices[BS_MAX_DEVICES];
		VulkanUtility::getDevices(rapi, mDeviceMask, devices);

		// Allocate buffers per-device
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] == nullptr)
				continue;

			mImages[i] = createImage(*devices[i]);
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Texture);
		TextureCore::initialize();
	}

	VulkanImage* VulkanTextureCore::createImage(VulkanDevice& device)
	{
		bool directlyMappable = mImageCI.tiling == VK_IMAGE_TILING_LINEAR;
		VkMemoryPropertyFlags flags = directlyMappable ?
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

		return device.getResourceManager().create<VulkanImage>(image, memory, mImageCI.initialLayout, getProperties());
	}

	VulkanBuffer* VulkanTextureCore::createStaging(VulkanDevice& device, const PixelData& pixelData, bool readable)
	{
		VkBufferCreateInfo bufferCI;
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.pNext = nullptr;
		bufferCI.flags = 0;
		bufferCI.size = pixelData.getSize();
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCI.queueFamilyIndexCount = 0;
		bufferCI.pQueueFamilyIndices = nullptr;

		if (readable)
			bufferCI.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		VkDevice vkDevice = device.getLogical();

		VkBuffer buffer;
		VkResult result = vkCreateBuffer(vkDevice, &bufferCI, gVulkanAllocator, &buffer);
		assert(result == VK_SUCCESS);

		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(vkDevice, buffer, &memReqs);

		VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		VkDeviceMemory memory = device.allocateMemory(memReqs, flags);
		result = vkBindBufferMemory(vkDevice, buffer, memory, 0);
		assert(result == VK_SUCCESS);

		VkBufferView view = VK_NULL_HANDLE;

		return device.getResourceManager().create<VulkanBuffer>(buffer, view, memory,
			pixelData.getRowPitch(), pixelData.getSlicePitch());
	}

	void VulkanTextureCore::copyImage(VulkanTransferBuffer* cb, VulkanImage* srcImage, VulkanImage* dstImage, 
									  VkImageLayout srcFinalLayout, VkImageLayout dstFinalLayout)
	{
		UINT32 numFaces = mProperties.getNumFaces();
		UINT32 numMipmaps = mProperties.getNumMipmaps() + 1;

		UINT32 mipWidth = mProperties.getWidth();
		UINT32 mipHeight = mProperties.getHeight();
		UINT32 mipDepth = mProperties.getDepth();

		VkImageCopy* imageRegions = bs_stack_alloc<VkImageCopy>(numMipmaps);

		for(UINT32 i = 0; i < numMipmaps; i++)
		{
			VkImageCopy& imageRegion = imageRegions[i];

			imageRegion.srcOffset = { 0, 0, 0 };
			imageRegion.dstOffset = { 0, 0, 0 };
			imageRegion.extent = { mipWidth, mipHeight, mipDepth };
			imageRegion.srcSubresource.baseArrayLayer = 0;
			imageRegion.srcSubresource.layerCount = numFaces;
			imageRegion.srcSubresource.mipLevel = i;
			imageRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageRegion.dstSubresource.baseArrayLayer = 0;
			imageRegion.dstSubresource.layerCount = numFaces;
			imageRegion.dstSubresource.mipLevel = i;
			imageRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			if (mipWidth != 1) mipWidth /= 2;
			if (mipHeight != 1) mipHeight /= 2;
			if (mipDepth != 1) mipDepth /= 2;
		}

		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseArrayLayer = 0;
		range.layerCount = numFaces;
		range.baseMipLevel = 0;
		range.levelCount = numMipmaps;

		VkAccessFlags srcAccessMask = srcImage->getAccessFlags(srcImage->getLayout());
		VkAccessFlags dstAccessMask = dstImage->getAccessFlags(dstImage->getLayout());

		VkImageLayout transferSrcLayout, transferDstLayout;
		if (mDirectlyMappable)
		{
			transferSrcLayout = VK_IMAGE_LAYOUT_GENERAL;
			transferDstLayout = VK_IMAGE_LAYOUT_GENERAL;
		}
		else
		{
			transferSrcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			transferDstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		}

		// Transfer textures to a valid layout
		cb->setLayout(srcImage->getHandle(), srcAccessMask, VK_ACCESS_TRANSFER_READ_BIT, srcImage->getLayout(),
							  transferSrcLayout, range);

		cb->setLayout(dstImage->getHandle(), dstAccessMask, VK_ACCESS_TRANSFER_WRITE_BIT,
							  dstImage->getLayout(), transferDstLayout, range);

		vkCmdCopyImage(cb->getCB()->getHandle(), srcImage->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						dstImage->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, numMipmaps, imageRegions);

		// Transfer back to final layouts
		srcAccessMask = srcImage->getAccessFlags(srcFinalLayout);
		cb->setLayout(srcImage->getHandle(), VK_ACCESS_TRANSFER_READ_BIT, srcAccessMask,
							  transferSrcLayout, srcFinalLayout, range);

		dstAccessMask = dstImage->getAccessFlags(dstFinalLayout);
		cb->setLayout(dstImage->getHandle(), VK_ACCESS_TRANSFER_WRITE_BIT, dstAccessMask,
							  transferDstLayout, dstFinalLayout, range);

		bs_stack_free(imageRegions);
	}

	VkImageLayout VulkanTextureCore::getOptimalLayout() const
	{
		int usage = getProperties().getUsage();
		if ((usage & TU_RENDERTARGET) != 0)
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		else if ((usage & TU_DEPTHSTENCIL) != 0)
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		else if ((usage & TU_LOADSTORE) != 0)
			return VK_IMAGE_LAYOUT_GENERAL;
		else if ((usage & TU_DYNAMIC) != 0)
			return VK_IMAGE_LAYOUT_GENERAL;
		else
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	void VulkanTextureCore::copyImpl(UINT32 srcFace, UINT32 srcMipLevel, UINT32 destFace, UINT32 destMipLevel,
									 const SPtr<TextureCore>& target, UINT32 queueIdx)
	{
		VulkanTextureCore* other = static_cast<VulkanTextureCore*>(target.get());

		const TextureProperties& srcProps = mProperties;
		const TextureProperties& dstProps = other->getProperties();

		bool srcHasMultisample = srcProps.getNumSamples() > 1;
		bool destHasMultisample = dstProps.getNumSamples() > 1;

		if ((srcProps.getUsage() & TU_DEPTHSTENCIL) != 0 || (dstProps.getUsage() & TU_DEPTHSTENCIL) != 0)
		{
			LOGERR("Texture copy/resolve isn't supported for depth-stencil textures.");
			return;
		}

		bool needsResolve = srcHasMultisample && !destHasMultisample;
		bool isMSCopy = srcHasMultisample || destHasMultisample;
		if (!needsResolve && isMSCopy)
		{
			if (srcProps.getNumSamples() != dstProps.getNumSamples())
			{
				LOGERR("When copying textures their multisample counts must match. Ignoring copy.");
				return;
			}
		}

		VulkanCommandBufferManager& cbManager = gVulkanCBManager();
		GpuQueueType queueType;
		UINT32 localQueueIdx = CommandSyncMask::getQueueIdxAndType(queueIdx, queueType);

		VkImageLayout transferSrcLayout, transferDstLayout;
		if (mDirectlyMappable)
		{
			transferSrcLayout = VK_IMAGE_LAYOUT_GENERAL;
			transferDstLayout = VK_IMAGE_LAYOUT_GENERAL;
		}
		else
		{
			transferSrcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			transferDstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		}

		UINT32 mipWidth, mipHeight, mipDepth;
		PixelUtil::getSizeForMipLevel(srcProps.getWidth(), srcProps.getHeight(), srcProps.getDepth(), srcMipLevel,
									  mipWidth, mipHeight, mipDepth);

		VkImageResolve resolveRegion;
		resolveRegion.srcOffset = { 0, 0, 0 };
		resolveRegion.dstOffset = { 0, 0, 0 };
		resolveRegion.extent = { mipWidth, mipHeight, mipDepth };
		resolveRegion.srcSubresource.baseArrayLayer = srcFace;
		resolveRegion.srcSubresource.layerCount = 1;
		resolveRegion.srcSubresource.mipLevel = srcMipLevel;
		resolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		resolveRegion.dstSubresource.baseArrayLayer = destFace;
		resolveRegion.dstSubresource.layerCount = 1;
		resolveRegion.dstSubresource.mipLevel = destMipLevel;
		resolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageCopy imageRegion;
		imageRegion.srcOffset = { 0, 0, 0 };
		imageRegion.dstOffset = { 0, 0, 0 };
		imageRegion.extent = { mipWidth, mipHeight, mipDepth };
		imageRegion.srcSubresource.baseArrayLayer = srcFace;
		imageRegion.srcSubresource.layerCount = 1;
		imageRegion.srcSubresource.mipLevel = srcMipLevel;
		imageRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageRegion.dstSubresource.baseArrayLayer = destFace;
		imageRegion.dstSubresource.layerCount = 1;
		imageRegion.dstSubresource.mipLevel = destMipLevel;
		imageRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageSubresourceRange srcRange;
		srcRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		srcRange.baseArrayLayer = srcFace;
		srcRange.layerCount = 1;
		srcRange.baseMipLevel = srcMipLevel;
		srcRange.levelCount = 1;

		VkImageSubresourceRange dstRange;
		dstRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		dstRange.baseArrayLayer = destFace;
		dstRange.layerCount = 1;
		dstRange.baseMipLevel = destMipLevel;
		dstRange.levelCount = 1;

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());
		for(UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			VulkanDevice& device = *rapi._getDevice(i);

			VulkanImage* srcImage = mImages[i];
			VulkanImage* dstImage = other->getResource(i);

			if (srcImage == nullptr || dstImage == nullptr)
				continue;

			VkImageLayout srcLayout = srcImage->getLayout();
			VkImageLayout dstLayout = dstImage->getLayout();

			VulkanTransferBuffer* transferCB = cbManager.getTransferBuffer(i, queueType, localQueueIdx);
			VkCommandBuffer vkCmdBuf = transferCB->getCB()->getHandle();

			// If destination subresource is queued for some operation on a CB (ignoring the ones we're waiting for), then 
			// we need to make a copy of the image to avoid modifying its use in the previous operation
			UINT32 useCount = dstImage->getUseCount();
			UINT32 boundCount = dstImage->getBoundCount();

			bool isBoundWithoutUse = boundCount > useCount;
			if (isBoundWithoutUse)
			{
				VulkanImage* newImage = createImage(device);

				// Avoid copying original contents if the image only has one sub-resource, which we'll overwrite anyway
				if (dstProps.getNumMipmaps() > 0 || dstProps.getNumFaces() > 1)
				{
					VkImageLayout oldDstLayout = dstLayout;
					if (oldDstLayout == VK_IMAGE_LAYOUT_UNDEFINED || oldDstLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
						oldDstLayout = getOptimalLayout();

					dstLayout = getOptimalLayout();
					copyImage(transferCB, dstImage, newImage, oldDstLayout, dstLayout);

					VkAccessFlags accessMask = dstImage->getAccessFlags(oldDstLayout);
					transferCB->getCB()->registerResource(dstImage, accessMask, oldDstLayout, oldDstLayout, oldDstLayout,
						VulkanUseFlag::Read);
				}

				dstImage->destroy();
				dstImage = newImage;
				mImages[i] = dstImage;
			}

			VkAccessFlags srcAccessMask = srcImage->getAccessFlags(srcLayout);
			VkAccessFlags dstAccessMask = dstImage->getAccessFlags(dstLayout);

			// Transfer textures to a valid layout
			transferCB->setLayout(srcImage->getHandle(), srcAccessMask, VK_ACCESS_TRANSFER_READ_BIT, srcLayout,
									transferSrcLayout, srcRange);

			transferCB->setLayout(dstImage->getHandle(), dstAccessMask, VK_ACCESS_TRANSFER_WRITE_BIT,
									dstLayout, transferDstLayout, dstRange);

			if (srcHasMultisample && !destHasMultisample) // Resolving from MS to non-MS texture
			{
				vkCmdResolveImage(vkCmdBuf, srcImage->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
								  dstImage->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveRegion);
			}
			else // Just a normal copy
			{
				vkCmdCopyImage(vkCmdBuf, srcImage->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
							   dstImage->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageRegion);
			}

			// Transfer back to original layouts
			if (srcLayout == VK_IMAGE_LAYOUT_UNDEFINED || srcLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
				srcLayout = getOptimalLayout();

			srcAccessMask = srcImage->getAccessFlags(srcLayout);
			transferCB->setLayout(srcImage->getHandle(), VK_ACCESS_TRANSFER_READ_BIT, srcAccessMask,
									transferSrcLayout, srcLayout, srcRange);

			if (dstLayout == VK_IMAGE_LAYOUT_UNDEFINED || dstLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
				dstLayout = other->getOptimalLayout();

			dstAccessMask = dstImage->getAccessFlags(dstLayout);
			transferCB->setLayout(dstImage->getHandle(), VK_ACCESS_TRANSFER_WRITE_BIT, dstAccessMask,
									transferDstLayout, dstLayout, dstRange);

			// Notify the command buffer that these resources are being used on it
			transferCB->getCB()->registerResource(srcImage, srcAccessMask, srcLayout, srcLayout, srcLayout, 
				VulkanUseFlag::Read);
			transferCB->getCB()->registerResource(dstImage, dstAccessMask, dstLayout, dstLayout, dstLayout, 
				VulkanUseFlag::Write);

			// Need to wait if subresource we're reading from is being written, or if the subresource we're writing to is
			// being accessed in any way

			VulkanImageSubresource* srcSubresource = srcImage->getSubresource(srcFace, srcMipLevel);
			VulkanImageSubresource* dstSubresource = dstImage->getSubresource(srcFace, srcMipLevel);

			UINT32 srcUseFlags = srcSubresource->getUseInfo(VulkanUseFlag::Write);
			UINT32 dstUseFlags = dstSubresource->getUseInfo(VulkanUseFlag::Read | VulkanUseFlag::Write);

			transferCB->appendMask(srcUseFlags | dstUseFlags);
		}
	}

	PixelData VulkanTextureCore::lockImpl(GpuLockOptions options, UINT32 mipLevel, UINT32 face, UINT32 deviceIdx,
										  UINT32 queueIdx)
	{
		const TextureProperties& props = getProperties();

		if (props.getNumSamples() > 1)
		{
			LOGERR("Multisampled textures cannot be accessed from the CPU directly.");
			return PixelData();
		}

#if BS_PROFILING_ENABLED
		if (options == GBL_READ_ONLY || options == GBL_READ_WRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_Texture);
		}

		if (options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_Texture);
		}
#endif

		UINT32 mipWidth = std::max(1u, props.getWidth() >> mipLevel);
		UINT32 mipHeight = std::max(1u, props.getHeight() >> mipLevel);
		UINT32 mipDepth = std::max(1u, props.getDepth() >> mipLevel);

		PixelData lockedArea(mipWidth, mipHeight, mipDepth, props.getFormat());

		VulkanImage* image = mImages[deviceIdx];

		if (image == nullptr)
			return PixelData();

		mIsMapped = true;
		mMappedDeviceIdx = deviceIdx;
		mMappedGlobalQueueIdx = queueIdx;
		mMappedFace = face;
		mMappedMip = mipLevel;
		mMappedLockOptions = options;

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());
		VulkanDevice& device = *rapi._getDevice(deviceIdx);

		VulkanCommandBufferManager& cbManager = gVulkanCBManager();
		GpuQueueType queueType;
		UINT32 localQueueIdx = CommandSyncMask::getQueueIdxAndType(queueIdx, queueType);

		VulkanImageSubresource* subresource = image->getSubresource(face, mipLevel);

		// If memory is host visible try mapping it directly
		if (mDirectlyMappable)
		{
			// Initially the texture will be in preinitialized layout, and it will transition to general layout on first
			// use in shader. No further transitions are allowed for directly mappable textures.
			assert(image->getLayout() == VK_IMAGE_LAYOUT_PREINITIALIZED || image->getLayout() == VK_IMAGE_LAYOUT_GENERAL);

			// GPU should never be allowed to write to a directly mappable texture, since only linear tiling is supported
			// for direct mapping, and we don't support using it with either storage textures or render targets.
			assert(!mSupportsGPUWrites);

			// Check is the GPU currently reading from the image
			UINT32 useMask = subresource->getUseInfo(VulkanUseFlag::Read);
			bool isUsedOnGPU = useMask != 0;

			// We're safe to map directly since GPU isn't using the subresource
			if (!isUsedOnGPU)
			{
				// If some CB has an operation queued that will be using the current contents of the image, create a new 
				// image so we don't modify the previous use of the image
				if (subresource->isBound())
				{
					VulkanImage* newImage = createImage(device);

					// Copy contents of the current image to the new one, unless caller explicitly specifies he doesn't
					// care about the current contents
					if (options != GBL_WRITE_ONLY_DISCARD)
					{
						VkMemoryRequirements memReqs;
						vkGetImageMemoryRequirements(device.getLogical(), image->getHandle(), &memReqs);

						UINT8* src = image->map(0, (UINT32)memReqs.size);
						UINT8* dst = newImage->map(0, (UINT32)memReqs.size);

						memcpy(dst, src, memReqs.size);

						image->unmap();
						newImage->unmap();
					}

					image->destroy();
					image = newImage;
					mImages[deviceIdx] = image;
				}

				image->map(face, mipLevel, lockedArea);
				return lockedArea;
			}

			// Caller guarantees he won't touch the same data as the GPU, so just map even though the GPU is using the
			// subresource
			if (options == GBL_WRITE_ONLY_NO_OVERWRITE)
			{
				image->map(face, mipLevel, lockedArea);
				return lockedArea;
			}

			// Caller doesn't care about buffer contents, so just discard the existing buffer and create a new one
			if (options == GBL_WRITE_ONLY_DISCARD)
			{
				// We need to discard the entire image, even though we're only writing to a single sub-resource
				image->destroy();

				image = createImage(device);
				mImages[deviceIdx] = image;

				image->map(face, mipLevel, lockedArea);
				return lockedArea;
			}

			// We need to read the buffer contents
			if (options == GBL_READ_ONLY || options == GBL_READ_WRITE)
			{
				VulkanTransferBuffer* transferCB = cbManager.getTransferBuffer(deviceIdx, queueType, localQueueIdx);

				// Ensure flush() will wait for all queues currently using to the texture (if any) to finish
				// If only reading, wait for all writes to complete, otherwise wait on both writes and reads
				if (options == GBL_READ_ONLY)
					useMask = subresource->getUseInfo(VulkanUseFlag::Write);
				else
					useMask = subresource->getUseInfo(VulkanUseFlag::Read | VulkanUseFlag::Write);

				transferCB->appendMask(useMask);

				// Submit the command buffer and wait until it finishes
				transferCB->flush(true);

				// If writing and some CB has an operation queued that will be using the current contents of the image, 
				// create a new image so we don't modify the previous use of the image
				if (options == GBL_READ_WRITE && subresource->isBound())
				{
					VulkanImage* newImage = createImage(device);

					VkMemoryRequirements memReqs;
					vkGetImageMemoryRequirements(device.getLogical(), image->getHandle(), &memReqs);

					UINT8* src = image->map(0, (UINT32)memReqs.size);
					UINT8* dst = newImage->map(0, (UINT32)memReqs.size);

					memcpy(dst, src, memReqs.size);

					image->unmap();
					newImage->unmap();

					image->destroy();
					image = newImage;
					mImages[deviceIdx] = image;
				}

				image->map(face, mipLevel, lockedArea);
				return lockedArea;
			}

			// Otherwise, we're doing write only, in which case it's best to use the staging buffer to avoid waiting
			// and blocking, so fall through
		}

		// Can't use direct mapping, so use a staging buffer

		// We might need to copy the current contents of the image to the staging buffer. Even if the user doesn't plan on
		// reading, it is still required as we will eventually copy all of the contents back to the original image,
		// and we can't write potentially uninitialized data. The only exception is when the caller specifies the image
		// contents should be discarded in which he guarantees he will overwrite the entire locked area with his own
		// contents.
		bool needRead = options != GBL_WRITE_ONLY_DISCARD_RANGE && options != GBL_WRITE_ONLY_DISCARD;

		// Allocate a staging buffer
		mStagingBuffer = createStaging(device, lockedArea, needRead);

		if (needRead) // If reading, we need to copy the current contents of the image to the staging buffer
		{
			VulkanTransferBuffer* transferCB = cbManager.getTransferBuffer(deviceIdx, queueType, localQueueIdx);

			// Similar to above, if image supports GPU writes or is currently being written to, we need to wait on any
			// potential writes to complete
			UINT32 writeUseMask = subresource->getUseInfo(VulkanUseFlag::Write);

			if (mSupportsGPUWrites || writeUseMask != 0)
			{
				// Ensure flush() will wait for all queues currently writing to the image (if any) to finish
				transferCB->appendMask(writeUseMask);
			}

			VkImageSubresourceRange range;
			if ((props.getUsage() & TU_DEPTHSTENCIL) != 0)
				range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			else
				range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			range.baseArrayLayer = face;
			range.layerCount = 1;
			range.baseMipLevel = mipLevel;
			range.levelCount = 1;

			VkImageSubresourceLayers rangeLayers;
			if ((props.getUsage() & TU_DEPTHSTENCIL) != 0)
				rangeLayers.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			else
				rangeLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			rangeLayers.baseArrayLayer = range.baseArrayLayer;
			rangeLayers.layerCount = range.layerCount;
			rangeLayers.mipLevel = range.baseMipLevel;

			VkExtent3D extent;
			PixelUtil::getSizeForMipLevel(props.getWidth(), props.getHeight(), props.getDepth(), mMappedMip,
										  extent.width, extent.height, extent.depth);

			// Transfer texture to a valid layout
			VkAccessFlags currentAccessMask = image->getAccessFlags(image->getLayout());
			transferCB->setLayout(image->getHandle(), currentAccessMask, VK_ACCESS_TRANSFER_READ_BIT, image->getLayout(),
								  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, range);

			// Queue copy command
			image->copy(transferCB, mStagingBuffer, extent, rangeLayers, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			// Transfer back to original layout
			VkImageLayout dstLayout = image->getLayout();
			if (dstLayout == VK_IMAGE_LAYOUT_UNDEFINED || dstLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
			{
				dstLayout = getOptimalLayout();
				currentAccessMask = image->getAccessFlags(dstLayout);
			}

			transferCB->setLayout(image->getHandle(), VK_ACCESS_TRANSFER_READ_BIT, currentAccessMask,
								  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstLayout, range);
			transferCB->getCB()->registerResource(image, currentAccessMask, dstLayout, dstLayout, dstLayout,
				VulkanUseFlag::Read);

			// Ensure data written to the staging buffer is visible
			VkAccessFlags stagingAccessFlags;
			if (options == GBL_READ_ONLY)
				stagingAccessFlags = VK_ACCESS_HOST_READ_BIT;
			else // Must be read/write
				stagingAccessFlags = VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT;

			transferCB->memoryBarrier(mStagingBuffer->getHandle(),
									  VK_ACCESS_TRANSFER_WRITE_BIT,
									  stagingAccessFlags,
									  VK_PIPELINE_STAGE_TRANSFER_BIT,
									  VK_PIPELINE_STAGE_HOST_BIT);

			// Submit the command buffer and wait until it finishes
			transferCB->flush(true);
		}

		UINT8* data = mStagingBuffer->map(0, lockedArea.getSize());
		lockedArea.setExternalBuffer(data);

		return lockedArea;
	}

	void VulkanTextureCore::unlockImpl()
	{
		// Possibly map() failed with some error
		if (!mIsMapped)
			return;

		// Note: If we did any writes they need to be made visible to the GPU. However there is no need to execute 
		// a pipeline barrier because (as per spec) host writes are implicitly visible to the device.

		if (mStagingBuffer == nullptr)
			mImages[mMappedDeviceIdx]->unmap();
		else
		{
			mStagingBuffer->unmap();

			bool isWrite = mMappedLockOptions != GBL_READ_ONLY;

			// We the caller wrote anything to the staging buffer, we need to upload it back to the main buffer
			if (isWrite)
			{
				VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());
				VulkanDevice& device = *rapi._getDevice(mMappedDeviceIdx);

				VulkanCommandBufferManager& cbManager = gVulkanCBManager();
				GpuQueueType queueType;
				UINT32 localQueueIdx = CommandSyncMask::getQueueIdxAndType(mMappedGlobalQueueIdx, queueType);

				VulkanImage* image = mImages[mMappedDeviceIdx];
				VkImageLayout curLayout = image->getLayout();
				VulkanTransferBuffer* transferCB = cbManager.getTransferBuffer(mMappedDeviceIdx, queueType, localQueueIdx);

				VulkanImageSubresource* subresource = image->getSubresource(mMappedFace, mMappedMip);

				// If the subresource is used in any way on the GPU, we need to wait for that use to finish before
				// we issue our copy
				UINT32 useMask = subresource->getUseInfo(VulkanUseFlag::Read | VulkanUseFlag::Write);
				bool isNormalWrite = false;
				if (useMask != 0) // Subresource is currently used on the GPU
				{
					// Try to avoid the wait by checking for special write conditions

					// Caller guarantees he won't touch the same data as the GPU, so just copy
					if (mMappedLockOptions == GBL_WRITE_ONLY_NO_OVERWRITE)
					{
						// Fall through to copy()
					}
					// Caller doesn't care about buffer contents, so just discard the existing buffer and create a new one
					else if (mMappedLockOptions == GBL_WRITE_ONLY_DISCARD)
					{
						// We need to discard the entire image, even though we're only writing to a single sub-resource
						image->destroy();

						image = createImage(device);
						mImages[mMappedDeviceIdx] = image;

						subresource = image->getSubresource(mMappedFace, mMappedMip);
					}
					else // Otherwise we have no choice but to issue a dependency between the queues
					{
						transferCB->appendMask(useMask);
						isNormalWrite = true;
					}
				}
				else
					isNormalWrite = true;

				const TextureProperties& props = getProperties();

				// Check if the subresource will still be bound somewhere after the CBs using it finish
				if (isNormalWrite)
				{
					UINT32 useCount = subresource->getUseCount();
					UINT32 boundCount = subresource->getBoundCount();

					bool isBoundWithoutUse = boundCount > useCount;

					// If image is queued for some operation on a CB, then we need to make a copy of the subresource to
					// avoid modifying its use in the previous operation
					if (isBoundWithoutUse)
					{
						VulkanImage* newImage = createImage(device);

						// Avoid copying original contents if the image only has one sub-resource, which we'll overwrite anyway
						if (props.getNumMipmaps() > 0 || props.getNumFaces() > 1)
						{
							VkImageLayout oldImgLayout = image->getLayout();
							if (oldImgLayout == VK_IMAGE_LAYOUT_UNDEFINED || oldImgLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
								oldImgLayout = getOptimalLayout();

							curLayout = getOptimalLayout();
							copyImage(transferCB, image, newImage, oldImgLayout, curLayout);

							VkAccessFlags accessMask = image->getAccessFlags(oldImgLayout);
							transferCB->getCB()->registerResource(image, accessMask, oldImgLayout, oldImgLayout, 
								oldImgLayout, VulkanUseFlag::Read);
						}

						image->destroy();
						image = newImage;
						mImages[mMappedDeviceIdx] = image;
					}
				}

				VkImageSubresourceRange range;
				if ((props.getUsage() & TU_DEPTHSTENCIL) != 0)
					range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
				else
					range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

				range.baseArrayLayer = mMappedFace;
				range.layerCount = 1;
				range.baseMipLevel = mMappedMip;
				range.levelCount = 1;

				VkImageSubresourceLayers rangeLayers;
				rangeLayers.aspectMask = range.aspectMask;
				rangeLayers.baseArrayLayer = range.baseArrayLayer;
				rangeLayers.layerCount = range.layerCount;
				rangeLayers.mipLevel = range.baseMipLevel;

				VkExtent3D extent;
				PixelUtil::getSizeForMipLevel(props.getWidth(), props.getHeight(), props.getDepth(), mMappedMip,
											  extent.width, extent.height, extent.depth);

				VkImageLayout transferLayout;
				if (mDirectlyMappable)
					transferLayout = VK_IMAGE_LAYOUT_GENERAL;
				else
					transferLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

				// Transfer texture to a valid layout
				VkAccessFlags currentAccessMask = image->getAccessFlags(curLayout);
				transferCB->setLayout(image->getHandle(), currentAccessMask, VK_ACCESS_TRANSFER_WRITE_BIT,
									  curLayout, transferLayout, range);

				// Queue copy command
				mStagingBuffer->copy(transferCB, image, extent, rangeLayers, transferLayout);

				// Transfer back to original  (or optimal if initial layout was undefined/preinitialized)
				VkImageLayout dstLayout = curLayout;
				if(dstLayout == VK_IMAGE_LAYOUT_UNDEFINED || dstLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
					dstLayout = getOptimalLayout();

				currentAccessMask = image->getAccessFlags(dstLayout);
				transferCB->setLayout(image->getHandle(), VK_ACCESS_TRANSFER_WRITE_BIT, currentAccessMask,
									  transferLayout, dstLayout, range);

				// Notify the command buffer that these resources are being used on it
				transferCB->getCB()->registerResource(mStagingBuffer, VK_ACCESS_TRANSFER_READ_BIT, VulkanUseFlag::Read);
				transferCB->getCB()->registerResource(image, currentAccessMask, dstLayout, dstLayout, dstLayout,
					VulkanUseFlag::Write);

				// We don't actually flush the transfer buffer here since it's an expensive operation, but it's instead
				// done automatically before next "normal" command buffer submission.
			}

			mStagingBuffer->destroy();
			mStagingBuffer = nullptr;
		}

		mIsMapped = false;
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

			PixelData myData = lock(discardWholeBuffer ? GBL_WRITE_ONLY_DISCARD : GBL_WRITE_ONLY_DISCARD_RANGE, 
				mipLevel, face, i, queueIdx);
			PixelUtil::bulkPixelConversion(src, myData);
			unlock();
		}

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_Texture);
	}
}
