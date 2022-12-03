//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsRenderAPI.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Implementation of a render system using Vulkan. Provides abstracted access to various low level Vulkan methods. */
		class VulkanRenderAPI : public RenderAPI
		{
		public:
			VulkanRenderAPI();
			~VulkanRenderAPI() = default;

			const StringID& GetName() const override;
			void SetGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetComputePipeline(const SPtr<ComputePipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetGpuParams(const SPtr<GpuParams>& gpuParams, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void ClearRenderTarget(u32 buffers, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void ClearViewport(u32 buffers, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags = 0, RenderSurfaceMask loadMask = RT_NONE, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetViewport(const Rect2& area, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetScissorRect(u32 left, u32 top, u32 right, u32 bottom, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetStencilRef(u32 value, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetVertexBuffers(u32 index, SPtr<VertexBuffer>* buffers, u32 numBuffers, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetIndexBuffer(const SPtr<IndexBuffer>& buffer, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void DispatchCompute(u32 numGroupsX, u32 numGroupsY = 1, u32 numGroupsZ = 1, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SwapBuffers(const SPtr<RenderTarget>& target, u32 syncMask = 0xFFFFFFFF) override;
			void AddCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary) override;
			void BeginLabel(const StringView& name, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void EndLabel(const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void InsertLabel(const StringView& name, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SubmitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, u32 syncMask = 0xFFFFFFFF) override;
			SPtr<CommandBuffer> GetMainCommandBuffer() const override;
			void ConvertProjectionMatrix(const Matrix4& matrix, Matrix4& dest) override;
			GpuParamBlockDesc GenerateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params) override;

			/**
			 * @name Internal
			 * @{
			 */

			/** Returns the internal Vulkan instance object. */
			VkInstance GetInstance() const { return mInstance; }

			/** Returns a Vulkan device at the specified index. Must be in range [0, GetNumDevicesInternal()) */
			SPtr<VulkanDevice> GetDevice(u32 idx) const { return mDevices[idx]; }

			/** Returns the primary device that supports swap chain present operations. */
			const SPtr<VulkanDevice>& GetPresentDevice() const { return mPrimaryDevices[0]; }

			/** Gets the total number of Vulkan compatible devices available on this system. */
			u32 GetDeviceCount() const { return (u32)mDevices.size(); }

			/**
			 * Returns one or multiple devices recognized as primary. This will be a single device in most cases, or multiple
			 * devices if using some kind of a supported multi-GPU setup.
			 */
			const Vector<SPtr<VulkanDevice>> GetPrimaryDevicesInternal() const { return mPrimaryDevices; }

			/** Returns the main command buffer, executing on the graphics queue. */
			VulkanCommandBuffer* GetMainVulkanCommandBuffer() const { return mMainCommandBuffer.get(); }

			/** @} */
		protected:
			friend class VulkanRenderAPIFactory;

			void Initialize() override;
			void DestroyCore() override;

			/** Creates and populates a set of render system capabilities describing which functionality is available. */
			void InitCapabilites();

			/**
			 * Returns a valid command buffer. Uses the provided buffer if not null. Otherwise returns the default command
			 * buffer.
			 */
			VulkanCommandBuffer* GetCb(const SPtr<CommandBuffer>& buffer);

		private:
			VkInstance mInstance = nullptr;

			Vector<SPtr<VulkanDevice>> mDevices;
			Vector<SPtr<VulkanDevice>> mPrimaryDevices;

			SPtr<VulkanCommandBuffer> mMainCommandBuffer;
			Vector<SPtr<CommandBuffer>> mSubmittedCommandBuffers;

			VulkanGLSLProgramFactory* mGLSLFactory;

			VkDebugReportCallbackEXT mDebugReportCallback;
			VkDebugUtilsMessengerEXT mDebugUtilsMessenger;
		};

		/**	Provides easy access to the VulkanRenderAPI. */
		VulkanRenderAPI& GetVulkanRenderAPI();

		extern PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
		extern PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;
		extern PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT;
		extern PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;

		extern PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
		extern PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
		extern PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
		extern PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;

		extern PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
		extern PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
		extern PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
		extern PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
		extern PFN_vkQueuePresentKHR vkQueuePresentKHR;

		/** @} */
	} // namespace ct
} // namespace bs
