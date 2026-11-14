//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanTexture.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanFramebuffer.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "CoreObject/B3DRenderThread.h"
#include "Profiling/B3DRenderStats.h"
#include "Math/B3DMath.h"
#include "GpuBackend/B3DGpuBackendUtility.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Determines the full aspect flags for a texture with the provided @p usage and @p format. */
	GpuTextureAspectFlags GetFullAspectFlags(TextureUsageFlags usage, VkFormat format)
	{
		if(usage.IsSet(TextureUsageFlag::DepthStencil))
		{
			const bool hasStencil = format == VK_FORMAT_D16_UNORM_S8_UINT ||
				format == VK_FORMAT_D24_UNORM_S8_UINT ||
				format == VK_FORMAT_D32_SFLOAT_S8_UINT;

			return hasStencil ? (GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil) : GpuTextureAspectFlags(GpuTextureAspectFlag::Depth);
		}

		return GpuTextureAspectFlag::Color;
	}

	/** Returns the Vulkan image view type of a texture view of @p viewType. */
	VkImageViewType GetImageViewType(TextureViewType viewType)
	{
		switch(viewType)
		{
		case TextureViewType::Texture1D:
			return VK_IMAGE_VIEW_TYPE_1D;
		case TextureViewType::Texture1DArray:
			return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		case TextureViewType::Texture2DArray:
			return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		case TextureViewType::Texture3D:
			return VK_IMAGE_VIEW_TYPE_3D;
		case TextureViewType::TextureCube:
			return VK_IMAGE_VIEW_TYPE_CUBE;
		case TextureViewType::TextureCubeArray:
			return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
		case TextureViewType::Texture2D:
		default:
			return VK_IMAGE_VIEW_TYPE_2D;
		}
	}
}

VulkanImage::VulkanImage(VulkanResourceManager* owner, const VulkanImageCreateInformation& createInformation, VkImage image, VulkanAllocationResult allocation)
	: TVulkanResource<IGpuImageResource>(owner, createInformation.CreateInfo.sharingMode == VK_SHARING_MODE_CONCURRENT, createInformation.DebugName, createInformation.FaceCount, createInformation.MipLevelCount, GetFullAspectFlags(createInformation.Usage, createInformation.Format)), mImage(image), mAllocation(allocation), mMappedMemory(allocation.MappedMemory), mUsage(createInformation.Usage), mOwnsImage(createInformation.OwnsImage), mIsShaderReadAllowed(createInformation.IsShaderReadAllowed), mDepthSliceCount(createInformation.DepthSliceCount), mTextureType(createInformation.Type)
{
	mImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	mImageViewCI.pNext = nullptr;
	mImageViewCI.flags = 0;
	mImageViewCI.image = image;
	mImageViewCI.format = createInformation.Format;
	mImageViewCI.components = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};

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

	InitializeNativeLayout(mFullRange.AspectMask, (u32)createInformation.Layout);
}

VulkanImage::~VulkanImage()
{
	VulkanGpuDevice& device = mOwner->GetDevice();
	VkDevice vkDevice = device.GetLogical();

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

	IGpuResource::Destroy();
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
	TextureSurface explicitSurface = surface;
	CalculateExplicitSurface(explicitSurface, isPartOfFramebuffer);

	VkImageViewCreateInfo viewCreateInfo = mImageViewCI;
	viewCreateInfo.viewType = GetImageViewType(explicitSurface.ViewType);
	viewCreateInfo.subresourceRange.aspectMask = aspectMask;
	viewCreateInfo.subresourceRange.baseMipLevel = explicitSurface.MipLevel;
	viewCreateInfo.subresourceRange.levelCount = explicitSurface.MipLevelCount;
	viewCreateInfo.subresourceRange.baseArrayLayer = explicitSurface.Face;
	viewCreateInfo.subresourceRange.layerCount = explicitSurface.FaceCount;
	viewCreateInfo.format = format;

	VulkanImageView view;
	view.Type = viewCreateInfo.viewType;

	VkResult result = vkCreateImageView(mOwner->GetDevice().GetLogical(), &viewCreateInfo, gVulkanAllocator, &view.Handle);
	B3D_ASSERT(result == VK_SUCCESS);

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
	if(!isPartOfFramebuffer)
	{
		surface = TextureProperties::ResolveSurface(surface, mTextureType, mFaceCount, mMipLevelCount, false);
		return surface;
	}

	// 3D textures are attached as 2D arrays of their depth slices
	const bool is3D = mTextureType == TEX_TYPE_3D;
	surface = TextureProperties::ResolveSurface(surface, mTextureType, is3D ? mDepthSliceCount : mFaceCount, mMipLevelCount, false);

	const bool is1D = surface.ViewType == TextureViewType::Texture1D || surface.ViewType == TextureViewType::Texture1DArray;
	if(!is1D)
		surface.ViewType = surface.FaceCount > 1 ? TextureViewType::Texture2DArray : TextureViewType::Texture2D;

	return surface;
}

VkImageAspectFlags VulkanImage::GetAspectFlags() const
{
	return VulkanUtility::GetAspectMask(GetFullAspectFlags(mUsage, mImageViewCI.format));
}

VkSubresourceLayout VulkanImage::GetSubresourceLayout(u32 face, u32 mipLevel) const
{
	VulkanGpuDevice& device = mOwner->GetDevice();

	VkImageSubresource subresourceRange;
	subresourceRange.mipLevel = mipLevel;
	subresourceRange.arrayLayer = face;

	if(!mUsage.IsSet(TextureUsageFlag::DepthStencil))
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	else
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; // Ignoring stencil

	VkSubresourceLayout layout;
	vkGetImageSubresourceLayout(device.GetLogical(), mImage, &subresourceRange, &layout);
	
	return layout;
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

	mMappedOffset = offset;
	mMappedSize = size;

	return device.MapMemory(mAllocation, offset);
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

VulkanTexture::VulkanTexture(VulkanGpuDevice& gpuDevice, const TextureCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags)
	: Texture(createInformation, allocation, flags), mGpuDevice(gpuDevice), mDirectlyMappable(false), mSupportsGPUWrites(false), mUsesGeneralLayout(false)
{
	// The supported format is valid before the image exists, such as while the transient resource allocator has not placed the texture
	mInternalFormat = GetInternalFormat(gpuDevice, mProperties);
}

VulkanTexture::~VulkanTexture()
{
	if (mImage != nullptr)
		mImage->Destroy();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Texture);
}

bool VulkanTexture::UsesLinearTiling(const TextureProperties& properties)
{
	// Dynamic textures use linear tiling, so they can be mapped and modified directly. Only 2D textures with one sample, one mip level and
	// one face qualify, and only for shader reads, not render targets or storage textures. (Optionally check
	// vkGetPhysicalDeviceFormatProperties & vkGetPhysicalDeviceImageFormatProperties for additional supported configs, but right now there
	// doesn't seem to be any additional support)
	const TextureUsageFlags gpuWriteUsage = TextureUsageFlag::RenderTarget | TextureUsageFlag::DepthStencil | TextureUsageFlag::AllowUnorderedAccessOnTheGPU;
	return properties.Usage.IsSet(TextureUsageFlag::StoreOnCPUWithGPUAccess) && !properties.Usage.IsSetAny(gpuWriteUsage) && properties.Type == TEX_TYPE_2D &&
		properties.SampleCount <= 1 && properties.MipMapCount == 0 && properties.GetFaceCount() == 1;
}

PixelFormat VulkanTexture::GetInternalFormat(const VulkanGpuDevice& device, const TextureProperties& properties)
{
	return VulkanUtility::GetClosestSupportedPixelFormat(device, properties.Format, properties.Type, properties.Usage, !UsesLinearTiling(properties), properties.UseHardwareSRGB);
}

void VulkanTexture::BuildDescription(const VulkanGpuDevice& device, const TextureProperties& properties, VulkanTextureDescription& outDescription)
{
	VkImageCreateInfo& createInfo = outDescription.CreateInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;

	TextureType texType = properties.Type;
	switch(texType)
	{
	case TEX_TYPE_1D:
		createInfo.imageType = VK_IMAGE_TYPE_1D;
		break;
	case TEX_TYPE_2D:
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		break;
	case TEX_TYPE_3D:
		createInfo.imageType = VK_IMAGE_TYPE_3D;

		if(properties.Usage.IsSet(TextureUsageFlag::RenderTarget))
			createInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;

		break;
	case TEX_TYPE_CUBE_MAP:
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		break;
	}

	// Note: I force rendertarget and depthstencil types to be readable in shader. Depending on performance impact
	// it might be beneficial to allow the user to enable this explicitly only when needed.

	createInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	TextureUsageFlags usage = properties.Usage;
	if(usage.IsSet(TextureUsageFlag::RenderTarget))
	{
		createInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		outDescription.SupportsGPUWrites = true;
	}
	else if(usage.IsSet(TextureUsageFlag::DepthStencil))
	{
		createInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		outDescription.SupportsGPUWrites = true;
	}

	if(usage.IsSet(TextureUsageFlag::AllowUnorderedAccessOnTheGPU))
	{
		createInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		outDescription.SupportsGPUWrites = true;
	}

	// Storage images are accessed in the general layout, and keeping CPU-accessible images in it lets them be mapped without a transition
	outDescription.UsesGeneralLayout = usage.IsSet(TextureUsageFlag::AllowUnorderedAccessOnTheGPU) || usage.IsSet(TextureUsageFlag::StoreOnCPUWithGPUAccess);

	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
	if(UsesLinearTiling(properties))
	{
		outDescription.DirectlyMappable = true;
		tiling = VK_IMAGE_TILING_LINEAR;
		layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	}

	if(usage.IsSet(TextureUsageFlag::MutableFormat))
		createInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

	u32 width = properties.Width;
	u32 height = properties.Height;
	u32 depth = properties.Depth;

	// 0-sized textures aren't supported by the API
	width = std::max(width, 1U);
	height = std::max(height, 1U);
	depth = std::max(depth, 1U);

	createInfo.extent = { width, height, depth };
	createInfo.mipLevels = properties.MipMapCount + 1;
	createInfo.arrayLayers = properties.GetFaceCount();
	createInfo.samples = VulkanUtility::GetSampleFlags(properties.SampleCount);
	createInfo.tiling = tiling;
	createInfo.initialLayout = layout;

	// Concurrently readable textures are shared so that reads on any queue need no ownership transfer
	const TInlineArray<u32, GQT_COUNT>& queueFamilies = device.GetQueueFamilies();
	const bool isShared = GpuBackendUtility::IsConcurrentlyReadable(usage);
	const bool usesConcurrentSharing = isShared && queueFamilies.Size() > 1;
	createInfo.sharingMode = usesConcurrentSharing ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = usesConcurrentSharing ? (u32)queueFamilies.Size() : 0;
	createInfo.pQueueFamilyIndices = usesConcurrentSharing ? queueFamilies.data() : nullptr;

	outDescription.InternalFormat = GetInternalFormat(device, properties);
	createInfo.format = VulkanUtility::GetPixelFormat(outDescription.InternalFormat, properties.UseHardwareSRGB);

	// Linearly-tiled images are CPU-accessible by spec and route through host-visible memory; optimally-tiled
	// images live exclusively in DEVICE_LOCAL memory and are populated via staging copies.
	if(outDescription.DirectlyMappable)
	{
		outDescription.MemoryFlags = VulkanMemoryFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}
	else
	{
		outDescription.MemoryFlags = VulkanMemoryFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
	}
}

void VulkanTexture::Initialize()
{
	ASSERT_IF_NOT_RENDER_THREAD;

	VulkanTextureDescription description;
	BuildDescription(mGpuDevice, mProperties, description);

	mImageCreateInformation = description.CreateInfo;
	mInternalFormat = description.InternalFormat;
	mDirectlyMappable = description.DirectlyMappable;
	mSupportsGPUWrites = description.SupportsGPUWrites;
	mUsesGeneralLayout = description.UsesGeneralLayout;

	mImage = CreateImage();
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
	return mImage->GetSubresourceUseInfo(arrayLayer, mipLevel, accessFlags);
}

u32 VulkanTexture::GetBoundCount(u32 subresourceIdx) const
{
	u32 face, mip;
	mProperties.MapFromSubresourceIndex(subresourceIdx, face, mip);

	return mImage->GetSubresourceBoundCount(face, mip);
}

u32 VulkanTexture::GetUseCount(u32 subresourceIdx) const
{
	u32 face, mip;
	mProperties.MapFromSubresourceIndex(subresourceIdx, face, mip);

	return mImage->GetSubresourceUseCount(face, mip);
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

VulkanImage* VulkanTexture::CreateImage()
{
	VulkanImageCreateInformation imageInfo = BuildImageCreateInformation();

	GpuAllocation allocation;
	VulkanImage* const vulkanImage = mGpuDevice.CreateImage(imageInfo, mRequestedAllocation, allocation);
	SetCurrentAllocation(allocation);

	if (vulkanImage != nullptr)
		vulkanImage->SetName(mName);

	if (vulkanImage != nullptr && StartsAtRest())
		vulkanImage->RequestRestingTransition(mUsesGeneralLayout ? GpuImageLayout::General : GpuImageLayout::ShaderReadOnly);

	return vulkanImage;
}

VulkanImageCreateInformation VulkanTexture::BuildImageCreateInformation() const
{
	VulkanImageCreateInformation imageInfo;
	imageInfo.Layout = mImageCreateInformation.initialLayout;
	imageInfo.Type = mProperties.Type;
	imageInfo.Format = mImageCreateInformation.format;
	imageInfo.FaceCount = mProperties.GetFaceCount();
	imageInfo.DepthSliceCount = mProperties.Depth;
	imageInfo.MipLevelCount = mProperties.MipMapCount + 1;
	imageInfo.Usage = mProperties.Usage;
	imageInfo.CreateInfo = mImageCreateInformation;
	imageInfo.OwnsImage = true;
	imageInfo.IsShaderReadAllowed = true;
	imageInfo.DebugName = mName;
	return imageInfo;
}

void VulkanTexture::RecreateInternalTexture()
{
	VulkanImage* const newImage = CreateImage();
	mImage->Destroy();
	mImage = newImage;
	mMappedMemory = mImage->GetMappedMemory();
}

void VulkanTexture::SwapInternalTexture(Texture& other)
{
	VulkanTexture& vulkanOther = static_cast<VulkanTexture&>(other);
	std::swap(mImage, vulkanOther.mImage);
	std::swap(mMappedMemory, vulkanOther.mMappedMemory);
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
		const GpuQueueMask useMask = mImage->GetSubresourceUseInfo(arrayLayer, mipLevel, accessFlags);

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
