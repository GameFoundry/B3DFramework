//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Represents a single attachment in a Vulkan render pass. */
		struct VulkanRenderPassAttachmentCreateInformation
		{
			/** Determines if the attachment is used in the pass. */
			bool IsEnabled = false;

			/** Determines if the attachment can be read in a shader. */
			bool IsShaderReadAllowed = true;

			/** Format of the attached image. */
			VkFormat Format = VK_FORMAT_UNDEFINED;

			/** Calculates hash that determines if two render pass attachments are considered compatible. @see VulkanRenderPassCreateInformation::IsCompatible() */
			size_t CalculateCompatibilityHash(bool isDepth) const;

			/** Checks is the render pass attachment described by this information compatible with the provided render pass attachment information. @see VulkanRenderPassCreateInformation::IsCompatible() */
			bool IsCompatible(bool isDepth, const VulkanRenderPassAttachmentCreateInformation& other) const;
		};

		/** Contains parameters used for creating a VulkanRenderPass object. */
		struct VulkanRenderPassCreateInformation
		{
			/** Description of the color attachments, and their enabled states. */
			VulkanRenderPassAttachmentCreateInformation ColorAttachments[B3D_MAXIMUM_RENDER_TARGET_COUNT];

			/** Description of the depth attachment, and its enabled state. */
			VulkanRenderPassAttachmentCreateInformation DepthAttachment;

			/** Number of samples in the attachments. All attachments must have the same number of samples. */
			u32 SampleCount = 0;

			/** Set to true if render pass will be rendering to an offscreen surface that will not be presented. */
			bool IsOffscreenSurface = false;

			/** Calculates hash that determines if two render passes are considered compatible. @see IsCompatible() */
			size_t CalculateCompatibilityHash() const;

			/**
			 * Checks is the render pass described by this information compatible with the provided render pass information.
			 * Compatible render passes can be used for graphics pipelines and framebuffers created with other compatible render passes.
			 */
			bool IsCompatible(const VulkanRenderPassCreateInformation& other) const;
		};

		/**
		 * Wrapper around a Vulkan render pass. Currently sub-passes are not used, so different render passes just
		 * represent a different number of attachments and their layout transitions as well as load and store operations.
		 */
		class VulkanRenderPass
		{
		public:
			/**  Creates a new render pass as described by @p desc. */
			VulkanRenderPass(const VkDevice& device, const VulkanRenderPassCreateInformation& createInformation);
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
			VkRenderPass GetVkRenderPass(RenderSurfaceMask loadMask, RenderSurfaceMask readMask, RenderSurfaceMask clearMask) const;

			/**
			 * Returns the attachment descriptor for the specified color attachment. The attachment index is sequential in
			 * range [0, getNumColorAttachments()).
			 */
			const VkAttachmentDescription& GetColorAttachmentDescription(u32 index) const { return mAttachments[index]; }

			/**
			 * Returns the attachment descriptor for the depth attachment. Only valid if depth attachment was requested
			 * during render pass creation.
			 */
			const VkAttachmentDescription& GetDepthAttachmentDescription() const { return mAttachments[mColorAttachmentCount]; }

			/** Gets the total number of frame-buffer attachments, including both color and depth. */
			u32 GetAttachmentCount() const { return mAttachmentCount; }

			/** Gets the number of color frame-buffer attachments. */
			u32 GetColorAttachmentCount() const { return mColorAttachmentCount; }

			/** Returns the largest index of all the assigned color attachments. */
			u32 GetMaximumColorAttachmentIndex() const { return mMaximumColorAttachmentIndex; }

			/** Returns true if the framebuffer has a depth attachment. */
			bool HasDepthAttachment() const { return mHasDepthAttachment; }

			/** Returns sample flags that determine if the framebuffer supports multi-sampling, and for how many samples. */
			VkSampleCountFlagBits GetSampleFlags() const { return mSampleFlags; }

			/**
			 * Returns the maximum required number of clear entries to provide in a render pass start structure. This depends on
			 * the clear mask and the number of attachments.
			 */
			u32 GetClearEntryCount(RenderSurfaceMask clearMask) const;

		private:
			/** Key used for identifying different types of frame-buffer variants. */
			struct VariantKey
			{
				VariantKey(RenderSurfaceMask loadMask, RenderSurfaceMask readMask, RenderSurfaceMask clearMask);

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
				RenderSurfaceMask ClearMask;
			};

			/** Creates a new variant of the render pass. */
			VkRenderPass CreateVariant(RenderSurfaceMask loadMask, RenderSurfaceMask readMask, RenderSurfaceMask clearMask) const;

			u32 mId;
			u32 mAttachmentCount;
			u32 mColorAttachmentCount;
			u32 mMaximumColorAttachmentIndex = 0;
			u32 mColorAttachmentSequentialToAttachmentIndexMap[B3D_MAXIMUM_RENDER_TARGET_COUNT]{ 0 };
			std::array<bool, B3D_MAXIMUM_RENDER_TARGET_COUNT> mIsShaderReadAllowedForColorAttachment { false };
			bool mHasDepthAttachment;
			VkSampleCountFlagBits mSampleFlags = VK_SAMPLE_COUNT_1_BIT;
			VkDevice mDevice;

			mutable VkAttachmentDescription mAttachments[B3D_MAXIMUM_RENDER_TARGET_COUNT + 1];
			mutable VkAttachmentReference mColorReferences[B3D_MAXIMUM_RENDER_TARGET_COUNT];
			mutable VkAttachmentReference mDepthReference;
			mutable VkSubpassDescription mSubpassDescription;
			mutable VkSubpassDependency mDependencies[2];
			mutable VkRenderPassCreateInfo mRenderPassCI;

			VkRenderPass mDefault;
			mutable UnorderedMap<VariantKey, VkRenderPass, VariantKey::HashFunction, VariantKey::EqualFunction> mVariants;
			mutable Mutex mMutex;

			static u32 sNextValidId;
		};

		/** Manages creation and caching of render passes. */
		class VulkanRenderPassCache : public Module<VulkanRenderPassCache>
		{
		public:
			~VulkanRenderPassCache();

			/**
			 * Returns an existing matching render pass or creates a new one with specified settings.
			 *
			 * @param[in]	device		Device to create the render pass on.
			 * @param[in]	desc		Descriptor describing the requested pass.
			 * @return					Brand new render pass, or an existing one if one was found matching the descriptor.
			 */
			VulkanRenderPass* FindOrCreateRenderPass(const VkDevice& device, const VulkanRenderPassCreateInformation& desc);

		private:
			/** Key used for identifying different types of frame-buffer variants. */
			struct VariantKey
			{
				VariantKey(const VkDevice& device, const VulkanRenderPassCreateInformation& createInformation);

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
				VulkanRenderPassCreateInformation CreateInformation;
			};

			mutable Mutex mMutex;
			mutable UnorderedMap<VariantKey, VulkanRenderPass*, VariantKey::HashFunction, VariantKey::EqualFunction> mVariants;
		};

		/** @} */
	} // namespace render
} // namespace b3d
