//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "B3DVulkanSubmitThread.h"
#include "Image/B3DTexture.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		class VulkanImageSubresource;

		/** Descriptor used for initializing a VulkanImage. */
		struct VulkanImageCreateInformation
		{
			VkImage Image = VK_NULL_HANDLE; /**< Internal Vulkan image object */
			VulkanAllocationResult Allocation; /** Information about the memory allocated for this image. */
			VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED; /**< Initial layout of the image. */
			TextureType Type = TEX_TYPE_2D; /**< Type of the image. */
			VkFormat Format = VK_FORMAT_UNDEFINED; /**< Pixel format of the image. */
			u32 FaceCount = 1; /**< Number of faces (array slices, or cube-map faces). */
			u32 DepthSliceCount = 1; /**< Number of depth slices (only relevant for 3D textures). */
			u32 MipLevelCount = 1; /**< Number of mipmap levels per face. */
			u32 Usage = 0; /** Determines how will the image be used. */
		};

		/** Wrapper around VkImageView. */
		struct VulkanImageView
		{
			VkImageView Handle = VK_NULL_HANDLE;
			VkImageViewType Type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
		};

		/** Wrapper around a Vulkan image object that manages its usage and lifetime. */
		class VulkanImage : public VulkanResource
		{
		public:
			/**
			 * @param	owner					Resource manager that keeps track of lifetime of this resource.
			 * @param	image					Internal image Vulkan object.
			 * @param	allocation				Information about the memory bound to the image.
			 * @param	layout					Initial layout of the image.
			 * @param	actualFormat			Actual format the image was created with (rather than the requested format).
			 * @param	textureProperties		Properties describing the image.
			 * @param	ownsImage				If true, this object will take care of releasing the image and its memory, otherwise it is expected they will be released externally.
			 * @param	isShaderReadAllowed		True if the image is allowed to be read in the shader. If not, it can only be used as a framebuffer attachment.
			 */
			VulkanImage(VulkanResourceManager* owner, VkImage image, VulkanAllocationResult allocation, VkImageLayout layout, VkFormat actualFormat, const TextureProperties& textureProperties, bool ownsImage = true, bool isShaderReadAllowed = true, const StringView& name = "");

			/**
			 * @param	owner					Resource manager that keeps track of lifetime of this resource.
			 * @param	createInformation		Describes the image to assign.
			 * @param	ownsImage				If true, this object will take care of releasing the image and its memory, otherwise it is expected they will be released externally.
			 * @param	isShaderReadAllowed		True if the image is allowed to be read in the shader. If not, it can only be used as a framebuffer attachment.
			 */
			VulkanImage(VulkanResourceManager* owner, const VulkanImageCreateInformation& createInformation, bool ownsImage = true, bool isShaderReadAllowed = true, const StringView& name = "");
			~VulkanImage();

			void Destroy() override;

			/** Returns the internal handle to the Vulkan object. */
			VkImage GetVulkanHandle() const { return mImage; }

			/** Assigns an name to the image, primarily used for easier debugging. */
			void SetName(const StringView& name);

			/** Returns true if the image can be read from a shader. If false, it may only be used as a framebuffer attachment. */
			bool IsShaderReadAllowed() const { return mIsShaderReadAllowed; }

			/**
			 * Returns an image view that covers all faces and mip maps of the texture.
			 *
			 * @param[in]	isPartOfFramebuffer	Set to true if the view will be used as a framebuffer attachment. Ensures proper
			 *									attachment flags are set on the view.
			 */
			VulkanImageView GetView(bool isPartOfFramebuffer) const;

			/**
			 * Returns an image view that covers the specified faces and mip maps of the texture.
			 *
			 * @param[in]	surface				Surface that describes which faces and mip levels to retrieve the view for.
			 * @param[in]	isPartOfFramebuffer	Set to true if the view will be used as a framebuffer attachment. Ensures proper
			 *									attachment flags are set on the view.
			 */
			VulkanImageView GetView(const TextureSurface& surface, bool isPartOfFramebuffer) const;

			/**
			 * Returns an image view with a specific format.
			 *
			 * @param[in]	format				Format to view the texture pixels as.
			 * @param[in]	isPartOfFramebuffer	Set to true if the view will be used as a framebuffer attachment. Ensures proper
			 *									attachment flags are set on the view.
			 */
			VulkanImageView GetView(VkFormat format, bool isPartOfFramebuffer = false) const;

			/**
			 * Returns an image view that covers the specified faces and mip maps of the texture, with a specific format.
			 *
			 * @param[in]	format		Format to view the texture pixels as.
			 * @param[in]	surface		Surface that describes which faces and mip levels to retrieve the view for.
			 * @param[in]	isPartOfFrameBuffer	Set to true if the view will be used as a framebuffer attachment. Ensures proper
			 *							attachment flags are set on the view.
			 */
			VulkanImageView GetView(VkFormat format, const TextureSurface& surface, bool isPartOfFrameBuffer) const;

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

			/** Returns a pointer to persistently mapped memory of the image, or null pointer if the image is not mappable. */
			void* GetMappedMemory() const { return mMappedMemory; }

			/**
			 * Returns a pointer to internal image memory for the specified sub-resource. Must be followed by Unmap(). Caller
			 * must ensure the image was created in CPU readable memory, and that image isn't currently being written to by the
			 * GPU.
			 *
			 * @param	face					Index of the face to map.
			 * @param	mipLevel				Index of the mip level to map.
			 * @param	output					Output object containing the pointer to the sub-resource data.
			 * @param	isInvalidateRequired	Ensures any GPU writes are made visible to the CPU before mapping. This is required for image
			 *									allocated in non-coherent memory and will be ignored for ones allocated in coherent memory.
			 */
			void Map(u32 face, u32 mipLevel, PixelData& output, bool isInvalidateRequired = false) const;

			/**
			 * Returns a pointer to internal image memory for the entire resource. Must be followed by Unmap(). Caller
			 * must ensure the image was created in CPU readable memory, and that image isn't currently being written to by the
			 * GPU.
			 *
			 * @param	offset					Offset into the allocation which to map from, in bytes.
			 * @param	size					Amount of bytes to map, starting with @p offset.
			 * @param	isInvalidateRequired	Ensures any GPU writes are made visible to the CPU before mapping. This is required for buffers
			 *									allocated in non-coherent memory and will be ignored for ones allocated in coherent memory.
			 */
			u8* Map(VkDeviceSize offset, VkDeviceSize size, bool isInvalidateRequired = false) const;

			/**
			 * Unmaps a buffer previously mapped with map().
			 *
			 * @param	isFlushRequired			Ensures any CPU writes are made visible to the GPU after unmapping. This is required for buffers
			 *									allocated in non-coherent memory and will be ignored for ones allocated in coherent memory.
			 */
			void Unmap(bool isFlushRequired = false);

			/** Flushes any CPU writes to the buffer to make them visible to the GPU. Only relevant for non-coherent memory. */
			void Flush(VkDeviceSize offset, VkDeviceSize size);

			/** Invalidates any GPU writes to the buffer to make them visible to the CPU. Only relevant for non-coherent memory. */
			void Invalidate(VkDeviceSize offset, VkDeviceSize size);

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
			 *
			 * @note	Submit thread only.
			 */
			void GetBarriers(const VkImageSubresourceRange& range, TArray<VkImageMemoryBarrier>& barriers);

			/** Returns the subresource layout (pitch values in bytes) for a specific image subresource. */
			VkSubresourceLayout GetSubresourceLayout(u32 face, u32 mipLevel) const;

			/** Converts a VkSubresourceLayout (which is in bytes) into blocks based on the provided format. */
			static ImageSubresourcePitch ConvertSubresourceLayoutToBlocks(const VkSubresourceLayout& subresourceLayout, PixelFormat format);

		private:
			/** Creates a new view of the provided part (or entirety) of surface. */
			VulkanImageView CreateView(const TextureSurface& surface, VkFormat format, VkImageAspectFlags aspectMask, bool isPartOfFramebuffer) const;

			/**
			 * If layer or mip count in the provided surface is set to zero, ensures they are set to the actual layer count.
			 * Set @p isPartOfFramebuffer to true if the surface is used as a framebuffer attachment. Returned surface is the
			 * same as @p surface, for convenience.
			 */
			const TextureSurface& CalculateExplicitSurface(TextureSurface& surface, bool isPartOfFramebuffer) const;

			/** Contains information about view for a specific surface(s) of this image. */
			struct ImageViewInformation
			{
				TextureSurface Surface;
				bool IsPartOfFramebuffer = false;
				VulkanImageView View;
				VkFormat Format = VK_FORMAT_UNDEFINED;
			};

			VkImage mImage;
			VmaAllocation mAllocation;
			VulkanImageView mMainView;
			VulkanImageView mFramebufferMainView;
			i32 mUsage;
			void* mMappedMemory = nullptr;
			bool mOwnsImage;
			bool mIsShaderReadAllowed = true;

			u32 mFaceCount;
			u32 mDepthSliceCount;
			u32 mMipLevelCount;
			VulkanImageSubresource** mSubresources;

			mutable VkImageViewCreateInfo mImageViewCI;
			mutable Vector<ImageViewInformation> mImageInfos;

			mutable VkDeviceSize mMappedOffset = 0;
			mutable VkDeviceSize mMappedSize = 0;
			mutable Mutex mViewsMutex;
		};

		/** Represents a single sub-resource (face & mip level) of a larger image object. */
		class VulkanImageSubresource : public VulkanResource
		{
		public:
			VulkanImageSubresource(VulkanResourceManager* owner, VkImageLayout layout, const StringView& name = "");

			/**
			 * Returns the layout the subresource is currently in. Note that this is only used to communicate layouts between
			 * different command buffers, and will only be updated only after command buffer submit. In short this means
			 * you should only care about this value on the submit thread.
			 *
			 * @note	Submit thread only.
			 */
			VkImageLayout GetLayout() const
			{
				AssertIfNotVulkanSubmitThread();

				return mLayout;
			}

			/**
			 * Notifies the resource that the current subresource layout has changed.
			 *
			 * @note	Submit thread only.
			 */
			void SetLayout(VkImageLayout layout)
			{
				AssertIfNotVulkanSubmitThread();

				mLayout = layout;
			}

		private:
			VkImageLayout mLayout;
		};

		/**	Vulkan implementation of a texture. */
		class VulkanTexture : public Texture
		{
		public:
			~VulkanTexture();

			/** Gets the resource wrapping the Vulkan image object. */
			VulkanImage* GetVulkanResource() const { return mImage; }

			/** Returns the internal format of the texture when used on the specified device. This may differ from the requested format if the device doesn't support it. */
			PixelFormat GetInternalFormat() const { return mInternalFormat; }

			void SetName(const StringView& name) override;
			void Flush(u32 face, u32 mipLevel) override;
			void Invalidate(u32 face, u32 mipLevel) override;

		protected:
			friend class VulkanGpuDevice;

			VulkanTexture(VulkanGpuDevice& gpuDevice, const TextureCreateInformation& createInformation);

			void Initialize() override;
			PixelData LockInternal(GpuLockOptions options, u32 mipLevel = 0, u32 face = 0) override;
			void UnlockInternal() override;
			void CopyInternal(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& target, const TextureCopyInformation& copyInformation) override;
			void BlitInternal(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& target, const TextureBlitInformation& blitInformation) override;
			TAsyncOp<SPtr<PixelData>> ReadDataAsync(GpuCommandBuffer& commandBuffer, u32 mipLevel = 0, u32 face = 0) override;
			void ReadDataInternal(PixelData& destination, u32 mipLevel = 0, u32 face = 0, const SPtr<GpuQueue>& gpuQueue = nullptr) override;
			void WriteDataInternal(const PixelData& source, u32 mipLevel = 0, u32 face = 0, bool discardWholeBuffer = false, const SPtr<GpuCommandBuffer>& commandBuffer = nullptr) override;

		private:
			/** Creates a new image for the specified device, matching the current properties. */
			VulkanImage* CreateImage(PixelFormat format);

			/**
			 * Creates a staging buffer that can be used for texture transfer operations.
			 *
			 * @param	pixelData	Object that describes the image sub-resource that will be in the buffer.
			 * @param	needsRead	True if we will be copying data from the buffer, false if just reading. True if both.
			 * @return					Newly allocated buffer.
			 */
			VulkanBuffer* CreateStaging(const PixelData& pixelData, bool needsRead);

			/**
			 * Copies all sub-resources from the source image to the destination image. Caller must ensure the images
			 * are of the same size. The operation will be queued on the provided command buffer. The system assumes the
			 * provided image matches the current texture properties (i.e. num faces, mips, size).
			 */
			void CopyImageToImage(VulkanGpuCommandBuffer& commandBuffer, VulkanImage* sourceImage, VulkanImage* destinationImage);

			/**
			 * Copies a single subresource from the source image into the destination buffer. Caller must ensure the destination buffer provides adequate
			 * space for the texture data. Set @p isBufferReadOnly to true if the CPU only needs to read from the destination buffer, or false if it also needs to write to it.
			 */
			void CopyImageSubresourceToBuffer(VulkanGpuCommandBuffer& commandBuffer, VulkanImage* sourceImage, u32 sourceFace, u32 sourceMipLevel, VulkanBuffer* destinationBuffer, bool isBufferReadOnly);

			/** Returns pitch information for a particular image subresource. */
			ImageSubresourcePitch GetPitchForSubresource(VulkanImage* image, u32 face, u32 mipLevel) const;

			VulkanGpuDevice& mGpuDevice;
			VulkanImage* mImage = nullptr;
			PixelFormat mInternalFormat = PF_UNKNOWN;

			VkImageCreateInfo mImageCreateInformation;
			bool mDirectlyMappable : 1;
			bool mSupportsGPUWrites : 1;
			bool mIsMapped : 1;
		};

		/** @} */
	} // namespace render
} // namespace b3d
