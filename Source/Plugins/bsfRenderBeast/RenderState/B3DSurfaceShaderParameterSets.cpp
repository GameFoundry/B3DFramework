//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DSurfaceShaderParameterSets.h"
#include "B3DRenderBeast.h"
#include "B3DRenderBeastScene.h"
#include "B3DRendererView.h"
#include "Shading/B3DLightGrid.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuParameterSetPool.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRenderQueue.h"
#include "FrameGraph/B3DFrameGraph.h"
#include "Components/B3DSkybox.h"
#include "Utility/B3DRendererTextures.h"

namespace b3d {
namespace render {

namespace
{
	/** Register numbers of the renderer inputs, as the shader includes declare them in set 2. Unique across register classes. */
	namespace InputRegister
	{
		constexpr u32 kPerCamera = 0;
		constexpr u32 kPerFrame = 1;
		constexpr u32 kSceneDepth = 2;
		constexpr u32 kMask = 3;
		constexpr u32 kGridParams = 4;
		constexpr u32 kGridLightOffsetsAndSize = 5;
		constexpr u32 kGridProbeOffsetsAndSize = 6;
		constexpr u32 kLightIndices = 7;
		constexpr u32 kLights = 8;
		constexpr u32 kReflectionProbeIndices = 9;
		constexpr u32 kReflectionProbes = 10;
		constexpr u32 kReflectionProbeParams = 11;
		constexpr u32 kSkyReflectionTexture = 12;
		constexpr u32 kSkyReflectionSampler = 13;
		constexpr u32 kReflectionProbeCubemaps = 14;
		constexpr u32 kReflectionProbeSampler = 15;
		constexpr u32 kAmbientOcclusionTexture = 16;
		constexpr u32 kAmbientOcclusionSampler = 17;
		constexpr u32 kSSRTexture = 18;
		constexpr u32 kSSRSampler = 19;
		constexpr u32 kPreintegratedEnvBRDFTexture = 20;
		constexpr u32 kPreintegratedEnvBRDFSampler = 21;
	}

	/** Returns the binding of the renderer input at @p registerIndex of @p registerClass in set 2. */
	GpuParameterBinding GetInputBinding(GpuDevice& device, ShaderRegisterClass registerClass, u32 registerIndex)
	{
		return GpuParameterBinding(GpuPipelineSet::kSurfaceShaderInputSetIndex, device.GetParameterSlot(registerClass, registerIndex));
	}

	/** Builds the description of a set 2 layout. */
	class InputLayoutBuilder
	{
	public:
		explicit InputLayoutBuilder(GpuDevice& device)
			: mDevice(device)
		{}

		/** Adds a uniform buffer of @p sizeInBytes, as defined by its renderer-side definition. */
		void AddUniformBuffer(const char* name, u32 registerIndex, u32 sizeInBytes)
		{
			GpuUniformBufferInformation information;
			information.Name = name;
			information.Set = GpuPipelineSet::kSurfaceShaderInputSetIndex;
			information.Slot = mDevice.GetParameterSlot(ShaderRegisterClass::ConstantBuffer, registerIndex);
			information.Size = Math::CeilToMultiple(sizeInBytes / 4u, 4u);
			information.Stages = kStages;
			information.IsShareable = true;

			mDescription.UniformBuffers[information.Name] = information;
		}

		/** Adds a sampled texture. */
		void AddTexture(const char* name, u32 registerIndex, GpuParameterObjectType type)
		{
			mDescription.SampledTextures[name] = CreateObject(name, ShaderRegisterClass::ShaderResource, registerIndex, type, BF_UNKNOWN);
		}

		/** Adds a read-only buffer. */
		void AddBuffer(const char* name, u32 registerIndex, GpuParameterObjectType type, GpuBufferFormat elementType)
		{
			mDescription.Buffers[name] = CreateObject(name, ShaderRegisterClass::ShaderResource, registerIndex, type, elementType);
		}

		/** Adds a sampler. */
		void AddSampler(const char* name, u32 registerIndex)
		{
			mDescription.Samplers[name] = CreateObject(name, ShaderRegisterClass::Sampler, registerIndex, GPOT_SAMPLER2D, BF_UNKNOWN);
		}

		/** Creates the layout from the added resources. */
		TShared<GpuPipelineParameterSetLayout> Create() const { return mDevice.CreateGpuPipelineParameterSetLayout(mDescription); }

	private:
		inline static const GpuProgramStageBits kStages = GpuProgramStageBit::Vertex | GpuProgramStageBit::Fragment;

		GpuObjectParameterInformation CreateObject(const char* name, ShaderRegisterClass registerClass, u32 registerIndex, GpuParameterObjectType type, GpuBufferFormat elementType) const
		{
			GpuObjectParameterInformation information;
			information.Name = name;
			information.Type = type;
			information.Set = GpuPipelineSet::kSurfaceShaderInputSetIndex;
			information.Slot = mDevice.GetParameterSlot(registerClass, registerIndex);
			information.ElementType = elementType;
			information.Stages = kStages;

			return information;
		}

		GpuDevice& mDevice;
		GpuProgramParameterDescription mDescription;
	};

	/** Adds the inputs of the LitDeferred layout, which every other layout extends. */
	void AddBasicInputs(InputLayoutBuilder& builder)
	{
		builder.AddUniformBuffer("PerCamera", InputRegister::kPerCamera, gPerCameraUniformDefinition.GetSize());
		builder.AddUniformBuffer("PerFrame", InputRegister::kPerFrame, gPerFrameUniformDefinition.GetSize());
	}

	/** Adds the scene depth input. */
	void AddSceneDepthInput(InputLayoutBuilder& builder, bool msaa)
	{
		builder.AddTexture("gDepthBufferTex", InputRegister::kSceneDepth, msaa ? GPOT_TEXTURE2DMS : GPOT_TEXTURE2D);
	}

	/** Adds the forward lighting and image based lighting inputs of the LitForward layout. */
	void AddLightingInputs(InputLayoutBuilder& builder)
	{
		builder.AddUniformBuffer("GridParams", InputRegister::kGridParams, gLightGridUniformDefinition.GetSize());
		builder.AddBuffer("gGridLightOffsetsAndSize", InputRegister::kGridLightOffsetsAndSize, GPOT_BYTE_BUFFER, BF_32X4U);
		builder.AddBuffer("gGridProbeOffsetsAndSize", InputRegister::kGridProbeOffsetsAndSize, GPOT_BYTE_BUFFER, BF_32X2U);
		builder.AddBuffer("gLightIndices", InputRegister::kLightIndices, GPOT_BYTE_BUFFER, BF_32X1U);
		builder.AddBuffer("gLights", InputRegister::kLights, GPOT_STRUCTURED_BUFFER, BF_UNKNOWN);
		builder.AddBuffer("gReflectionProbeIndices", InputRegister::kReflectionProbeIndices, GPOT_BYTE_BUFFER, BF_32X1U);
		builder.AddBuffer("gReflectionProbes", InputRegister::kReflectionProbes, GPOT_STRUCTURED_BUFFER, BF_UNKNOWN);
		builder.AddUniformBuffer("ReflProbeParams", InputRegister::kReflectionProbeParams, gGlobalReflectionProbeUniformBufferDefinition.GetSize());
		builder.AddTexture("gSkyReflectionTex", InputRegister::kSkyReflectionTexture, GPOT_TEXTURECUBE);
		builder.AddSampler("gSkyReflectionSamp", InputRegister::kSkyReflectionSampler);
		builder.AddTexture("gReflProbeCubemaps", InputRegister::kReflectionProbeCubemaps, GPOT_TEXTURECUBEARRAY);
		builder.AddSampler("gReflProbeSamp", InputRegister::kReflectionProbeSampler);
		builder.AddTexture("gAmbientOcclusionTex", InputRegister::kAmbientOcclusionTexture, GPOT_TEXTURE2D);
		builder.AddSampler("gAmbientOcclusionSamp", InputRegister::kAmbientOcclusionSampler);
		builder.AddTexture("gSSRTex", InputRegister::kSSRTexture, GPOT_TEXTURE2D);
		builder.AddSampler("gSSRSamp", InputRegister::kSSRSampler);
		builder.AddTexture("gPreintegratedEnvBRDF", InputRegister::kPreintegratedEnvBRDFTexture, GPOT_TEXTURE2D);
		builder.AddSampler("gPreintegratedEnvBRDFSamp", InputRegister::kPreintegratedEnvBRDFSampler);
	}

	/** Creates the Decal layout, with multisampled scene inputs if @p msaa. */
	TShared<GpuPipelineParameterSetLayout> CreateDecalLayout(GpuDevice& device, bool msaa)
	{
		InputLayoutBuilder builder(device);
		AddBasicInputs(builder);
		AddSceneDepthInput(builder, msaa);
		builder.AddTexture("gMaskTex", InputRegister::kMask, msaa ? GPOT_TEXTURE2DMS : GPOT_TEXTURE2D);

		return builder.Create();
	}

}

void SurfaceShaderParameterSetInfo::Initialize(GpuDevice& device)
{
	{
		InputLayoutBuilder builder(device);
		AddBasicInputs(builder);
		mLayouts[(u32)SurfaceShaderType::LitDeferred] = builder.Create();
	}

	// Unlit surfaces declare the same inputs as LitDeferred ones
	mLayouts[(u32)SurfaceShaderType::Unlit] = mLayouts[(u32)SurfaceShaderType::LitDeferred];

	{
		InputLayoutBuilder builder(device);
		AddBasicInputs(builder);
		AddLightingInputs(builder);
		mLayouts[(u32)SurfaceShaderType::LitForward] = builder.Create();
	}

	{
		InputLayoutBuilder builder(device);
		AddBasicInputs(builder);
		AddSceneDepthInput(builder, false);
		mLayouts[(u32)SurfaceShaderType::UnlitParticle] = builder.Create();
	}

	mLayouts[(u32)SurfaceShaderType::Decal] = CreateDecalLayout(device, false);
	mDecalMSAALayout = CreateDecalLayout(device, true);
}

const TShared<GpuPipelineParameterSetLayout>& SurfaceShaderParameterSetInfo::Get(SurfaceShaderType type, bool msaa) const
{
	if(type == SurfaceShaderType::Decal && msaa)
		return mDecalMSAALayout;

	return mLayouts[(u32)type];
}

void SurfaceShaderParameterSets::Prepare(const RendererView& view, const RenderBeastScene& scene)
{
	const SurfaceShaderParameterSetInfo& parameterSetInfo = GetRenderBeast()->GetSurfaceShaderParameterSetInfo();
	const bool msaa = view.GetProperties().Target.NumSamples > 1;

	GpuDevice& device = *GetRenderBeast()->GetGpuDevice();
	GpuParameterSetPool& pool = GetRenderer()->GetGpuContext().GetParameterSetPool();
	for(u32 typeIndex = 0; typeIndex < (u32)SurfaceShaderType::Count; typeIndex++)
	{
		const SurfaceShaderType type = (SurfaceShaderType)typeIndex;
		const TShared<GpuPipelineParameterSetLayout>& layout = parameterSetInfo.Get(type, msaa);

		TShared<GpuParameterSet>& set = mSets[typeIndex];
		if(set == nullptr || set->GetLayout() != layout)
		{
			set = pool.Create(layout, GpuPipelineSet::kSurfaceShaderInputSetIndex);

			mPerCameraParameters[typeIndex] = GpuParameterUniformBuffer(GetInputBinding(device, ShaderRegisterClass::ConstantBuffer, InputRegister::kPerCamera), set);
			mPerFrameParameters[typeIndex] = GpuParameterUniformBuffer(GetInputBinding(device, ShaderRegisterClass::ConstantBuffer, InputRegister::kPerFrame), set);

			if(type == SurfaceShaderType::UnlitParticle)
				mUnlitParticleDepthParameter = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kSceneDepth), set);
			else if(type == SurfaceShaderType::Decal)
			{
				mDecalDepthParameter = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kSceneDepth), set);
				mDecalMaskParameter = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kMask), set);
			}
			else if(type == SurfaceShaderType::LitForward)
			{
				LitForwardParameters& parameters = mLitForwardParameters;
				parameters.GridParams = GpuParameterUniformBuffer(GetInputBinding(device, ShaderRegisterClass::ConstantBuffer, InputRegister::kGridParams), set);
				parameters.GridLightOffsetsAndSize = GpuParameterStorageBuffer(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kGridLightOffsetsAndSize), set);
				parameters.GridProbeOffsetsAndSize = GpuParameterStorageBuffer(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kGridProbeOffsetsAndSize), set);
				parameters.LightIndices = GpuParameterStorageBuffer(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kLightIndices), set);
				parameters.Lights = GpuParameterStorageBuffer(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kLights), set);
				parameters.ReflectionProbeIndices = GpuParameterStorageBuffer(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kReflectionProbeIndices), set);
				parameters.ReflectionProbes = GpuParameterStorageBuffer(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kReflectionProbes), set);
				parameters.ReflectionProbeParams = GpuParameterUniformBuffer(GetInputBinding(device, ShaderRegisterClass::ConstantBuffer, InputRegister::kReflectionProbeParams), set);
				parameters.SkyReflectionTexture = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kSkyReflectionTexture), set);
				parameters.ReflectionProbeCubemaps = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kReflectionProbeCubemaps), set);
				parameters.AmbientOcclusionTexture = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kAmbientOcclusionTexture), set);
				parameters.SSRTexture = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kSSRTexture), set);
				parameters.PreintegratedEnvBRDF = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kPreintegratedEnvBRDFTexture), set);

				// Set 2 has no material to apply the sampler states the shader include declares, so assign matching ones
				SamplerStateCreateInformation pointClamp;
				pointClamp.MinFilter = FO_POINT;
				pointClamp.MagFilter = FO_POINT;
				pointClamp.MipFilter = FO_POINT;
				pointClamp.AddressMode.U = TAM_CLAMP;
				pointClamp.AddressMode.V = TAM_CLAMP;
				const TShared<SamplerState> pointClampState = device.FindOrCreateSamplerState(pointClamp);

				SamplerStateCreateInformation linearClamp;
				linearClamp.AddressMode.U = TAM_CLAMP;
				linearClamp.AddressMode.V = TAM_CLAMP;
				const TShared<SamplerState> linearClampState = device.FindOrCreateSamplerState(linearClamp);

				const TShared<SamplerState> defaultState = device.FindOrCreateSamplerState(SamplerStateCreateInformation());

				GpuParameterSampler(GetInputBinding(device, ShaderRegisterClass::Sampler, InputRegister::kSkyReflectionSampler), set).Set(defaultState);
				GpuParameterSampler(GetInputBinding(device, ShaderRegisterClass::Sampler, InputRegister::kReflectionProbeSampler), set).Set(defaultState);
				GpuParameterSampler(GetInputBinding(device, ShaderRegisterClass::Sampler, InputRegister::kAmbientOcclusionSampler), set).Set(pointClampState);
				GpuParameterSampler(GetInputBinding(device, ShaderRegisterClass::Sampler, InputRegister::kSSRSampler), set).Set(pointClampState);
				GpuParameterSampler(GetInputBinding(device, ShaderRegisterClass::Sampler, InputRegister::kPreintegratedEnvBRDFSampler), set).Set(linearClampState);
			}
		}

		mPerCameraParameters[typeIndex].Set(view.GetPerViewBuffer());
		mPerFrameParameters[typeIndex].Set(scene.GetPerFrameSuballocation());
	}
}

void SurfaceShaderParameterSets::SetSceneDepth(const TShared<Texture>& depth)
{
	mUnlitParticleDepthParameter.Set(depth);
}

void SurfaceShaderParameterSets::SetDecalInputs(const TShared<Texture>& depth, const TShared<Texture>& mask)
{
	mDecalDepthParameter.Set(depth);
	mDecalMaskParameter.Set(mask);
}

void SurfaceShaderParameterSets::SetForwardLightingInputs(const RendererView& view, const VisibleLightData& lights,
	const VisibleReflectionProbeData& reflectionProbes, const RenderBeastScene& scene, const Skybox* skybox)
{
	GpuBufferSuballocation reflectionProbeParameters = gGlobalReflectionProbeUniformBufferDefinition.AllocateTransient();
	ReflectionProbeRenderState::PopulateGlobalReflectionProbeUniformBuffer(reflectionProbeParameters, skybox, reflectionProbes.GetProbeCount(),
		scene.GetReflectionProbeCubemapsTex(), view.GetProperties().CapturingReflections);

	TShared<Texture> skyFilteredRadiance;
	if(skybox != nullptr)
		skyFilteredRadiance = skybox->GetFilteredRadiance();

	const LightGridOutputs lightGridOutputs = view.GetLightGrid().GetOutputs();

	const LitForwardParameters& parameters = mLitForwardParameters;
	parameters.GridParams.Set(lightGridOutputs.UniformBuffer);
	parameters.GridLightOffsetsAndSize.Set(lightGridOutputs.GridLightOffsetsAndSize);
	parameters.GridProbeOffsetsAndSize.Set(lightGridOutputs.GridProbeOffsetsAndSize);
	parameters.LightIndices.Set(lightGridOutputs.GridLightIndices);
	parameters.Lights.Set(lights.GetLightBuffer());
	parameters.ReflectionProbeIndices.Set(lightGridOutputs.GridProbeIndices);
	parameters.ReflectionProbes.Set(reflectionProbes.GetProbeBuffer());
	parameters.ReflectionProbeParams.Set(reflectionProbeParameters);
	parameters.SkyReflectionTexture.Set(skyFilteredRadiance);
	parameters.ReflectionProbeCubemaps.Set(scene.GetReflectionProbeCubemapsTex());
	parameters.AmbientOcclusionTexture.Set(Texture::kWhite); // Note: Add SSAO here?
	parameters.SSRTexture.Set(Texture::kBlack); // Note: Add SSR here?
	parameters.PreintegratedEnvBRDF.Set(RendererTextures::preintegratedEnvGF);
}

void SurfaceShaderParameterSets::DeclareSets(FrameGraphRasterPass& pass, const Vector<RenderQueueEntry>& entries) const
{
	bool declared[(u32)SurfaceShaderType::Count] = {};
	for(const RenderQueueEntry& entry : entries)
	{
		const u32 type = entry.DrawCommand->SurfaceShaderType;
		if(!declared[type])
		{
			pass.AddParameterSet(mSets[type]);
			declared[type] = true;
		}
	}
}

void SurfaceShaderParameterSets::Bind(GpuCommandBuffer& commandBuffer, const RenderQueueEntry& entry, u32& boundType) const
{
	const u32 type = entry.DrawCommand->SurfaceShaderType;
	if(type != boundType)
	{
		commandBuffer.SetGpuParameterSet(mSets[type]);
		boundType = type;
	}
}

}} // namespace b3d::render
