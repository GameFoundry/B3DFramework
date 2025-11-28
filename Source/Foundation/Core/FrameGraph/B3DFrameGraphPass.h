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
	 * to validate correct usage and to calculate synchronization barriers.
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
	 * Use the provided FrameGraphPass reference to call Read(), Write(), or ReadWrite().
	 */
	using FrameGraphPassSetupFunc = std::function<void(class FrameGraphPass&)>;

	/**
	 * Function type for pass execution callback.
	 *
	 * The execute function is called during frame graph execution to record GPU commands.
	 * Use the provided GpuCommandBuffer to record rendering or compute commands.
	 * Use the provided FrameGraphPassResources to retrieve allocated textures and buffers
	 * (both imported and transient resources).
	 */
	using FrameGraphPassExecuteFunc = std::function<void(GpuCommandBuffer&, FrameGraphPassResources&)>;

	class FrameGraph;

	/**
	 * Represents a dependency edge between two passes in the frame graph.
	 * Dependencies are determined by resource usage (read-after-write, write-after-read, write-after-write).
	 */
	struct FrameGraphPassDependency
	{
		/** The pass that must execute before this dependency is satisfied */
		FrameGraphPass* ProducerPass = nullptr;

		/** The pass that depends on the producer pass */
		FrameGraphPass* ConsumerPass = nullptr;

		/** The resource that creates this dependency */
		FrameGraphResourceId Resource;

		/** Type of dependency (read-after-write, write-after-read, write-after-write) */
		enum class Type
		{
			/** Consumer reads after producer writes (true dependency) */
			ReadAfterWrite,

			/** Consumer writes after producer reads (anti-dependency) */
			WriteAfterRead,

			/** Consumer writes after producer writes (output dependency) */
			WriteAfterWrite
		};

		Type DependencyType = Type::ReadAfterWrite;
	};

	/**
	 * Represents a single pass in the frame graph.
	 *
	 * A pass represents a unit of GPU work (rendering, compute, or transfer) that operates on
	 * a set of resources. Passes are created via FrameGraph::DeclarePass() and go through two phases:
	 *
	 * Setup Phase (during Compile()):
	 * - Setup callback is invoked
	 * - Resource dependencies are declared via Read(), Write(), ReadWrite()
	 * - Dependencies are validated
	 *
	 * Execution Phase (during Execute()):
	 * - Execute callback is invoked with a GpuCommandBuffer
	 * - User records GPU commands (SetPipeline, Draw, Dispatch, etc.)
	 * - Commands are submitted to the appropriate GPU queue
	 *
	 * Passes execute in dependency-sorted order with automatic synchronization and barriers.
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
			FrameGraph* frameGraph,
			FrameGraphPassType type);

		/** Returns the pass index (assigned during creation) */
		u32 GetIndex() const { return mIndex; }

		/** Returns the pass name */
		const String& GetName() const { return mName; }

		/** Returns the queue this pass executes on */
		GpuQueueUsage GetQueue() const { return mQueue; }

		/** Get the pass type */
		FrameGraphPassType GetPassType() const { return mType; }

		/**
		 * Declares that this pass will read from a resource.
		 *
		 * Creates a read-after-write dependency on any pass that writes to this resource,
		 * ensuring proper execution order. The frame graph automatically inserts memory
		 * barriers to make the resource visible to this pass.
		 *
		 * Common usage flags:
		 * - GpuResourceUseFlag::ShaderAccess - Reading as shader texture (sampled) or storage image/buffer
		 * - GpuResourceUseFlag::UniformBuffer - Reading from uniform/constant buffer
		 * - GpuResourceUseFlag::VertexBuffer - Reading as vertex buffer
		 * - GpuResourceUseFlag::IndexBuffer - Reading as index buffer
		 *
		 * @param resource  Resource to read from (must be imported)
		 * @param usage     How the resource is used during the read
		 */
		void Read(FrameGraphResourceId resource, GpuResourceUseFlags usage);

		/**
		 * Declares that this pass will write to a resource.
		 *
		 * Creates dependencies with any pass that reads or writes this resource, ensuring
		 * proper execution order. The frame graph automatically inserts memory barriers to
		 * make the write visible to subsequent passes.
		 *
		 * Common usage flags:
		 * - GpuResourceUseFlag::ColorAttachment - Writing as render target
		 * - GpuResourceUseFlag::DepthStencilAttachment - Writing depth/stencil
		 * - GpuResourceUseFlag::ShaderAccess - Writing to storage texture/image or storage buffer (UAV)
		 *
		 * For render attachments, prefer WriteColor(), WriteDepth(), or ReadDepth() for
		 * automatic render target creation.
		 *
		 * @param resource  Resource to write to (must be imported)
		 * @param usage     How the resource is used during the write
		 */
		void Write(FrameGraphResourceId resource, GpuResourceUseFlags usage);

		/**
		 * Declares that this pass will both read and write a resource.
		 *
		 * Equivalent to calling both Read() and Write() for the same resource. Creates
		 * appropriate dependencies and barriers for both access types. Commonly used for
		 * storage resources (UAVs) in compute shaders that both consume and produce data.
		 *
		 * Typical usage:
		 * - Compute shader that updates a buffer in-place
		 * - Image processing that reads and writes to the same texture
		 * - Particle simulation with read-modify-write of particle data
		 *
		 * @param resource  Resource to access (must be imported)
		 * @param usage     How the resource is used (typically StorageTexture or UnorderedAccess)
		 */
		void ReadWrite(FrameGraphResourceId resource, GpuResourceUseFlags usage);

		/**
		 * Mark a resource as a color attachment (render target) for this render pass.
		 *
		 * Color attachments are textures that receive rendering output. The frame graph
		 * automatically creates a RenderTarget from all declared color and depth attachments
		 * during compilation.
		 *
		 * This method implicitly calls Write() with ColorAttachment usage, so the resource
		 * will be in ColorAttachment layout during execution. The frame graph handles all
		 * layout transitions automatically.
		 *
		 * Only valid for render passes. Call this in the setup callback.
		 *
		 * @param resource  Texture resource to use as color attachment (must be imported texture)
		 * @param index     Attachment index (0-7, default 0) for multiple render targets
		 */
		void WriteColor(FrameGraphResourceId resource, u32 index = 0);

		/**
		 * Mark a resource as a writable depth/stencil attachment for this render pass.
		 *
		 * Use this when you need to both read and write depth (e.g., standard depth testing
		 * and writing). The resource will be in DepthStencil layout which allows both
		 * operations.
		 *
		 * This method implicitly calls Write() with DepthStencil usage. The frame graph
		 * handles layout transitions automatically.
		 *
		 * For read-only depth (e.g., depth testing without writes), use ReadDepth() instead,
		 * which uses the more efficient DepthStencilReadOnly layout.
		 *
		 * Only valid for render passes. Call this in the setup callback.
		 *
		 * @param resource  Texture resource to use as depth/stencil (must be imported depth texture)
		 */
		void WriteDepth(FrameGraphResourceId resource);

		/**
		 * Mark a resource as a read-only depth/stencil attachment for this render pass.
		 *
		 * Use this when you need depth testing but not depth writes (e.g., rendering
		 * transparent objects after opaque, or rendering overlays). The resource will be
		 * in DepthStencilReadOnly layout, which is more efficient than the read-write layout
		 * and allows concurrent reads on some GPUs.
		 *
		 * This method implicitly calls Read() with DepthStencil usage. The frame graph
		 * handles layout transitions automatically.
		 *
		 * For depth writes, use WriteDepth() instead.
		 *
		 * Only valid for render passes. Call this in the setup callback.
		 *
		 * @param resource  Texture resource to use as read-only depth/stencil
		 */
		void ReadDepth(FrameGraphResourceId resource);

		/**
		 * Imports and declares all resources from a GpuParameterSet object.
		 *
		 * Automatically imports resources and determines correct access flags:
		 * - Sampled textures -> Read
		 * - Storage textures -> Read | Write
		 * - Uniform buffers -> Read
		 * - Storage buffers -> Read or Read | Write (based on shader reflection)
		 *
		 * Access flags are determined from GpuParameterObjectType in the pipeline layout,
		 * matching the behavior of VulkanGpuParameters::PrepareForBind.
		 *
		 * Note: Does NOT automatically bind parameters during execute - user must call
		 * cmd.SetGpuParameterSet() explicitly to allow different parameters per draw call.
		 *
		 * @param params  GpuParameterSet object containing resources to import
		 */
		void UseParameters(const SPtr<GpuParameterSet>& params);

		/** Get color attachments (for render passes) */
		const UnorderedMap<u32, FrameGraphResourceId>& GetColorAttachments() const { return mColorAttachments; }

		/** Get depth/stencil attachment (for render passes) */
		FrameGraphResourceId GetDepthAttachment() const { return mDepthAttachment; }

		/** Check if depth is read-only */
		bool IsDepthReadOnly() const { return mDepthReadOnly; }


		/** Returns all declared resource accesses */
		const Vector<FrameGraphResourceAccess>& GetResourceAccesses() const
		{
			return mResourceAccesses;
		}

		/** Sets the setup function (internal) */
		void SetSetupFunction(FrameGraphPassSetupFunc setupFunc)
		{
			mSetupFunction = std::move(setupFunc);
		}

		/** Sets the execute function (internal) */
		void SetExecuteFunction(FrameGraphPassExecuteFunc executeFunc)
		{
			mExecuteFunction = std::move(executeFunc);
		}

		/** Executes the setup function (internal) */
		void ExecuteSetup();

		/** Executes the command recording function (internal) */
		void ExecuteCommands(GpuCommandBuffer& commandBuffer, FrameGraphPassResources& resources);

		/** Resets the pass for reuse (internal) */
		void Reset();

		/** @name Dependency Graph (Internal - used by compiler)
		 *  @{
		 */

		/** Add an incoming dependency (a pass that must execute before this one) */
		void AddIncomingDependency(const FrameGraphPassDependency& dependency);

		/** Add an outgoing dependency (a pass that depends on this one) */
		void AddOutgoingDependency(const FrameGraphPassDependency& dependency);

		/** Get all incoming dependencies */
		const Vector<FrameGraphPassDependency>& GetIncomingDependencies() const { return mIncomingDependencies; }

		/** Get all outgoing dependencies */
		const Vector<FrameGraphPassDependency>& GetOutgoingDependencies() const { return mOutgoingDependencies; }

		/** Get the reference count (number of unresolved incoming dependencies) */
		u32 GetReferenceCount() const { return mReferenceCount; }

		/** Set the reference count */
		void SetReferenceCount(u32 count) { mReferenceCount = count; }

		/** Decrement the reference count */
		void DecrementReferenceCount() { mReferenceCount--; }

		/** Check if this node is ready to execute (all dependencies satisfied) */
		bool IsReady() const { return mReferenceCount == 0; }

		/** Mark this node as culled (will not execute) */
		void SetCulled(bool culled) { mCulled = culled; }

		/** Check if this node is culled */
		bool IsCulled() const { return mCulled; }

		/** @} */

	private:
		u32 mIndex;
		String mName;
		GpuQueueUsage mQueue;
		FrameGraph* mFrameGraph;
		FrameGraphPassType mType;
		Vector<FrameGraphResourceAccess> mResourceAccesses;
		FrameGraphPassSetupFunc mSetupFunction;
		FrameGraphPassExecuteFunc mExecuteFunction;

		// Render pass attachments
		UnorderedMap<u32, FrameGraphResourceId> mColorAttachments; // index -> resource
		FrameGraphResourceId mDepthAttachment;
		bool mDepthReadOnly = false;

		// Dependency graph metadata (populated during compilation)
		Vector<FrameGraphPassDependency> mIncomingDependencies;
		Vector<FrameGraphPassDependency> mOutgoingDependencies;
		u32 mReferenceCount = 0;
		bool mCulled = false;
	};

	/** @} */
}
