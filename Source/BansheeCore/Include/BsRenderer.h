//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsStringID.h"
#include "BsRendererMeshData.h"

namespace bs 
{ 
	class RendererExtension;
	struct PostProcessSettings;

	namespace ct
	{
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/**
	 * Available parameter block semantics that allow the renderer to identify the use of a GPU program parameter block 
	 * specified in a shader.
	 */
	static StringID RBS_Static = "Static";
	static StringID RBS_PerCamera = "PerCamera";
	static StringID RBS_PerFrame = "PerFrame";
	static StringID RBS_PerObject = "PerObject";
	static StringID RBS_PerCall = "PerCall";

	/**
	 * Available parameter semantics that allow the renderer to identify the use of a GPU parameter specified in a shader.
	 */
	static StringID RPS_WorldViewProjTfrm = "WVP";
	static StringID RPS_ViewProjTfrm = "VP";
	static StringID RPS_ProjTfrm = "P";
	static StringID RPS_ViewTfrm = "V";
	static StringID RPS_WorldTfrm = "W";
	static StringID RPS_InvWorldTfrm = "IW";
	static StringID RPS_WorldNoScaleTfrm = "WNoScale";
	static StringID RPS_InvWorldNoScaleTfrm = "IWNoScale";
	static StringID RPS_WorldDeterminantSign = "WorldDeterminantSign";
	static StringID RPS_Diffuse = "Diffuse";
	static StringID RPS_ViewDir = "ViewDir";

	/** Technique tags. */
	static StringID RTag_Skinned = "Skinned";
	static StringID RTag_Morph = "Morph";
	static StringID RTag_SkinnedMorph = "SkinnedMorph";

	/**	Set of options that can be used for controlling the renderer. */	
	struct BS_CORE_EXPORT RendererOptions
	{
		virtual ~RendererOptions() { }
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
		virtual void notifyReflectionProbeUpdated(ReflectionProbe* probe) { }

		/**
		 * Called whenever a reflection probe is destroyed.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyReflectionProbeRemoved(ReflectionProbe* probe) { }

		/**
		 * Called whenever a skybox is created.
		 *
		 * @note	Core thread.
		 */
		virtual void notifySkyboxAdded(Skybox* skybox) { }

		/**
		 * Called whenever the texture assigned to a skybox is changed.
		 *
		 * @note	Core thread.
		 */
		virtual void notifySkyboxTextureChanged(Skybox* skybox) { }

		/**
		 * Called whenever a skybox is destroyed.
		 *
		 * @note	Core thread.
		 */
		virtual void notifySkyboxRemoved(Skybox* skybox) { }

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

		/**	Sets options used for controlling the rendering. */
		virtual void setOptions(const SPtr<RendererOptions>& options) { }

		/**	Returns current set of options used for controlling the rendering. */
		virtual SPtr<RendererOptions> getOptions() const { return SPtr<RendererOptions>(); }

		/** Creates post process settings that can be attached to a camera and processed by the active renderer. */
		virtual SPtr<PostProcessSettings> createPostProcessSettings() const = 0;

	protected:
		/**	Contains information about a render callback. */
		struct RenderCallbackData
		{
			bool overlay;
			std::function<void()> callback;
		};

		/** Callback to trigger when comparing the order in which renderer extensions are called. */
		static bool compareCallback(const RendererExtension* a, const RendererExtension* b);

		Set<RendererExtension*, std::function<bool(const RendererExtension*, const RendererExtension*)>> mCallbacks;
	};

	/**	Provides easy access to Renderer. */
	SPtr<Renderer> BS_CORE_EXPORT gRenderer();

	/** @} */
}}