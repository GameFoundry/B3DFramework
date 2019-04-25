//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsRenderAPI.h"

namespace bs { namespace ct
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

		/** @copydoc RenderAPI::getName */
		const StringID& getName() const override;
		
		/** @copydoc RenderAPI::setGraphicsPipeline */
		void setGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setComputePipeline */
		void setComputePipeline(const SPtr<ComputePipelineState>& pipelineState,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setGpuParams */
		void setGpuParams(const SPtr<GpuParams>& gpuParams, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::clearRenderTarget */
		void clearRenderTarget(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0, 
			UINT8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::clearViewport */
		void clearViewport(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0,
			UINT8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setRenderTarget */
		void setRenderTarget(const SPtr<RenderTarget>& target, UINT32 readOnlyFlags = 0,
			RenderSurfaceMask loadMask = RT_NONE, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setViewport */
		void setViewport(const Rect2& area, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setScissorRect */
		void setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setStencilRef */
		void setStencilRef(UINT32 value, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setVertexBuffers */
		void setVertexBuffers(UINT32 index, SPtr<VertexBuffer>* buffers, UINT32 numBuffers,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setIndexBuffer */
		void setIndexBuffer(const SPtr<IndexBuffer>& buffer, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setVertexDeclaration */
		void setVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setDrawOperation */
		void setDrawOperation(DrawOperationType op,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::draw */
		void draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount = 0,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::drawIndexed */
		void drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount, 
			UINT32 instanceCount = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::dispatchCompute */
		void dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY = 1, UINT32 numGroupsZ = 1,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::swapBuffers() */
		void swapBuffers(const SPtr<RenderTarget>& target, UINT32 syncMask = 0xFFFFFFFF) override;

		/** @copydoc RenderAPI::addCommands() */
		void addCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary) override;

		/** @copydoc RenderAPI::submitCommandBuffer() */
		void submitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, UINT32 syncMask = 0xFFFFFFFF) override;

		/** @copydoc RenderAPI::convertProjectionMatrix */
		void convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest) override;

		/** @copydoc RenderAPI::generateParamBlockDesc() */
		GpuParamBlockDesc generateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params) override;

		/**
		 * @name Internal
		 * @{
		 */

		/** Returns the internal Vulkan instance object. */
		VkInstance _getInstance() const { return mInstance; }

		/** Returns a Vulkan device at the specified index. Must be in range [0, _getNumDevices()) */
		SPtr<VulkanDevice> _getDevice(UINT32 idx) const { return mDevices[idx]; }

		/** Returns the primary device that supports swap chain present operations. */
		const SPtr<VulkanDevice>& _getPresentDevice() const { return mPrimaryDevices[0]; }

		/** Gets the total number of Vulkan compatible devices available on this system. */
		UINT32 _getNumDevices() const { return (UINT32)mDevices.size(); }

		/** 
		 * Returns one or multiple devices recognized as primary. This will be a single device in most cases, or multiple
		 * devices if using some kind of a supported multi-GPU setup.
		 */
		const Vector<SPtr<VulkanDevice>> _getPrimaryDevices() const { return mPrimaryDevices; }

		/** Returns the main command buffer, executing on the graphics queue. */
		VulkanCommandBuffer* _getMainCommandBuffer() const { return mMainCommandBuffer.get(); }

		/** @} */
	protected:
		friend class VulkanRenderAPIFactory;

		/** @copydoc RenderAPI::initialize */
		void initialize() override;

		/** @copydoc RenderAPI::destroyCore */
		void destroyCore() override;

		/** Creates and populates a set of render system capabilities describing which functionality is available. */
		void initCapabilites();

		/** 
		 * Returns a valid command buffer. Uses the provided buffer if not null. Otherwise returns the default command 
		 * buffer. 
		 */
		VulkanCommandBuffer* getCB(const SPtr<CommandBuffer>& buffer);

	private:
		VkInstance mInstance = nullptr;

		Vector<SPtr<VulkanDevice>> mDevices;
		Vector<SPtr<VulkanDevice>> mPrimaryDevices;
		SPtr<VulkanCommandBuffer> mMainCommandBuffer;

		VulkanGLSLProgramFactory* mGLSLFactory;

#if BS_DEBUG_MODE
		VkDebugReportCallbackEXT mDebugCallback;
#endif
	};

	/**	Provides easy access to the VulkanRenderAPI. */
	VulkanRenderAPI& gVulkanRenderAPI();

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
}}