//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Utility/B3DTriangulation.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DMatrixNxM.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuResourcePool.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "B3DRendererLight.h"

namespace b3d
{
	namespace render
	{
		struct LightProbesInfo;
		struct GBufferTextures;
		struct FrameInfo;
		class LightProbeVolume;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		B3D_UNIFORM_BUFFER_BEGIN(TetrahedraRenderUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gDepthTexSize)
		B3D_UNIFORM_BUFFER_END

		extern TetrahedraRenderUniformDefinition gTetrahedraRenderUniformDefinition;

		/**
		 * Shader that renders the tetrahedra used for light probe evaluation. Tetrahedra depth is compare with current scene
		 * depth, and for each scene pixel the matching tetrahedron index is written to the output target.
		 */
		class TetrahedraRenderMaterial : public RendererMaterial<TetrahedraRenderMaterial>
		{
			RMAT_DEF("TetrahedraRender.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool msaa, bool singleSampleMSAA>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MSAA", msaa),
					  ShaderVariationParameter("MSAA_RESOLVE_0TH", singleSampleMSAA) });

				return variation;
			}

		public:
			TetrahedraRenderMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering. Must be called before Execute().
			 *
			 * @param	view			View that is currently being rendered.
			 * @param	sceneDepth		Depth of scene objects that should be lit.
			 */
			void Prepare(const RendererView& view, const SPtr<Texture>& sceneDepth);

			/**
			 * Executes the material using the provided parameters.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	mesh			Mesh to render.
			 * @param	output			Output texture created using the descriptor returned by getOutputDesc().
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Mesh>& mesh, const SPtr<RenderTexture>& output);

			/**
			 * Returns the descriptors that can be used for creating the output render texture for this material. The render
			 * texture is expected to have a single color attachment, and a depth attachment.
			 */
			static void GetOutputDesc(const RendererView& view, PooledRenderTextureCreateInformation& colorDesc, PooledRenderTextureCreateInformation& depthDesc);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	msaa				True if the shader will operate on a multisampled surface.
			 * @param	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static TetrahedraRenderMaterial* GetVariation(bool msaa, bool singleSampleMSAA);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mDepthBufferTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(IrradianceEvaluateUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(float, gSkyBrightness)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gNumTetrahedra)
		B3D_UNIFORM_BUFFER_END

		extern IrradianceEvaluateUniformDefinition gIrradianceEvaluateUniformDefinition;

		/** Evaluates radiance from the light probe volume, or the sky if light probes are not available. */
		class IrradianceEvaluateMaterial : public RendererMaterial<IrradianceEvaluateMaterial>
		{
			RMAT_DEF("IrradianceEvaluate.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool msaa, bool singleSampleMSAA, bool skyOnly>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MSAA", msaa),
					  ShaderVariationParameter("MSAA_RESOLVE_0TH", singleSampleMSAA),
					  ShaderVariationParameter("SKY_ONLY", skyOnly) });

				return variation;
			}

		public:
			IrradianceEvaluateMaterial() = default;
			void Initialize() override;

			/**
			 * Executes the material using the provided parameters.
			 *
			 * @param	commandBuffer		Command buffer to execute on.
			 * @param	view				View that is currently being rendered.
			 * @param	gbuffer				Previously rendered GBuffer textures.
			 * @param	lightProbeIndices	Indices calculated by TetrahedraRenderMaterial.
			 * @param	lightProbesInfo		Information about light probes.
			 * @param	skybox				Skybox, if available. If sky is not available, but sky rendering is enabled,
			 *								the system will instead use a default irradiance texture.
			 * @param	ambientOcclusion	Texture containing per-pixel ambient occlusion.
			 * @param	output				Output texture to write the radiance to. The evaluated value will be added to
			 *								existing radiance in the texture, using blending.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const GBufferTextures& gbuffer, const SPtr<Texture>& lightProbeIndices, const LightProbesInfo& lightProbesInfo, const Skybox* skybox, const SPtr<Texture>& ambientOcclusion, const SPtr<RenderTexture>& output);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	msaa				True if the shader will operate on a multisampled surface.
			 * @param	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @param	skyOnly				When true, only the sky irradiance will be evaluated. Otherwise light probe
			 *									irradiance will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static IrradianceEvaluateMaterial* GetVariation(bool msaa, bool singleSampleMSAA, bool skyOnly);

		private:
			GBufferParameterBinding mGBufferParams;
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
			GpuParameterSampledTexture mSkyIrradianceTextureParameter;
			GpuParameterSampledTexture mAmbientOcclusionTextureParameter;
			GpuParameterSampledTexture mSHCoeffsTextureParameter;
			GpuParameterStorageBuffer mTetrahedraBufferParameter;
			GpuParameterStorageBuffer mTetFacesBufferParameter;
			bool mSkyOnly;
		};

		/** Contains information required by light probe shaders. Output by LightProbes. */
		struct LightProbesInfo
		{
			/** Contains a set of spherical harmonic coefficients for every light probe. */
			SPtr<Texture> ShCoefficients;

			/**
			 * Contains information about tetrahedra formed by light probes. First half of the buffer is populated by actual
			 * tetrahedrons, while the second half is populated by information about outer faces (triangles). @p numTetrahedra
			 * marks the spot where split happens.
			 */
			SPtr<GpuBuffer> Tetrahedra;

			/** Contains additional information about outer tetrahedron faces, required for extrapolating tetrahedron data. */
			SPtr<GpuBuffer> Faces;

			/**
			 * Mesh representing the entire light probe volume. Each vertex has an associated tetrahedron (or face) index which
			 * can be used to map into the tetrahedra array to retrieve probe information.
			 */
			SPtr<Mesh> TetrahedraVolume;

			/** Total number of valid tetrahedra in the @p tetrahedra buffer. */
			u32 NumTetrahedra;
		};

		/** Handles any pre-processing for light (irradiance) probe lighting. */
		class LightProbes
		{
			/** Internal information about a single light probe volume. */
			struct VolumeInfo
			{
				/** Volume containing the information about the probes. */
				LightProbeVolume* Volume;
				/** Remains true as long as there are dirty probes in the volume. */
				bool IsDirty;
			};

			/**
			 * Information about a single tetrahedron, including neighbor information. Neighbor 4th index will be set to -1
			 * if the tetrahedron represents an outer face (which is not actually a tetrahedron, but a triangle, but is stored
			 * in the same array for convenience).
			 */
			struct TetrahedronData
			{
				Tetrahedron Volume;
				Matrix4 Transform;
			};

			/**
			 * Information about a single tetrahedron face, with information about extrusion and how to project a point in
			 * the extrusion volume, on to the face.
			 */
			struct TetrahedronFaceData
			{
				u32 InnerVertices[3];
				u32 OuterVertices[3];
				Vector3 Normals[3];
				Matrix4 Transform;
				u32 Tetrahedron;
				bool Quadratic;
			};

		public:
			LightProbes();

			/** Notifies sthe manager that the provided light probe volume has been added. */
			void NotifyAdded(LightProbeVolume* volume);

			/** Notifies the manager that the provided light probe volume has some dirty light probes. */
			void NotifyDirty(LightProbeVolume* volume);

			/** Notifies the manager that all the probes in the provided volume have been removed. */
			void NotifyRemoved(LightProbeVolume* volume);

			/** Updates light probe tetrahedron data after probes changed (added/removed/moved). */
			void UpdateProbes(GpuCommandBuffer& commandBuffer);

			/** Returns true if there are any registered light probes. */
			bool HasAnyProbes() const;

			/**
			 * Returns a set of buffers that can be used for rendering the light probes. updateProbes() must be called
			 * at least once before the buffer is populated. If the probes changed since the last call, call updateProbes()
			 * to refresh the buffer.
			 */
			LightProbesInfo GetInfo() const;

		private:
			/**
			 * Perform tetrahedrization of the provided point list, and outputs a list of tetrahedrons and outer faces of the
			 * volume. Each entry contains connections to nearby tetrahedrons/faces, as well as a matrix that can be used for
			 * calculating barycentric coordinates within the tetrahedron (or projected triangle barycentric coordinates for
			 * faces).
			 *
			 * @param[in,out]	positions					A set of positions to generate the tetrahedra from. If
			 *												@p generateExtrapolationVolume is enabled then this array will be
			 *												appended with new vertices forming that volume.
			 * @param[out]		tetrahedra					A list of generated tetrahedra and relevant data.
			 * @param[out]		faces						A list of faces representing the surface of the tetrahedra volume.
			 * @param[in]		generateExtrapolationVolume	If true, the tetrahedron volume will be surrounded with points
			 *												at "infinity" (technically just far away).
			 */
			void GenerateTetrahedronData(Vector<Vector3>& positions, Vector<TetrahedronData>& tetrahedra, Vector<TetrahedronFaceData>& faces, bool generateExtrapolationVolume = false);

			/** Resizes the GPU buffer used for holding tetrahedron data, to the specified size (in number of tetraheda). */
			void ResizeTetrahedronBuffer(u32 count);

			/** Resizes the GPU buffer used for holding tetrahedron face data, to the specified size (in number of faces). */
			void ResizeTetrahedronFaceBuffer(u32 count);

			/**
			 * Resized the GPU buffer that stores light probe SH coefficients, to the specified number of rows (each row
			 * holds 4096 coefficients, and each volume starts in its own row.).
			 */
			void ResizeCoefficientTexture(GpuCommandBuffer& commandBuffer, u32 numRows);

			SPtr<GpuDevice> mGpuDevice;
			Vector<VolumeInfo> mVolumes;
			bool mTetrahedronVolumeDirty;

			u32 mMaxCoefficientRows;
			u32 mMaxTetrahedra;
			u32 mMaxFaces;

			Vector<TetrahedronData> mTetrahedronInfos;

			SPtr<Texture> mProbeCoefficientsGPU;
			SPtr<GpuBuffer> mTetrahedronInfosGPU;
			SPtr<GpuBuffer> mTetrahedronFaceInfosGPU;
			SPtr<Mesh> mVolumeMesh;
			u32 mNumValidTetrahedra;

			// Temporary buffers
			Vector<Vector3> mTempTetrahedronPositions;
			Vector<u32> mTempTetrahedronBufferIndices;
			Vector<Vector2I> mTempTetrahedronBufferOffsets;
		};

		/** @} */
	} // namespace render
} // namespace b3d
