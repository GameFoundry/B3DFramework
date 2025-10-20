//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DFrameGraphTypes.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DRenderTarget.h"

namespace b3d::render
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Describes how a resource is accessed in a pass.
	 *
	 * This combines the resource ID with usage and access information to describe
	 * exactly how a pass interacts with a resource. The frame graph uses this information
	 * to validate correct usage and (in later phases) to calculate synchronization barriers.
	 */
	struct FrameGraphResourceAccess
	{
		FrameGraphResourceId Resource; /**< Resource being accessed */
		GpuResourceUseFlags Usage; /**< How the resource is used (e.g., ColorAttachment, ShaderAccess) */
		GpuAccessFlags Access; /**< Whether the resource is read, written, or both */

		FrameGraphResourceAccess(
			FrameGraphResourceId resource,
			GpuResourceUseFlags usage,
			GpuAccessFlags access)
			: Resource(resource), Usage(usage), Access(access)
		{}
	};

	/**
	 * Function type for pass setup callback.
	 *
	 * The setup function is called during compilation to declare resource dependencies.
	 * Use the provided FrameGraphPass reference to call Read(), Write(), ReadWrite(), or
	 * CreateRenderTarget().
	 */
	using FrameGraphPassSetupFunc = std::function<void(class FrameGraphPass&)>;

	/**
	 * Function type for pass execution callback.
	 *
	 * The execute function is called during frame graph execution to record GPU commands.
	 * Use the provided GpuCommandBuffer to record rendering or compute commands.
	 */
	using FrameGraphPassExecuteFunc = std::function<void(GpuCommandBuffer&)>;

	class FrameGraph;

	/**
	 * Represents a single pass in the frame graph.
	 *
	 * A pass represents a unit of GPU work (rendering, compute, or transfer) that operates on
	 * a set of resources. Passes are created via FrameGraph::DeclarePass() and go through two phases:
	 *
	 * Setup Phase (during Compile()):
	 * - Setup callback is invoked
	 * - Resource dependencies are declared via Read(), Write(), ReadWrite()
	 * - Optionally a render target is created via CreateRenderTarget()
	 * - Dependencies are validated
	 *
	 * Execution Phase (during Execute()):
	 * - Execute callback is invoked with a GpuCommandBuffer
	 * - User records GPU commands (SetPipeline, Draw, Dispatch, etc.)
	 * - Commands are submitted to the appropriate GPU queue
	 *
	 * Phase 1 Note: Passes execute in declaration order with no automatic synchronization.
	 * Phase 2+ will add dependency analysis and optimal execution ordering.
	 *
	 * @note
	 * All FrameGraphPass instances are owned and managed by the FrameGraph.
	 * Do not store pointers to passes beyond the scope of setup/execute callbacks.
	 */
	class B3D_EXPORT FrameGraphPass
	{
	public:
		FrameGraphPass(
			u32 index,
			const StringView& name,
			GpuQueueUsage queue,
			FrameGraph* frameGraph);

		/** Returns the pass index (assigned during creation) */
		u32 GetIndex() const { return mIndex; }

		/** Returns the pass name */
		const String& GetName() const { return mName; }

		/** Returns the queue this pass executes on */
		GpuQueueUsage GetQueue() const { return mQueue; }

		/**
		 * Declares that this pass will read from a resource.
		 *
		 * @param resource  Resource to read from
		 * @param usage     How the resource is used (e.g., ShaderAccess)
		 */
		void Read(FrameGraphResourceId resource, GpuResourceUseFlags usage);

		/**
		 * Declares that this pass will write to a resource.
		 *
		 * @param resource  Resource to write to
		 * @param usage     How the resource is used (e.g., ColorAttachment)
		 */
		void Write(FrameGraphResourceId resource, GpuResourceUseFlags usage);

		/**
		 * Declares that this pass will both read and write a resource.
		 *
		 * @param resource  Resource to access
		 * @param usage     How the resource is used (e.g., StorageTexture for UAV)
		 */
		void ReadWrite(FrameGraphResourceId resource, GpuResourceUseFlags usage);

		/**
		 * Creates a render target from the specified attachments.
		 *
		 * This method automatically declares Write access to all specified attachments and creates
		 * a RenderTarget that can be used with BeginRenderPass(). The render target is cached for
		 * the duration of the frame and released during Reset().
		 *
		 * @param colorAttachments      Array of color attachment resource IDs (must be textures)
		 * @param colorAttachmentCount  Number of color attachments (1 to B3D_MAXIMUM_RENDER_TARGET_COUNT)
		 * @param depthAttachment       Depth/stencil attachment resource ID (must be texture, or kInvalidFrameGraphResourceId)
		 * @return                      The created render target, or nullptr if creation failed
		 *
		 * @note This method must be called during the setup phase (inside the setup callback).
		 * @note All attachment resources must be imported textures with appropriate formats.
		 */
		SPtr<RenderTarget> CreateRenderTarget(// TODO - Don't split parameters over multiple lines unless it's a very long line
			FrameGraphResourceId* colorAttachments, // TODO - This should be TArrayView
			u32 colorAttachmentCount,
			FrameGraphResourceId depthAttachment = kInvalidFrameGraphResourceId);

		/** Returns all declared resource accesses */
		const Vector<FrameGraphResourceAccess>& GetResourceAccesses() const
		{
			return mResourceAccesses;
		}

		/** Sets the setup function (internal) */
		void SetSetupFunction(FrameGraphPassSetupFunc setupFunc)
		{
			mSetupFunc = std::move(setupFunc);
		}

		/** Sets the execute function (internal) */
		void SetExecuteFunction(FrameGraphPassExecuteFunc executeFunc)
		{
			mExecuteFunc = std::move(executeFunc);
		}

		/** Executes the setup function (internal) */
		void ExecuteSetup();

		/** Executes the command recording function (internal) */
		void ExecuteCommands(GpuCommandBuffer& commandBuffer);

		/** Resets the pass for reuse (internal) */
		void Reset();

	private:
		u32 mIndex;
		String mName;
		GpuQueueUsage mQueue;
		FrameGraph* mFrameGraph;
		Vector<FrameGraphResourceAccess> mResourceAccesses;
		FrameGraphPassSetupFunc mSetupFunc; // TODO - Func -> Function
		FrameGraphPassExecuteFunc mExecuteFunc; // TODO - Func -> Function
		SPtr<RenderTarget> mRenderTarget; // Cached render target created by CreateRenderTarget()
	};

	/** @} */
}
