//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup RenderBeast RenderBeast
 *	Framework's default renderer implementation.
 */

/** @} */

namespace b3d
{
	namespace render
	{
		/** @addtogroup RenderBeast
		 *  @{
		 */

		/** Descriptor set indices used by RenderBeast for GPU pipeline parameter binding. */
		namespace GpuPipelineSet
		{
			/**
			 * Descriptor set for material parameters. In surface shaders it holds the user material parameters and the
			 * renderer data stored in the material (bone matrices, particle system textures). In other renderer shaders it
			 * holds all parameters except per-object data.
			 */
			constexpr u32 kMaterialSetIndex = 0;

			/** Descriptor set for per-object uniform buffers and other per-object data. */
			constexpr u32 kPerObjectSetIndex = 1;

			/** Descriptor set for per-pass renderer inputs of surface shaders (see SurfaceShaderType). */
			constexpr u32 kSurfaceShaderInputSetIndex = 2;
		}

		/** Available implementation of the DrawCommand class. */
		enum class DrawCommandType
		{
			/** See RenderableDrawCommand. */
			Renderable,
			/** See ParticlesDrawCommand. */
			Particle,
			/** See DecalDrawCommand. */
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
			u64 FrameIndex = 0;
		};

		/** @} */

		struct RenderBeastOptions;
		struct PooledRenderTexture;
		class RenderTargets;
		class RendererView;
		struct LightData;
	} // namespace render
} // namespace b3d
