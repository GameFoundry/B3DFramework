//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanTexture.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanUtility.h"
#include "Managers/BsVulkanCommandBufferManager.h"
#include "BsVulkanHardwareBuffer.h"
#include "CoreThread/BsCoreThread.h"
#include "Profiling/BsRenderStats.h"
#include "Math/BsMath.h"

namespace bs { namespace ct
{
	VULKAN_IMAGE_DESC createDesc(VkImage image, VmaAllocation allocation, VkImageLayout layout, const TextureProperties& props)
	{
		VULKAN_IMAGE_DESC desc;
		desc.image = image;
		desc.allocation = allocation;
		desc.type = props.getTextureType();
		desc.format = VulkanUtility::getPixelFormat(props.getFormat(), props.isHardwareGammaEnabled());
		desc.numFaces = props.getNumFaces();
		desc.numMipLevels = props.getNumMipmaps() + 1;
		desc.layout = layout;
		desc.usage = (UINT32)props.getUsage();

		return desc;
	}

	VulkanImage::VulkanImage(VulkanResourceManager* owner, VkImage image, VmaAllocation allocation, VkImageLayout layout,
							 const TextureProperties& props, bool ownsImage)
		: VulkanImage(owner, createDesc(image, allocation, layout, props), ownsImage)
	{ }

	VulkanImage::VulkanImage(VulkanResourceManager* owner, const VULKAN_IMAGE_DESC& desc, bool ownsImage)
		: VulkanResource(owner, false), mImage(desc.image), mAllocation(desc.allocation)
		, mFramebufferMainView(VK_NULL_HANDLE), mUsage(desc.usage), mOwnsImage(ownsImage), mNumFaces(desc.numFaces)
		, mNumMipLevels(desc.numMipLevels)
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

		TextureSurface completeSurface(0, desc.numMipLevels, 0, desc.numFaces);
		if ((mUsage & TU_DEPTHSTENCIL) != 0)
		{
			mFramebufferMainView = createView(completeSurface, getAspectFlags());
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
			mSubresources[i] = owner->create<VulkanImageSubresource>(desc.layout);
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
			device.freeMemory(mAllocation);
		}
	}

	VkImageView VulkanImage::getView(bool framebuffer) const
	{
		if(framebuffer && (mUsage & TU_DEPTHSTENCIL) != 0)
			return mFramebufferMainView;

		return mMainView;
	}

	VkImageView VulkanImage::getView(const TextureSurface& surface, bool framebuffer) const
	{
		for(auto& entry : mImageInfos)
		{
			if (surface.mipLevel == entry.surface.mipLevel &&
				surface.numMipLevels == entry.surface.numMipLevels &&
				surface.face == entry.surface.face &&
				surface.numFaces == entry.surface.numFaces)
			{
				if((mUsage & TU_DEPTHSTENCIL) == 0)
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

		if ((mUsage & TU_DEPTHSTENCIL) != 0)
		{
			if(framebuffer)
				info.view = createView(surface, getAspectFlags());
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

		const UINT32 numFaces = surface.numFaces == 0 ? mNumFaces : surface.numFaces;

		switch (oldViewType)
		{
		case VK_IMAGE_VIEW_TYPE_CUBE:
			if(numFaces == 1)
				mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			else if(numFaces % 6 == 0)
			{
				if(mNumFaces > 6)
					mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
			}
			else
				mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			break;
		case VK_IMAGE_VIEW_TYPE_1D:
			if(mNumFaces > 1)
				mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			break;
		case VK_IMAGE_VIEW_TYPE_2D:
		case VK_IMAGE_VIEW_TYPE_3D:
			if (mNumFaces > 1)
				mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			break;
		default:
			break;
		}

		mImageViewCI.subresourceRange.aspectMask = aspectMask;
		mImageViewCI.subresourceRange.baseMipLevel = surface.mipLevel;
		mImageViewCI.subresourceRange.levelCount = surface.numMipLevels == 0 ? VK_REMAINING_MIP_LEVELS : surface.numMipLevels;
		mImageViewCI.subresourceRange.baseArrayLayer = surface.face;
		mImageViewCI.subresourceRange.layerCount = surface.numFaces == 0 ? VK_REMAINING_ARRAY_LAYERS : surface.numFaces;

		VkImageView view;
		VkResult result = vkCreateImageView(mOwner->getDevice().getLogical(), &mImageViewCI, gVulkanAllocator, &view);
		assert(result == VK_SUCCESS);

		mImageViewCI.viewType = oldViewType;
		return view;
	}

	VkImageLayout VulkanImage::getOptimalLayout() const
	{
		// If it's load-store, no other flags matter, it must be in general layout
		if ((mUsage & TU_LOADSTORE) != 0)
			return VK_IMAGE_LAYOUT_GENERAL;
		
		if ((mUsage & TU_RENDERTARGET) != 0)
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		else if ((mUsage & TU_DEPTHSTENCIL) != 0)
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		else
		{
			if ((mUsage & TU_DYNAMIC) != 0)
				return VK_IMAGE_LAYOUT_GENERAL;

			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	VkImageAspectFlags VulkanImage::getAspectFlags() const
	{
		if ((mUsage & TU_DEPTHSTENCIL) != 0)
		{
			bool hasStencil = mImageViewCI.format == VK_FORMAT_D16_UNORM_S8_UINT ||
				mImageViewCI.format == VK_FORMAT_D24_UNORM_S8_UINT ||
				mImageViewCI.format == VK_FORMAT_D32_SFLOAT_S8_UINT;

			if (hasStencil)
				return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			
			return VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkImageSubresourceRange VulkanImage::getRange() const
	{
		VkImageSubresourceRange range;
		range.baseArrayLayer = 0;
		range.layerCount = mNumFaces;
		range.baseMipLevel = 0;
		range.levelCount = mNumMipLevels;
		range.aspectMask = getAspectFlags();

		return range;
	}

	VkImageSubresourceRange VulkanImage::getRange(const TextureSurface& surface) const
	{
		VkImageSubresourceRange range;
		range.baseArrayLayer = surface.face;
		range.layerCount = surface.numFaces == 0 ? mNumFaces : surface.numFaces;
		range.baseMipLevel = surface.mipLevel;
		range.levelCount = surface.numMipLevels == 0 ? mNumMipLevels : surface.numMipLevels;
		range.aspectMask = getAspectFlags();

		return range;
	}

	VulkanImageSubresource* VulkanImage::getSubresource(UINT32 face, UINT32 mipLevel)
	{
		assert(mipLevel * mNumFaces + face < mNumFaces * mNumMipLevels);
		return mSubresources[mipLevel * mNumFaces + face];
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

		const UINT32 pixelSize = PixelUtil::getNumElemBytes(output.getFormat());
		assert((UINT32)layout.rowPitch % pixelSize == 0);
		assert((UINT32)layout.depthPitch % pixelSize == 0);

		output.setRowPitch((UINT32)layout.rowPitch / pixelSize);
		output.setSlicePitch((UINT32)layout.depthPitch / pixelSize);

		VkDeviceMemory memory;
		VkDeviceSize memoryOffset;
		device.getAllocationInfo(mAllocation, memory, memoryOffset);

		UINT8* data;
		VkResult result = vkMapMemory(device.getLogical(), memory, memoryOffset + layout.offset, layout.size, 0, (void**)&data);
		assert(result == VK_SUCCESS);

		output.setExternalBuffer(data);
	}

	UINT8* VulkanImage::map(UINT32 offset, UINT32 size) const
	{
		VulkanDevice& device = mOwner->getDevice();

		VkDeviceMemory memory;
		VkDeviceSize memoryOffset;
		device.getAllocationInfo(mAllocation, memory, memoryOffset);

		UINT8* data;
		VkResult result = vkMapMemory(device.getLogical(), memory, memoryOffset + offset, size, 0, (void**)&data);
		assert(result == VK_SUCCESS);

		return data;
	}

	void VulkanImage::unmap()
	{
		VulkanDevice& device = mOwner->getDevice();

		VkDeviceMemory memory;
		VkDeviceSize memoryOffset;
		device.getAllocationInfo(mAllocation, memory, memoryOffset);

		vkUnmapMemory(device.getLogical(), memory);
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

	VkAccessFlags VulkanImage::getAccessFlags(VkImageLayout layout, bool readOnly)
	{
		VkAccessFlags accessFlags;

		switch (layout)
		{
		case VK_IMAGE_LAYOUT_GENERAL:
			{
				accessFlags = VK_ACCESS_SHADER_READ_BIT;
				if ((mUsage & TU_LOADSTORE) != 0)
				{
					if (!readOnly)
						accessFlags |= VK_ACCESS_SHADER_WRITE_BIT;
				}

				if ((mUsage & TU_RENDERTARGET) != 0)
				{
					accessFlags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

					if(!readOnly)
						accessFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				}
				else if ((mUsage & TU_DEPTHSTENCIL) != 0)
				{
					accessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

					if (!readOnly)
						accessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				}
			}

			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			accessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			accessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR:
			accessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			accessFlags = VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			accessFlags = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			accessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			accessFlags = VK_ACCESS_MEMORY_READ_BIT;
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

	void VulkanImage::getBarriers(const VkImageSubresourceRange& range, Vector<VkImageMemoryBarrier>& barriers)
	{
		UINT32 numSubresources = range.levelCount * range.layerCount;

		// Nothing to do
		if (numSubresources == 0)
			return;

		UINT32 mip = range.baseMipLevel;
		UINT32 face = range.baseArrayLayer;
		UINT32 lastMip = range.baseMipLevel + range.levelCount - 1;
		UINT32 lastFace = range.baseArrayLayer + range.layerCount - 1;

		VkImageMemoryBarrier defaultBarrier;
		defaultBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		defaultBarrier.pNext = nullptr;
		defaultBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		defaultBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		defaultBarrier.image = getHandle();
		defaultBarrier.subresourceRange.aspectMask = range.aspectMask;
		defaultBarrier.subresourceRange.layerCount = 1;
		defaultBarrier.subresourceRange.levelCount = 1;
		defaultBarrier.subresourceRange.baseArrayLayer = 0;
		defaultBarrier.subresourceRange.baseMipLevel = 0;

		auto addNewBarrier = [&](VulkanImageSubresource* subresource, UINT32 face, UINT32 mip)
		{
			barriers.push_back(defaultBarrier);
			VkImageMemoryBarrier* barrier = &barriers.back();

			barrier->subresourceRange.baseArrayLayer = face;
			barrier->subresourceRange.baseMipLevel = mip;
			barrier->srcAccessMask = getAccessFlags(subresource->getLayout());
			barrier->oldLayout = subresource->getLayout();

			return barrier;
		};

		bs_frame_mark();
		{
			FrameVector<bool> processed(numSubresources, false);

			// Add first subresource
			VulkanImageSubresource* subresource = getSubresource(face, mip);
			addNewBarrier(subresource, face, mip);
			numSubresources--;
			processed[0] = true;

			while (numSubresources > 0)
			{
				// Try to expand the barrier as much as possible
				VkImageMemoryBarrier* barrier = &barriers.back();

				while (true)
				{
					// Expand by one in the X direction
					bool expandedFace = true;
					if (face < lastFace)
					{
						for (UINT32 i = 0; i < barrier->subresourceRange.levelCount; i++)
						{
							UINT32 curMip = barrier->subresourceRange.baseMipLevel + i;
							VulkanImageSubresource* subresource = getSubresource(face + 1, curMip);
							if (barrier->oldLayout != subresource->getLayout())
							{
								expandedFace = false;
								break;
							}
						}

						if (expandedFace)
						{
							barrier->subresourceRange.layerCount++;
							numSubresources -= barrier->subresourceRange.levelCount;
							face++;

							for (UINT32 i = 0; i < barrier->subresourceRange.levelCount; i++)
							{
								UINT32 curMip = (barrier->subresourceRange.baseMipLevel + i) - range.baseMipLevel;
								UINT32 idx = curMip * range.layerCount + (face - range.baseArrayLayer);
								processed[idx] = true;
							}
						}
					}
					else
						expandedFace = false;

					// Expand by one in the Y direction
					bool expandedMip = true;
					if (mip < lastMip)
					{
						for (UINT32 i = 0; i < barrier->subresourceRange.layerCount; i++)
						{
							UINT32 curFace = barrier->subresourceRange.baseArrayLayer + i;
							VulkanImageSubresource* subresource = getSubresource(curFace, mip + 1);
							if (barrier->oldLayout != subresource->getLayout())
							{
								expandedMip = false;
								break;
							}
						}

						if (expandedMip)
						{
							barrier->subresourceRange.levelCount++;
							numSubresources -= barrier->subresourceRange.layerCount;
							mip++;

							for (UINT32 i = 0; i < barrier->subresourceRange.layerCount; i++)
							{
								UINT32 curFace = (barrier->subresourceRange.baseArrayLayer + i) - range.baseArrayLayer;
								UINT32 idx = (mip - range.baseMipLevel) * range.layerCount + curFace;
								processed[idx] = true;
							}
						}
					}
					else
						expandedMip = false;

					// If we can't grow no more, we're done with this square
					if (!expandedMip && !expandedFace)
						break;
				}

				// Look for a new starting point (sub-resource we haven't processed yet)
				for (UINT32 i = 0; i < range.levelCount; i++)
				{
					bool found = false;
					for (UINT32 j = 0; j < range.layerCount; j++)
					{
						UINT32 idx = i * range.layerCount + j;
						if (!processed[idx])
						{
							mip = range.baseMipLevel + i;
							face = range.baseArrayLayer + j;

							found = true;
							processed[idx] = true;
							break;
						}
					}

					if (found)
					{
						VulkanImageSubresource* subresource = getSubresource(face, mip);
						addNewBarrier(subresource, face, mip);
						numSubresources--;
						break;
					}
				}
			}
		}
		bs_frame_clear();
	}

	VulkanImageSubresource::VulkanImageSubresource(VulkanResourceManager* owner, VkImageLayout layout)
		:VulkanResource(owner, false), mLayout(layout)
	{ }

	VulkanTexture::VulkanTexture(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData,
										 GpuDeviceFlags deviceMask)
		: Texture(desc, initialData, deviceMask), mImages(), mInternalFormats(), mDeviceMask(deviceMask)
		, mStagingBuffer(nullptr), mMappedDeviceIdx((UINT32)-1), mMappedGlobalQueueIdx((UINT32)-1)
		, mMappedMip(0), mMappedFace(0), mMappedRowPitch(0), mMappedSlicePitch(0)
		, mMappedLockOptions(GBL_WRITE_ONLY), mDirectlyMappable(false), mSupportsGPUWrites(false), mIsMapped(false)
	{
		
	}

	VulkanTexture::~VulkanTexture()
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

	void VulkanTexture::initialize()
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
		
		mImageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		int usage = props.getUsage();
		if ((usage & TU_RENDERTARGET) != 0)
		{
			mImageCI.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			mSupportsGPUWrites = true;
		}
		else if ((usage & TU_DEPTHSTENCIL) != 0)
		{
			mImageCI.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			mSupportsGPUWrites = true;
		}
		
		if ((usage & TU_LOADSTORE) != 0)
		{
			mImageCI.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
			mSupportsGPUWrites = true;
		}

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

		mImageCI.extent = { props.getWidth(), props.getHeight(), props.getDepth() };
		mImageCI.mipLevels = props.getNumMipmaps() + 1;
		mImageCI.arrayLayers = props.getNumFaces();
		mImageCI.samples = VulkanUtility::getSampleFlags(props.getNumSamples());
		mImageCI.tiling = tiling;
		mImageCI.initialLayout = layout;
		mImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		mImageCI.queueFamilyIndexCount = 0;
		mImageCI.pQueueFamilyIndices = nullptr;

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::instance());
		VulkanDevice* devices[BS_MAX_DEVICES];
		VulkanUtility::getDevices(rapi, mDeviceMask, devices);

		// Allocate buffers per-device
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] == nullptr)
				continue;

			bool optimalTiling = tiling == VK_IMAGE_TILING_OPTIMAL;

			mInternalFormats[i] = VulkanUtility::getClosestSupportedPixelFormat(
				*devices[i], props.getFormat(), props.getTextureType(), props.getUsage(), optimalTiling,
				props.isHardwareGammaEnabled());

			mImages[i] = createImage(*devices[i], mInternalFormats[i]);
			
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Texture);
		Texture::initialize();
	}

	VulkanImage* VulkanTexture::createImage(VulkanDevice& device, PixelFormat format)
	{
		bool directlyMappable = mImageCI.tiling == VK_IMAGE_TILING_LINEAR;
		VkMemoryPropertyFlags flags = directlyMappable ?
			(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) : // Note: Try using cached memory
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VkDevice vkDevice = device.getLogical();

		mImageCI.format = VulkanUtility::getPixelFormat(format, mProperties.isHardwareGammaEnabled());;

		VkImage image;
		VkResult result = vkCreateImage(vkDevice, &mImageCI, gVulkanAllocator, &image);
		assert(result == VK_SUCCESS);

		VmaAllocation allocation = device.allocateMemory(image, flags);
		return device.getResourceManager().create<VulkanImage>(image, allocation, mImageCI.initialLayout, getProperties());
	}

	VulkanBuffer* VulkanTexture::createStaging(VulkanDevice& device, const PixelData& pixelData, bool readable)
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

		VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		VmaAllocation allocation = device.allocateMemory(buffer, flags);

		return device.getResourceManager().create<VulkanBuffer>(buffer, allocation,
			pixelData.getRowPitch(), pixelData.getSlicePitch());
	}

	void VulkanTexture::copyImage(VulkanTransferBuffer* cb, VulkanImage* srcImage, VulkanImage* dstImage, 
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
		cb->setLayout(srcImage, range, VK_ACCESS_TRANSFER_READ_BIT, transferSrcLayout);
		cb->setLayout(dstImage, range, VK_ACCESS_TRANSFER_WRITE_BIT, transferDstLayout);

		vkCmdCopyImage(cb->getCB()->getHandle(), srcImage->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						dstImage->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, numMipmaps, imageRegions);

		// Transfer back to final layouts
		VkAccessFlags srcAccessMask = srcImage->getAccessFlags(srcFinalLayout);
		cb->setLayout(srcImage->getHandle(), VK_ACCESS_TRANSFER_READ_BIT, srcAccessMask,
							  transferSrcLayout, srcFinalLayout, range);

		VkAccessFlags dstAccessMask = dstImage->getAccessFlags(dstFinalLayout);
		cb->setLayout(dstImage->getHandle(), VK_ACCESS_TRANSFER_WRITE_BIT, dstAccessMask,
							  transferDstLayout, dstFinalLayout, range);

		cb->getCB()->registerImageTransfer(srcImage, range, srcFinalLayout, VulkanAccessFlag::Read);
		cb->getCB()->registerImageTransfer(dstImage, range, dstFinalLayout, VulkanAccessFlag::Write);

		bs_stack_free(imageRegions);
	}

	void VulkanTexture::copyImpl(const SPtr<Texture>& target, const TEXTURE_COPY_DESC& desc, 
			const SPtr<CommandBuffer>& commandBuffer)
	{
		VulkanTexture* other = static_cast<VulkanTexture*>(target.get());

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

		VkImageLayout transferSrcLayout = mDirectlyMappable ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		VkImageLayout transferDstLayout = other->mDirectlyMappable ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		UINT32 mipWidth, mipHeight, mipDepth;

		bool copyEntireSurface = desc.srcVolume.getWidth() == 0 || 
			desc.srcVolume.getHeight() == 0 || 
			desc.srcVolume.getDepth() == 0;

		if(copyEntireSurface)
		{
			PixelUtil::getSizeForMipLevel(
				srcProps.getWidth(),
				srcProps.getHeight(),
				srcProps.getDepth(),
				desc.srcMip,
				mipWidth,
				mipHeight,
				mipDepth);
		}
		else
		{
			mipWidth = desc.srcVolume.getWidth();
			mipHeight = desc.srcVolume.getHeight();
			mipDepth = desc.srcVolume.getDepth();
		}

		VkImageResolve resolveRegion;
		resolveRegion.srcOffset = { (INT32)desc.srcVolume.left, (INT32)desc.srcVolume.top, (INT32)desc.srcVolume.front };
		resolveRegion.dstOffset = { desc.dstPosition.x, desc.dstPosition.y, desc.dstPosition.z };
		resolveRegion.extent = { mipWidth, mipHeight, mipDepth };
		resolveRegion.srcSubresource.baseArrayLayer = desc.srcFace;
		resolveRegion.srcSubresource.layerCount = 1;
		resolveRegion.srcSubresource.mipLevel = desc.srcMip;
		resolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		resolveRegion.dstSubresource.baseArrayLayer = desc.dstFace;
		resolveRegion.dstSubresource.layerCount = 1;
		resolveRegion.dstSubresource.mipLevel = desc.dstMip;
		resolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageCopy imageRegion;
		imageRegion.srcOffset = { (INT32)desc.srcVolume.left, (INT32)desc.srcVolume.top, (INT32)desc.srcVolume.front };
		imageRegion.dstOffset = { desc.dstPosition.x, desc.dstPosition.y, desc.dstPosition.z };
		imageRegion.extent = { mipWidth, mipHeight, mipDepth };
		imageRegion.srcSubresource.baseArrayLayer = desc.srcFace;
		imageRegion.srcSubresource.layerCount = 1;
		imageRegion.srcSubresource.mipLevel = desc.srcMip;
		imageRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageRegion.dstSubresource.baseArrayLayer = desc.dstFace;
		imageRegion.dstSubresource.layerCount = 1;
		imageRegion.dstSubresource.mipLevel = desc.dstMip;
		imageRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageSubresourceRange srcRange;
		srcRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		srcRange.baseArrayLayer = desc.srcFace;
		srcRange.layerCount = 1;
		srcRange.baseMipLevel = desc.srcMip;
		srcRange.levelCount = 1;

		VkImageSubresourceRange dstRange;
		dstRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		dstRange.baseArrayLayer = desc.dstFace;
		dstRange.layerCount = 1;
		dstRange.baseMipLevel = desc.dstMip;
		dstRange.levelCount = 1;

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::instance());

		VulkanCmdBuffer* vkCB;
		if (commandBuffer != nullptr)
			vkCB = static_cast<VulkanCommandBuffer*>(commandBuffer.get())->getInternal();
		else
			vkCB = rapi._getMainCommandBuffer()->getInternal();

		UINT32 deviceIdx = vkCB->getDeviceIdx();

		VulkanImage* srcImage = mImages[deviceIdx];
		VulkanImage* dstImage = other->getResource(deviceIdx);

		if (srcImage == nullptr || dstImage == nullptr)
			return;

		VkImageLayout srcLayout = vkCB->getCurrentLayout(srcImage, srcRange, false);
		VkImageLayout dstLayout = vkCB->getCurrentLayout(dstImage, dstRange, false);

		VkCommandBuffer vkCmdBuf = vkCB->getHandle();

		VkAccessFlags srcAccessMask = srcImage->getAccessFlags(srcLayout);
		VkAccessFlags dstAccessMask = dstImage->getAccessFlags(dstLayout);

		if (vkCB->isInRenderPass())
			vkCB->endRenderPass();

		// Transfer textures to a valid layout
		vkCB->setLayout(srcImage->getHandle(), srcAccessMask, VK_ACCESS_TRANSFER_READ_BIT, srcLayout,
								transferSrcLayout, srcRange);

		vkCB->setLayout(dstImage->getHandle(), dstAccessMask, VK_ACCESS_TRANSFER_WRITE_BIT,
								dstLayout, transferDstLayout, dstRange);

		if (srcHasMultisample && !destHasMultisample) // Resolving from MS to non-MS texture
		{
			vkCmdResolveImage(vkCmdBuf, srcImage->getHandle(), transferSrcLayout, dstImage->getHandle(), transferDstLayout, 
				1, &resolveRegion);
		}
		else // Just a normal copy
		{
			vkCmdCopyImage(vkCmdBuf, srcImage->getHandle(), transferSrcLayout, dstImage->getHandle(), transferDstLayout, 
				1, &imageRegion);
		}

		// Notify the command buffer that these resources are being used on it
		vkCB->registerImageTransfer(srcImage, srcRange, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VulkanAccessFlag::Read);
		vkCB->registerImageTransfer(dstImage, dstRange, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VulkanAccessFlag::Write);
	}

	PixelData VulkanTexture::lockImpl(GpuLockOptions options, UINT32 mipLevel, UINT32 face, UINT32 deviceIdx,
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

		PixelData lockedArea(mipWidth, mipHeight, mipDepth, mInternalFormats[deviceIdx]);

		VulkanImage* image = mImages[deviceIdx];

		if (image == nullptr)
			return PixelData();

		mIsMapped = true;
		mMappedDeviceIdx = deviceIdx;
		mMappedGlobalQueueIdx = queueIdx;
		mMappedFace = face;
		mMappedMip = mipLevel;
		mMappedLockOptions = options;

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::instance());
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
			assert(subresource->getLayout() == VK_IMAGE_LAYOUT_PREINITIALIZED || 
				   subresource->getLayout() == VK_IMAGE_LAYOUT_GENERAL);

			// GPU should never be allowed to write to a directly mappable texture, since only linear tiling is supported
			// for direct mapping, and we don't support using it with either storage textures or render targets.
			assert(!mSupportsGPUWrites);

			// Check is the GPU currently reading from the image
			UINT32 useMask = subresource->getUseInfo(VulkanAccessFlag::Read);
			bool isUsedOnGPU = useMask != 0;

			// We're safe to map directly since GPU isn't using the subresource
			if (!isUsedOnGPU)
			{
				// If some CB has an operation queued that will be using the current contents of the image, create a new 
				// image so we don't modify the previous use of the image
				if (subresource->isBound())
				{
					VulkanImage* newImage = createImage(device, mInternalFormats[deviceIdx]);

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

				image = createImage(device, mInternalFormats[deviceIdx]);
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
					useMask = subresource->getUseInfo(VulkanAccessFlag::Write);
				else
					useMask = subresource->getUseInfo(VulkanAccessFlag::Read | VulkanAccessFlag::Write);

				transferCB->appendMask(useMask);

				// Submit the command buffer and wait until it finishes
				transferCB->flush(true);

				// If writing and some CB has an operation queued that will be using the current contents of the image, 
				// create a new image so we don't modify the previous use of the image
				if (options == GBL_READ_WRITE && subresource->isBound())
				{
					VulkanImage* newImage = createImage(device, mInternalFormats[deviceIdx]);

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
			UINT32 writeUseMask = subresource->getUseInfo(VulkanAccessFlag::Write);

			if (mSupportsGPUWrites || writeUseMask != 0)
			{
				// Ensure flush() will wait for all queues currently writing to the image (if any) to finish
				transferCB->appendMask(writeUseMask);
			}

			VkImageSubresourceRange range;
			range.aspectMask = image->getAspectFlags();
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
			VkAccessFlags currentAccessMask = image->getAccessFlags(subresource->getLayout());
			transferCB->setLayout(image->getHandle(), currentAccessMask, VK_ACCESS_TRANSFER_READ_BIT, subresource->getLayout(),
								  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, range);

			// Queue copy command
			image->copy(transferCB, mStagingBuffer, extent, rangeLayers, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			// Transfer back to original layout
			VkImageLayout dstLayout = image->getOptimalLayout();
			currentAccessMask = image->getAccessFlags(dstLayout);

			transferCB->setLayout(image->getHandle(), VK_ACCESS_TRANSFER_READ_BIT, currentAccessMask,
								  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstLayout, range);
			transferCB->getCB()->registerImageTransfer(image, range, dstLayout, VulkanAccessFlag::Read);

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

	void VulkanTexture::unlockImpl()
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

			// If the caller wrote anything to the staging buffer, we need to upload it back to the main buffer
			if (isWrite)
			{
				VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::instance());
				VulkanDevice& device = *rapi._getDevice(mMappedDeviceIdx);

				VulkanCommandBufferManager& cbManager = gVulkanCBManager();
				GpuQueueType queueType;
				UINT32 localQueueIdx = CommandSyncMask::getQueueIdxAndType(mMappedGlobalQueueIdx, queueType);

				VulkanImage* image = mImages[mMappedDeviceIdx];
				VulkanTransferBuffer* transferCB = cbManager.getTransferBuffer(mMappedDeviceIdx, queueType, localQueueIdx);

				VulkanImageSubresource* subresource = image->getSubresource(mMappedFace, mMappedMip);
				VkImageLayout curLayout = subresource->getLayout();

				// If the subresource is used in any way on the GPU, we need to wait for that use to finish before
				// we issue our copy
				UINT32 useMask = subresource->getUseInfo(VulkanAccessFlag::Read | VulkanAccessFlag::Write);
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

						image = createImage(device, mInternalFormats[mMappedDeviceIdx]);
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
						VulkanImage* newImage = createImage(device, mInternalFormats[mMappedDeviceIdx]);

						// Avoid copying original contents if the image only has one sub-resource, which we'll overwrite anyway
						if (props.getNumMipmaps() > 0 || props.getNumFaces() > 1)
						{
							VkImageLayout oldImgLayout = image->getOptimalLayout();

							curLayout = newImage->getOptimalLayout();
							copyImage(transferCB, image, newImage, oldImgLayout, curLayout);
						}

						image->destroy();
						image = newImage;
						mImages[mMappedDeviceIdx] = image;
					}
				}

				VkImageSubresourceRange range;
				range.aspectMask = image->getAspectFlags();
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
				mStagingBuffer->copy(transferCB->getCB(), image, extent, rangeLayers, transferLayout);

				// Transfer back to original  (or optimal if initial layout was undefined/preinitialized)
				VkImageLayout dstLayout = image->getOptimalLayout();

				currentAccessMask = image->getAccessFlags(dstLayout);
				transferCB->setLayout(image->getHandle(), VK_ACCESS_TRANSFER_WRITE_BIT, currentAccessMask,
									  transferLayout, dstLayout, range);

				// Notify the command buffer that these resources are being used on it
				transferCB->getCB()->registerBuffer(mStagingBuffer, BufferUseFlagBits::Transfer, VulkanAccessFlag::Read);
				transferCB->getCB()->registerImageTransfer(image, range, dstLayout, VulkanAccessFlag::Write);

				// We don't actually flush the transfer buffer here since it's an expensive operation, but it's instead
				// done automatically before next "normal" command buffer submission.
			}

			mStagingBuffer->destroy();
			mStagingBuffer = nullptr;
		}

		mIsMapped = false;
	}

	void VulkanTexture::readDataImpl(PixelData& dest, UINT32 mipLevel, UINT32 face, UINT32 deviceIdx, UINT32 queueIdx)
	{
		if (mProperties.getNumSamples() > 1)
		{
			LOGERR("Multisampled textures cannot be accessed from the CPU directly.");
			return;
		}

		PixelData myData = lock(GBL_READ_ONLY, mipLevel, face, deviceIdx, queueIdx);
		PixelUtil::bulkPixelConversion(myData, dest);
		unlock();

		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_Texture);
	}

	void VulkanTexture::writeDataImpl(const PixelData& src, UINT32 mipLevel, UINT32 face, bool discardWholeBuffer,
									  UINT32 queueIdx)
	{
		if (mProperties.getNumSamples() > 1)
		{
			LOGERR("Multisampled textures cannot be accessed from the CPU directly.");
			return;
		}

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
}}
