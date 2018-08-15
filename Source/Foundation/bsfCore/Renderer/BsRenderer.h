//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
	struct EvaluatedAnimationData;
	struct ParticleSimulationData;

	/** Contains various data evaluated by external systems on a per-frame basis that is to be used by the renderer. */
	struct PerFrameData
	{
		const EvaluatedAnimationData* animation = nullptr;
		const ParticleSimulationData* particles = nullptr;
	};

	namespace ct
	{
	class RendererTask;
	class LightProbeVolume;

	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/** Common shader variations. */

	/** Returns a specific vertex input shader variation. */
	template<bool skinned, bool morph>
	static const ShaderVariation& getVertexInputVariation()
	{
		static ShaderVariation variation = ShaderVariation(
		Vector<ShaderVariation::Param>{
			ShaderVariation::Param("SKINNED", skinned),
			ShaderVariation::Param("MORPH", morph),
		});

		return variation;
	}

	/** Returns a specific forward rendering shader variation. */
	template<bool skinned, bool morph, bool clustered>
	static const ShaderVariation& getForwardRenderingVariation()
	{
		static ShaderVariation variation = ShaderVariation(
		Vector<ShaderVariation::Param>{
			ShaderVariation::Param("SKINNED", skinned),
			ShaderVariation::Param("MORPH", morph),
			ShaderVariation::Param("CLUSTERED", clustered),
		});

		return variation;
	}

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
	 * Information about a shader that is part of a renderer extension point. These shaders can be specialized by the
	 * outside world, by overriding parts of their functionality through mixins. Those specialized shaders are then,
	 * depending on the extension point, either attached to a normal Shader as subshaders, or sent back to the renderer
	 * in some other way.
	 */
	struct ExtensionShaderInfo
	{
		String name; /**< Unique name of the sub-shader type that is recognized by the renderer. */
		Path path; /**< Path to the original shader. */
		ShaderDefines defines; /**< Additional defines to use when compiling the shader. */
	};

	/**
	 * Information about a shader extension point provided by the renderer. Extension points allow the outside world to
	 * generate a customized version of shaders used by the renderer, usually overriding some functionality with custom 
	 * code. Extension point can contain one or multiple shaders whose functionality can be overriden. 
	 */
	struct ShaderExtensionPointInfo
	{
		Vector<ExtensionShaderInfo> shaders;
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
		virtual void renderAll(PerFrameData perFrameData) = 0;

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
		 * Called whenever a new particle system is created.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyParticleSystemAdded(ParticleSystem* particleSystem) { }

		/**
		 * Called whenever a particle system is updated.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyParticleSystemUpdated(ParticleSystem* particleSystem, bool tfrmOnly) { }

		/**
		 * Called whenever a particle system is destroyed.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyParticleSystemRemoved(ParticleSystem* particleSystem) { }

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

		/** 
		 * Returns information about a set of shaders corresponding to a renderer extension point. These shaders are the
		 * built-in shaders used by the renderer for specific functionality (e.g. deferred rendering). These shaders can
		 * be customized (usually through BSL mixin overrides) to provide user specific functionality (e.g. a custom BRDF 
		 * model), without having to modify the (usually complex) original shaders. 
		 * 
		 * These modified shaders can then be passed back to the renderer through some means. If it's an extension point
		 * that does some kind of per-object rendering (e.g. shadow projection, or lighting in a forward renderer), then
		 * the shader should be passed as a SubShader of the Shader used by the Renderable's Material. If it's some global
		 * shader (e.g. deferred rendering lighting) then call setGlobalShaderOverride(). In both cases, the name of the
		 * extension point shader should be provided, as returned by this method.
		 * 
		 * A single extension point can provide multiple shaders (e.g. deferred rendering can be implemented as both
		 * standard and tiled deferred), in which case you'll likely want to override all the relevant shaders.
		 * 
		 * @param[in]	name		Name of the sub-shader extension point, e.g. "DeferredDirectLighting", 
		 *							"ShadowProjection". These are renderer specific and you should look them up in
		 *							renderer documentation.
		 * @returns					A list of overridable shaders in the extension point.
		 */
		virtual ShaderExtensionPointInfo getShaderExtensionPointInfo(const String& name) { return {}; }

		/**
		 * Allows the caller to override some built-in renderer functionality with a custom shader. The exact set of
		 * overridable shaders can be retrieved through getShaderExtensionPointInfo().
		 */
		virtual void setGlobalShaderOverride(const String& name, const SPtr<bs::Shader>& shader) { }

		/**
		 * Provides the same functionality as setGlobalShaderOverride(const String&, const SPtr<bs::Shader>&), but it automatically
		 * evaluates all sub-shaders of the provided shader and registers them as overrides.
		 */
		void setGlobalShaderOverride(const SPtr<bs::Shader>& shader);
	protected:
		friend class RendererTask;

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
		RendererTask(const PrivatelyConstruct& dummy, String name, std::function<bool()> taskWorker);

		/**
		 * Creates a new task. Task should be provided to Renderer in order for it to start.
		 *
		 * @param[in]	name		Name you can use to more easily identify the task.
		 * @param[in]	taskWorker	Worker method that does all of the work in the task. Tasks can run over the course of
		 *							multiple frames, in which case this method should return false (if there's more
		 *							work to be done), or true (if the task has completed).
		 */
		static SPtr<RendererTask> create(String name, std::function<bool()> taskWorker);

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
		std::atomic<UINT32> mState{0}; /**< 0 - Inactive, 1 - In progress, 2 - Completed, 3 - Canceled */
	};

	/** @} */
}}
