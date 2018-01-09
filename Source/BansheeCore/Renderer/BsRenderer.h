//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "String/BsStringID.h"
#include "Renderer/BsRendererMeshData.h"
#include "Material/BsShaderVariation.h"

namespace bs 
{ 
	class RendererExtension;
	class LightProbeVolume;
	struct RenderSettings;

	namespace ct
	{
	class RendererTask;
	class LightProbeVolume;

	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/** Common shader variations. */
	static ShaderVariation SVar_Static = ShaderVariation({
		ShaderVariation::Param("SKINNED", false),
		ShaderVariation::Param("MORPH", false),
	});

	static ShaderVariation SVar_Skinned = ShaderVariation({
		ShaderVariation::Param("SKINNED", true),
		ShaderVariation::Param("MORPH", false),
	});

	static ShaderVariation SVar_Morph = ShaderVariation({
		ShaderVariation::Param("SKINNED", false),
		ShaderVariation::Param("MORPH", true),
	});

	static ShaderVariation SVar_SkinnedMorph = ShaderVariation({
		ShaderVariation::Param("SKINNED", true),
		ShaderVariation::Param("MORPH", true),
	});

	/** Technique tags. */
	static StringID RTag_Skinned = "Skinned";
	static StringID RTag_Morph = "Morph";
	static StringID RTag_SkinnedMorph = "SkinnedMorph";

	/**	Set of options that can be used for controlling the renderer. */	
	struct BS_CORE_EXPORT RendererOptions
	{
		virtual ~RendererOptions() { }
	};

	/** Settings that control renderer scene capture. */
	struct CaptureSettings
	{	
		/** If true scene will be captured in a format that supports high dynamic range. */
		bool hdr = true;

		/** 
		 * When enabled the alpha channel of the final render target will be populated with an encoded depth value. 
		 * Parameters @p depthEncodeNear and @p depthEncodeFar control which range of the depth buffer to encode.
		 */
		bool encodeDepth = false;

		/**
		 * Controls at which position to start encoding depth, in view space. Only relevant with @p encodeDepth is enabled.
		 * Depth will be linearly interpolated between this value and @p depthEncodeFar.
		 */
		float depthEncodeNear = 0.0f;

		/**
		 * Controls at which position to stop encoding depth, in view space. Only relevant with @p encodeDepth is enabled.
		 * Depth will be linearly interpolated between @p depthEncodeNear and this value.
		 */
		float depthEncodeFar = 0.0f;
	};


	/**
	 * Primarily rendering class that allows you to specify how to render objects that exist in the scene graph. You need
	 * to provide your own implementation of your class.
	 *
	 * @note	
	 * Normally you would iterate over all cameras, find visible objects for each camera and render those objects in some 
	 * way.
	 */
	class BS_CORE_EXPORT Renderer
	{
	public:
		Renderer();
		virtual ~Renderer() { }

		/** Initializes the renderer. Must be called before using the renderer. */
		virtual void initialize() { }

		/** Called every frame. Triggers render task callbacks. */
		void update();

		/**	Cleans up the renderer. Must be called before the renderer is deleted. */
		virtual void destroy() { }

		/** Name of the renderer. Used by materials to find an appropriate technique for this renderer. */
		virtual const StringID& getName() const = 0;

		/** Called in order to render all currently active cameras. */
		virtual void renderAll() = 0;

		/**
		 * Called whenever a new camera is created.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyCameraAdded(Camera* camera) { }

		/**
		 * Called whenever a camera's position or rotation is updated.
		 *
		 * @param[in]	camera		Camera that was updated.
		 * @param[in]	updateFlag	Optional flag that allows the camera to signal to the renderer exactly what was updated.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyCameraUpdated(Camera* camera, UINT32 updateFlag) { }

		/**
		 * Called whenever a camera is destroyed.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyCameraRemoved(Camera* camera) { }

		/**
		 * Called whenever a new renderable is created.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyRenderableAdded(Renderable* renderable) { }

		/**
		 * Called whenever a renderable is updated.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyRenderableUpdated(Renderable* renderable) { }

		/**
		 * Called whenever a renderable is destroyed.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyRenderableRemoved(Renderable* renderable) { }

		/**
		 * Called whenever a new light is created.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyLightAdded(Light* light) { }

		/**
		 * Called whenever a light is updated.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyLightUpdated(Light* light) { }

		/**
		 * Called whenever a light is destroyed.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyLightRemoved(Light* light) { }

		/**
		 * Called whenever a new reflection probe is created.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyReflectionProbeAdded(ReflectionProbe* probe) { }

		/**
		 * Called whenever a reflection probe is updated.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyReflectionProbeUpdated(ReflectionProbe* probe, bool texture) { }

		/**
		 * Called whenever a reflection probe is destroyed.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyReflectionProbeRemoved(ReflectionProbe* probe) { }

		/**
		 * Called whenever a new light probe volume is created.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyLightProbeVolumeAdded(LightProbeVolume* volume) { }

		/**
		 * Called whenever a light probe volume is updated.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyLightProbeVolumeUpdated(LightProbeVolume* volume) { }

		/**
		 * Called whenever a light probe volume is destroyed.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyLightProbeVolumeRemoved(LightProbeVolume* volume) { }

		/**
		 * Called whenever a skybox is created.
		 *
		 * @note	Core thread.
		 */
		virtual void notifySkyboxAdded(Skybox* skybox) { }

		/**
		 * Called whenever a skybox is destroyed.
		 *
		 * @note	Core thread.
		 */
		virtual void notifySkyboxRemoved(Skybox* skybox) { }

		/** 
		 * Captures the scene at the specified location into a cubemap. 
		 * 
		 * @param[in]	cubemap		Cubemap to store the results in.
		 * @param[in]	position	Position to capture the scene at.
		 * @param[in]	settings	Settings that allow you to customize the capture.
		 *
		 * @note	Core thread.
		 */
		virtual void captureSceneCubeMap(const SPtr<Texture>& cubemap, const Vector3& position, 
			const CaptureSettings& settings) = 0;

		/**
		 * Creates a new empty renderer mesh data.
		 *
		 * @note	Sim thread.
		 *			
		 * @see		RendererMeshData
		 */
		virtual SPtr<RendererMeshData> _createMeshData(UINT32 numVertices, UINT32 numIndices, VertexLayout layout, 
													   IndexType indexType = IT_32BIT);

		/**
		 * Creates a new renderer mesh data using an existing generic mesh data buffer.
		 *
		 * @note	Sim thread.
		 *			
		 * @see		RendererMeshData
		 */
		virtual SPtr<RendererMeshData> _createMeshData(const SPtr<MeshData>& meshData);

		/** 
		 * Registers an extension object that will be called every frame by the renderer. Allows external code to perform
		 * custom rendering interleaved with the renderer's output.
		 * 
		 * @note	Core thread.
		 */
		void addPlugin(RendererExtension* plugin) { mCallbacks.insert(plugin); }

		/** 
		 * Unregisters an extension registered with addPlugin(). 
		 * 
		 * @note	Core thread.
		 */
		void removePlugin(RendererExtension* plugin) { mCallbacks.erase(plugin); }

		/**
		 * Registers a new task for execution on the core thread.
		 * 
		 * @note	Thread safe.
		 */
		void addTask(const SPtr<RendererTask>& task);

		/**	Sets options used for controlling the rendering. */
		virtual void setOptions(const SPtr<RendererOptions>& options) { }

		/**	Returns current set of options used for controlling the rendering. */
		virtual SPtr<RendererOptions> getOptions() const { return SPtr<RendererOptions>(); }

	protected:
		friend class RendererTask;

		/**	Contains information about a render callback. */
		struct RenderCallbackData
		{
			bool overlay;
			std::function<void()> callback;
		};

		/**
		 * Executes all renderer tasks queued for this frame.
		 *
		 * @param[in]	forceAll	If true, multi-frame tasks will be forced to execute fully within this call.
		 * 
		 * @note	Core thread.
		 */
		void processTasks(bool forceAll);

		/**
		 * Executes the provided renderer task.
		 *
		 * @param[in]	task		Task to execute.
		 * @param[in]	forceAll	If true, multi-frame tasks will be forced to execute fully within this call.
		 * 
		 * @note	Core thread.
		 */
		void processTask(RendererTask& task, bool forceAll);

		/** Callback to trigger when comparing the order in which renderer extensions are called. */
		static bool compareCallback(const RendererExtension* a, const RendererExtension* b);

		Set<RendererExtension*, std::function<bool(const RendererExtension*, const RendererExtension*)>> mCallbacks;

		Vector<SPtr<RendererTask>> mQueuedTasks; // Sim & core thread
		Vector<SPtr<RendererTask>> mUnresolvedTasks; // Sim thread
		Vector<SPtr<RendererTask>> mRemainingUnresolvedTasks; // Sim thread
		Vector<SPtr<RendererTask>> mRunningTasks; // Core thread
		Vector<SPtr<RendererTask>> mRemainingTasks; // Core thread
		Mutex mTaskMutex;
	};

	/**	Provides easy access to Renderer. */
	SPtr<Renderer> BS_CORE_EXPORT gRenderer();

	/**
	 * Task that represents an asynchonous operation queued for execution on the core thread. All such tasks are executed
	 * before main rendering happens, every frame.
	 *
	 * @note	Thread safe except where stated otherwise.
	 */
	class BS_CORE_EXPORT RendererTask
	{
		struct PrivatelyConstruct {};

	public:
		RendererTask(const PrivatelyConstruct& dummy, const String& name, std::function<bool()> taskWorker);

		/**
		 * Creates a new task. Task should be provided to Renderer in order for it to start.
		 *
		 * @param[in]	name		Name you can use to more easily identify the task.
		 * @param[in]	taskWorker	Worker method that does all of the work in the task. Tasks can run over the course of
		 *							multiple frames, in which case this method should return false (if there's more
		 *							work to be done), or true (if the task has completed).
		 */
		static SPtr<RendererTask> create(const String& name, std::function<bool()> taskWorker);

		/** Returns true if the task has completed. */
		bool isComplete() const;

		/**	Returns true if the task has been canceled. */
		bool isCanceled() const;

		/** Blocks the current thread until the task has completed. */
		void wait();

		/** Cancels the task and removes it from the Renderer's queue. */
		void cancel();

		/** 
		 * Callback triggered on the sim thread, when the task completes. Is not triggered if the task is cancelled.
		 *
		 * @note	Sim thread only.
		 */
		Event<void()> onComplete;

	private:
		friend class Renderer;

		String mName;
		std::function<bool()> mTaskWorker;
		std::atomic<UINT32> mState; /**< 0 - Inactive, 1 - In progress, 2 - Completed, 3 - Canceled */
	};

	/** @} */
}}
