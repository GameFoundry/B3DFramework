//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanTexture.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanFramebuffer.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanSubmitThread.h"
#include "CoreObject/B3DRenderThread.h"
#include "Profiling/B3DRenderStats.h"
#include "Math/B3DMath.h"

using namespace b3d;
using namespace b3d::render;

static VulkanImageCreateInformation BuildImageCreateInformation(VkImage image, VulkanAllocationResult allocation, VkImageLayout layout, VkFormat actualFormat, const TextureProperties& props)
{
	VulkanImageCreateInformation desc;
	desc.Image = image;
	desc.Allocation = allocation;
	desc.Type = props.Type;
	desc.Format = actualFormat;
	desc.FaceCount = props.GetFaceCount();
	desc.DepthSliceCount = props.Depth;
	desc.MipLevelCount = props.MipMapCount + 1;
	desc.Layout = layout;
	desc.Usage = props.Usage;

	return desc;
}

VulkanImage::VulkanImage(VulkanResourceManager* owner, VkImage image, VulkanAllocationResult allocation, VkImageLayout layout, VkFormat actualFormat, const TextureProperties& textureProperties, bool ownsImage, bool isShaderReadAllowed, const StringView& name)
	: VulkanImage(owner, BuildImageCreateInformation(image, allocation, layout, actualFormat, textureProperties), ownsImage, isShaderReadAllowed, name)
{}

VulkanImage::VulkanImage(VulkanResourceManager* owner, const VulkanImageCreateInformation& createInformation, bool ownsImage, bool isShaderReadAllowed, const StringView& name)
	: VulkanResource(owner, false, name), mImage(createInformation.Image), mAllocation(createInformation.Allocation.Handle), mMappedMemory(createInformation.Allocation.MappedMemory), mUsage(createInformation.Usage), mOwnsImage(ownsImage), mIsShaderReadAllowed(isShaderReadAllowed), mFaceCount(createInformation.FaceCount), mDepthSliceCount(createInformation.DepthSliceCount), mMipLevelCount(createInformation.MipLevelCount)
{
	mImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	mImageViewCI.pNext = nullptr;
	mImageViewCI.flags = 0;
	mImageViewCI.image = createInformation.Image;
	mImageViewCI.format = createInformation.Format;
	mImageViewCI.components = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};

	switch(createInformation.Type)
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

	TextureSurface completeSurface(0, 0, 0, 0);

	// For depth stencil attachments we need a special view for shader reads and for framebuffer attachment, so we create two main views
	if(mUsage.IsSet(TextureUsageFlag::DepthStencil))
	{
		mFramebufferMainView = CreateView(completeSurface, createInformation.Format, GetAspectFlags(), true);
		mMainView = CreateView(completeSurface, createInformation.Format, VK_IMAGE_ASPECT_DEPTH_BIT, false);
	}
	else
	{
		// For 3D render attachments we also require a special view for framebuffer attachments
		if(mDepthSliceCount > 1 && mUsage.IsSet(TextureUsageFlag::RenderTarget))
			mFramebufferMainView = CreateView(completeSurface, createInformation.Format, VK_IMAGE_ASPECT_COLOR_BIT, true);

		// For all other cases (non-framebuffer attachment, or a non-3D non-depth-stencil attachment) regular view will suffice.
		mMainView = CreateView(completeSurface, createInformation.Format, VK_IMAGE_ASPECT_COLOR_BIT, false);
	}

	const u32 subresourceCount = mFaceCount * mMipLevelCount;
	mSubresources = (VulkanImageSubresource**)B3DAllocate(sizeof(VulkanImageSubresource*) * subresourceCount);
	for(u32 i = 0; i < subresourceCount; i++)
		mSubresources[i] = owner->Create<VulkanImageSubresource>(createInformation.Layout);
}

VulkanImage::~VulkanImage()
{
	VulkanGpuDevice& device = mOwner->GetDevice();
	VkDevice vkDevice = device.GetLogical();

	const u32 subresourceCount = mFaceCount * mMipLevelCount;
	for(u32 i = 0; i < subresourceCount; i++)
	{
		B3D_ASSERT(!mSubresources[i]->IsBound()); // Image beeing freed but its subresources are still bound somewhere

		mSubresources[i]->Destroy();
	}

	B3DFree(mSubresources);
	mSubresources = nullptr;

	{
		Lock lock(mViewsMutex);

		for(auto& entry : mImageInfos)
			vkDestroyImageView(vkDevice, entry.View.Handle, gVulkanAllocator);
	}

	if(mOwnsImage)
	{
		vkDestroyImage(vkDevice, mImage, gVulkanAllocator);
		device.FreeMemory(mAllocation);
	}
}

void VulkanImage::Destroy()
{
	const bool isUsedAsRenderTarget = mUsage.IsSetAny(TextureUsageFlag::RenderTarget | TextureUsageFlag::DepthStencil);
	if(isUsedAsRenderTarget && VulkanFramebufferCache::IsStarted())
	{
		VulkanFramebufferCache::Instance().NotifyImageDestroyed(*this);
	}

	VulkanResource::Destroy();
}

void VulkanImage::SetName(const StringView& name)
{
	SetDebugName(name);

	if(vkSetDebugUtilsObjectNameEXT == nullptr)
		return;

	VkDebugUtilsObjectNameInfoEXT objectNameInfo;
	objectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	objectNameInfo.pNext = nullptr;
	objectNameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
	objectNameInfo.objectHandle = (uint64_t)mImage;
	objectNameInfo.pObjectName = name.data();

	vkSetDebugUtilsObjectNameEXT(mOwner->GetDevice().GetLogical(), &objectNameInfo);
}

VulkanImageView VulkanImage::GetView(bool isPartOfFramebuffer) const
{
	if(isPartOfFramebuffer)
	{
		if(mUsage.IsSetAny(TextureUsageFlag::DepthStencil))
			return mFramebufferMainView;

		if(mDepthSliceCount > 1 && mUsage.IsSet(TextureUsageFlag::RenderTarget))
			return mFramebufferMainView;
	}

	return mMainView;
}

VulkanImageView VulkanImage::GetView(const TextureSurface& surface, bool isPartOfFramebuffer) const
{
	return GetView(mImageViewCI.format, surface, isPartOfFramebuffer);
}

VulkanImageView VulkanImage::GetView(VkFormat format, bool isPartOfFramebuffer) const
{
	TextureSurface completeSurface(0, mMipLevelCount, 0, mFaceCount);
	return GetView(format, completeSurface, isPartOfFramebuffer);
}

VulkanImageView VulkanImage::GetView(VkFormat format, const TextureSurface& surface, bool isPartOfFrameBuffer) const
{
	TextureSurface explicitSurface = surface;
	CalculateExplicitSurface(explicitSurface, isPartOfFrameBuffer);

	Lock lock(mViewsMutex);
	for(auto& entry : mImageInfos)
	{
		// Check if framebuffer field matches, but only if this is a depth-stencil framebuffer attachment or a 3D render texture attachment. For all other
		// cases we're free to use the regular view.
		const bool isFramebufferFieldMatching =
			isPartOfFrameBuffer == entry.IsPartOfFramebuffer ||
			(!mUsage.IsSet(TextureUsageFlag::DepthStencil) && (!mUsage.IsSet(TextureUsageFlag::RenderTarget) || mDepthSliceCount <= 1));

		if(explicitSurface == entry.Surface && format == entry.Format && isFramebufferFieldMatching)
			return entry.View;
	}

	VulkanImageView view;
	if(mUsage.IsSet(TextureUsageFlag::DepthStencil))
	{
		if(isPartOfFrameBuffer)
			view = CreateView(explicitSurface, format, GetAspectFlags(), isPartOfFrameBuffer);
		else
			view = CreateView(explicitSurface, format, VK_IMAGE_ASPECT_DEPTH_BIT, isPartOfFrameBuffer);
	}
	else
	{
		view = CreateView(explicitSurface, format, VK_IMAGE_ASPECT_COLOR_BIT, isPartOfFrameBuffer);
	}

	return view;
}

VulkanImageView VulkanImage::CreateView(const TextureSurface& surface, VkFormat format, VkImageAspectFlags aspectMask, bool isPartOfFramebuffer) const
{
	VkImageViewType oldViewType = mImageViewCI.viewType;
	VkFormat oldFormat = mImageViewCI.format;

	u32 layerCount = surface.FaceCount;
	if(layerCount == 0)
	{
		// 3D textures bound as framebuffer attachments are bound as 2D texture arrays
		if(isPartOfFramebuffer && mDepthSliceCount > 1)
			layerCount = mDepthSliceCount;
		else
			layerCount = mFaceCount;
	}

	switch(oldViewType)
	{
	case VK_IMAGE_VIEW_TYPE_CUBE:
		if(layerCount == 1)
			mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		else if(layerCount % 6 == 0)
		{
			if(surface.IsBoundAs2DArray)
			{
				mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			}
			else
			{
				if(mFaceCount > 6)
				{
					mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
				}
			}
		}
		else
			mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		break;
	case VK_IMAGE_VIEW_TYPE_1D:
		if(layerCount > 1)
			mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		break;
	case VK_IMAGE_VIEW_TYPE_2D:
	case VK_IMAGE_VIEW_TYPE_3D:
		if(layerCount > 1 || surface.IsBoundAs2DArray)
			mImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		break;
	default:
		break;
	}

	mImageViewCI.subresourceRange.aspectMask = aspectMask;
	mImageViewCI.subresourceRange.baseMipLevel = surface.MipLevel;
	mImageViewCI.subresourceRange.levelCount = surface.MipLevelCount == 0 ? VK_REMAINING_MIP_LEVELS : surface.MipLevelCount;
	mImageViewCI.subresourceRange.baseArrayLayer = surface.Face;
	mImageViewCI.subresourceRange.layerCount = layerCount;
	mImageViewCI.format = format;

	VulkanImageView view;
	view.Type = mImageViewCI.viewType;

	VkResult result = vkCreateImageView(mOwner->GetDevice().GetLogical(), &mImageViewCI, gVulkanAllocator, &view.Handle);
	B3D_ASSERT(result == VK_SUCCESS);

	mImageViewCI.viewType = oldViewType;
	mImageViewCI.format = oldFormat;

	// Always use explicit surface for lookup
	TextureSurface explicitSurface = surface;
	CalculateExplicitSurface(explicitSurface, isPartOfFramebuffer);

	ImageViewInformation viewInformation;
	viewInformation.Surface = explicitSurface;
	viewInformation.IsPartOfFramebuffer = isPartOfFramebuffer;
	viewInformation.View = view;
	viewInformation.Format = format;

	mImageInfos.push_back(viewInformation);

	return view;
}

const TextureSurface& VulkanImage::CalculateExplicitSurface(TextureSurface& surface, bool isPartOfFramebuffer) const
{
	if(surface.MipLevelCount == 0)
		surface.MipLevelCount = mMipLevelCount;

	if(surface.FaceCount == 0)
	{
		const bool is3D = mDepthSliceCount > 1;

		if(is3D)
		{
			if(isPartOfFramebuffer || surface.IsBoundAs2DArray)
			{
				// This forces CreateView() to view the 3D texture as a 2D array
				surface.FaceCount = mDepthSliceCount;
			}
		}
		else
		{
			surface.FaceCount = mFaceCount;
		}
	}

	return surface;
}

VkImageAspectFlags VulkanImage::GetAspectFlags() const
{
	if(mUsage.IsSet(TextureUsageFlag::DepthStencil))
	{
		bool hasStencil = mImageViewCI.format == VK_FORMAT_D16_UNORM_S8_UINT ||
			mImageViewCI.format == VK_FORMAT_D24_UNORM_S8_UINT ||
			mImageViewCI.format == VK_FORMAT_D32_SFLOAT_S8_UINT;

		if(hasStencil)
			return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

		return VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	return VK_IMAGE_ASPECT_COLOR_BIT;
}

VkImageSubresourceRange VulkanImage::GetRange() const
{
	VkImageSubresourceRange range;
	range.baseArrayLayer = 0;
	range.layerCount = mFaceCount;
	range.baseMipLevel = 0;
	range.levelCount = mMipLevelCount;
	range.aspectMask = GetAspectFlags();

	return range;
}

VkImageSubresourceRange VulkanImage::GetRange(const TextureSurface& surface) const
{
	VkImageSubresourceRange range;
	range.baseArrayLayer = surface.Face;
	range.layerCount = Math::Min(surface.FaceCount == 0 ? mFaceCount : surface.FaceCount, mFaceCount);
	range.baseMipLevel = surface.MipLevel;
	range.levelCount = Math::Min(surface.MipLevelCount == 0 ? mMipLevelCount : surface.MipLevelCount, mMipLevelCount);
	range.aspectMask = GetAspectFlags();

	return range;
}

VulkanImageSubresource* VulkanImage::GetSubresource(u32 face, u32 mipLevel)
{
	B3D_ASSERT(mipLevel * mFaceCount + face < mFaceCount * mMipLevelCount);
	return mSubresources[mipLevel * mFaceCount + face];
}

VkSubresourceLayout VulkanImage::GetSubresourceLayout(u32 face, u32 mipLevel) const
{
	VulkanGpuDevice& device = mOwner->GetDevice();

	VkImageSubresource subresourceRange;
	subresourceRange.mipLevel = mipLevel;
	subresourceRange.arrayLayer = face;

	if(mImageViewCI.subresourceRange.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT)
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	else
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; // Ignoring stencil

	VkSubresourceLayout layout;
	vkGetImageSubresourceLayout(device.GetLogical(), mImage, &subresourceRange, &layout);
	
	return layout;
}

ImageSubresourcePitch VulkanImage::ConvertSubresourceLayoutToBlocks(const VkSubresourceLayout& subresourceLayout, PixelFormat format)
{
	const u32 blockSize = PixelUtility::GetBlockSize(format);

	B3D_ASSERT(subresourceLayout.rowPitch % blockSize == 0);
	B3D_ASSERT(subresourceLayout.depthPitch % blockSize == 0);

	u32 rowPitchInPixels = (u32)(subresourceLayout.rowPitch / blockSize);
	u32 depthPitch = (u32)(subresourceLayout.depthPitch / blockSize);

	if(PixelUtility::IsCompressed(format))
	{
		// For compressed formats, we return the pitch in blocks
		const Vector2I blockDimension = PixelUtility::GetBlockDimensions(format);
		rowPitchInPixels *= blockDimension.X;
		depthPitch *= blockDimension.X * blockDimension.Y;
	}

	return ImageSubresourcePitch(rowPitchInPixels, rowPitchInPixels != 0 ? depthPitch / rowPitchInPixels : 0);
}

void VulkanImage::ApplyRowAndSlicePitch(const VkSubresourceLayout& layout, PixelData& pixelData)
{
	u32 slicePitch = (u32)layout.depthPitch;
	if(slicePitch == 0)
		slicePitch = (u32)layout.rowPitch * pixelData.GetHeight();

	pixelData.SetRowPitch((u32)layout.rowPitch);
	pixelData.SetSlicePitch(slicePitch);
}

void VulkanImage::Map(u32 mipLevel, u32 arrayLayer, PixelData& output, bool isInvalidateRequired) const
{
	VkSubresourceLayout layout = GetSubresourceLayout(arrayLayer, mipLevel);
	ApplyRowAndSlicePitch(layout, output);

	u8 *const data = Map(layout.offset, layout.size, isInvalidateRequired);
	output.SetExternalBuffer(data);
}

u8* VulkanImage::Map(VkDeviceSize offset, VkDeviceSize size, bool isInvalidateRequired) const
{
	VulkanGpuDevice& device = mOwner->GetDevice();

	if(isInvalidateRequired)
		device.InvalidateMemory(mAllocation, offset, size);

	void* data = device.MapMemory(mAllocation);

	mMappedOffset = offset;
	mMappedSize = size;

	return (u8*)data + offset;
}

void VulkanImage::Unmap(bool isFlushRequired)
{
	VulkanGpuDevice& device = mOwner->GetDevice();

	device.UnmapMemory(mAllocation);

	if(isFlushRequired)
		device.FlushMemory(mAllocation, mMappedOffset, mMappedSize);
}

void VulkanImage::Flush(VkDeviceSize offset, VkDeviceSize size)
{
	VulkanGpuDevice& device = mOwner->GetDevice();
	device.FlushMemory(mAllocation, offset, size);
}

void VulkanImage::Invalidate(VkDeviceSize offset, VkDeviceSize size)
{
	VulkanGpuDevice& device = mOwner->GetDevice();
	device.InvalidateMemory(mAllocation, offset, size);
}

VkAccessFlags VulkanImage::GetAccessFlags(VkImageLayout layout, bool readOnly)
{
	VkAccessFlags accessFlags;

	switch(layout)
	{
	case VK_IMAGE_LAYOUT_GENERAL:
		{
			accessFlags = VK_ACCESS_SHADER_READ_BIT;
			if(mUsage.IsSet(TextureUsageFlag::AllowUnorderedAccessOnTheGPU))
			{
				if(!readOnly)
					accessFlags |= VK_ACCESS_SHADER_WRITE_BIT;
			}

			if(mUsage.IsSet(TextureUsageFlag::RenderTarget))
			{
				accessFlags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

				if(!readOnly)
					accessFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}
			else if(mUsage.IsSet(TextureUsageFlag::DepthStencil))
			{
				accessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

				if(!readOnly)
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
		B3D_LOG(Warning, LogRenderBackend, "Unsupported source layout for Vulkan image.");
		break;
	}

	return accessFlags;
}

void VulkanImage::GetBarriers(const VkImageSubresourceRange& range, TArray<VkImageMemoryBarrier>& barriers)
{
	AssertIfNotVulkanSubmitThread();

	// Nothing to do
	if (range.levelCount == 0 || range.layerCount == 0)
		return;

	u32 mipLevel = range.baseMipLevel;
	u32 face = range.baseArrayLayer;
	u32 lastMip = range.baseMipLevel + range.levelCount - 1;
	u32 lastFace = range.baseArrayLayer + range.layerCount - 1;

	VkImageMemoryBarrier defaultBarrier;
	defaultBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	defaultBarrier.pNext = nullptr;
	defaultBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	defaultBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	defaultBarrier.image = GetVulkanHandle();
	defaultBarrier.subresourceRange.aspectMask = range.aspectMask;
	defaultBarrier.subresourceRange.layerCount = 1;
	defaultBarrier.subresourceRange.levelCount = 1;
	defaultBarrier.subresourceRange.baseArrayLayer = 0;
	defaultBarrier.subresourceRange.baseMipLevel = 0;

	auto addNewBarrier = [&](VulkanImageSubresource* subresource, u32 face, u32 mip)
	{
		barriers.Add(defaultBarrier);
		VkImageMemoryBarrier* barrier = &barriers.Back();

		barrier->subresourceRange.baseArrayLayer = face;
		barrier->subresourceRange.baseMipLevel = mip;
		barrier->srcAccessMask = GetAccessFlags(subresource->GetLayout());
		barrier->oldLayout = subresource->GetLayout();

		return barrier;
	};

	B3DMarkAllocatorFrame();
	{
		const u32 totalSubresourceCount = (range.baseMipLevel + range.levelCount) * (range.baseArrayLayer + range.layerCount);
		FrameVector<bool> processed(totalSubresourceCount, false);

		u32 subresourceCount = range.levelCount * range.layerCount;

		// Add first subresource
		VulkanImageSubresource* subresource = GetSubresource(face, mipLevel);
		addNewBarrier(subresource, face, mipLevel);
		subresourceCount--;
		processed[0] = true;

		while(subresourceCount > 0)
		{
			// Try to expand the barrier as much as possible
			VkImageMemoryBarrier* barrier = &barriers.back();

			while(true)
			{
				// Expand by one in the X direction
				bool expandedFace = true;
				if(face < lastFace)
				{
					for(u32 i = 0; i < barrier->subresourceRange.levelCount; i++)
					{
						const u32 currentMipLevel = barrier->subresourceRange.baseMipLevel + i;
						const u32 currentFace = face + 1;
						const u32 sequentialIndex = currentMipLevel * range.layerCount + (currentFace - range.baseArrayLayer);

						VulkanImageSubresource* currentSubresource = GetSubresource(currentFace, currentMipLevel);
						if (processed[sequentialIndex] || barrier->oldLayout != currentSubresource->GetLayout()) {
							expandedFace = false;
							break;
						}
					}

					if(expandedFace)
					{
						barrier->subresourceRange.layerCount++;

						B3D_ASSERT(subresourceCount >= barrier->subresourceRange.levelCount);
						subresourceCount -= barrier->subresourceRange.levelCount;
						face++;

						for(u32 i = 0; i < barrier->subresourceRange.levelCount; i++)
						{
							u32 curMip = (barrier->subresourceRange.baseMipLevel + i) - range.baseMipLevel;
							u32 idx = curMip * range.layerCount + (face - range.baseArrayLayer);
							processed[idx] = true;
						}
					}
				}
				else
					expandedFace = false;

				// Expand by one in the Y direction
				bool expandedMip = true;
				if(mipLevel < lastMip)
				{
					for(u32 i = 0; i < barrier->subresourceRange.layerCount; i++)
					{
						const u32 currentMipLevel = mipLevel + 1;
						const u32 currentFace = barrier->subresourceRange.baseArrayLayer + i;
						const u32 sequentialIndex = currentMipLevel * range.layerCount + (currentFace - range.baseArrayLayer);

						VulkanImageSubresource* currentSubresource = GetSubresource(currentFace, currentMipLevel);
						if (processed[sequentialIndex] || barrier->oldLayout != currentSubresource->GetLayout()) {
							expandedMip = false;
							break;
						}
					}

					if(expandedMip)
					{
						barrier->subresourceRange.levelCount++;

						B3D_ASSERT(subresourceCount >= barrier->subresourceRange.layerCount);
						subresourceCount -= barrier->subresourceRange.layerCount;
						mipLevel++;

						for(u32 i = 0; i < barrier->subresourceRange.layerCount; i++)
						{
							u32 curFace = (barrier->subresourceRange.baseArrayLayer + i) - range.baseArrayLayer;
							u32 idx = (mipLevel - range.baseMipLevel) * range.layerCount + curFace;
							processed[idx] = true;
						}
					}
				}
				else
					expandedMip = false;

				// If we can't grow no more, we're done with this square
				if(!expandedMip && !expandedFace)
					break;
			}

			// Look for a new starting point (sub-resource we haven't processed yet)
			for(u32 i = 0; i < range.levelCount; i++)
			{
				bool found = false;
				for(u32 j = 0; j < range.layerCount; j++)
				{
					u32 idx = i * range.layerCount + j;
					if(!processed[idx])
					{
						mipLevel = range.baseMipLevel + i;
						face = range.baseArrayLayer + j;

						found = true;
						processed[idx] = true;
						break;
					}
				}

				if(found)
				{
					VulkanImageSubresource* subresource = GetSubresource(face, mipLevel);
					addNewBarrier(subresource, face, mipLevel);

					B3D_ASSERT(subresourceCount > 0);
					subresourceCount--;
					break;
				}
			}
		}
	}
	B3DClearAllocatorFrame();
}

VulkanImageSubresource::VulkanImageSubresource(VulkanResourceManager* owner, VkImageLayout layout, const StringView& name)
	: VulkanResource(owner, false, name), mLayout(layout)
{}

VulkanTexture::VulkanTexture(VulkanGpuDevice& gpuDevice, const TextureCreateInformation& createInformation)
	: Texture(createInformation), mGpuDevice(gpuDevice), mDirectlyMappable(false), mSupportsGPUWrites(false)
{
}

VulkanTexture::~VulkanTexture()
{
	if (mImage != nullptr)
		mImage->Destroy();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Texture);
}

void VulkanTexture::Initialize()
{
	ASSERT_IF_NOT_RENDER_THREAD;

	const TextureProperties& props = mProperties;

	mImageCreateInformation.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	mImageCreateInformation.pNext = nullptr;
	mImageCreateInformation.flags = 0;

	TextureType texType = props.Type;
	switch(texType)
	{
	case TEX_TYPE_1D:
		mImageCreateInformation.imageType = VK_IMAGE_TYPE_1D;
		break;
	case TEX_TYPE_2D:
		mImageCreateInformation.imageType = VK_IMAGE_TYPE_2D;
		break;
	case TEX_TYPE_3D:
		mImageCreateInformation.imageType = VK_IMAGE_TYPE_3D;

		if(mProperties.Usage.IsSet(TextureUsageFlag::RenderTarget))
			mImageCreateInformation.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;

		break;
	case TEX_TYPE_CUBE_MAP:
		mImageCreateInformation.imageType = VK_IMAGE_TYPE_2D;
		mImageCreateInformation.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		break;
	}

	// Note: I force rendertarget and depthstencil types to be readable in shader. Depending on performance impact
	// it might be beneficial to allow the user to enable this explicitly only when needed.

	mImageCreateInformation.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	TextureUsageFlags usage = props.Usage;
	if(usage.IsSet(TextureUsageFlag::RenderTarget))
	{
		mImageCreateInformation.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		mSupportsGPUWrites = true;
	}
	else if(usage.IsSet(TextureUsageFlag::DepthStencil))
	{
		mImageCreateInformation.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		mSupportsGPUWrites = true;
	}

	if(usage.IsSet(TextureUsageFlag::AllowUnorderedAccessOnTheGPU))
	{
		mImageCreateInformation.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		mSupportsGPUWrites = true;
	}

	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
	if(usage.IsSet(TextureUsageFlag::StoreOnCPUWithGPUAccess)) // Attempt to use linear tiling for dynamic textures, so we can directly map and modify them
	{
		// Only support 2D textures, with one sample and one mip level, only used for shader reads
		// (Optionally check vkGetPhysicalDeviceFormatProperties & vkGetPhysicalDeviceImageFormatProperties for
		// additional supported configs, but right now there doesn't seem to be any additional support)
		if(texType == TEX_TYPE_2D && props.SampleCount <= 1 && props.MipMapCount == 0 &&
		   props.GetFaceCount() == 1 && (mImageCreateInformation.usage & VK_IMAGE_USAGE_SAMPLED_BIT) != 0)
		{
			// Also, only support normal textures, not render targets or storage textures
			if(!mSupportsGPUWrites)
			{
				mDirectlyMappable = true;
				tiling = VK_IMAGE_TILING_LINEAR;
				layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
			}
		}
	}

	if(usage.IsSet(TextureUsageFlag::MutableFormat))
		mImageCreateInformation.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

	u32 width = mProperties.Width;
	u32 height = mProperties.Height;
	u32 depth = mProperties.Depth;

	// 0-sized textures aren't supported by the API
	width = std::max(width, 1U);
	height = std::max(height, 1U);
	depth = std::max(depth, 1U);

	mImageCreateInformation.extent = { width, height, depth };
	mImageCreateInformation.mipLevels = props.MipMapCount + 1;
	mImageCreateInformation.arrayLayers = props.GetFaceCount();
	mImageCreateInformation.samples = VulkanUtility::GetSampleFlags(props.SampleCount);
	mImageCreateInformation.tiling = tiling;
	mImageCreateInformation.initialLayout = layout;
	mImageCreateInformation.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	mImageCreateInformation.queueFamilyIndexCount = 0;
	mImageCreateInformation.pQueueFamilyIndices = nullptr;

	bool optimalTiling = tiling == VK_IMAGE_TILING_OPTIMAL;

	mInternalFormat = VulkanUtility::GetClosestSupportedPixelFormat(mGpuDevice, props.Format, props.Type, props.Usage, optimalTiling, props.UseHardwareSRGB);
	mImage = CreateImage(mInternalFormat);
	mMappedMemory = mImage->GetMappedMemory();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_Texture);
	Texture::Initialize();
}

void VulkanTexture::SetName(const StringView& name)
{
	Texture::SetName(name);

	if(mImage != nullptr)
		mImage->SetName(name);
}

GpuQueueMask VulkanTexture::GetUseMask(u32 mipLevel, u32 arrayLayer, GpuAccessFlags accessFlags) const
{
	VulkanImageSubresource* subresource = mImage->GetSubresource(arrayLayer, mipLevel);
	return subresource->GetUseInfo(accessFlags);
}

u32 VulkanTexture::GetBoundCount(u32 mipLevel, u32 arrayLayer) const
{
	VulkanImageSubresource* subresource = mImage->GetSubresource(arrayLayer, mipLevel);
	return subresource->GetBoundCount();
}

u32 VulkanTexture::GetUseCount(u32 mipLevel, u32 arrayLayer) const
{
	VulkanImageSubresource* subresource = mImage->GetSubresource(arrayLayer, mipLevel);
	return subresource->GetUseCount();
}

void VulkanTexture::Flush(u32 mipLevel, u32 arrayLayer)
{
	if(mImage == nullptr || !mDirectlyMappable)
		return;

	VkSubresourceLayout layout = mImage->GetSubresourceLayout(arrayLayer, mipLevel);
	mImage->Flush(layout.offset, layout.size);
}

void VulkanTexture::Invalidate(u32 mipLevel, u32 arrayLayer)
{
	if(mImage == nullptr || !mDirectlyMappable)
		return;

	VkSubresourceLayout layout = mImage->GetSubresourceLayout(arrayLayer, mipLevel);
	mImage->Invalidate(layout.offset, layout.size);
}

VulkanImage* VulkanTexture::CreateImage(PixelFormat format)
{
	bool directlyMappable = mImageCreateInformation.tiling == VK_IMAGE_TILING_LINEAR;
	VmaMemoryUsage memoryUsage = directlyMappable ? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_GPU_ONLY;

	VkDevice vkDevice = mGpuDevice.GetLogical();

	mImageCreateInformation.format = VulkanUtility::GetPixelFormat(format, mProperties.UseHardwareSRGB);

	VkImage image;
	VkResult result = vkCreateImage(vkDevice, &mImageCreateInformation, gVulkanAllocator, &image);
	B3D_ASSERT(result == VK_SUCCESS);

	VulkanAllocationResult allocation = mGpuDevice.AllocateMemory(image, memoryUsage);

	VulkanImage *const vulkanImage = mGpuDevice.GetResourceManager().Create<VulkanImage>(image, allocation, mImageCreateInformation.initialLayout, mImageCreateInformation.format, GetProperties());
	vulkanImage->SetName(mName);

	return vulkanImage;
}

VulkanBuffer* VulkanTexture::CreateStaging(const PixelData& pixelData, bool readable) // TODO - Replace usages with TextureUtility::CreateStagingBuffer
{
	VkBufferCreateInfo bufferCI;
	bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCI.pNext = nullptr;
	bufferCI.flags = 0;
	bufferCI.size = pixelData.GetSize();
	bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCI.queueFamilyIndexCount = 0;
	bufferCI.pQueueFamilyIndices = nullptr;

	if(readable)
		bufferCI.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VkDevice vkDevice = mGpuDevice.GetLogical();

	VkBuffer buffer;
	VkResult result = vkCreateBuffer(vkDevice, &bufferCI, gVulkanAllocator, &buffer);
	B3D_ASSERT(result == VK_SUCCESS);

	VulkanAllocationResult allocation = mGpuDevice.AllocateMemory(buffer, VMA_MEMORY_USAGE_CPU_ONLY);

	VulkanBuffer *const vulkanBuffer = mGpuDevice.GetResourceManager().Create<VulkanBuffer>(readable ? GpuBufferType::StagingRead : GpuBufferType::StagingWrite, (GpuBufferFlags)0, buffer, allocation);
	vulkanBuffer->SetName(StringUtil::Format("Staging buffer ({0})", mName));

	return vulkanBuffer;
}

void VulkanTexture::CopyImageToImage(VulkanGpuCommandBuffer& commandBuffer, VulkanImage* sourceImage, VulkanImage* destinationImage)
{
	const u32 faceCount = mProperties.GetFaceCount();
	const u32 mipCount = mProperties.MipMapCount + 1;

	u32 mipWidth = mProperties.Width;
	u32 mipHeight = mProperties.Height;
	u32 mipDepth = mProperties.Depth;

	VkImageCopy *const imageRegions = B3DStackAllocate<VkImageCopy>(mipCount);

	for(u32 i = 0; i < mipCount; i++)
	{
		VkImageCopy& imageRegion = imageRegions[i];

		imageRegion.srcOffset = { 0, 0, 0 };
		imageRegion.dstOffset = { 0, 0, 0 };
		imageRegion.extent = { mipWidth, mipHeight, mipDepth };
		imageRegion.srcSubresource.baseArrayLayer = 0;
		imageRegion.srcSubresource.layerCount = faceCount;
		imageRegion.srcSubresource.mipLevel = i;
		imageRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageRegion.dstSubresource.baseArrayLayer = 0;
		imageRegion.dstSubresource.layerCount = faceCount;
		imageRegion.dstSubresource.mipLevel = i;
		imageRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		if(mipWidth != 1) mipWidth /= 2;
		if(mipHeight != 1) mipHeight /= 2;
		if(mipDepth != 1) mipDepth /= 2;
	}

	VkImageSubresourceRange range;
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseArrayLayer = 0;
	range.layerCount = faceCount;
	range.baseMipLevel = 0;
	range.levelCount = mipCount;

	VkImageLayout transferSourceLayout, transferDestinationLayout;
	if(mDirectlyMappable)
	{
		transferSourceLayout = VK_IMAGE_LAYOUT_GENERAL;
		transferDestinationLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	else
	{
		transferSourceLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		transferDestinationLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}

	commandBuffer.CopyImageToImage(sourceImage, destinationImage, transferSourceLayout, transferDestinationLayout, range, range, mipCount, imageRegions);

	B3DStackFree(imageRegions);
}

ImageSubresourcePitch VulkanTexture::GetPitchForSubresource(u32 face, u32 mipLevel) const
{
	VkSubresourceLayout subresourceLayout;
	if(mDirectlyMappable && mImage != nullptr)
	{
		subresourceLayout = mImage->GetSubresourceLayout(face, mipLevel);
	}
	else
	{
		u32 mipWidth, mipHeight, mipDepth;
		PixelUtility::GetSizeForMipLevel(mProperties.Width, mProperties.Height, mProperties.Depth, mipLevel, mipWidth, mipHeight, mipDepth);

		u32 rowPitch, depthPitch;
		PixelUtility::GetPitch(mipWidth, mipHeight, mipDepth, mProperties.Format, rowPitch, depthPitch);

		subresourceLayout.rowPitch = rowPitch;
		subresourceLayout.depthPitch = depthPitch;
	}

	return VulkanImage::ConvertSubresourceLayoutToBlocks(subresourceLayout, mProperties.Format);
}

void VulkanTexture::RecreateInternalTexture()
{
	VulkanImage* const newImage = CreateImage(mInternalFormat);
	mImage->Destroy();
	mImage = newImage;
	mMappedMemory = mImage->GetMappedMemory();
}

void VulkanTexture::CopyImageSubresourceToBuffer(VulkanGpuCommandBuffer& commandBuffer, VulkanImage* sourceImage, u32 sourceFace, u32 sourceMipLevel, VulkanBuffer* destinationBuffer)
{
	VkExtent3D extent;
	PixelUtility::GetSizeForMipLevel(mProperties.Width, mProperties.Height, mProperties.Depth, sourceMipLevel, extent.width, extent.height, extent.depth);

	const ImageSubresourcePitch pitch = GetPitchForSubresource(sourceFace, sourceMipLevel);

	VkImageSubresourceRange subresourceRange;
	subresourceRange.baseArrayLayer = sourceFace;
	subresourceRange.layerCount = 1;
	subresourceRange.baseMipLevel = sourceMipLevel;
	subresourceRange.levelCount = 1;

	if(mProperties.Usage.IsSet(TextureUsageFlag::DepthStencil))
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	else
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	commandBuffer.CopyImageToBuffer(sourceImage, destinationBuffer, extent, subresourceRange, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pitch.RowPitch, pitch.SliceHeight);
}

render::GpuTextureMappedScope VulkanTexture::Map(u32 mipLevel, u32 arrayLayer, GpuMapOptions options)
{
	ASSERT_IF_NOT_RENDER_THREAD;

	if(mipLevel > mProperties.MipMapCount)
	{
		B3D_LOG(Error, LogTexture, "Invalid mip level: {0}. Min is 0, max is {1}", mipLevel, mProperties.MipMapCount);
		return GpuTextureMappedScope();
	}

	if(arrayLayer >= mProperties.GetFaceCount())
	{
		B3D_LOG(Error, LogTexture, "Invalid face index: {0}. Min is 0, max is {1}", mipLevel, mProperties.GetFaceCount());
		return GpuTextureMappedScope();
	}

	if(mImage == nullptr || !mDirectlyMappable || mMappedMemory == nullptr)
		return GpuTextureMappedScope();

	const TextureProperties& props = GetProperties();

	if(props.SampleCount > 1)
	{
		B3D_LOG(Error, LogRenderBackend, "Multisampled textures cannot be accessed from the CPU directly.");
		return GpuTextureMappedScope();
	}

#if B3D_PROFILING_ENABLED
	if(options.IsSet(GpuMapOption::Read))
		B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResRead, RenderStatObject_Texture);

	if(options.IsSet(GpuMapOption::Write))
		B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResWrite, RenderStatObject_Texture);
#endif

	VulkanImageSubresource* const subresource = mImage->GetSubresource(arrayLayer, mipLevel);

	// Initially the texture will be in preinitialized layout, and it will transition to general layout on first
	// use in shader. No further transitions are allowed for directly mappable textures.
	B3D_ASSERT(subresource->GetLayout() == VK_IMAGE_LAYOUT_PREINITIALIZED || subresource->GetLayout() == VK_IMAGE_LAYOUT_GENERAL);

	// GPU should never be allowed to write to a directly mappable texture, since only linear tiling is supported
	// for direct mapping, and we don't support using it with either storage textures or render targets.
	B3D_ASSERT(!mSupportsGPUWrites);

	const bool isReadRequired = options.IsSet(GpuMapOption::Read);
	const bool isWriteRequired = options.IsSet(GpuMapOption::Write);

	// Check GPU usage unless NoOverwrite is set
	if(!options.IsSet(GpuMapOption::NoOverwrite))
	{
		const GpuAccessFlags accessFlags = (isReadRequired ? GpuAccessFlag::Read : GpuAccessFlags()) |
		                                   (isWriteRequired ? GpuAccessFlag::Write : GpuAccessFlags());
		const GpuQueueMask useMask = subresource->GetUseInfo(accessFlags);

		if(!useMask.IsEmpty())
		{
			B3D_LOG(Error, LogRenderBackend, "Cannot map texture '{0}': subresource is currently being used by the GPU.", mName);
			return GpuTextureMappedScope();
		}
	}

	// Calculate mip dimensions
	const u32 mipWidth = std::max(1u, props.Width >> mipLevel);
	const u32 mipHeight = std::max(1u, props.Height >> mipLevel);
	const u32 mipDepth = std::max(1u, props.Depth >> mipLevel);

	// Get subresource layout and apply pitches
	VkSubresourceLayout layout = mImage->GetSubresourceLayout(arrayLayer, mipLevel);

	PixelData pixelData(mipWidth, mipHeight, mipDepth, mInternalFormat);
	VulkanImage::ApplyRowAndSlicePitch(layout, pixelData);

	// Invalidate memory if read is required (makes GPU writes visible to CPU)
	if(isReadRequired)
		mImage->Invalidate(layout.offset, layout.size);

	// Set external buffer to persistently mapped memory + offset
	pixelData.SetExternalBuffer(static_cast<u8*>(mMappedMemory) + layout.offset);

	return GpuTextureMappedScope(pixelData, std::static_pointer_cast<Texture>(GetShared()), GpuTextureSubresource(mipLevel, arrayLayer), options);
}
