//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsStringID.h"
#include "BsRendererMeshData.h"

namespace BansheeEngine
{
	struct PostProcessSettings;

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
	struct BS_CORE_EXPORT CoreRendererOptions
	{
		virtual ~CoreRendererOptions() { }
	};

	/**
	 * Primarily rendering class that allows you to specify how to render objects that exist in the scene graph. You need
	 * to provide your own implementation of your class.
	 *
	 * @note	
	 * Normally you would iterate over all cameras, find visible objects for each camera and render those objects in some way.
	 */
	class BS_CORE_EXPORT CoreRenderer
	{
	public:
		CoreRenderer();
		virtual ~CoreRenderer() { }

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
		virtual void notifyCameraAdded(const CameraCore* camera) { }

		/**
		 * Called whenever a camera's position or rotation is updated.
		 *
		 * @param[in]	camera		Camera that was updated.
		 * @param[in]	updateFlag	Optional flag that allows the camera to signal to the renderer exactly what was updated.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyCameraUpdated(const CameraCore* camera, UINT32 updateFlag) { }

		/**
		 * Called whenever a camera is destroyed.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyCameraRemoved(const CameraCore* camera) { }

		/**
		 * Creates a new empty renderer mesh data.
		 *
		 * @note	Sim thread.
		 *			
		 * @see		RendererMeshData
		 */
		virtual SPtr<RendererMeshData> _createMeshData(UINT32 numVertices, UINT32 numIndices, VertexLayout layout, IndexType indexType = IT_32BIT);

		/**
		 * Creates a new renderer mesh data using an existing generic mesh data buffer.
		 *
		 * @note	Sim thread.
		 *			
		 * @see		RendererMeshData
		 */
		virtual SPtr<RendererMeshData> _createMeshData(const SPtr<MeshData>& meshData);

		/**
		 * Registers a new callback that will be executed when the the specify camera is being rendered.
		 *
		 * @param[in]	camera		Camera for which to trigger the callback.
		 * @param[in]	index		Index that determines the order of rendering when there are multiple registered 
		 *							callbacks. This must be unique. Lower indices get rendered sooner. Indices below 0 get 
		 *							rendered before the main viewport elements, while indices equal or greater to zero after. 
		 * @param[in]	callback	Callback to trigger when the specified camera is being rendered.
		 * @param[in]	isOverlay	If true the render callback guarantees that it will only render overlay data. Overlay 
		 *							data doesn't require a depth buffer, a multisampled render target and is usually cheaper
		 *							to render (although this depends on the exact renderer). 
		 *							Overlay callbacks are always rendered after all other callbacks, even if their index is negative.
		 *
		 * @note	Core thread.
		 */
		void registerRenderCallback(const CameraCore* camera, INT32 index, const std::function<void()>& callback, bool isOverlay = false);

		/** Removes a previously registered callback registered with _registerRenderCallback(). */
		void unregisterRenderCallback(const CameraCore* camera, INT32 index);

		/**	Sets options used for controlling the rendering. */
		virtual void setOptions(const SPtr<CoreRendererOptions>& options) { }

		/**	Returns current set of options used for controlling the rendering. */
		virtual SPtr<CoreRendererOptions> getOptions() const { return SPtr<CoreRendererOptions>(); }

		/** Creates post process settings that can be attached to a camera and processed by the active renderer. */
		virtual SPtr<PostProcessSettings> createPostProcessSettings() const = 0;

	protected:
		/**	Contains information about a render callback. */
		struct RenderCallbackData
		{
			bool overlay;
			std::function<void()> callback;
		};

		UnorderedMap<const CameraCore*, Map<INT32, RenderCallbackData>> mRenderCallbacks;
	};

	/** @} */
}