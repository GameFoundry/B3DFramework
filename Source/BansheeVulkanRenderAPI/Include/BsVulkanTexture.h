//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "BsTexture.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	class VulkanImageSubresource;

	/** Type of a Vulkan image, determining its usage. */
	enum class VulkanImageUsage
	{
		Sampled,
		SampledDynamic,
		Storage,
		ColorAttachment,
		DepthAttachment
	};

	/** Descriptor used for initializing a VulkanImage. */
	struct VULKAN_IMAGE_DESC
	{
		VkImage image; /**< Internal Vulkan image object */
		VkDeviceMemory memory; /**< Memory bound to the image. */
		VkImageLayout layout; /**< Initial layout of the image. */
		TextureType type; /**< Type of the image. */
		VkFormat format; /**< Pixel format of the image. */
		UINT32 numFaces; /**< Number of faces (array slices, or cube-map faces). */
		UINT32 numMipLevels; /**< Number of mipmap levels per face. */
		VulkanImageUsage usage; /** Determines how will the image be used. */
	};

	/** Wrapper around a Vulkan image object that manages its usage and lifetime. */
	class VulkanImage : public VulkanResource
	{
	public:
		/**
		 * @param[in]	owner		Resource manager that keeps track of lifetime of this resource.
		 * @param[in]	image		Internal image Vulkan object.
		 * @param[in]	memory		Memory bound to the image.
		 * @param[in]	layout		Initial layout of the image.
		 * @param[in]	props		Properties describing the image.
		 * @param[in]	ownsImage	If true, this object will take care of releasing the image and its memory, otherwise
		 *							it is expected they will be released externally.
		 */
		VulkanImage(VulkanResourceManager* owner, VkImage image, VkDeviceMemory memory, VkImageLayout layout,
					const TextureProperties& props, bool ownsImage = true);

		/**
		 * @param[in]	owner		Resource manager that keeps track of lifetime of this resource.
		 * @param[in]	desc		Describes the image to assign.
		 * @param[in]	ownsImage	If true, this object will take care of releasing the image and its memory, otherwise
		 *							it is expected they will be released externally.
		 */
		VulkanImage(VulkanResourceManager* owner, const VULKAN_IMAGE_DESC& desc, bool ownsImage = true);
		~VulkanImage();

		/** Returns the internal handle to the Vulkan object. */
		VkImage getHandle() const { return mImage; }

		/** Returns the preferred (not necessarily current) layout of the image. */
		VkImageLayout getOptimalLayout() const;

		/** 
		 * Returns the layout the image is currently in. Note that this is only used to communicate layouts between 
		 * different command buffers, and will only be updated only after command buffer submit() call. In short this means
		 * you should only care about this value on the core thread.
		 */
		VkImageLayout getLayout() const { return mLayout; }

		/** Notifies the resource that the current image layout has changed. */
		void setLayout(VkImageLayout layout) { mLayout = layout; }

		/** 
		 * Returns an image view that covers all faces and mip maps of the texture. 
		 * 
		 * @param[in]	framebuffer	Set to true if the view will be used as a framebuffer attachment. Ensures proper
		 *							attachment flags are set on the view.
		 */
		VkImageView getView(bool framebuffer) const;

		/** 
		 * Returns an image view that covers the specified faces and mip maps of the texture. 
		 *
		 * @param[in]	surface		Surface that describes which faces and mip levels to retrieve the view for.
		 * @param[in]	framebuffer	Set to true if the view will be used as a framebuffer attachment. Ensures proper
		 *							attachment flags are set on the view.
		 */
		VkImageView getView(const TextureSurface& surface, bool framebuffer) const;
		
		/** Retrieves a subresource range covering all the sub-resources of the image. */
		VkImageSubresourceRange getRange() const;

		/** 
		 * Retrieves a separate resource for a specific image face & mip level. This allows the caller to track subresource
		 * usage individually, instead for the entire image. 
		 */
		VulkanImageSubresource* getSubresource(UINT32 face, UINT32 mipLevel);

		/** 
		 * Returns a pointer to internal image memory for the specified sub-resource. Must be followed by unmap(). Caller
		 * must ensure the image was created in CPU readable memory, and that image isn't currently being written to by the
		 * GPU.
		 * 
		 * @param[in]	face		Index of the face to map.
		 * @param[in]	mipLevel	Index of the mip level to map.
		 * @param[in]	output		Output object containing the pointer to the sub-resource data.
		 */
		void map(UINT32 face, UINT32 mipLevel, PixelData& output) const;

		/** 
		 * Returns a pointer to internal image memory for the entire resource. Must be followed by unmap(). Caller
		 * must ensure the image was created in CPU readable memory, and that image isn't currently being written to by the
		 * GPU.
		 */
		UINT8* map(UINT32 offset, UINT32 size) const;

		/** Unmaps a buffer previously mapped with map(). */
		void unmap();

		/** 
		 * Queues a command on the provided command buffer. The command copies the contents of the current image
		 * subresource to the destination buffer. 
		 */
		void copy(VulkanTransferBuffer* cb, VulkanBuffer* destination, const VkExtent3D& extent, 
			const VkImageSubresourceLayers& range, VkImageLayout layout);

		/** 
		 * Determines a set of access flags based on the current image and provided image layout. This method makes 
		 * certain assumptions about image usage, so it might not be valid in all situations. 
		 * 
		 * @param[in]	layout		Layout the image is currently in.
		 * @param[in]	readOnly	True if the image is only going to be read without writing, allows the system to
		 *							set less general access flags. If unsure, set to false.
		 */
		VkAccessFlags getAccessFlags(VkImageLayout layout, bool readOnly = false);

	private:
		/** Creates a new view of the provided part (or entirety) of surface. */
		VkImageView createView(const TextureSurface& surface, VkImageAspectFlags aspectMask) const;

		/** Contains information about view for a specific surface(s) of this image. */
		struct ImageViewInfo
		{
			TextureSurface surface;
			bool framebuffer;
			VkImageView view;
		};

		VkImage mImage;
		VkDeviceMemory mMemory;
		VkImageLayout mLayout;
		VkImageView mMainView;
		VkImageView mFramebufferMainView;
		VulkanImageUsage mUsage;
		bool mOwnsImage;

		UINT32 mNumFaces;
		UINT32 mNumMipLevels;
		VulkanImageSubresource** mSubresources;

		mutable VkImageViewCreateInfo mImageViewCI;
		mutable Vector<ImageViewInfo> mImageInfos;
	};

	/** Represents a single sub-resource (face & mip level) of a larger image object. */
	class VulkanImageSubresource : public VulkanResource
	{
	public:
		VulkanImageSubresource(VulkanResourceManager* owner);
	};

	/**	Vulkan implementation of a texture. */
	class VulkanTextureCore : public TextureCore
	{
	public:
		~VulkanTextureCore();

		/** 
		 * Gets the resource wrapping the Vulkan image object, on the specified device. If texture device mask doesn't 
		 * include the provided device, null is returned. 
		 */
		VulkanImage* getResource(UINT32 deviceIdx) const { return mImages[deviceIdx]; }

	protected:
		friend class VulkanTextureCoreManager;

		VulkanTextureCore(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask);

		/** @copydoc CoreObjectCore::initialize() */
		void initialize() override;

		/** @copydoc TextureCore::lockImpl */
		PixelData lockImpl(GpuLockOptions options, UINT32 mipLevel = 0, UINT32 face = 0, UINT32 deviceIdx = 0,
						   UINT32 queueIdx = 0) override;

		/** @copydoc TextureCore::unlockImpl */
		void unlockImpl() override;

		/** @copydoc TextureCore::copyImpl */
		void copyImpl(UINT32 srcFace, UINT32 srcMipLevel, UINT32 destFace, UINT32 destMipLevel,
					  const SPtr<TextureCore>& target, UINT32 queueIdx = 0) override;

		/** @copydoc TextureCore::readData */
		void readDataImpl(PixelData& dest, UINT32 mipLevel = 0, UINT32 face = 0, UINT32 deviceIdx = 0,
					  UINT32 queueIdx = 0) override;

		/** @copydoc TextureCore::writeData */
		void writeDataImpl(const PixelData& src, UINT32 mipLevel = 0, UINT32 face = 0, bool discardWholeBuffer = false,
					   UINT32 queueIdx = 0) override;

	private:
		/** Creates a new image for the specified device, matching the current properties. */
		VulkanImage* createImage(VulkanDevice& device);

		/** 
		 * Creates a staging buffer that can be used for texture transfer operations.
		 * 
		 * @param[in]	device		Device to create the buffer on.
		 * @param[in]	pixelData	Object that describes the image sub-resource that will be in the buffer.
		 * @param[in]	needsRead	True if we will be copying data from the buffer, false if just reading. True if both.
		 * @return					Newly allocated buffer.
		 */
		VulkanBuffer* createStaging(VulkanDevice& device, const PixelData& pixelData, bool needsRead);

		/** 
		 * Copies all sub-resources from the source image to the destination image. Caller must ensure the images
		 * are of the same size. The operation will be queued on the provided command buffer. The system assumes the 
		 * provided image matches the current texture properties (i.e. num faces, mips, size).
		 */
		void copyImage(VulkanTransferBuffer* cb, VulkanImage* srcImage, VulkanImage* dstImage, 
			VkImageLayout srcFinalLayout, VkImageLayout dstFinalLayout);

		VulkanImage* mImages[BS_MAX_DEVICES];
		GpuDeviceFlags mDeviceMask;

		VulkanBuffer* mStagingBuffer;
		UINT32 mMappedDeviceIdx;
		UINT32 mMappedGlobalQueueIdx;
		UINT32 mMappedMip;
		UINT32 mMappedFace;
		UINT32 mMappedRowPitch;
		UINT32 mMappedSlicePitch;
		GpuLockOptions mMappedLockOptions;

		VkImageCreateInfo mImageCI;
		bool mDirectlyMappable : 1;
		bool mSupportsGPUWrites : 1;
		bool mIsMapped : 1;
	};

	/** @} */
}