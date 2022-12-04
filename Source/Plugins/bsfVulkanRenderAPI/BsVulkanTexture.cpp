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

using namespace bs;
using namespace bs::ct;

static VulkanImageCreateInformation BuildImageCreateInformation(VkImage image, VmaAllocation allocation, VkImageLayout layout, VkFormat actualFormat, const TextureProperties& props)
{
	VulkanImageCreateInformation desc;
	desc.Image = image;
	desc.Allocation = allocation;
	desc.Type = props.GetTextureType();
	desc.Format = actualFormat;
	desc.FaceCount = props.GetNumFaces();
	desc.DepthSliceCount = props.GetDepth();
	desc.MipLevelCount = props.GetNumMipmaps() + 1;
	desc.Layout = layout;
	desc.Usage = (u32)props.GetUsage();

	return desc;
}

VulkanImage::VulkanImage(VulkanResourceManager* owner, VkImage image, VmaAllocation allocation, VkImageLayout layout, VkFormat actualFormat, const TextureProperties& props, bool ownsImage, bool isShaderReadAllowed)
	: VulkanImage(owner, BuildImageCreateInformation(image, allocation, layout, actualFormat, props), ownsImage, isShaderReadAllowed)
{}

VulkanImage::VulkanImage(VulkanResourceManager* owner, const VulkanImageCreateInformation& desc, bool ownsImage, bool isShaderReadAllowed)
	: VulkanResource(owner, false), mImage(desc.Image), mAllocation(desc.Allocation), mFramebufferMainView(VK_NULL_HANDLE), mUsage(desc.Usage), mOwnsImage(ownsImage), mIsShaderReadAllowed(isShaderReadAllowed), mFaceCount(desc.FaceCount), mDepthSliceCount(desc.DepthSliceCount), mMipLevelCount(desc.MipLevelCount)
{
	mImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	mImageViewCI.pNext = nullptr;
	mImageViewCI.flags = 0;
	mImageViewCI.image = desc.Image;
	mImageViewCI.format = desc.Format;
	mImageViewCI.components = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};

	switch(desc.Type)
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
	if((mUsage & TU_DEPTHSTENCIL) != 0)
	{
		mFramebufferMainView = CreateView(completeSurface, desc.Format, GetAspectFlags(), true);
		mMainView = CreateView(completeSurface, desc.Format, VK_IMAGE_ASPECT_DEPTH_BIT, false);
	}
	else
	{
		// For 3D render attachments we also require a special view for framebuffer attachments
		if(mDepthSliceCount > 1 && (mUsage & TU_RENDERTARGET) != 0)
			mFramebufferMainView = CreateView(completeSurface, desc.Format, VK_IMAGE_ASPECT_COLOR_BIT, true);

		// For all other cases (non-framebuffer attachment, or a non-3D non-depth-stencil attachment) regular view will suffice.
		mMainView = CreateView(completeSurface, desc.Format, VK_IMAGE_ASPECT_COLOR_BIT, false);
	}

	const u32 subresourceCount = mFaceCount * mMipLevelCount;
	mSubresources = (VulkanImageSubresource**)B3DAllocate(sizeof(VulkanImageSubresource*) * subresourceCount);
	for(u32 i = 0; i < subresourceCount; i++)
		mSubresources[i] = owner->Create<VulkanImageSubresource>(desc.Layout);
}

VulkanImage::~VulkanImage()
{
	VulkanDevice& device = mOwner->GetDevice();
	VkDevice vkDevice = device.GetLogical();

	const u32 subresourceCount = mFaceCount * mMipLevelCount;
	for(u32 i = 0; i < subresourceCount; i++)
	{
		B3D_ASSERT(!mSubresources[i]->IsBound()); // Image beeing freed but its subresources are still bound somewhere

		mSubresources[i]->Destroy();
	}

	B3DFree(mSubresources);
	mSubresources = nullptr;

	for(auto& entry : mImageInfos)
		vkDestroyImageView(vkDevice, entry.View, gVulkanAllocator);

	if(mOwnsImage)
	{
		vkDestroyImage(vkDevice, mImage, gVulkanAllocator);
		device.FreeMemory(mAllocation);
	}
}

void VulkanImage::SetName(const StringView& name)
{
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

VkImageView VulkanImage::GetView(bool isPartOfFramebuffer) const
{
	if(isPartOfFramebuffer)
	{
		if((mUsage & TU_DEPTHSTENCIL) != 0)
			return mFramebufferMainView;

		if(mDepthSliceCount > 1 && (mUsage & TU_RENDERTARGET) != 0)
			return mFramebufferMainView;
	}

	return mMainView;
}

VkImageView VulkanImage::GetView(const TextureSurface& surface, bool isPartOfFramebuffer) const
{
	return GetView(mImageViewCI.format, surface, isPartOfFramebuffer);
}

VkImageView VulkanImage::GetView(VkFormat format, bool isPartOfFramebuffer) const
{
	TextureSurface completeSurface(0, mMipLevelCount, 0, mFaceCount);
	return GetView(format, completeSurface, isPartOfFramebuffer);
}

VkImageView VulkanImage::GetView(VkFormat format, const TextureSurface& surface, bool isPartOfFrameBuffer) const
{
	TextureSurface explicitSurface = surface;
	CalculateExplicitSurface(explicitSurface, isPartOfFrameBuffer);

	for(auto& entry : mImageInfos)
	{
		// Check if framebuffer field matches, but only if this is a depth-stencil framebuffer attachment or a 3D render texture attachment. For all other
		// cases we're free to use the regular view.
		const bool isFramebufferFieldMatching =
			isPartOfFrameBuffer == entry.IsPartOfFramebuffer ||
			((mUsage & TU_DEPTHSTENCIL) == 0 && ((mUsage & TU_RENDERTARGET) == 0 || mDepthSliceCount <= 1));

		if(explicitSurface == entry.Surface && format == entry.Format && isFramebufferFieldMatching)
			return entry.View;
	}

	VkImageView view = VK_NULL_HANDLE;
	if((mUsage & TU_DEPTHSTENCIL) != 0)
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

VkImageView VulkanImage::CreateView(const TextureSurface& surface, VkFormat format, VkImageAspectFlags aspectMask, bool isPartOfFramebuffer) const
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

	VkImageView view;
	VkResult result = vkCreateImageView(mOwner->GetDevice().GetLogical(), &mImageViewCI, gVulkanAllocator, &view);
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
	if((mUsage & TU_DEPTHSTENCIL) != 0)
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
	VulkanDevice& device = mOwner->GetDevice();

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
	const u32 blockSize = PixelUtil::GetBlockSize(format);

	B3D_ASSERT(subresourceLayout.rowPitch % blockSize == 0);
	B3D_ASSERT(subresourceLayout.depthPitch % blockSize == 0);

	u32 rowPitchInPixels = (u32)(subresourceLayout.rowPitch / blockSize);
	u32 depthPitch = (u32)(subresourceLayout.depthPitch / blockSize);

	if(PixelUtil::IsCompressed(format))
	{
		// For compressed formats, we return the pitch in blocks
		const Vector2I blockDimension = PixelUtil::GetBlockDimensions(format);
		rowPitchInPixels *= blockDimension.X;
		depthPitch *= blockDimension.X * blockDimension.Y;
	}

	return ImageSubresourcePitch(rowPitchInPixels, rowPitchInPixels != 0 ? depthPitch / rowPitchInPixels : 0);
}

void VulkanImage::Map(u32 face, u32 mipLevel, PixelData& output, bool isInvalidateRequired) const
{
	VulkanDevice& device = mOwner->GetDevice();

	VkImageSubresource range;
	range.mipLevel = mipLevel;
	range.arrayLayer = face;

	if(mImageViewCI.subresourceRange.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT)
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	else // Depth stencil, but we only map depth
		range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	VkSubresourceLayout layout;
	vkGetImageSubresourceLayout(device.GetLogical(), mImage, &range, &layout);

	output.SetRowPitch((u32)layout.rowPitch);
	output.SetSlicePitch((u32)layout.depthPitch);

	u8 *const data = Map(layout.offset, layout.size, isInvalidateRequired);
	output.SetExternalBuffer(data);
}

u8* VulkanImage::Map(VkDeviceSize offset, VkDeviceSize size, bool isInvalidateRequired) const
{
	VulkanDevice& device = mOwner->GetDevice();

	if(isInvalidateRequired)
		device.InvalidateMemory(mAllocation, offset, size);

	void* data = device.MapMemory(mAllocation);

	mMappedOffset = offset;
	mMappedSize = size;

	return (u8*)data + offset;
}

void VulkanImage::Unmap(bool isFlushRequired)
{
	VulkanDevice& device = mOwner->GetDevice();

	device.UnmapMemory(mAllocation);

	if(isFlushRequired)
		device.FlushMemory(mAllocation, mMappedOffset, mMappedSize);
}

VkAccessFlags VulkanImage::GetAccessFlags(VkImageLayout layout, bool readOnly)
{
	VkAccessFlags accessFlags;

	switch(layout)
	{
	case VK_IMAGE_LAYOUT_GENERAL:
		{
			accessFlags = VK_ACCESS_SHADER_READ_BIT;
			if((mUsage & TU_LOADSTORE) != 0)
			{
				if(!readOnly)
					accessFlags |= VK_ACCESS_SHADER_WRITE_BIT;
			}

			if((mUsage & TU_RENDERTARGET) != 0)
			{
				accessFlags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

				if(!readOnly)
					accessFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}
			else if((mUsage & TU_DEPTHSTENCIL) != 0)
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
		B3D_LOG(Warning, RenderBackend, "Unsupported source layout for Vulkan image.");
		break;
	}

	return accessFlags;
}

void VulkanImage::GetBarriers(const VkImageSubresourceRange& range, Vector<VkImageMemoryBarrier>& barriers)
{
	u32 numSubresources = range.levelCount * range.layerCount;

	// Nothing to do
	if(numSubresources == 0)
		return;

	u32 mip = range.baseMipLevel;
	u32 face = range.baseArrayLayer;
	u32 lastMip = range.baseMipLevel + range.levelCount - 1;
	u32 lastFace = range.baseArrayLayer + range.layerCount - 1;

	VkImageMemoryBarrier defaultBarrier;
	defaultBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	defaultBarrier.pNext = nullptr;
	defaultBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	defaultBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	defaultBarrier.image = GetHandle();
	defaultBarrier.subresourceRange.aspectMask = range.aspectMask;
	defaultBarrier.subresourceRange.layerCount = 1;
	defaultBarrier.subresourceRange.levelCount = 1;
	defaultBarrier.subresourceRange.baseArrayLayer = 0;
	defaultBarrier.subresourceRange.baseMipLevel = 0;

	auto addNewBarrier = [&](VulkanImageSubresource* subresource, u32 face, u32 mip)
	{
		barriers.push_back(defaultBarrier);
		VkImageMemoryBarrier* barrier = &barriers.back();

		barrier->subresourceRange.baseArrayLayer = face;
		barrier->subresourceRange.baseMipLevel = mip;
		barrier->srcAccessMask = GetAccessFlags(subresource->GetLayout());
		barrier->oldLayout = subresource->GetLayout();

		return barrier;
	};

	B3DMarkAllocatorFrame();
	{
		FrameVector<bool> processed(numSubresources, false);

		// Add first subresource
		VulkanImageSubresource* subresource = GetSubresource(face, mip);
		addNewBarrier(subresource, face, mip);
		numSubresources--;
		processed[0] = true;

		while(numSubresources > 0)
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
						u32 curMip = barrier->subresourceRange.baseMipLevel + i;
						VulkanImageSubresource* subresource = GetSubresource(face + 1, curMip);
						if(barrier->oldLayout != subresource->GetLayout())
						{
							expandedFace = false;
							break;
						}
					}

					if(expandedFace)
					{
						barrier->subresourceRange.layerCount++;
						numSubresources -= barrier->subresourceRange.levelCount;
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
				if(mip < lastMip)
				{
					for(u32 i = 0; i < barrier->subresourceRange.layerCount; i++)
					{
						u32 curFace = barrier->subresourceRange.baseArrayLayer + i;
						VulkanImageSubresource* subresource = GetSubresource(curFace, mip + 1);
						if(barrier->oldLayout != subresource->GetLayout())
						{
							expandedMip = false;
							break;
						}
					}

					if(expandedMip)
					{
						barrier->subresourceRange.levelCount++;
						numSubresources -= barrier->subresourceRange.layerCount;
						mip++;

						for(u32 i = 0; i < barrier->subresourceRange.layerCount; i++)
						{
							u32 curFace = (barrier->subresourceRange.baseArrayLayer + i) - range.baseArrayLayer;
							u32 idx = (mip - range.baseMipLevel) * range.layerCount + curFace;
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
						mip = range.baseMipLevel + i;
						face = range.baseArrayLayer + j;

						found = true;
						processed[idx] = true;
						break;
					}
				}

				if(found)
				{
					VulkanImageSubresource* subresource = GetSubresource(face, mip);
					addNewBarrier(subresource, face, mip);
					numSubresources--;
					break;
				}
			}
		}
	}
	B3DClearAllocatorFrame();
}

VulkanImageSubresource::VulkanImageSubresource(VulkanResourceManager* owner, VkImageLayout layout)
	: VulkanResource(owner, false), mLayout(layout)
{}

VulkanTexture::VulkanTexture(const TextureCreateInformation& createInformation, const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask)
	: Texture(createInformation, initialData, deviceMask), mImages(), mInternalFormats(), mDeviceMask(deviceMask), mStagingBuffer(nullptr), mMappedDeviceIdx((u32)-1), mMappedGlobalQueueIdx((u32)-1), mMappedMip(0), mMappedFace(0), mMappedRowPitch(0), mMappedSlicePitch(0), mMappedLockOptions(GBL_WRITE_ONLY), mDirectlyMappable(false), mSupportsGPUWrites(false), mIsMapped(false)
{
}

VulkanTexture::~VulkanTexture()
{
	for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		if(mImages[i] == nullptr)
			return;

		mImages[i]->Destroy();
	}

	B3D_ASSERT(mStagingBuffer == nullptr);

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Texture);
}

void VulkanTexture::Initialize()
{
	THROW_IF_NOT_CORE_THREAD;

	const TextureProperties& props = mProperties;

	mImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	mImageCI.pNext = nullptr;
	mImageCI.flags = 0;

	TextureType texType = props.GetTextureType();
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

		if((mProperties.GetUsage() & TU_RENDERTARGET) != 0)
			mImageCI.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;

		break;
	case TEX_TYPE_CUBE_MAP:
		mImageCI.imageType = VK_IMAGE_TYPE_2D;
		mImageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		break;
	}

	// Note: I force rendertarget and depthstencil types to be readable in shader. Depending on performance impact
	// it might be beneficial to allow the user to enable this explicitly only when needed.

	mImageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	int usage = props.GetUsage();
	if((usage & TU_RENDERTARGET) != 0)
	{
		mImageCI.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		mSupportsGPUWrites = true;
	}
	else if((usage & TU_DEPTHSTENCIL) != 0)
	{
		mImageCI.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		mSupportsGPUWrites = true;
	}

	if((usage & TU_LOADSTORE) != 0)
	{
		mImageCI.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		mSupportsGPUWrites = true;
	}

	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
	if((usage & TU_DYNAMIC) != 0) // Attempt to use linear tiling for dynamic textures, so we can directly map and modify them
	{
		// Only support 2D textures, with one sample and one mip level, only used for shader reads
		// (Optionally check vkGetPhysicalDeviceFormatProperties & vkGetPhysicalDeviceImageFormatProperties for
		// additional supported configs, but right now there doesn't seem to be any additional support)
		if(texType == TEX_TYPE_2D && props.GetNumSamples() <= 1 && props.GetNumMipmaps() == 0 &&
		   props.GetNumFaces() == 1 && (mImageCI.usage & VK_IMAGE_USAGE_SAMPLED_BIT) != 0)
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

	if((usage & TU_MUTABLEFORMAT) != 0)
		mImageCI.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

	u32 width = mProperties.GetWidth();
	u32 height = mProperties.GetHeight();
	u32 depth = mProperties.GetDepth();

	// 0-sized textures aren't supported by the API
	width = std::max(width, 1U);
	height = std::max(height, 1U);
	depth = std::max(depth, 1U);

	mImageCI.extent = { width, height, depth };
	mImageCI.mipLevels = props.GetNumMipmaps() + 1;
	mImageCI.arrayLayers = props.GetNumFaces();
	mImageCI.samples = VulkanUtility::GetSampleFlags(props.GetNumSamples());
	mImageCI.tiling = tiling;
	mImageCI.initialLayout = layout;
	mImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	mImageCI.queueFamilyIndexCount = 0;
	mImageCI.pQueueFamilyIndices = nullptr;

	VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
	VulkanDevice* devices[B3D_MAX_DEVICES];
	VulkanUtility::GetDevices(rapi, mDeviceMask, devices);

	// Allocate buffers per-device
	for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		if(devices[i] == nullptr)
			continue;

		bool optimalTiling = tiling == VK_IMAGE_TILING_OPTIMAL;

		mInternalFormats[i] = VulkanUtility::GetClosestSupportedPixelFormat(
			*devices[i], props.GetFormat(), props.GetTextureType(), props.GetUsage(), optimalTiling,
			props.IsHardwareGammaEnabled());

		mImages[i] = CreateImage(*devices[i], mInternalFormats[i]);
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_Texture);
	Texture::Initialize();
}

void VulkanTexture::SetName(const StringView& name)
{
	Texture::SetName(name);

	for(UINT32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		if(mImages[i] == nullptr)
			continue;

		mImages[i]->SetName(name);
	}
}

VulkanImage* VulkanTexture::CreateImage(VulkanDevice& device, PixelFormat format)
{
	bool directlyMappable = mImageCI.tiling == VK_IMAGE_TILING_LINEAR;
	VmaMemoryUsage memoryUsage = directlyMappable ? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_GPU_ONLY;

	VkDevice vkDevice = device.GetLogical();

	mImageCI.format = VulkanUtility::GetPixelFormat(format, mProperties.IsHardwareGammaEnabled());
	;

	VkImage image;
	VkResult result = vkCreateImage(vkDevice, &mImageCI, gVulkanAllocator, &image);
	B3D_ASSERT(result == VK_SUCCESS);

	VmaAllocation allocation = device.AllocateMemory(image, memoryUsage);
	VulkanImage *const vulkanImage = device.GetResourceManager().Create<VulkanImage>(image, allocation, mImageCI.initialLayout, mImageCI.format, GetProperties());
	vulkanImage->SetName(mName);

	return vulkanImage;
}

VulkanBuffer* VulkanTexture::CreateStaging(VulkanDevice& device, const PixelData& pixelData, bool readable)
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

	VkDevice vkDevice = device.GetLogical();

	VkBuffer buffer;
	VkResult result = vkCreateBuffer(vkDevice, &bufferCI, gVulkanAllocator, &buffer);
	B3D_ASSERT(result == VK_SUCCESS);

	VmaAllocation allocation = device.AllocateMemory(buffer, VMA_MEMORY_USAGE_CPU_ONLY);

	u32 blockSize = PixelUtil::GetBlockSize(pixelData.GetFormat());

	B3D_ASSERT(pixelData.GetRowPitch() % blockSize == 0);
	B3D_ASSERT(pixelData.GetSlicePitch() % blockSize == 0);

	u32 rowPitchInPixels = pixelData.GetRowPitch() / blockSize;
	u32 slicePitchInPixels = pixelData.GetSlicePitch() / blockSize;

	if(PixelUtil::IsCompressed(pixelData.GetFormat()))
	{
		Vector2I blockDim = PixelUtil::GetBlockDimensions(pixelData.GetFormat());
		rowPitchInPixels *= blockDim.X;
		slicePitchInPixels *= blockDim.X * blockDim.Y;
	}

	VulkanBuffer *const vulkanBuffer = device.GetResourceManager().Create<VulkanBuffer>(buffer, allocation, rowPitchInPixels, slicePitchInPixels);
	vulkanBuffer->SetName(StringUtil::Format("Staging buffer ({0})", mName));

	return vulkanBuffer;
}

void VulkanTexture::CopyImageToImage(VulkanCmdBuffer* commandBuffer, VulkanImage* sourceImage, VulkanImage* destinationImage)
{
	const u32 faceCount = mProperties.GetNumFaces();
	const u32 mipCount = mProperties.GetNumMipmaps() + 1;

	u32 mipWidth = mProperties.GetWidth();
	u32 mipHeight = mProperties.GetHeight();
	u32 mipDepth = mProperties.GetDepth();

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

	commandBuffer->CopyImageToImage(sourceImage, destinationImage, transferSourceLayout, transferDestinationLayout, range, range, mipCount, imageRegions);

	B3DStackFree(imageRegions);
}

ImageSubresourcePitch VulkanTexture::GetPitchForSubresource(VulkanImage* image, u32 face, u32 mipLevel) const
{
	VkSubresourceLayout subresourceLayout;
	if(mDirectlyMappable)
	{
		subresourceLayout = image->GetSubresourceLayout(face, mipLevel);
	}
	else
	{
		u32 mipWidth, mipHeight, mipDepth;
		PixelUtil::GetSizeForMipLevel(mProperties.GetWidth(), mProperties.GetHeight(), mProperties.GetDepth(), mipLevel, mipWidth, mipHeight, mipDepth);

		u32 rowPitch, depthPitch;
		PixelUtil::GetPitch(mipWidth, mipHeight, mipDepth, mProperties.GetFormat(), rowPitch, depthPitch);

		subresourceLayout.rowPitch = rowPitch;
		subresourceLayout.depthPitch = depthPitch;
	}

	return VulkanImage::ConvertSubresourceLayoutToBlocks(subresourceLayout, mProperties.GetFormat());
}

void VulkanTexture::CopyImageSubresourceToBuffer(VulkanCmdBuffer* commandBuffer, VulkanImage* sourceImage, u32 sourceFace, u32 sourceMipLevel, VulkanBuffer* destinationBuffer, bool isBufferReadOnly)
{
	VkExtent3D extent;
	PixelUtil::GetSizeForMipLevel(mProperties.GetWidth(), mProperties.GetHeight(), mProperties.GetDepth(), sourceMipLevel, extent.width, extent.height, extent.depth);

	const ImageSubresourcePitch pitch = GetPitchForSubresource(sourceImage, sourceFace, sourceMipLevel);

	VkImageSubresourceRange subresourceRange;
	subresourceRange.baseArrayLayer = sourceFace;
	subresourceRange.layerCount = 1;
	subresourceRange.baseMipLevel = sourceMipLevel;
	subresourceRange.levelCount = 1;

	if((mProperties.GetUsage() & TU_DEPTHSTENCIL) != 0)
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	else
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	commandBuffer->CopyImageToBuffer(sourceImage, destinationBuffer, extent, subresourceRange, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pitch.RowPitch, pitch.SliceHeight);

	const VkAccessFlags stagingAccessFlags = VK_ACCESS_HOST_READ_BIT | (isBufferReadOnly ? 0 : VK_ACCESS_HOST_WRITE_BIT);
	commandBuffer->MemoryBarrier(destinationBuffer->GetHandle(), VK_ACCESS_TRANSFER_WRITE_BIT, stagingAccessFlags, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT);
}

void VulkanTexture::CopyImpl(const SPtr<Texture>& target, const TextureCopyInformation& copyInformation, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanTexture* other = static_cast<VulkanTexture*>(target.get());

	const TextureProperties& srcProps = mProperties;
	const TextureProperties& dstProps = other->GetProperties();

	bool srcHasMultisample = srcProps.GetNumSamples() > 1;
	bool destHasMultisample = dstProps.GetNumSamples() > 1;

	if((srcProps.GetUsage() & TU_DEPTHSTENCIL) != 0 || (dstProps.GetUsage() & TU_DEPTHSTENCIL) != 0)
	{
		B3D_LOG(Error, RenderBackend, "Texture copy/resolve isn't supported for depth-stencil textures.");
		return;
	}

	bool needsResolve = srcHasMultisample && !destHasMultisample;
	bool isMSCopy = srcHasMultisample || destHasMultisample;
	if(!needsResolve && isMSCopy)
	{
		if(srcProps.GetNumSamples() != dstProps.GetNumSamples())
		{
			B3D_LOG(Error, RenderBackend, "When copying textures their multisample counts must match. Ignoring copy.");
			return;
		}
	}

	VkImageLayout transferSourceLayout = mDirectlyMappable ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	VkImageLayout transferDestinationLayout = other->mDirectlyMappable ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	u32 mipWidth, mipHeight, mipDepth;

	bool copyEntireSurface = copyInformation.SourceVolume.GetWidth() == 0 ||
		copyInformation.SourceVolume.GetHeight() == 0 ||
		copyInformation.SourceVolume.GetDepth() == 0;

	if(copyEntireSurface)
	{
		PixelUtil::GetSizeForMipLevel(
			srcProps.GetWidth(),
			srcProps.GetHeight(),
			srcProps.GetDepth(),
			copyInformation.SourceMip,
			mipWidth,
			mipHeight,
			mipDepth);
	}
	else
	{
		mipWidth = copyInformation.SourceVolume.GetWidth();
		mipHeight = copyInformation.SourceVolume.GetHeight();
		mipDepth = copyInformation.SourceVolume.GetDepth();
	}

	if(mipWidth == 0 || mipHeight == 0 || mipDepth == 0)
		return;

	VkImageSubresourceRange sourceRange;
	sourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	sourceRange.baseArrayLayer = copyInformation.SourceFace;
	sourceRange.layerCount = 1;
	sourceRange.baseMipLevel = copyInformation.SourceMip;
	sourceRange.levelCount = 1;

	VkImageSubresourceRange destinationRange;
	destinationRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	destinationRange.baseArrayLayer = copyInformation.DestinationFace;
	destinationRange.layerCount = 1;
	destinationRange.baseMipLevel = copyInformation.DestinationMip;
	destinationRange.levelCount = 1;

	VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());

	VulkanCmdBuffer* vkCB;
	if(commandBuffer != nullptr)
		vkCB = static_cast<VulkanCommandBuffer*>(commandBuffer.get())->GetInternal();
	else
		vkCB = rapi.GetMainVulkanCommandBuffer()->GetInternal();

	u32 deviceIdx = vkCB->GetDeviceIdx();

	VulkanImage* sourceImage = mImages[deviceIdx];
	VulkanImage* destinationImage = other->GetResource(deviceIdx);

	if(sourceImage == nullptr || destinationImage == nullptr)
		return;

	if(vkCB->IsInRenderPass())
		vkCB->EndRenderPass(true);

	if(srcHasMultisample && !destHasMultisample) // Resolving from MS to non-MS texture
	{
		VkImageResolve resolveRegion;
		resolveRegion.srcOffset = { (i32)copyInformation.SourceVolume.Left, (i32)copyInformation.SourceVolume.Top, (i32)copyInformation.SourceVolume.Front };
		resolveRegion.dstOffset = { copyInformation.DestinationPosition.X, copyInformation.DestinationPosition.Y, copyInformation.DestinationPosition.Z };
		resolveRegion.extent = { mipWidth, mipHeight, mipDepth };
		resolveRegion.srcSubresource.baseArrayLayer = copyInformation.SourceFace;
		resolveRegion.srcSubresource.layerCount = 1;
		resolveRegion.srcSubresource.mipLevel = copyInformation.SourceMip;
		resolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		resolveRegion.dstSubresource.baseArrayLayer = copyInformation.DestinationFace;
		resolveRegion.dstSubresource.layerCount = 1;
		resolveRegion.dstSubresource.mipLevel = copyInformation.DestinationMip;
		resolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		vkCB->Resolve(sourceImage, destinationImage, transferSourceLayout, transferDestinationLayout, sourceRange, destinationRange, 1, &resolveRegion);
	}
	else // Just a normal copy
	{
		VkImageCopy imageRegion;
		imageRegion.srcOffset = { (i32)copyInformation.SourceVolume.Left, (i32)copyInformation.SourceVolume.Top, (i32)copyInformation.SourceVolume.Front };
		imageRegion.dstOffset = { copyInformation.DestinationPosition.X, copyInformation.DestinationPosition.Y, copyInformation.DestinationPosition.Z };
		imageRegion.extent = { mipWidth, mipHeight, mipDepth };
		imageRegion.srcSubresource.baseArrayLayer = copyInformation.SourceFace;
		imageRegion.srcSubresource.layerCount = 1;
		imageRegion.srcSubresource.mipLevel = copyInformation.SourceMip;
		imageRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageRegion.dstSubresource.baseArrayLayer = copyInformation.DestinationFace;
		imageRegion.dstSubresource.layerCount = 1;
		imageRegion.dstSubresource.mipLevel = copyInformation.DestinationMip;
		imageRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		vkCB->CopyImageToImage(sourceImage, destinationImage, transferSourceLayout, transferDestinationLayout, sourceRange, destinationRange, 1, &imageRegion);
	}
}

PixelData VulkanTexture::LockImpl(GpuLockOptions options, u32 mipLevel, u32 face, u32 deviceIdx, u32 queueIdx)
{
	const TextureProperties& props = GetProperties();

	if(props.GetNumSamples() > 1)
	{
		B3D_LOG(Error, RenderBackend, "Multisampled textures cannot be accessed from the CPU directly.");
		return PixelData();
	}

#if B3D_PROFILING_ENABLED
	if(options == GBL_READ_ONLY || options == GBL_READ_WRITE)
	{
		B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResRead, RenderStatObject_Texture);
	}

	if(options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
	{
		B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResWrite, RenderStatObject_Texture);
	}
#endif

	u32 mipWidth = std::max(1u, props.GetWidth() >> mipLevel);
	u32 mipHeight = std::max(1u, props.GetHeight() >> mipLevel);
	u32 mipDepth = std::max(1u, props.GetDepth() >> mipLevel);

	PixelData lockedArea(mipWidth, mipHeight, mipDepth, mInternalFormats[deviceIdx]);

	VulkanImage* image = mImages[deviceIdx];

	if(image == nullptr)
		return PixelData();

	mIsMapped = true;
	mMappedDeviceIdx = deviceIdx;
	mMappedGlobalQueueIdx = queueIdx;
	mMappedFace = face;
	mMappedMip = mipLevel;
	mMappedLockOptions = options;

	VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
	VulkanDevice& device = *rapi.GetDevice(deviceIdx);

	VulkanCommandBufferManager& cbManager = GetVulkanCommandBufferManager();
	GpuQueueType queueType;
	u32 localQueueIdx = CommandSyncMask::GetQueueIdxAndType(queueIdx, queueType);

	VulkanImageSubresource* subresource = image->GetSubresource(face, mipLevel);

	// If memory is host visible try mapping it directly
	if(mDirectlyMappable)
	{
		// Initially the texture will be in preinitialized layout, and it will transition to general layout on first
		// use in shader. No further transitions are allowed for directly mappable textures.
		B3D_ASSERT(subresource->GetLayout() == VK_IMAGE_LAYOUT_PREINITIALIZED || subresource->GetLayout() == VK_IMAGE_LAYOUT_GENERAL);

		// GPU should never be allowed to write to a directly mappable texture, since only linear tiling is supported
		// for direct mapping, and we don't support using it with either storage textures or render targets.
		B3D_ASSERT(!mSupportsGPUWrites);

		// Check is the GPU currently reading from the image
		u32 useMask = subresource->GetUseInfo(VulkanAccessFlag::Read);
		bool isUsedOnGPU = useMask != 0;
		const bool isReadRequired = options == GBL_READ_ONLY || options == GBL_READ_WRITE;

		// We're safe to map directly since GPU isn't using the subresource
		if(!isUsedOnGPU)
		{
			// If some CB has an operation queued that will be using the current contents of the image, create a new
			// image so we don't modify the previous use of the image
			if(subresource->IsBound())
			{
				VulkanImage* newImage = CreateImage(device, mInternalFormats[deviceIdx]);

				// Copy contents of the current image to the new one, unless caller explicitly specifies he doesn't
				// care about the current contents
				if(options != GBL_WRITE_ONLY_DISCARD)
				{
					VkMemoryRequirements memReqs;
					vkGetImageMemoryRequirements(device.GetLogical(), image->GetHandle(), &memReqs);

					u8* src = image->Map(0, (u32)memReqs.size, true);
					u8* dst = newImage->Map(0, (u32)memReqs.size);

					memcpy(dst, src, memReqs.size);

					image->Unmap();
					newImage->Unmap(true);
				}

				image->Destroy();
				image = newImage;
				mImages[deviceIdx] = image;
			}

			image->Map(face, mipLevel, lockedArea, isReadRequired);
			return lockedArea;
		}

		// Caller guarantees he won't touch the same data as the GPU, so just map even though the GPU is using the
		// subresource
		if(options == GBL_WRITE_ONLY_NO_OVERWRITE)
		{
			image->Map(face, mipLevel, lockedArea);
			return lockedArea;
		}

		// Caller doesn't care about buffer contents, so just discard the existing buffer and create a new one
		if(options == GBL_WRITE_ONLY_DISCARD)
		{
			// We need to discard the entire image, even though we're only writing to a single sub-resource
			image->Destroy();

			image = CreateImage(device, mInternalFormats[deviceIdx]);
			mImages[deviceIdx] = image;

			image->Map(face, mipLevel, lockedArea);
			return lockedArea;
		}

		// We need to read the buffer contents
		if(isReadRequired)
		{
			VulkanTransferBuffer* transferCB = cbManager.GetTransferBuffer(deviceIdx, queueType, localQueueIdx);

			// Ensure flush() will wait for all queues currently using to the texture (if any) to finish
			// If only reading, wait for all writes to complete, otherwise wait on both writes and reads
			if(options == GBL_READ_ONLY)
				useMask = subresource->GetUseInfo(VulkanAccessFlag::Write);
			else
				useMask = subresource->GetUseInfo(VulkanAccessFlag::Read | VulkanAccessFlag::Write);

			transferCB->AppendMask(useMask);

			// Submit the command buffer and wait until it finishes
			transferCB->Flush(true);

			// If writing and some CB has an operation queued that will be using the current contents of the image,
			// create a new image so we don't modify the previous use of the image
			if(options == GBL_READ_WRITE && subresource->IsBound())
			{
				VulkanImage* newImage = CreateImage(device, mInternalFormats[deviceIdx]);

				VkMemoryRequirements memReqs;
				vkGetImageMemoryRequirements(device.GetLogical(), image->GetHandle(), &memReqs);

				u8* src = image->Map(0, (u32)memReqs.size, true);
				u8* dst = newImage->Map(0, (u32)memReqs.size);

				memcpy(dst, src, memReqs.size);

				image->Unmap();
				newImage->Unmap(true);

				image->Destroy();
				image = newImage;
				mImages[deviceIdx] = image;
			}

			image->Map(face, mipLevel, lockedArea, true);
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
	mStagingBuffer = CreateStaging(device, lockedArea, needRead);

	if(needRead) // If reading, we need to copy the current contents of the image to the staging buffer
	{
		VulkanTransferBuffer* transferCB = cbManager.GetTransferBuffer(deviceIdx, queueType, localQueueIdx);

		// Similar to above, if image supports GPU writes or is currently being written to, we need to wait on any
		// potential writes to complete
		u32 writeUseMask = subresource->GetUseInfo(VulkanAccessFlag::Write);

		if(mSupportsGPUWrites || writeUseMask != 0)
		{
			// Ensure flush() will wait for all queues currently writing to the image (if any) to finish
			transferCB->AppendMask(writeUseMask);
		}

		const bool isReadOnly = (options & (GBL_READ_WRITE | GBL_WRITE_ONLY | GBL_WRITE_ONLY_DISCARD | GBL_WRITE_ONLY_DISCARD_RANGE | GBL_WRITE_ONLY_NO_OVERWRITE)) == 0;
		CopyImageSubresourceToBuffer(transferCB->GetCb(), image, face, mipLevel, mStagingBuffer, isReadOnly);

		// Submit the command buffer and wait until it finishes
		transferCB->Flush(true);
	}

	u8* data = mStagingBuffer->Map(0, lockedArea.GetSize());
	lockedArea.SetExternalBuffer(data);

	return lockedArea;
}

void VulkanTexture::UnlockImpl()
{
	// Possibly map() failed with some error
	if(!mIsMapped)
		return;

	// Note: If we did any writes they need to be made visible to the GPU. However there is no need to execute
	// a pipeline barrier because (as per spec) host writes are implicitly visible to the device.

	if(mStagingBuffer == nullptr)
		mImages[mMappedDeviceIdx]->Unmap(true);
	else
	{
		mStagingBuffer->Unmap();

		bool isWrite = mMappedLockOptions != GBL_READ_ONLY;

		// If the caller wrote anything to the staging buffer, we need to upload it back to the main buffer
		if(isWrite)
		{
			VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
			VulkanDevice& device = *rapi.GetDevice(mMappedDeviceIdx);

			VulkanCommandBufferManager& cbManager = GetVulkanCommandBufferManager();
			GpuQueueType queueType;
			u32 localQueueIdx = CommandSyncMask::GetQueueIdxAndType(mMappedGlobalQueueIdx, queueType);

			VulkanImage* image = mImages[mMappedDeviceIdx];
			VulkanTransferBuffer* transferCB = cbManager.GetTransferBuffer(mMappedDeviceIdx, queueType, localQueueIdx);

			VulkanImageSubresource* subresource = image->GetSubresource(mMappedFace, mMappedMip);
			VkImageLayout curLayout = subresource->GetLayout();

			// If the subresource is used in any way on the GPU, we need to wait for that use to finish before
			// we issue our copy
			u32 useMask = subresource->GetUseInfo(VulkanAccessFlag::Read | VulkanAccessFlag::Write);
			bool isNormalWrite = false;
			if(useMask != 0) // Subresource is currently used on the GPU
			{
				// Try to avoid the wait by checking for special write conditions

				// Caller guarantees he won't touch the same data as the GPU, so just copy
				if(mMappedLockOptions == GBL_WRITE_ONLY_NO_OVERWRITE)
				{
					// Fall through to copy()
				}
				// Caller doesn't care about buffer contents, so just discard the existing buffer and create a new one
				else if(mMappedLockOptions == GBL_WRITE_ONLY_DISCARD)
				{
					// We need to discard the entire image, even though we're only writing to a single sub-resource
					image->Destroy();

					image = CreateImage(device, mInternalFormats[mMappedDeviceIdx]);
					mImages[mMappedDeviceIdx] = image;

					subresource = image->GetSubresource(mMappedFace, mMappedMip);
				}
				else // Otherwise we have no choice but to issue a dependency between the queues
				{
					transferCB->AppendMask(useMask);
					isNormalWrite = true;
				}
			}
			else
				isNormalWrite = true;

			const TextureProperties& props = GetProperties();

			// Check if the subresource will still be bound somewhere after the CBs using it finish
			if(isNormalWrite)
			{
				u32 useCount = subresource->GetUseCount();
				u32 boundCount = subresource->GetBoundCount();

				bool isBoundWithoutUse = boundCount > useCount;

				// If image is queued for some operation on a CB, then we need to make a copy of the subresource to
				// avoid modifying its use in the previous operation
				if(isBoundWithoutUse)
				{
					VulkanImage* newImage = CreateImage(device, mInternalFormats[mMappedDeviceIdx]);

					// Avoid copying original contents if the image only has one sub-resource, which we'll overwrite anyway
					if(props.GetNumMipmaps() > 0 || props.GetNumFaces() > 1)
					{
						CopyImageToImage(transferCB->GetCb(), image, newImage);
					}

					image->Destroy();
					image = newImage;
					mImages[mMappedDeviceIdx] = image;
				}
			}

			VkImageSubresourceRange range;
			range.aspectMask = image->GetAspectFlags();
			range.baseArrayLayer = mMappedFace;
			range.layerCount = 1;
			range.baseMipLevel = mMappedMip;
			range.levelCount = 1;

			VkExtent3D extent;
			PixelUtil::GetSizeForMipLevel(props.GetWidth(), props.GetHeight(), props.GetDepth(), mMappedMip, extent.width, extent.height, extent.depth);

			VkImageLayout transferLayout;
			if(mDirectlyMappable)
				transferLayout = VK_IMAGE_LAYOUT_GENERAL;
			else
				transferLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

			const ImageSubresourcePitch pitch = GetPitchForSubresource(image, mMappedFace, mMappedMip);

			// Queue copy command
			transferCB->GetCb()->CopyBufferToImage(mStagingBuffer, image, extent, range, transferLayout, pitch.RowPitch, pitch.SliceHeight);

			// We don't actually flush the transfer buffer here since it's an expensive operation, but it's instead
			// done automatically before next "normal" command buffer submission.
		}

		mStagingBuffer->Destroy();
		mStagingBuffer = nullptr;
	}

	mIsMapped = false;
}


TAsyncOp<SPtr<PixelData>> VulkanTexture::ReadDataAsync(u32 mipLevel, u32 face, u32 deviceIndex, const SPtr<CommandBuffer>& commandBuffer)
{
	VulkanImage *const image = mImages[deviceIndex];
	if(image == nullptr)
	{
		TAsyncOp<SPtr<PixelData>> operation;
		operation.CompleteOperation(nullptr);

		return operation;
	}

	VulkanRenderAPI& vulkanBackend = GetVulkanRenderAPI();
	VulkanDevice& device = *vulkanBackend.GetDevice(deviceIndex);

	VulkanCmdBuffer* vulkanCommandBufffer;
	if(commandBuffer != nullptr)
		vulkanCommandBufffer = static_cast<VulkanCommandBuffer*>(commandBuffer.get())->GetInternal();
	else
		vulkanCommandBufffer = vulkanBackend.GetMainVulkanCommandBuffer()->GetInternal();

	const u32 mipWidth = Math::Max(1u, mProperties.GetWidth() >> mipLevel);
	const u32 mipHeight = Math::Max(1u, mProperties.GetHeight() >> mipLevel);
	const u32 mipDepth = Math::Max(1u, mProperties.GetDepth() >> mipLevel);

	const SPtr<PixelData> pixelData = B3DMakeShared<PixelData>(mipWidth, mipHeight, mipDepth, mInternalFormats[deviceIndex]);

	VulkanBuffer* const buffer = CreateStaging(device, *pixelData, true);
	CopyImageSubresourceToBuffer(vulkanCommandBufffer, image, face, mipLevel, buffer, true);

	TAsyncOp<SPtr<PixelData>> op;
	auto fnOnCommandBufferCompleted = [buffer, op, pixelData]() mutable
	{
		UINT8* data = buffer->Map(0, pixelData->GetSize());

		pixelData->AllocateInternalBuffer();
		memcpy(pixelData->GetData(), data, pixelData->GetSize());

		buffer->Unmap();
		buffer->Destroy();

		op.CompleteOperation(pixelData);
	};

	auto fnOnCommandBufferDestroyed = [buffer, op](bool isSubmitted) mutable
	{
		// In this case the completion callback will trigger.
		if(isSubmitted)
			return;

		buffer->Destroy();
		op.CompleteOperation(nullptr);
	};

	commandBuffer->OnDidComplete.Connect(fnOnCommandBufferCompleted);
	commandBuffer->OnDestroyed.Connect(fnOnCommandBufferDestroyed);

	return op;
}

void VulkanTexture::ReadDataImpl(PixelData& dest, u32 mipLevel, u32 face, u32 deviceIdx, u32 queueIdx)
{
	if(mProperties.GetNumSamples() > 1)
	{
		B3D_LOG(Error, RenderBackend, "Multisampled textures cannot be accessed from the CPU directly.");
		return;
	}

	PixelData myData = Lock(GBL_READ_ONLY, mipLevel, face, deviceIdx, queueIdx);
	PixelUtil::BulkPixelConversion(myData, dest);
	Unlock();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResRead, RenderStatObject_Texture);
}

void VulkanTexture::WriteDataImpl(const PixelData& src, u32 mipLevel, u32 face, bool discardWholeBuffer, u32 queueIdx)
{
	if(src.GetSize() == 0)
		return;

	if(mProperties.GetNumSamples() > 1)
	{
		B3D_LOG(Error, RenderBackend, "Multisampled textures cannot be accessed from the CPU directly.");
		return;
	}

	mipLevel = Math::Clamp(mipLevel, (u32)mipLevel, mProperties.GetNumMipmaps());
	face = Math::Clamp(face, (u32)0, mProperties.GetNumFaces() - 1);

	if(face > 0 && mProperties.GetTextureType() == TEX_TYPE_3D)
	{
		B3D_LOG(Error, RenderBackend, "3D texture arrays are not supported.");
		return;
	}

	// Write to every device
	for(u32 i = 0; i < B3D_MAX_DEVICES; i++)
	{
		if(mImages[i] == nullptr)
			continue;

		PixelData myData = Lock(discardWholeBuffer ? GBL_WRITE_ONLY_DISCARD : GBL_WRITE_ONLY_DISCARD_RANGE, mipLevel, face, i, queueIdx);
		PixelUtil::BulkPixelConversion(src, myData);
		Unlock();
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResWrite, RenderStatObject_Texture);
}
