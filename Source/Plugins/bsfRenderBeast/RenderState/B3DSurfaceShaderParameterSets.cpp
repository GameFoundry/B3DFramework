//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DSurfaceShaderParameterSets.h"
#include "B3DRenderBeast.h"
#include "B3DRenderBeastScene.h"
#include "B3DRendererView.h"
#include "Shading/B3DLightGrid.h"
#include "Shading/B3DGpuParticleSimulation.h"
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
		constexpr u32 kGpuParticlePositionAndTime = 22;
		constexpr u32 kGpuParticleSizeAndRotation = 23;
		constexpr u32 kGpuParticleCurves = 24;
		constexpr u32 kGpuParticleCurvesSampler = 25;
		constexpr u32 kGpuParticleSortedIndices = 26;
		constexpr u32 kParticleIndices = 27;
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

	/** Adds the outputs of the GPU particle simulation, which the GPU particle layouts add to their base layout. */
	void AddGpuParticleInputs(InputLayoutBuilder& builder)
	{
		builder.AddTexture("gPositionTimeTex", InputRegister::kGpuParticlePositionAndTime, GPOT_TEXTURE2D);
		builder.AddTexture("gSizeRotationTex", InputRegister::kGpuParticleSizeAndRotation, GPOT_TEXTURE2D);
		builder.AddTexture("gCurvesTex", InputRegister::kGpuParticleCurves, GPOT_TEXTURE2D);
		builder.AddSampler("gCurvesSampler", InputRegister::kGpuParticleCurvesSampler);
		builder.AddBuffer("gSortedIndices", InputRegister::kGpuParticleSortedIndices, GPOT_BYTE_BUFFER, BF_32X2U);
	}

	/** Adds the view's indices of CPU simulated particles, which the CPU particle layouts add to their base layout. */
	void AddCpuParticleInputs(InputLayoutBuilder& builder)
	{
		builder.AddBuffer("gParticleIndices", InputRegister::kParticleIndices, GPOT_BYTE_BUFFER, BF_32X2U);
	}

	/** Creates a table mapping each SurfaceShaderType to its position in @p types, or to -1 for types not in it. */
	template<size_t N>
	constexpr Array<i8, (u32)SurfaceShaderType::Count> CreateIndexTable(const SurfaceShaderType (&types)[N])
	{
		Array<i8, (u32)SurfaceShaderType::Count> table = {};
		for(u32 i = 0; i < (u32)SurfaceShaderType::Count; i++)
			table[i] = -1;

		for(u32 i = 0; i < (u32)N; i++)
			table[(u32)types[i]] = (i8)i;

		return table;
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

constexpr Array<i8, (u32)SurfaceShaderType::Count> SurfaceShaderParameterSets::kUnlitParticleIndices = CreateIndexTable(kUnlitParticleTypes);
constexpr Array<i8, (u32)SurfaceShaderType::Count> SurfaceShaderParameterSets::kLitForwardIndices = CreateIndexTable(kLitForwardTypes);
constexpr Array<i8, (u32)SurfaceShaderType::Count> SurfaceShaderParameterSets::kCpuParticleIndices = CreateIndexTable(kCpuParticleTypes);
constexpr Array<i8, (u32)SurfaceShaderType::Count> SurfaceShaderParameterSets::kGpuParticleIndices = CreateIndexTable(kGpuParticleTypes);

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
		AddCpuParticleInputs(builder);
		mLayouts[(u32)SurfaceShaderType::CpuParticleUnlit] = builder.Create();
	}

	{
		InputLayoutBuilder builder(device);
		AddBasicInputs(builder);
		AddCpuParticleInputs(builder);
		mLayouts[(u32)SurfaceShaderType::CpuParticleLitDeferred] = builder.Create();
	}

	{
		InputLayoutBuilder builder(device);
		AddBasicInputs(builder);
		AddLightingInputs(builder);
		AddCpuParticleInputs(builder);
		mLayouts[(u32)SurfaceShaderType::CpuParticleLitForward] = builder.Create();
	}

	mLayouts[(u32)SurfaceShaderType::Decal] = CreateDecalLayout(device, false);
	mDecalMSAALayout = CreateDecalLayout(device, true);

	{
		InputLayoutBuilder builder(device);
		AddBasicInputs(builder);
		AddGpuParticleInputs(builder);
		mLayouts[(u32)SurfaceShaderType::GpuParticleLitDeferred] = builder.Create();
	}

	{
		InputLayoutBuilder builder(device);
		AddBasicInputs(builder);
		AddLightingInputs(builder);
		AddGpuParticleInputs(builder);
		mLayouts[(u32)SurfaceShaderType::GpuParticleLitForward] = builder.Create();
	}

	{
		InputLayoutBuilder builder(device);
		AddBasicInputs(builder);
		AddSceneDepthInput(builder, false);
		AddGpuParticleInputs(builder);
		mLayouts[(u32)SurfaceShaderType::GpuParticleUnlit] = builder.Create();
	}
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

			mPerCameraParameters[typeIndex] = GpuParameterUniformBuffer(GetInputBinding(device, ShaderRegisterClass::ConstantBuffer, InputRegister::kPerCamera), layout.get());
			mPerFrameParameters[typeIndex] = GpuParameterUniformBuffer(GetInputBinding(device, ShaderRegisterClass::ConstantBuffer, InputRegister::kPerFrame), layout.get());

			const i8 unlitParticleIndex = kUnlitParticleIndices[typeIndex];
			const i8 litForwardIndex = kLitForwardIndices[typeIndex];
			const i8 gpuParticleIndex = kGpuParticleIndices[typeIndex];
			const i8 cpuParticleIndex = kCpuParticleIndices[typeIndex];

			if(unlitParticleIndex >= 0)
			{
				mUnlitParticleDepthParameters[unlitParticleIndex] = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kSceneDepth), layout.get());
			}
			else if(type == SurfaceShaderType::Decal)
			{
				mDecalDepthParameter = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kSceneDepth), layout.get());
				mDecalMaskParameter = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kMask), layout.get());
			}
			else if(litForwardIndex >= 0)
			{
				LitForwardParameters& parameters = mLitForwardParameters[litForwardIndex];
				parameters.GridParams = GpuParameterUniformBuffer(GetInputBinding(device, ShaderRegisterClass::ConstantBuffer, InputRegister::kGridParams), layout.get());
				parameters.GridLightOffsetsAndSize = GpuParameterStorageBuffer(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kGridLightOffsetsAndSize), layout.get());
				parameters.GridProbeOffsetsAndSize = GpuParameterStorageBuffer(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kGridProbeOffsetsAndSize), layout.get());
				parameters.LightIndices = GpuParameterStorageBuffer(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kLightIndices), layout.get());
				parameters.Lights = GpuParameterStorageBuffer(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kLights), layout.get());
				parameters.ReflectionProbeIndices = GpuParameterStorageBuffer(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kReflectionProbeIndices), layout.get());
				parameters.ReflectionProbes = GpuParameterStorageBuffer(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kReflectionProbes), layout.get());
				parameters.ReflectionProbeParams = GpuParameterUniformBuffer(GetInputBinding(device, ShaderRegisterClass::ConstantBuffer, InputRegister::kReflectionProbeParams), layout.get());
				parameters.SkyReflectionTexture = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kSkyReflectionTexture), layout.get());
				parameters.ReflectionProbeCubemaps = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kReflectionProbeCubemaps), layout.get());
				parameters.AmbientOcclusionTexture = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kAmbientOcclusionTexture), layout.get());
				parameters.SSRTexture = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kSSRTexture), layout.get());
				parameters.PreintegratedEnvBRDF = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kPreintegratedEnvBRDFTexture), layout.get());

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

				GpuParameterSampler(GetInputBinding(device, ShaderRegisterClass::Sampler, InputRegister::kSkyReflectionSampler), layout.get()).Set(set, defaultState);
				GpuParameterSampler(GetInputBinding(device, ShaderRegisterClass::Sampler, InputRegister::kReflectionProbeSampler), layout.get()).Set(set, defaultState);
				GpuParameterSampler(GetInputBinding(device, ShaderRegisterClass::Sampler, InputRegister::kAmbientOcclusionSampler), layout.get()).Set(set, pointClampState);
				GpuParameterSampler(GetInputBinding(device, ShaderRegisterClass::Sampler, InputRegister::kSSRSampler), layout.get()).Set(set, pointClampState);
				GpuParameterSampler(GetInputBinding(device, ShaderRegisterClass::Sampler, InputRegister::kPreintegratedEnvBRDFSampler), layout.get()).Set(set, linearClampState);
			}

			if(cpuParticleIndex >= 0)
				mCpuParticleIndicesParameters[cpuParticleIndex] = GpuParameterStorageBuffer(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kParticleIndices), layout.get());

			if(gpuParticleIndex >= 0)
			{
				GpuParticleParameters& parameters = mGpuParticleParameters[gpuParticleIndex];
				parameters.PositionAndTime = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kGpuParticlePositionAndTime), layout.get());
				parameters.SizeAndRotation = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kGpuParticleSizeAndRotation), layout.get());
				parameters.Curves = GpuParameterSampledTexture(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kGpuParticleCurves), layout.get());
				parameters.SortedIndices = GpuParameterStorageBuffer(GetInputBinding(device, ShaderRegisterClass::ShaderResource, InputRegister::kGpuParticleSortedIndices), layout.get());

				// The shader declares no state for the curve sampler, so it is the default one
				const TShared<SamplerState> defaultState = device.FindOrCreateSamplerState(SamplerStateCreateInformation());
				GpuParameterSampler(GetInputBinding(device, ShaderRegisterClass::Sampler, InputRegister::kGpuParticleCurvesSampler), layout.get()).Set(set, defaultState);
			}
		}

		mPerCameraParameters[typeIndex].Set(set, view.GetPerViewBuffer());
		mPerFrameParameters[typeIndex].Set(set, scene.GetPerFrameSuballocation());
	}
}

void SurfaceShaderParameterSets::SetSceneDepth(const TShared<Texture>& depth)
{
	for(u32 typeIndex = 0; typeIndex < (u32)SurfaceShaderType::Count; typeIndex++)
	{
		const i8 unlitParticleIndex = kUnlitParticleIndices[typeIndex];
		if(unlitParticleIndex >= 0)
			mUnlitParticleDepthParameters[unlitParticleIndex].Set(mSets[typeIndex], depth);
	}
}

void SurfaceShaderParameterSets::SetGpuParticleInputs(const GpuParticleDrawInputs& inputs)
{
	for(u32 typeIndex = 0; typeIndex < (u32)SurfaceShaderType::Count; typeIndex++)
	{
		const i8 gpuParticleIndex = kGpuParticleIndices[typeIndex];
		if(gpuParticleIndex < 0)
			continue;

		const TShared<GpuParameterSet>& set = mSets[typeIndex];
		const GpuParticleParameters& parameters = mGpuParticleParameters[gpuParticleIndex];
		parameters.PositionAndTime.Set(set, inputs.PositionAndTime);
		parameters.SizeAndRotation.Set(set, inputs.SizeAndRotation);
		parameters.Curves.Set(set, inputs.Curves);
		parameters.SortedIndices.Set(set, inputs.SortedIndices);
	}
}

void SurfaceShaderParameterSets::SetCpuParticleIndices(const TShared<GpuBuffer>& indices)
{
	for(u32 typeIndex = 0; typeIndex < (u32)SurfaceShaderType::Count; typeIndex++)
	{
		const i8 cpuParticleIndex = kCpuParticleIndices[typeIndex];
		if(cpuParticleIndex >= 0)
			mCpuParticleIndicesParameters[cpuParticleIndex].Set(mSets[typeIndex], indices);
	}
}

void SurfaceShaderParameterSets::SetDecalInputs(const TShared<Texture>& depth, const TShared<Texture>& mask)
{
	const TShared<GpuParameterSet>& set = mSets[(u32)SurfaceShaderType::Decal];
	mDecalDepthParameter.Set(set, depth);
	mDecalMaskParameter.Set(set, mask);
}

void SurfaceShaderParameterSets::SetForwardLightingInputs(const RendererView& view, const LightGridOutputs& lightGrid, const VisibleLightData& lights,
	const VisibleReflectionProbeData& reflectionProbes, const RenderBeastScene& scene, const Skybox* skybox)
{
	GpuBufferSuballocation reflectionProbeParameters = gGlobalReflectionProbeUniformBufferDefinition.AllocateTransient();
	ReflectionProbeRenderState::PopulateGlobalReflectionProbeUniformBuffer(reflectionProbeParameters, skybox, reflectionProbes.GetProbeCount(),
		scene.GetReflectionProbeCubemapsTex(), view.GetProperties().CapturingReflections);

	TShared<Texture> skyFilteredRadiance;
	if(skybox != nullptr)
		skyFilteredRadiance = skybox->GetFilteredRadiance();

	for(u32 typeIndex = 0; typeIndex < (u32)SurfaceShaderType::Count; typeIndex++)
	{
		const i8 litForwardIndex = kLitForwardIndices[typeIndex];
		if(litForwardIndex < 0)
			continue;

		const TShared<GpuParameterSet>& set = mSets[typeIndex];
		const LitForwardParameters& parameters = mLitForwardParameters[litForwardIndex];
		parameters.GridParams.Set(set, lightGrid.UniformBuffer);
		parameters.GridLightOffsetsAndSize.Set(set, lightGrid.GridLightOffsetsAndSize);
		parameters.GridProbeOffsetsAndSize.Set(set, lightGrid.GridProbeOffsetsAndSize);
		parameters.LightIndices.Set(set, lightGrid.GridLightIndices);
		parameters.Lights.Set(set, lights.GetLightBuffer());
		parameters.ReflectionProbeIndices.Set(set, lightGrid.GridProbeIndices);
		parameters.ReflectionProbes.Set(set, reflectionProbes.GetProbeBuffer());
		parameters.ReflectionProbeParams.Set(set, reflectionProbeParameters);
		parameters.SkyReflectionTexture.Set(set, skyFilteredRadiance);
		parameters.ReflectionProbeCubemaps.Set(set, scene.GetReflectionProbeCubemapsTex());
		parameters.AmbientOcclusionTexture.Set(set, Texture::kWhite); // Note: Add SSAO here?
		parameters.SSRTexture.Set(set, Texture::kBlack); // Note: Add SSR here?
		parameters.PreintegratedEnvBRDF.Set(set, RendererTextures::preintegratedEnvGF);
	}
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
