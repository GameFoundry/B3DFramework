//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{namespace ct {
		struct RendererViewContext;
	}

	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/** A set of available locations at which the renderer can call RendererExtension's render() method. */
	enum class RenderLocation
	{
		/**
		 * Rendering happens before any scene objects are rendered and before the render target for scene objects is
		 * bound (e.g. GBuffer).
		 */
		Prepare,

		/**
		 * Rendering happens before any scene objects are rendered. The renderer guarantees the render targets used for
		 * rendering scene objects will be bound (e.g. GBuffer).
		 */
		PreBasePass,

		/**
		 * Rendering happens after all scene objects are rendered. The renderer guarantees the render targets used for
		 * rendering scene objects will be bound (e.g. GBuffer).
		 */
		PostBasePass,

		/**
		 * Rendering happens after all scene objects have been rendered and their final information has been written to
		 * the final scene color buffer, without any post-processing. The renderer guarantees the final scene color render
		 * target will be bound.
		 */
		PostLightPass,

		/**
		 * Rendering happens after all scene objects have been rendered and their final information has been written to
		 * the final scene color buffer, with post-processing. The renderer guarantees the final scene color render target
		 * will be bound.
		 */
		Overlay,

		Count // Keep at end
	};

	/** Determines possible results from RendererExtension::check method. */
	enum class RendererExtensionRequest
	{
		/**
		 * Ensures RendererExtension::render method() will be called. You want to set this to true
		 * if the internal data that's being rendered has changed since the last render() call for
		 * the specified camera.
		 */
		ForceRender,

		/**
		 * RendererExtension::render() method will be called only if the underlying render target
		 * will change and its contents need to be redrawn. You want to set this to true if the
		 * internal data being rendered is the same as the previous frame. If your scene is fully
		 * static and nothing is changing then this will avoid doing unnecessary redrawing.
		 *
		 * Note that if any extension for the same camera and render location returns ForceRender
		 * that will dirty the render target and require all relevant extensions to be redrawn.
		 */
		RenderIfTargetDirty,

		/**
		 * RendererExtension::render() method will not be called. e.g. use this if the camera provided
		 * is not relevant for the purposes of the extension.
		 */
		DontRender
	};

	/**
	 * Interface that can be implemented in order to provide custom rendering code to the renderer.
	 * See Renderer::addPlugin().
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT RendererExtension
	{
	public:
		/**
		 * Creates a brand new instance of a specific implementation of a renderer extension. Queues initialization of the
		 * object on the core thread and registers it with the renderer. Destruction will be queued on the core thread when
		 * the object goes out of scope.
		 *
		 * @note	Sim thread.
		 */
		template<class T>
		static SPtr<T> create(const Any& data)
		{
			T* ext = new (bs_alloc<T>()) T();
			_initializer(ext, data);

			return SPtr<T>(ext, &RendererExtension::_deleter);
		}

		/** Called when the renderer extension is first initialized. */
		virtual void initialize(const Any& data) {}

		/** Called just before the renderer extension is destroyed. */
		virtual void destroy() {}

		/** Returns true if the render() method should be called for the provided camera. */
		virtual RendererExtensionRequest check(const ct::Camera& camera) = 0;

		/**
		 * Called at the point at which rendering should be performed for the provided camera. Relevant render targets
		 * are guaranteed to be already bound to the render API, depending on the RenderLocation. Note that actual structure
		 * of the render targets depends on the active renderer.
		 *
		 * @param[in]	camera			Camera through which the renderer is currently rendering.
		 * @param[in]	viewContext		Additional information about the currently rendered view.
		 */
		virtual void render(const ct::Camera& camera, const ct::RendererViewContext& viewContext) = 0;

		/**
		 * Determines when will the render() method execute, compared to other plugins using the same RenderLocation.
		 * Higher number means the extension will execute before extensions with lower numbers. Priorities only matter for
		 * extensions that share the same RenderLocation.
		 */
		UINT32 getPriority() const { return mPriority; }

		/**
		 * Returns a location that determines at which point in rendering should the system call the render() method. See
		 * RenderLocation.
		 */
		RenderLocation getLocation() const { return mLocation; }

	protected:
		RendererExtension(RenderLocation location, UINT32 priority)
			:mLocation(location), mPriority(priority)
		{ }

		virtual ~RendererExtension() = default;
	private:
		/** Initializer that triggers when a renderer extension is first constructed. */
		static void _initializer(RendererExtension* obj, const Any& data);

		/** Deleter that triggers when a renderer extension object goes out of scope. */
		static void _deleter(RendererExtension* obj);

		RenderLocation mLocation;
		UINT32 mPriority;
	};

	/** @} */
}
