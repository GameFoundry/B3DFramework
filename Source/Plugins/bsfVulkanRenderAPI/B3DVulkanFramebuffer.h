//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanRenderPass.h"
#include "B3DVulkanResource.h"

namespace b3d
{
	namespace render
	{
		class VulkanRenderPass;

		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Represents a single attachment in a Vulkan frame-buffer. */
		struct VulkanFramebufferAttachmentInformation
		{
			/** Image to attach or null if none. */
			VulkanImage* Image = nullptr;

			/** Surface representing the sub-resource of the image to use as an attachment. */
			TextureSurface Surface;

			/** Initial layer of the surface as pointed to by the provided image view. */
			u32 BaseLayer = 0;
		};

		/** Contains parameters used for initializing VulkanFrameBuffer object. */
		struct VulkanFramebufferInformation
		{
			/** Images describing the color attachments. */
			VulkanFramebufferAttachmentInformation Color[B3D_MAXIMUM_RENDER_TARGET_COUNT];

			/** Image describing the depth attachment. */
			VulkanFramebufferAttachmentInformation Depth;

			/** Width of the images, in pixels. All images must be the same size. */
			u32 Width = 0;

			/** Height of the images, in pixels. All images must be the same size. */
			u32 Height = 0;

			/** Number of image layers to render to. This value is used for all provided surfaces. */
			u32 Layers = 0;
		};

		/** Information about a single framebuffer attachment. */
		struct VulkanFramebufferAttachment
		{
			VulkanImage* Image = nullptr;
			TextureSurface Surface;
			u32 BaseLayer = 0;
			VkImageLayout FinalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			u32 Index = 0;
		};

		/** Vulkan frame buffer containing one or multiple color surfaces, and an optional depth surface. */
		class VulkanFramebuffer : public VulkanResource
		{
		public:
			/** Creates a new frame buffer with the specified image views attached.
			 *
			 * @param	owner		Resource manager that allocated this resource.
			 * @param	renderPass	Render pass that will be used for rendering to the frame buffer. Note that the
			 *						framebuffer will be usable with this specific render pass, but also with any compatible
			 *						render pass. Render passes are compatible if they use the same attachments and their
			 *						formats and sample counts match.
			 * @param	desc		Description of the frame buffer.
			 * @param	name		Optional name of the resource, for debugging purposes.
			 */
			VulkanFramebuffer(VulkanResourceManager* owner, VulkanRenderPass* renderPass, const VulkanFramebufferInformation& desc, const StringView& name = "");
			~VulkanFramebuffer();

			/** Returns a unique ID of this framebuffer. */
			u32 GetId() const { return mId; }

			/** Returns the width of the framebuffer, in pixels. */
			u32 GetWidth() const { return mWidth; }

			/** Returns the height of the framebuffer, in pixels. */
			u32 GetHeight() const { return mHeight; }

			/** Gets the internal Vulkan framebuffer object. */
			VkFramebuffer GetVulkanHandle() const { return mVkFramebuffer; }

			/** Returns the render pass that this framebuffer is tied to. */
			VulkanRenderPass* GetRenderPass() const { return mRenderPass; }

			/**
			 * Gets the number of layers in each framebuffer surface. A layer is an element in a texture array, or a depth
			 * slice in a 3D texture).
			 */
			u32 GetLayerCount() const { return mNumLayers; }

			/** Returns information about a color attachment at the specified index. */
			const VulkanFramebufferAttachment& GetColorAttachment(u32 colorIdx) const { return mColorAttachments[colorIdx]; }

			/** Returns information about a depth-stencil attachment. */
			const VulkanFramebufferAttachment& GetDepthStencilAttachment() const { return mDepthStencilAttachment; }

		private:
			u32 mId;
			VkFramebuffer mVkFramebuffer;
			VulkanRenderPass* mRenderPass;

			u32 mWidth;
			u32 mHeight;
			u32 mNumLayers;
			VulkanFramebufferAttachment mColorAttachments[B3D_MAXIMUM_RENDER_TARGET_COUNT]{};
			VulkanFramebufferAttachment mDepthStencilAttachment;

			static u32 sNextValidId;
		};

		/** Creates and caches framebuffers. */
		class VulkanFramebufferCache : public Module<VulkanFramebufferCache>
		{
		public:
			~VulkanFramebufferCache();

			/**
			 * Searches for a framebuffer matching the provided framebuffer information, that is also compatible with the provided render pass information. If one
			 * cannot be found a new one is created, added to the internal cache, and returned.
			 *
			 * @note	Thread safe.
			 *
			 * @param	device						Device to create the framebuffer on.
			 * @param	framebufferInformation		Information about the framebuffer.
			 * @param	renderPassInformation		Information about the render pass the framebuffer will be used with.
			 * @return								Existing cached framebuffer, or a brand new framebuffer.
			 */
			VulkanFramebuffer* FindOrCreateFramebuffer(const VulkanGpuDevice& device, const VulkanFramebufferInformation& framebufferInformation, const VulkanRenderPassCreateInformation& renderPassInformation);

			/**
			 * Notifies the system that an image was destroyed. If any framebuffer is using the image, their cache entries will be invalidated.
			 *
			 * @note	Thread safe.
			 */
			void NotifyImageDestroyed(const VulkanImage& image);

		private:
			struct FramebufferVariantKey
			{
				FramebufferVariantKey(const VkDevice& device, const VulkanFramebufferInformation& framebufferInformation, const VulkanRenderPassCreateInformation& renderPassInformation);

				class HashFunction
				{
				public:
					size_t operator()(const FramebufferVariantKey& key) const;
				};

				class EqualFunction
				{
				public:
					bool operator()(const FramebufferVariantKey& lhs, const FramebufferVariantKey& rhs) const;
				};

				VkDevice Device = VK_NULL_HANDLE;
				VulkanFramebufferInformation FramebufferInformation;
				VulkanRenderPassCreateInformation RenderPassInformation;
			};

			mutable Mutex mMutex;
			mutable UnorderedMap<FramebufferVariantKey, VulkanFramebuffer*, FramebufferVariantKey::HashFunction, FramebufferVariantKey::EqualFunction> mCache;
		};

		/** @} */
	} // namespace render
} // namespace b3d
