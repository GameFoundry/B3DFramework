//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "Image/BsTexture.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		class VulkanImageSubresource;

		/** Descriptor used for initializing a VulkanImage. */
		struct VULKAN_IMAGE_DESC
		{
			VkImage Image; /**< Internal Vulkan image object */
			VmaAllocation Allocation; /** Information about the memory allocated for this image. */
			VkImageLayout Layout; /**< Initial layout of the image. */
			TextureType Type; /**< Type of the image. */
			VkFormat Format; /**< Pixel format of the image. */
			u32 NumFaces; /**< Number of faces (array slices, or cube-map faces). */
			u32 NumMipLevels; /**< Number of mipmap levels per face. */
			u32 Usage; /** Determines how will the image be used. */
		};

		/** Wrapper around a Vulkan image object that manages its usage and lifetime. */
		class VulkanImage : public VulkanResource
		{
		public:
			/**
			 * @param[in]	owner			Resource manager that keeps track of lifetime of this resource.
			 * @param[in]	image			Internal image Vulkan object.
			 * @param[in]	allocation		Information about the memory bound to the image.
			 * @param[in]	layout			Initial layout of the image.
			 * @param[in]	actualFormat	Actual format the image was created with (rather than the requested format).
			 * @param[in]	props			Properties describing the image.
			 * @param[in]	ownsImage		If true, this object will take care of releasing the image and its memory, otherwise
			 *								it is expected they will be released externally.
			 */
			VulkanImage(VulkanResourceManager* owner, VkImage image, VmaAllocation allocation, VkImageLayout layout, VkFormat actualFormat, const TextureProperties& props, bool ownsImage = true);

			/**
			 * @param[in]	owner		Resource manager that keeps track of lifetime of this resource.
			 * @param[in]	desc		Describes the image to assign.
			 * @param[in]	ownsImage	If true, this object will take care of releasing the image and its memory, otherwise
			 *							it is expected they will be released externally.
			 */
			VulkanImage(VulkanResourceManager* owner, const VULKAN_IMAGE_DESC& desc, bool ownsImage = true);
			~VulkanImage();

			/** Returns the internal handle to the Vulkan object. */
			VkImage GetHandle() const { return mImage; }

			/** Returns the preferred (not necessarily current) layout of the image. */
			VkImageLayout GetOptimalLayout() const;

			/**
			 * Returns an image view that covers all faces and mip maps of the texture.
			 *
			 * @param[in]	framebuffer	Set to true if the view will be used as a framebuffer attachment. Ensures proper
			 *							attachment flags are set on the view.
			 */
			VkImageView GetView(bool framebuffer) const;

			/**
			 * Returns an image view that covers the specified faces and mip maps of the texture.
			 *
			 * @param[in]	surface		Surface that describes which faces and mip levels to retrieve the view for.
			 * @param[in]	framebuffer	Set to true if the view will be used as a framebuffer attachment. Ensures proper
			 *							attachment flags are set on the view.
			 */
			VkImageView GetView(const TextureSurface& surface, bool framebuffer) const;

			/**
			 * Returns an image view with a specific format.
			 *
			 * @param[in]	format		Format to view the texture pixels as.
			 * @param[in]	framebuffer	Set to true if the view will be used as a framebuffer attachment. Ensures proper
			 *							attachment flags are set on the view.
			 */
			VkImageView GetView(VkFormat format, bool framebuffer) const;

			/**
			 * Returns an image view that covers the specified faces and mip maps of the texture, with a specific format.
			 *
			 * @param[in]	format		Format to view the texture pixels as.
			 * @param[in]	surface		Surface that describes which faces and mip levels to retrieve the view for.
			 * @param[in]	framebuffer	Set to true if the view will be used as a framebuffer attachment. Ensures proper
			 *							attachment flags are set on the view.
			 */
			VkImageView GetView(VkFormat format, const TextureSurface& surface, bool framebuffer) const;

			/** Get aspect flags that represent the contents of this image. */
			VkImageAspectFlags GetAspectFlags() const;

			/** Retrieves a subresource range covering all the sub-resources of the image. */
			VkImageSubresourceRange GetRange() const;

			/** Retrieves a subresource range covering all the specified sub-resource range of the image. */
			VkImageSubresourceRange GetRange(const TextureSurface& surface) const;

			/**
			 * Retrieves a separate resource for a specific image face & mip level. This allows the caller to track subresource
			 * usage individually, instead for the entire image.
			 */
			VulkanImageSubresource* GetSubresource(u32 face, u32 mipLevel);

			/**
			 * Returns a pointer to internal image memory for the specified sub-resource. Must be followed by unmap(). Caller
			 * must ensure the image was created in CPU readable memory, and that image isn't currently being written to by the
			 * GPU.
			 *
			 * @param[in]	face		Index of the face to map.
			 * @param[in]	mipLevel	Index of the mip level to map.
			 * @param[in]	output		Output object containing the pointer to the sub-resource data.
			 */
			void Map(u32 face, u32 mipLevel, PixelData& output) const;

			/**
			 * Returns a pointer to internal image memory for the entire resource. Must be followed by unmap(). Caller
			 * must ensure the image was created in CPU readable memory, and that image isn't currently being written to by the
			 * GPU.
			 */
			u8* Map(u32 offset, u32 size) const;

			/** Unmaps a buffer previously mapped with map(). */
			void Unmap();

			/**
			 * Queues a command on the provided command buffer. The command copies the contents of the current image
			 * subresource to the destination buffer.
			 */
			void Copy(VulkanTransferBuffer* cb, VulkanBuffer* destination, const VkExtent3D& extent, const VkImageSubresourceLayers& range, VkImageLayout layout);

			/**
			 * Determines a set of access flags based on the current image and provided image layout. This method makes
			 * certain assumptions about image usage, so it might not be valid in all situations.
			 *
			 * @param[in]	layout		Layout the image is currently in.
			 * @param[in]	readOnly	True if the image is only going to be read without writing, allows the system to
			 *							set less general access flags. If unsure, set to false.
			 */
			VkAccessFlags GetAccessFlags(VkImageLayout layout, bool readOnly = false);

			/**
			 * Generates a set of image barriers that are grouped depending on the current layout of individual sub-resources
			 * in the specified range. The method will try to reduce the number of generated barriers by grouping as many
			 * sub-resources as possibly.
			 */
			void GetBarriers(const VkImageSubresourceRange& range, Vector<VkImageMemoryBarrier>& barriers);

		private:
			/** Creates a new view of the provided part (or entirety) of surface. */
			VkImageView CreateView(const TextureSurface& surface, VkFormat format, VkImageAspectFlags aspectMask) const;

			/** Contains information about view for a specific surface(s) of this image. */
			struct ImageViewInfo
			{
				TextureSurface Surface;
				bool Framebuffer;
				VkImageView View;
				VkFormat Format;
			};

			VkImage mImage;
			VmaAllocation mAllocation;
			VkImageView mMainView;
			VkImageView mFramebufferMainView;
			i32 mUsage;
			bool mOwnsImage;

			u32 mNumFaces;
			u32 mNumMipLevels;
			VulkanImageSubresource** mSubresources;

			mutable VkImageViewCreateInfo mImageViewCI;
			mutable Vector<ImageViewInfo> mImageInfos;
		};

		/** Represents a single sub-resource (face & mip level) of a larger image object. */
		class VulkanImageSubresource : public VulkanResource
		{
		public:
			VulkanImageSubresource(VulkanResourceManager* owner, VkImageLayout layout);

			/**
			 * Returns the layout the subresource is currently in. Note that this is only used to communicate layouts between
			 * different command buffers, and will only be updated only after command buffer submit() call. In short this means
			 * you should only care about this value on the core thread.
			 */
			VkImageLayout GetLayout() const { return mLayout; }

			/** Notifies the resource that the current subresource layout has changed. */
			void SetLayout(VkImageLayout layout) { mLayout = layout; }

		private:
			VkImageLayout mLayout;
		};

		/**	Vulkan implementation of a texture. */
		class VulkanTexture : public Texture
		{
		public:
			~VulkanTexture();

			/**
			 * Gets the resource wrapping the Vulkan image object, on the specified device. If texture device mask doesn't
			 * include the provided device, null is returned.
			 */
			VulkanImage* GetResource(u32 deviceIdx) const { return mImages[deviceIdx]; }

		protected:
			friend class VulkanTextureManager;

			VulkanTexture(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask);

			void Initialize() override;
			PixelData LockImpl(GpuLockOptions options, u32 mipLevel = 0, u32 face = 0, u32 deviceIdx = 0, u32 queueIdx = 0) override;
			void UnlockImpl() override;
			void CopyImpl(const SPtr<Texture>& target, const TEXTURE_COPY_DESC& desc, const SPtr<CommandBuffer>& commandBuffer) override;
			void ReadDataImpl(PixelData& dest, u32 mipLevel = 0, u32 face = 0, u32 deviceIdx = 0, u32 queueIdx = 0) override;
			void WriteDataImpl(const PixelData& src, u32 mipLevel = 0, u32 face = 0, bool discardWholeBuffer = false, u32 queueIdx = 0) override;

		private:
			/** Creates a new image for the specified device, matching the current properties. */
			VulkanImage* CreateImage(VulkanDevice& device, PixelFormat format);

			/**
			 * Creates a staging buffer that can be used for texture transfer operations.
			 *
			 * @param[in]	device		Device to create the buffer on.
			 * @param[in]	pixelData	Object that describes the image sub-resource that will be in the buffer.
			 * @param[in]	needsRead	True if we will be copying data from the buffer, false if just reading. True if both.
			 * @return					Newly allocated buffer.
			 */
			VulkanBuffer* CreateStaging(VulkanDevice& device, const PixelData& pixelData, bool needsRead);

			/**
			 * Copies all sub-resources from the source image to the destination image. Caller must ensure the images
			 * are of the same size. The operation will be queued on the provided command buffer. The system assumes the
			 * provided image matches the current texture properties (i.e. num faces, mips, size).
			 */
			void CopyImage(VulkanTransferBuffer* cb, VulkanImage* srcImage, VulkanImage* dstImage, VkImageLayout srcFinalLayout, VkImageLayout dstFinalLayout);

			VulkanImage* mImages[BS_MAX_DEVICES];
			PixelFormat mInternalFormats[BS_MAX_DEVICES];
			GpuDeviceFlags mDeviceMask;

			VulkanBuffer* mStagingBuffer;
			u32 mMappedDeviceIdx;
			u32 mMappedGlobalQueueIdx;
			u32 mMappedMip;
			u32 mMappedFace;
			u32 mMappedRowPitch;
			u32 mMappedSlicePitch;
			GpuLockOptions mMappedLockOptions;

			VkImageCreateInfo mImageCI;
			bool mDirectlyMappable : 1;
			bool mSupportsGPUWrites : 1;
			bool mIsMapped : 1;
		};

		/** @} */
	} // namespace ct
} // namespace bs
