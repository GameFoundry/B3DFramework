//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup RenderBeast RenderBeast
 *	Framework's default renderer implementation.
 */

/** @} */

namespace bs { namespace ct
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	/**
	 * Determines the feature set to be used by RenderBeast. Feature sets control the quality and type of rendering
	 * effects depending on available hardware (For example a desktop computer can handle higher end rendering than a
	 * mobile device).
	 */
	enum class RenderBeastFeatureSet
	{
		/** High end feature set utilizing the latest and greatest effects. */
		Desktop,
		/** Mid-range feature set optimized for macOS and its obsolete OpenGL 4.1 version. */
		DesktopMacOS
	};

	/** Available implementation of the RenderElement class. */
	enum class RenderElementType
	{
		/** See RenderableElement. */
		Renderable,
		/** See ParticlesRenderElement. */
		Particle,
		/** See DecalRenderElement. */
		Decal
	};

	/** Types of ways for shaders to handle MSAA. */
	enum class MSAAMode
	{
		/** No MSAA supported. */
		None,
		/** Single MSAA sample will be resolved. */
		Single,
		/** All MSAA samples will be resolved. */
		Full,
	};

	/** @} */

	struct RenderBeastOptions;
	struct PooledRenderTexture;
	class RenderTargets;
	class RendererView;
	struct LightData;
}}
