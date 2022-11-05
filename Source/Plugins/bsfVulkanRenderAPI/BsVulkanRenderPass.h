//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Represents a single attachment in a Vulkan render pass. */
		struct VULKAN_RENDER_PASS_ATTACHMENT_DESC
		{
			/** Determines if the attachment is used in the pass. */
			bool Enabled = false;

			/** Format of the attached image. */
			VkFormat Format = VK_FORMAT_UNDEFINED;
		};

		/** Contains parameters used for initializing VulkanRenderPass object. */
		struct VULKAN_RENDER_PASS_DESC
		{
			/** Description of the color attachments, and their enabled states. */
			VULKAN_RENDER_PASS_ATTACHMENT_DESC Color[B3D_MAXIMUM_RENDER_TARGET_COUNT];

			/** Description of the depth attachment, and its enabled state. */
			VULKAN_RENDER_PASS_ATTACHMENT_DESC Depth;

			/** Number of samples in the attachments. All attachments must have the same number of samples. */
			u32 NumSamples = 0;

			/** Set to true if render pass will be rendering to an offscreen surface that will not be presented. */
			bool Offscreen = false;
		};

		/**
		 * Wrapper around a Vulkan render pass. Currently sub-passes are not used, so different render passes just
		 * represent a different number of attachments and their layout transitions as well as load and store operations.
		 */
		class VulkanRenderPass
		{
		public:
			/**  Creates a new render pass as described by @p desc. */
			VulkanRenderPass(const VkDevice& device, const VULKAN_RENDER_PASS_DESC& desc);
			~VulkanRenderPass();

			/** Returns a unique ID of this render pass. */
			u32 GetId() const { return mId; }

			/**
			 * Gets internal Vulkan render pass object.
			 *
			 * @param[in]	loadMask	Mask that control which render target surface contents should be preserved on load.
			 * @param[in]	readMask	Mask that controls which render targets can be read by shaders while they're bound.
			 * @param[in]	clearMask	Mask that controls which render targets should be cleared on render pass start. Target
			 *							cannot have both load and clear bits set. If load bit is set, clear will be ignored.
			 */
			VkRenderPass GetVkRenderPass(RenderSurfaceMask loadMask, RenderSurfaceMask readMask, ClearMask clearMask) const;

			/**
			 * Returns the attachment descriptor for the specified color attachment. The attachment index is sequential in
			 * range [0, getNumColorAttachments()).
			 */
			const VkAttachmentDescription& GetColorDesc(u32 idx) const { return mAttachments[idx]; }

			/**
			 * Returns the attachment descriptor for the depth attachment. Only valid if depth attachment was requested
			 * during render pass creation.
			 */
			const VkAttachmentDescription& GetDepthDesc() const { return mAttachments[mNumColorAttachments]; }

			/** Gets the total number of frame-buffer attachments, including both color and depth. */
			u32 GetNumAttachments() const { return mNumAttachments; }

			/** Gets the number of color frame-buffer attachments. */
			u32 GetNumColorAttachments() const { return mNumColorAttachments; }

			/** Returns true if the framebuffer has a depth attachment. */
			bool HasDepthAttachment() const { return mHasDepth; }

			/** Returns sample flags that determine if the framebuffer supports multi-sampling, and for how many samples. */
			VkSampleCountFlagBits GetSampleFlags() const { return mSampleFlags; }

			/**
			 * Returns the maximum required number of clear entries to provide in a render pass start structure. This depends on
			 * the clear mask and the number of attachments.
			 */
			u32 GetNumClearEntries(ClearMask clearMask) const;

		private:
			/** Key used for identifying different types of frame-buffer variants. */
			struct VariantKey
			{
				VariantKey(RenderSurfaceMask loadMask, RenderSurfaceMask readMask, ClearMask clearMask);

				class HashFunction
				{
				public:
					size_t operator()(const VariantKey& key) const;
				};

				class EqualFunction
				{
				public:
					bool operator()(const VariantKey& lhs, const VariantKey& rhs) const;
				};

				RenderSurfaceMask LoadMask;
				RenderSurfaceMask ReadMask;
				ClearMask ClearMask;
			};

			/** Creates a new variant of the render pass. */
			VkRenderPass CreateVariant(RenderSurfaceMask loadMask, RenderSurfaceMask readMask, ClearMask clearMask) const;

			u32 mId;
			u32 mNumAttachments;
			u32 mNumColorAttachments;
			u32 mIndices[B3D_MAXIMUM_RENDER_TARGET_COUNT]{ 0 };
			bool mHasDepth;
			VkSampleCountFlagBits mSampleFlags = VK_SAMPLE_COUNT_1_BIT;
			VkDevice mDevice;

			mutable VkAttachmentDescription mAttachments[B3D_MAXIMUM_RENDER_TARGET_COUNT + 1];
			mutable VkAttachmentReference mColorReferences[B3D_MAXIMUM_RENDER_TARGET_COUNT];
			mutable VkAttachmentReference mDepthReference;
			mutable VkSubpassDescription mSubpassDesc;
			mutable VkSubpassDependency mDependencies[2];
			mutable VkRenderPassCreateInfo mRenderPassCI;

			VkRenderPass mDefault;
			mutable UnorderedMap<VariantKey, VkRenderPass, VariantKey::HashFunction, VariantKey::EqualFunction> mVariants;

			static u32 sNextValidId;
		};

		/** Manages creation and caching of render passes. */
		class VulkanRenderPasses : public Module<VulkanRenderPasses>
		{
		public:
			~VulkanRenderPasses();

			/**
			 * Returns an existing matching render pass or creates a new one with specified settings.
			 *
			 * @param[in]	device		Device to create the render pass on.
			 * @param[in]	desc		Descriptor describing the requested pass.
			 * @return					Brand new render pass, or an existing one if one was found matching the descriptor.
			 */
			VulkanRenderPass* Get(const VkDevice& device, const VULKAN_RENDER_PASS_DESC& desc);

		private:
			/** Key used for identifying different types of frame-buffer variants. */
			struct VariantKey
			{
				VariantKey(const VkDevice& device, const VULKAN_RENDER_PASS_DESC& desc);

				class HashFunction
				{
				public:
					size_t operator()(const VariantKey& key) const;
				};

				class EqualFunction
				{
				public:
					bool operator()(const VariantKey& lhs, const VariantKey& rhs) const;
				};

				VkDevice Device = VK_NULL_HANDLE;
				VULKAN_RENDER_PASS_DESC Desc;
			};

			mutable Mutex mMutex;
			mutable UnorderedMap<VariantKey, VulkanRenderPass*, VariantKey::HashFunction, VariantKey::EqualFunction> mVariants;
		};

		/** @} */
	} // namespace ct
} // namespace bs
