//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

/**
 * Stage up to which RenderBeast compiles the code that waits for its port to frame graphs. Each stage brings back a set of renderer
 * features, and each feature has a stage number below. Code under #if B3D_FRAME_GRAPH_PORT_PENDING(Feature) waits for the port of
 * the feature and is compiled once this stage reaches the feature's stage. Code under #if !B3D_FRAME_GRAPH_PORT_PENDING(Feature)
 * stands in for the feature until then.
 *
 * Raising the stage restores the old renderer code of the features up to it, as long as that code still compiles against the ported
 * code around it.
 */
#define B3D_FRAME_GRAPH_PORT_STAGE 0

#define B3D_FRAME_GRAPH_PORT_STAGE_Shadows 1
#define B3D_FRAME_GRAPH_PORT_STAGE_IndirectLighting 2
#define B3D_FRAME_GRAPH_PORT_STAGE_Forward 3
#define B3D_FRAME_GRAPH_PORT_STAGE_Particles 3
#define B3D_FRAME_GRAPH_PORT_STAGE_Decals 3
#define B3D_FRAME_GRAPH_PORT_STAGE_PostProcessing 4
#define B3D_FRAME_GRAPH_PORT_STAGE_ScreenSpace 5
#define B3D_FRAME_GRAPH_PORT_STAGE_TemporalAA 5
#define B3D_FRAME_GRAPH_PORT_STAGE_Msaa 6
#define B3D_FRAME_GRAPH_PORT_STAGE_Extensions 7
#define B3D_FRAME_GRAPH_PORT_STAGE_RendererTasks 8

/**
 * True if the code that waits for the port of @p feature to frame graphs is compiled. See B3D_FRAME_GRAPH_PORT_STAGE. The division
 * rejects an unknown feature, which the preprocessor would otherwise read as stage 0.
 */
#define B3D_FRAME_GRAPH_PORT_PENDING(feature) \
	(B3D_FRAME_GRAPH_PORT_STAGE >= B3D_FRAME_GRAPH_PORT_STAGE_##feature + 0 / B3D_FRAME_GRAPH_PORT_STAGE_##feature)

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
