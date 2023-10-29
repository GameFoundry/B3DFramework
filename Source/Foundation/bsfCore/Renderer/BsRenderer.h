//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "String/BsStringID.h"
#include "Renderer/BsRendererMeshData.h"
#include "Material/BsShaderVariation.h"
#include "RenderAPI/BsGpuCommandBuffer.h"

namespace bs
{
	class RendererExtension;
	class LightProbeVolume;
	struct RenderSettings;
	struct EvaluatedAnimationData;
	struct ParticlePerFrameData;

	/** Contains various data evaluated by external systems on a per-frame basis that is to be used by the renderer. */
	struct PerFrameData
	{
		const EvaluatedAnimationData* Animation = nullptr;
		const ParticlePerFrameData* Particles = nullptr;
	};

	namespace ct
	{
		class GpuCommandBufferPool;
		class RendererTask;
		class LightProbeVolume;
		class Decal;

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

		/**	Set of options that can be used for controlling the renderer. */
		struct B3D_CORE_EXPORT RendererOptions
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
			SPtr<RenderTarget> CurrentTarget;

			/** Command buffer that's currently being recorded. */
			SPtr<GpuCommandBuffer> CommandBuffer;
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

		/**
		 * Primarily rendering class that allows you to specify how to render objects that exist in the scene graph. You need
		 * to provide your own implementation of your class.
		 *
		 * @note
		 * Normally you would iterate over all cameras, find visible objects for each camera and render those objects in some
		 * way.
		 */
		class B3D_CORE_EXPORT Renderer
		{
		public:
			Renderer();
			virtual ~Renderer() = default;

			/** Returns the command buffer pool usable for rendering on the render thread. */
			const SPtr<GpuCommandBufferPool>& GetCommandBufferPool() const { return mCommandBufferPool; } // TODO - Refactor this so there is one pool per frame, whose CBs we reset all at once

			/** Initializes the renderer with the provided GPU device. Must be called before using the renderer. */
			virtual void Initialize(const SPtr<GpuDevice>& gpuDevice);

			/** Called every frame. Triggers render task callbacks. */
			void Update();

			/**	Cleans up the renderer. Must be called before the renderer is deleted. */
			virtual void Destroy() {}

			/** Name of the renderer. Used by materials to find an appropriate technique for this renderer. */
			virtual const StringID& GetName() const = 0;

			/** Called in order to render all currently active cameras. */
			virtual void RenderAll(PerFrameData perFrameData) = 0;

			/**
			 * Called whenever a new camera is created.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyCameraAdded(Camera* camera) {}

			/**
			 * Called whenever a camera's position or rotation is updated.
			 *
			 * @param[in]	camera		Camera that was updated.
			 * @param[in]	updateFlag	Optional flag that allows the camera to signal to the renderer exactly what was updated.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyCameraUpdated(Camera* camera, u32 updateFlag) {}

			/**
			 * Called whenever a camera is destroyed.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyCameraRemoved(Camera* camera) {}

			/**
			 * Called whenever a new renderable is created.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyRenderableAdded(Renderable* renderable) {}

			/**
			 * Called whenever a renderable is updated.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyRenderableUpdated(Renderable* renderable) {}

			/**
			 * Called whenever a renderable is destroyed.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyRenderableRemoved(Renderable* renderable) {}

			/**
			 * Called whenever a new light is created.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyLightAdded(Light* light) {}

			/**
			 * Called whenever a light is updated.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyLightUpdated(Light* light) {}

			/**
			 * Called whenever a light is destroyed.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyLightRemoved(Light* light) {}

			/**
			 * Called whenever a new reflection probe is created.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyReflectionProbeAdded(ReflectionProbe* probe) {}

			/**
			 * Called whenever a reflection probe is updated.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyReflectionProbeUpdated(ReflectionProbe* probe, bool texture) {}

			/**
			 * Called whenever a reflection probe is destroyed.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyReflectionProbeRemoved(ReflectionProbe* probe) {}

			/**
			 * Called whenever a new light probe volume is created.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyLightProbeVolumeAdded(LightProbeVolume* volume) {}

			/**
			 * Called whenever a light probe volume is updated.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyLightProbeVolumeUpdated(LightProbeVolume* volume) {}

			/**
			 * Called whenever a light probe volume is destroyed.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyLightProbeVolumeRemoved(LightProbeVolume* volume) {}

			/**
			 * Called whenever a skybox is created.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifySkyboxAdded(Skybox* skybox) {}

			/**
			 * Called whenever a skybox is destroyed.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifySkyboxRemoved(Skybox* skybox) {}

			/**
			 * Called whenever a new particle system is created.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyParticleSystemAdded(ParticleSystem* particleSystem) {}

			/**
			 * Called whenever a particle system is updated.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyParticleSystemUpdated(ParticleSystem* particleSystem, bool tfrmOnly) {}

			/**
			 * Called whenever a particle system is destroyed.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyParticleSystemRemoved(ParticleSystem* particleSystem) {}

			/**
			 * Called whenever a new decal is created.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyDecalAdded(Decal* decal) {}

			/**
			 * Called whenever a decal is updated.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyDecalUpdated(Decal* decal) {}

			/**
			 * Called whenever a decal is destroyed.
			 *
			 * @note	Render thread.
			 */
			virtual void NotifyDecalRemoved(Decal* decal) {}

			/**
			 * Captures the scene at the specified location into a cubemap.
			 *
			 * @param	commandBuffer	Command buffer on which to encode the capture.
			 * @param	cubemap			Cubemap to store the results in.
			 * @param	position		Position to capture the scene at.
			 * @param	settings		Settings that allow you to customize the capture.
			 *
			 * @note	Render thread.
			 */
			virtual void CaptureSceneCubeMap(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, const Vector3& position, const CaptureSettings& settings) = 0;

			/**
			 * Creates a new empty renderer mesh data.
			 *
			 * @note	Main thread.
			 *
			 * @see		RendererMeshData
			 */
			virtual SPtr<RendererMeshData> CreateMeshDataInternal(u32 numVertices, u32 numIndices, VertexLayout layout, IndexType indexType = IT_32BIT);

			/**
			 * Creates a new renderer mesh data using an existing generic mesh data buffer.
			 *
			 * @note	Main thread.
			 *
			 * @see		RendererMeshData
			 */
			virtual SPtr<RendererMeshData> CreateMeshDataInternal(const SPtr<MeshData>& meshData);

			/** Queues GPU command capture of the next frame, if a frame capture is set up. */
			virtual void RequestFrameCapture() { }

			/**
			 * Registers an extension object that will be called every frame by the renderer. Allows external code to perform
			 * custom rendering interleaved with the renderer's output.
			 *
			 * @note	Render thread.
			 */
			void AddPlugin(RendererExtension* plugin) { mCallbacks.insert(plugin); }

			/**
			 * Unregisters an extension registered with addPlugin().
			 *
			 * @note	Render thread.
			 */
			void RemovePlugin(RendererExtension* plugin) { mCallbacks.erase(plugin); }

			/**
			 * Registers a new task for execution on the render thread.
			 *
			 * @note	Thread safe.
			 */
			void AddTask(const SPtr<RendererTask>& task);

			/**	Sets options used for controlling the rendering. */
			virtual void SetOptions(const SPtr<RendererOptions>& options) {}

			/**	Returns current set of options used for controlling the rendering. */
			virtual SPtr<RendererOptions> GetOptions() const { return SPtr<RendererOptions>(); }

		protected:
			friend class RendererTask;

			/** Information about a renderer task queued to be executed. */
			struct RendererTaskQueuedInfo
			{
				RendererTaskQueuedInfo(const SPtr<RendererTask>& task, u64 frameIdx)
					: Task(task), FrameIdx(frameIdx)
				{}

				SPtr<RendererTask> Task;
				u64 FrameIdx;
			};

			/** Performs the initialization on the render thread. */
			virtual void InitializeOnRenderThread();

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

			/** Callback to trigger when comparing the order in which renderer extensions are called. */
			static bool CompareCallback(const RendererExtension* a, const RendererExtension* b);

			SPtr<GpuDevice> mDevice;
			SPtr<GpuCommandBufferPool> mCommandBufferPool;

			Set<RendererExtension*, std::function<bool(const RendererExtension*, const RendererExtension*)>> mCallbacks;

			Vector<RendererTaskQueuedInfo> mQueuedTasks; // Main & render thread
			Vector<SPtr<RendererTask>> mUnresolvedTasks; // Main thread
			Vector<SPtr<RendererTask>> mRemainingUnresolvedTasks; // Main thread
			Vector<SPtr<RendererTask>> mRunningTasks; // Render thread
			Vector<SPtr<RendererTask>> mRemainingTasks; // Render thread
			Mutex mTaskMutex;

		};

		/**	Provides easy access to Renderer. */
		SPtr<Renderer> B3D_CORE_EXPORT GetRenderer();

		/**
		 * Task that represents an asynchonous operation queued for execution on the render thread. All such tasks are executed
		 * before main rendering happens, every frame.
		 *
		 * @note	Thread safe except where stated otherwise.
		 */
		class B3D_CORE_EXPORT RendererTask
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
			static SPtr<RendererTask> Create(String name, std::function<bool(GpuCommandBufferPool&)> taskWorker);

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
	} // namespace ct
} // namespace bs
