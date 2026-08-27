//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DRendererExtension.h"
#include "String/B3DStringID.h"
#include "Renderer/B3DRendererMeshData.h"
#include "Material/B3DShaderVariation.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuCommandBufferPoolRing.h"
#include "GpuBackend/B3DGpuCompletionTracker.h"
#include "GpuBackend/B3DGpuParameterSetPool.h"

namespace b3d
{
	class RendererExtension;
	class LightProbeVolume;
	class PixelData;
	class GpuWorkContext;
	struct RenderSettings;
	struct EvaluatedAnimationData;
	struct EvaluatedParticleData;

	/** Contains various data evaluated by external systems on a per-frame basis, for a particular scene. */
	struct PerSceneFrameData
	{
		const EvaluatedAnimationData* Animation = nullptr;
		const EvaluatedParticleData* Particles = nullptr;
	};

	/** Contains per-frame data for all scenes. See @p PerSceneFrameData. */
	struct PerFrameData
	{
		UnorderedMap<render::RendererScene*, PerSceneFrameData> PerSceneData;
	};

	namespace render
	{
		class RendererScene;
		class GpuCommandBufferPool;
		class RendererTask;
		class LightProbeVolume;
		class Decal;
		class RenderWindow;

		/** @addtogroup Renderer-Internal
		 *  @{
		 */

		/** Returns a specific vertex input shader variation. */
		template <bool SKINNED, bool MORPH, bool WRITE_VELOCITY>
		static const ShaderVariationParameters& GetVertexInputVariation(bool supportsVelocityWrites)
		{
			if(!supportsVelocityWrites)
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{
						ShaderVariationParameter("SKINNED", SKINNED),
						ShaderVariationParameter("MORPH", MORPH),
					});

				return variation;
			}
			else
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{
						ShaderVariationParameter("SKINNED", SKINNED),
						ShaderVariationParameter("MORPH", MORPH),
						ShaderVariationParameter("WRITE_VELOCITY", WRITE_VELOCITY),
					});

				return variation;
			}
		}

		/** @} */

		/** @addtogroup Renderer
		 *  @{
		 */

		/**	Set of options that can be used for controlling the renderer. */
		struct B3D_EXPORT RendererOptions
		{
			virtual ~RendererOptions() = default;
		};

		/**
		 * Contains information about the current state of a particular renderer view. This will be updated
		 * during rendering of a single frame.
		 */
		struct RendererViewContext
		{
			/** Current target the view is rendering to. */
			TShared<RenderTarget> CurrentTarget;

			/** Command buffer that's currently being recorded. */
			TShared<GpuCommandBuffer> CommandBuffer;
		};

		/** Settings that control renderer scene capture. */
		struct CaptureSettings
		{
			/** If true scene will be captured in a format that supports high dynamic range. */
			bool Hdr = true;

			/**
			 * When enabled the alpha channel of the final render target will be populated with an encoded depth value.
			 * Parameters @p depthEncodeNear and @p depthEncodeFar control which range of the depth buffer to encode.
			 */
			bool EncodeDepth = false;

			/**
			 * Controls at which position to start encoding depth, in view space. Only relevant with @p encodeDepth is enabled.
			 * Depth will be linearly interpolated between this value and @p depthEncodeFar.
			 */
			float DepthEncodeNear = 0.0f;

			/**
			 * Controls at which position to stop encoding depth, in view space. Only relevant with @p encodeDepth is enabled.
			 * Depth will be linearly interpolated between @p depthEncodeNear and this value.
			 */
			float DepthEncodeFar = 0.0f;
		};

		/**
		 * Information about a shader that is part of a renderer extension point. These shaders can be specialized by the
		 * outside world, by overriding parts of their functionality through mixins. Those specialized shaders are then,
		 * depending on the extension point, either attached to a normal Shader as subshaders, or sent back to the renderer
		 * in some other way.
		 */
		struct ExtensionShaderInfo
		{
			String Name; /**< Unique name of the sub-shader type that is recognized by the renderer. */
			Path Path; /**< Path to the original shader. */
			ShaderDefines Defines; /**< Additional defines to use when compiling the shader. */
		};

		/**
		 * Information about a shader extension point provided by the renderer. Extension points allow the outside world to
		 * generate a customized version of shaders used by the renderer, usually overriding some functionality with custom
		 * code. Extension point can contain one or multiple shaders whose functionality can be overriden.
		 */
		struct ShaderExtensionPointInfo
		{
			Vector<ExtensionShaderInfo> Shaders;
		};

		/** @} */

		/** @addtogroup Renderer
		 *  @{
		 */

		/**
		 * Primarily rendering class that allows you to specify how to render objects that exist in the scene graph. You need
		 * to provide your own implementation of your class.
		 *
		 * @note
		 * Normally you would iterate over all cameras, find visible objects for each camera and render those objects in some
		 * way.
		 */
		class B3D_EXPORT Renderer
		{
		public:
			Renderer() = default;
			virtual ~Renderer() = default;

			/** Returns the command buffer pool for the current frame. */
			GpuCommandBufferPool& GetCurrentCommandBufferPool() { return mCommandBufferPoolRing->GetCurrentPool(); }

			/**
			 * Returns the GPU work context intended for use explicitly on the render thread, for primary
			 * rendering and various helper work. Worker threads must create their own GpuWorkContext.
			 *
			 * @note	Render thread only.
			 */
			GpuWorkContext& GetGpuContext();

			/**
			 * Returns completion tracker that lets us know when a GPU has finished executing a frame.
			 * Used by the primary GPU work context. Advanced via BeginFrame/EndFrame methods.
			 */
			GpuFrameCompletionTracker& GetFrameCompletionTracker() { return mFrameCompletionTracker; }

			/**
			 * Call once per frame before recording regular frame rendering commands onto the 
			 * renderer's GPU context. Pair with EndFrame(). Render thread only.
			 */
			void BeginFrame();

			/**
			 * Ends the current frame: runs the incremental defragmentation pass on GPU memory, flushes
			 * the work context's pending transfers, blocks until previous frame is done (if needed)
			 * advances the GPU context across the frame boundary (recycling transfer pools and reclaiming 
			 * transient memory) and finally advances the frame completion tracker. Call once per frame after the
			 * frame's GPU work is recorded, paired with BeginFrame(). Render thread only.
			 */
			void EndFrame();

			/**
			 * Binds the GPU device the renderer performs its work against and creates the renderer's GPU
			 * work context. Called on the main thread when the renderer becomes the active renderer, before
			 * any GPU resources are created, so the work context is reachable via GetGpuContext() from that
			 * point on.
			 */
			virtual void Initialize(const TShared<GpuDevice>& gpuDevice);

			/**
			 * Completes renderer initialization, making it ready to render. Must be called after Initialize() and
			 * after the built-in resources have been loaded, as the renderer may depend on them.
			 */
			virtual void Activate() {}

			/** Called every frame. Triggers render task callbacks. */
			void Update();

			/**	Cleans up the renderer. Must be called before the renderer is deleted. */
			virtual void Destroy() {}

			/** Name of the renderer. Used by materials to find an appropriate variation for this renderer. */
			virtual const StringID& GetName() const = 0;

			/** Called in order to render all currently active cameras. */
			virtual void RenderAll(PerFrameData perFrameData) = 0;

			/**
			 * Captures the scene at the specified location into a cubemap.
			 *
			 * @param	scene			Scene to capture.
			 * @param	commandBuffer	Command buffer on which to encode the capture.
			 * @param	cubemap			Cubemap to store the results in.
			 * @param	position		Position to capture the scene at.
			 * @param	settings		Settings that allow you to customize the capture.
			 *
			 * @note	Render thread.
			 */
			virtual void CaptureSceneCubeMap(RendererScene& scene, GpuCommandBuffer& commandBuffer, const TShared<Texture>& cubemap, const Vector3& position, const CaptureSettings& settings) = 0;

			virtual TShared<RendererScene> CreateScene() = 0;

			/**
			 * Creates a new empty renderer mesh data.
			 *
			 * @note	Main thread.
			 *
			 * @see		RendererMeshData
			 */
			virtual TShared<RendererMeshData> CreateMeshDataInternal(u32 numVertices, u32 numIndices, VertexLayout layout, IndexType indexType = IT_32BIT);

			/**
			 * Creates a new renderer mesh data using an existing generic mesh data buffer.
			 *
			 * @note	Main thread.
			 *
			 * @see		RendererMeshData
			 */
			virtual TShared<RendererMeshData> CreateMeshDataInternal(const TShared<MeshData>& meshData);

			/** Queues capture of the next frame's GPU commands, if a command capture tool is available. */
			virtual void RequestGPUCommandCapture() { }

			/**
			 * Requests capture of the specified camera's view the next time it is rendered.
			 *
			 * @param	camera		Camera whose view should be captured.
			 * @param	asyncOp		Operation to complete when capture finishes.
			 *
			 * @note	Render thread.
			 */
			virtual void RequestViewCapture(Camera* camera, TAsyncOp<TShared<PixelData>> asyncOp) { asyncOp.CompleteOperation(nullptr); }

			/**
			 * Requests capture of the final image rendered to the specified window before it is presented.
			 *
			 * @param	window		Window whose next rendered image should be captured.
			 * @param	asyncOp		Operation to complete when capture finishes.
			 *
			 * @note	Render thread.
			 */
			void RequestScreenCapture(const TShared<RenderWindow>& window, TAsyncOp<TShared<PixelData>> asyncOp);

			/**
			 * Registers an extension object that will be called every frame, for each scene and view. Allows external code to perform
			 * custom rendering interleaved with the renderer's output.
			 *
			 * @note	Render thread.
			 */
			void AddExtension(RendererExtension* extension) { mRendererExtensions.insert(extension); mRendererExtensionsDirty = true; }

			/**
			 * Unregisters an extension registered with AddRendererExtension().
			 *
			 * @note	Render thread.
			 */
			void RemoveExtension(RendererExtension* extension) { mRendererExtensions.erase(extension); mRendererExtensionsDirty = true; }

			/**
			 * Registers a new task for execution on the render thread.
			 *
			 * @note	Thread safe.
			 */
			void AddTask(const TShared<RendererTask>& task);

			/**	Sets options used for controlling the rendering. */
			virtual void SetOptions(const TShared<RendererOptions>& options) {}

			/**	Returns current set of options used for controlling the rendering. */
			virtual TShared<RendererOptions> GetOptions() const { return TShared<RendererOptions>(); }

		protected:
			friend class RendererTask;

			/** Information about a renderer task queued to be executed. */
			struct RendererTaskQueuedInfo
			{
				RendererTaskQueuedInfo(const TShared<RendererTask>& task, u64 frameIdx)
					: Task(task), FrameIdx(frameIdx)
				{}

				TShared<RendererTask> Task;
				u64 FrameIdx;
			};

			/**
			 * Creates the renderer's GPU work context on the render thread. Queued from Initialize()
			 * Distinct from ActivateOnRenderThread(), which runs later.
			 */
			virtual void InitializeOnRenderThread();

			/**
			 * Performs per-renderer render-thread activation - command buffer pool ring, uniform buffer
			 * manager, and any derived-renderer render-thread setup. Queued from the Activate(). Runs after
			 * InitializeOnRenderThread().
			 */
			virtual void ActivateOnRenderThread();

			/** Performs tear-down on the render thread. */
			virtual void DestroyOnRenderThread();

			/**
			 * Executes all renderer tasks queued for this frame.
			 *
			 * @param[in]	forceAll	If true, multi-frame tasks will be forced to execute fully within this call.
			 * @param[in]	upToFrame	Only tasks that were queued before or during the frame with the provided index will
			 *							be processed.
			 *
			 * @note	Render thread.
			 */
			void ProcessTasks(bool forceAll, u64 upToFrame = std::numeric_limits<u64>::max());

			/**
			 * Executes the provided renderer task.
			 *
			 * @param[in]	task		Task to execute.
			 * @param[in]	forceAll	If true, multi-frame tasks will be forced to execute fully within this call.
			 *
			 * @note	Render thread.
			 */
			void ProcessTask(RendererTask& task, bool forceAll);

			/**
			 * Returns whether the specified window has a pending screen capture.
			 *
			 * @note	Render thread.
			 */
			bool IsScreenCaptureRequested(const TShared<RenderWindow>& window) const;

			/**
			 * Records a window readback into @p commandBuffer and forwards its result to the capture
			 * operations pending on @p window, removing them from the pending list. Returns false if no
			 * capture was pending or the window does not support readback.
			 *
			 * @note	Render thread.
			 */
			bool ResolveScreenCaptures(GpuCommandBuffer& commandBuffer, const TShared<RenderWindow>& window);

			/**
			 * Resolves pending screen captures for windows the frame's rendering did not cover (e.g. windows
			 * rendered outside of the renderer), submitting a readback command buffer per window. Call once
			 * per frame after regular rendering, before EndFrame().
			 *
			 * @note	Render thread.
			 */
			void ResolveOutstandingScreenCaptures();

			TShared<GpuDevice> mDevice;

			/**
			 * Frame-count completion tracker for render-thread GPU work, advanced once per frame by
			 * EndFrame(). Declared before mGpuContext, which borrows it and must not outlive it.
			 */
			GpuFrameCompletionTracker mFrameCompletionTracker;

			/**
			 * Render-thread work context this renderer owns: transient allocators, transfer command
			 * buffers, parameter set pool and command buffer submission for render-thread work. 
			 * Borrows the renderer's frame completion tracker.
			 */
			TShared<GpuWorkContext> mGpuContext;

			TUnique<GpuCommandBufferPoolRing> mCommandBufferPoolRing;

			Set<RendererExtension*, RendererExtension::SortFunction> mRendererExtensions;
			bool mRendererExtensionsDirty = true;

			Vector<RendererTaskQueuedInfo> mQueuedTasks; // Main & render thread
			Vector<TShared<RendererTask>> mUnresolvedTasks; // Main thread
			Vector<TShared<RendererTask>> mRemainingUnresolvedTasks; // Main thread
			Vector<TShared<RendererTask>> mRunningTasks; // Render thread
			Vector<TShared<RendererTask>> mRemainingTasks; // Render thread
			Mutex mTaskMutex;

		private:
			/** Capture operations waiting for the next image rendered to a window. */
			struct ScreenCaptureRequest
			{
				ScreenCaptureRequest(const TShared<RenderWindow>& window, TAsyncOp<TShared<PixelData>> asyncOp)
					: Window(window)
				{
					Operations.Add(std::move(asyncOp));
				}

				TShared<RenderWindow> Window;
				TInlineArray<TAsyncOp<TShared<PixelData>>, 1> Operations;
			};

			TInlineArray<ScreenCaptureRequest, 1> mScreenCaptureRequests; // Render thread
		};

		/**	Provides easy access to Renderer. */
		TShared<Renderer> B3D_EXPORT GetRenderer();

		/**
		 * Task that represents an asynchonous operation queued for execution on the render thread. All such tasks are executed
		 * before main rendering happens, every frame.
		 *
		 * @note	Thread safe except where stated otherwise.
		 */
		class B3D_EXPORT RendererTask
		{
			struct PrivatelyConstruct
			{};

		public:
			RendererTask(const PrivatelyConstruct& dummy, String name, std::function<bool(GpuCommandBufferPool&)> taskWorker);

			/**
			 * Creates a new task. Task should be provided to Renderer in order for it to start.
			 *
			 * @param[in]	name		Name you can use to more easily identify the task.
			 * @param[in]	taskWorker	Worker method that does all of the work in the task. Tasks can run over the course of
			 *							multiple frames, in which case this method should return false (if there's more
			 *							work to be done), or true (if the task has completed).
			 */
			static TShared<RendererTask> Create(String name, std::function<bool(GpuCommandBufferPool&)> taskWorker);

			/** Returns true if the task has completed. */
			bool IsComplete() const;

			/**	Returns true if the task has been canceled. */
			bool IsCanceled() const;

			/** Blocks the current thread until the task has completed. */
			void Wait();

			/** Cancels the task and removes it from the Renderer's queue. */
			void Cancel();

			/**
			 * Callback triggered on the main thread, when the task completes. Is not triggered if the task is cancelled.
			 *
			 * @note	Main thread only.
			 */
			Event<void()> OnComplete;

		private:
			friend class Renderer;

			String mName;
			std::function<bool(GpuCommandBufferPool&)> mTaskWorker;
			std::atomic<u32> mState{ 0 }; /**< 0 - Inactive, 1 - In progress, 2 - Completed, 3 - Canceled */
		};

		/** @} */
	} // namespace render
} // namespace b3d
