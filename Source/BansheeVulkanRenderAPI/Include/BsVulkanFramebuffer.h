//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Represents a single attachment in a Vulkan frame-buffer. */
	struct VULKAN_ATTACHMENT_DESC
	{
		/** Image to attach or null if none. */
		VulkanImage* image;

		/** View of the image to attach or VK_NULL_HANDLE if none. */
		VkImageView view;

		/** Format of the attached image. */
		VkFormat format;

		/** Initial layer of the surface as pointed to by the provided image view. */
		UINT32 baseLayer;
	};

	/** Contains parameters used for initializing VulkanFrameBuffer object. */
	struct VULKAN_FRAMEBUFFER_DESC
	{
		/** Images describing the color attachments. */
		VULKAN_ATTACHMENT_DESC color[BS_MAX_MULTIPLE_RENDER_TARGETS];

		/** Image describing the depth attachment. */
		VULKAN_ATTACHMENT_DESC depth;

		/** Width of the images, in pixels. All images must be the same size. */
		UINT32 width;

		/** Height of the images, in pixels. All images must be the same size. */
		UINT32 height;

		/** Number of image layers to render to. This value is used for all provided surfaces. */
		UINT32 layers;

		/** Number of samples in the attached images. All attachments must have the same number of samples. */
		UINT32 numSamples;

		/** Set to true if framebuffer represents an offscreen surface that will not be presented. */
		bool offscreen;
	};

	/** Vulkan frame buffer containing one or multiple color surfaces, and an optional depth surface. */
	class VulkanFramebuffer : public VulkanResource
	{
	public:
		/** Creates a new frame buffer with the specified image views attached. 
		 *
		 * @param[in]	owner	Resource manager that allocated this resource.
		 * @param[in]	desc	Description of the frame buffer.
		 */
		VulkanFramebuffer(VulkanResourceManager* owner, const VULKAN_FRAMEBUFFER_DESC& desc);
		~VulkanFramebuffer();

		/** Returns a unique ID of this framebuffer. */
		UINT32 getId() const { return mId; }

		/** 
		 * Gets internal Vulkan render pass object. 
		 * 
		 * @param[in]	preserveContents	If true, returns render pass that preserves existing framebuffer attachment 
		 *									contents on load. Otherwise returns render pass that discards them (more 
		 *									efficient).
		 */
		VkRenderPass getRenderPass(bool preserveContents) const;

		/**
		 * Gets internal Vulkan framebuffer object.
		 *
		 * @param[in]	preserveContents	If true, returns render pass that preserves existing framebuffer attachment
		 *									contents on load. Otherwise returns render pass that discards them (more
		 *									efficient).
		 */
		VkFramebuffer getFramebuffer(bool preserveContents) const;

		/** 
		 * Gets the number of layers in each framebuffer surface. A layer is an element in a texture array, or a depth 
		 * slice in a 3D texture).
		 */
		UINT32 getNumLayers() const { return mNumLayers; }

		/** Returns the image representing the color attachment at the provided index. */
		VulkanImage* getColorImage(UINT32 colorIdx) const { return mColorImages[colorIdx]; }

		/** Returns the image representing the depth/stencil attachment, if one exists. */
		VulkanImage* getDepthStencilImage() const { return mDepthStencilImage; }

		/** Returns the initial layer of the color texture surface in which to start rendering. */
		UINT32 getColorBaseLayer(UINT32 colorIdx) const { return mColorBaseLayers[colorIdx]; }

		/** Returns the initial layer of the depth-stencil texture surface in which to start rendering. */
		UINT32 getDepthStencilBaseLayer() const { return mDepthBaseLayer; }

		/** Gets the total number of frame-buffer attachments, including both color and depth. */
		UINT32 getNumAttachments() const { return mNumAttachments; }

		/** Gets the number of color frame-buffer attachments. */
		UINT32 getNumColorAttachments() const { return mNumColorAttachments; }

		/** Returns true if the framebuffer has a depth attachment. */
		bool hasDepthAttachment() const { return mHasDepth; }

		/** Returns sample flags that determine if the framebuffer supports multi-sampling, and for how many samples. */
		VkSampleCountFlagBits getSampleFlags() const { return mSampleFlags; }
	private:
		UINT32 mId;
		VkRenderPass mRenderPassDiscard;
		VkFramebuffer mFramebufferDiscard;

		VkRenderPass mRenderPassPreserve;
		VkFramebuffer mFramebufferPreserve;

		UINT32 mNumAttachments;
		UINT32 mNumColorAttachments;
		UINT32 mNumLayers;
		VulkanImage* mColorImages[BS_MAX_MULTIPLE_RENDER_TARGETS];
		VulkanImage* mDepthStencilImage;
		UINT32 mColorBaseLayers[BS_MAX_MULTIPLE_RENDER_TARGETS];
		UINT32 mDepthBaseLayer;
		bool mHasDepth;
		VkSampleCountFlagBits mSampleFlags;

		static UINT32 sNextValidId;
	};

	/** @} */
}
