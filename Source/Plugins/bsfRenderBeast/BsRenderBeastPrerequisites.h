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

namespace bs
{
	namespace ct
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

		/** State used to controlling how are properties that need to maintain their previous frame state updated. */
		enum class PrevFrameDirtyState
		{
			/** Most recent version of the property was updated this frame, and its old data stored as prev. version. */
			Updated,
			/** No update has been done this frame, most recent version of the properties should be copied into prev. frame. */
			CopyMostRecent,
			/** Most recent and prev. frame versions are the same and require no updates. */
			Clean
		};

		/** Information about current time and frame index. */
		struct FrameTimings
		{
			float Time = 0.0f;
			float TimeDelta = 0.0f;
			u64 FrameIdx = 0;
		};

		/** @} */

		struct RenderBeastOptions;
		struct PooledRenderTexture;
		class RenderTargets;
		class RendererView;
		struct LightData;
	} // namespace ct
} // namespace bs
