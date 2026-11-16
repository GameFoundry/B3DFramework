//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuPipelineState.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuProgram.h"
#include "B3DD3D12Utility.h"
#include "B3DD3D12GpuPipelineParameterLayout.h"
#include "B3DD3D12ResourceManager.h"
#include "B3DD3D12VertexInput.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "Utility/B3DTimer.h"

using namespace b3d;
using namespace b3d::render;

bool D3D12PipelineVariantKey::operator==(const D3D12PipelineVariantKey& other) const
{
	if (RenderTargetCount != other.RenderTargetCount || DepthStencilFormat != other.DepthStencilFormat || SampleCount != other.SampleCount || TopologyType != other.TopologyType || VertexInputId != other.VertexInputId)
		return false;

	for (u32 i = 0; i < RenderTargetCount; i++)
	{
		if (RenderTargetFormats[i] != other.RenderTargetFormats[i])
			return false;
	}

	return true;
}

size_t D3D12PipelineVariantKey::Hash::operator()(const D3D12PipelineVariantKey& key) const
{
	size_t hash = 0;
	B3DCombineHash(hash, key.RenderTargetCount);
	B3DCombineHash(hash, (u32)key.DepthStencilFormat);
	B3DCombineHash(hash, key.SampleCount);
	B3DCombineHash(hash, (u32)key.TopologyType);
	B3DCombineHash(hash, key.VertexInputId);

	for (u32 i = 0; i < key.RenderTargetCount; i++)
		B3DCombineHash(hash, (u32)key.RenderTargetFormats[i]);

	return hash;
}

D3D12GpuGraphicsPipelineState::D3D12GpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuDevice& device)
	: GpuGraphicsPipelineState(device, createInformation)
{
}

D3D12GpuGraphicsPipelineState::~D3D12GpuGraphicsPipelineState()
{
	for (auto& pipeline : mPipelines)
		pipeline.second->Destroy();
}

void D3D12GpuGraphicsPipelineState::Initialize()
{
	GpuGraphicsPipelineState::Initialize();

	if (mData.VertexProgram != nullptr)
		mVertexDescription = mData.VertexProgram->GetVertexInputDescription();
}

D3D12GpuPipelineParameterLayout* D3D12GpuGraphicsPipelineState::GetD3D12ParameterLayout() const
{
	return static_cast<D3D12GpuPipelineParameterLayout*>(mParameterLayout.get());
}

D3D12RootSignature* D3D12GpuGraphicsPipelineState::GetRootSignature() const
{
	D3D12GpuPipelineParameterLayout* parameterLayout = GetD3D12ParameterLayout();
	return parameterLayout != nullptr ? parameterLayout->GetRootSignature() : nullptr;
}

D3D12Pipeline* D3D12GpuGraphicsPipelineState::FindOrCreatePipeline(const D3D12PipelineVariantKey& key, const D3D12VertexInput& vertexInput)
{
	Lock lock(mMutex);

	auto found = mPipelines.find(key);
	if (found != mPipelines.end())
		return found->second;

	D3D12Pipeline* pipeline = CreatePipelineState(key, vertexInput);
	if (pipeline != nullptr)
		mPipelines[key] = pipeline;

	return pipeline;
}

D3D12Pipeline* D3D12GpuGraphicsPipelineState::CreatePipelineState(const D3D12PipelineVariantKey& key, const D3D12VertexInput& vertexInput)
{
	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	auto fnGetShaderBytecode = [](const TShared<GpuProgram>& program)
	{
		return program != nullptr ? static_cast<D3D12GpuProgram*>(program.get())->GetShaderBytecode() : D3D12_SHADER_BYTECODE{};
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	D3D12RootSignature* rootSignature = GetRootSignature();
	psoDesc.pRootSignature = rootSignature != nullptr ? rootSignature->Get() : nullptr;

	psoDesc.VS = fnGetShaderBytecode(mData.VertexProgram);
	psoDesc.PS = fnGetShaderBytecode(mData.FragmentProgram);
	psoDesc.GS = fnGetShaderBytecode(mData.GeometryProgram);
	psoDesc.HS = fnGetShaderBytecode(mData.HullProgram);
	psoDesc.DS = fnGetShaderBytecode(mData.DomainProgram);

	// Input layout, mapping the bound vertex buffer layout to the vertex shader's inputs
	psoDesc.InputLayout = vertexInput.GetLayoutDescription();

	// Rasterizer state
	const RasterizerStateInformation& rasterizerState = GetRasterizerState();
	psoDesc.RasterizerState.FillMode = D3D12Utility::GetFillMode(rasterizerState.PolygonMode);
	psoDesc.RasterizerState.CullMode = D3D12Utility::GetCullMode(rasterizerState.CullMode);
	psoDesc.RasterizerState.FrontCounterClockwise = FALSE; // D3D12 uses clockwise as front face
	psoDesc.RasterizerState.DepthBias = (INT)rasterizerState.DepthBias;
	psoDesc.RasterizerState.DepthBiasClamp = rasterizerState.DepthBiasClamp;
	psoDesc.RasterizerState.SlopeScaledDepthBias = rasterizerState.SlopeScaledDepthBias;
	psoDesc.RasterizerState.DepthClipEnable = rasterizerState.DepthClipEnable;
	psoDesc.RasterizerState.MultisampleEnable = FALSE; // TODO: Get from render target
	psoDesc.RasterizerState.AntialiasedLineEnable = rasterizerState.AntialiasedLineEnable;
	psoDesc.RasterizerState.ForcedSampleCount = 0;
	psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	// Blend state
	const BlendStateInformation& blendState = GetBlendState();
	psoDesc.BlendState.AlphaToCoverageEnable = blendState.EnableAlphaToCoverage;
	psoDesc.BlendState.IndependentBlendEnable = blendState.EnableIndependantBlend;

	for (u32 renderTargetIndex = 0; renderTargetIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; renderTargetIndex++)
	{
		// Without independent blending every render target uses the first target's blend state.
		const RenderTargetBlendStateInformation& sourceBlendState = blendState.RenderTargets[blendState.EnableIndependantBlend ? renderTargetIndex : 0];
		D3D12_RENDER_TARGET_BLEND_DESC& targetBlendDesc = psoDesc.BlendState.RenderTarget[renderTargetIndex];

		targetBlendDesc.BlendEnable = sourceBlendState.BlendEnable;
		targetBlendDesc.LogicOpEnable = FALSE;
		targetBlendDesc.SrcBlend = D3D12Utility::GetBlend(sourceBlendState.ColorSourceFactor);
		targetBlendDesc.DestBlend = D3D12Utility::GetBlend(sourceBlendState.ColorDestinationFactor);
		targetBlendDesc.BlendOp = D3D12Utility::GetBlendOp(sourceBlendState.ColorBlendOperation);
		targetBlendDesc.SrcBlendAlpha = D3D12Utility::GetBlend(sourceBlendState.AlphaSourceFactor);
		targetBlendDesc.DestBlendAlpha = D3D12Utility::GetBlend(sourceBlendState.AlphaDestinationFactor);
		targetBlendDesc.BlendOpAlpha = D3D12Utility::GetBlendOp(sourceBlendState.AlphaBlendOperation);
		targetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
		targetBlendDesc.RenderTargetWriteMask = sourceBlendState.RenderTargetWriteMask & 0xF;

		// MIN/MAX blend operations ignore the blend factors (treated as ONE); set them explicitly so the debug layer doesn't warn about the unused values
		if (targetBlendDesc.BlendOp == D3D12_BLEND_OP_MIN || targetBlendDesc.BlendOp == D3D12_BLEND_OP_MAX)
		{
			targetBlendDesc.SrcBlend = D3D12_BLEND_ONE;
			targetBlendDesc.DestBlend = D3D12_BLEND_ONE;
		}

		if (targetBlendDesc.BlendOpAlpha == D3D12_BLEND_OP_MIN || targetBlendDesc.BlendOpAlpha == D3D12_BLEND_OP_MAX)
		{
			targetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
			targetBlendDesc.DestBlendAlpha = D3D12_BLEND_ONE;
		}
	}

	// Depth-stencil state
	const DepthStencilStateInformation& depthStencilState = GetDepthStencilState();
	psoDesc.DepthStencilState.DepthEnable = depthStencilState.DepthReadEnable;
	psoDesc.DepthStencilState.DepthWriteMask = depthStencilState.DepthWriteEnable ?  D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = D3D12Utility::GetComparisonFunc(depthStencilState.DepthComparisonFunc);
	psoDesc.DepthStencilState.StencilEnable = depthStencilState.StencilEnable;
	psoDesc.DepthStencilState.StencilReadMask = (UINT8)depthStencilState.StencilReadMask;
	psoDesc.DepthStencilState.StencilWriteMask = (UINT8)depthStencilState.StencilWriteMask;

	// Front face stencil
	psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12Utility::GetStencilOp(depthStencilState.FrontStencilFailOp);
	psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12Utility::GetStencilOp(depthStencilState.FrontStencilZFailOp);
	psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12Utility::GetStencilOp(depthStencilState.FrontStencilPassOp);
	psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12Utility::GetComparisonFunc(depthStencilState.FrontStencilComparisonFunc);

	// Back face stencil
	psoDesc.DepthStencilState.BackFace.StencilFailOp = D3D12Utility::GetStencilOp(depthStencilState.BackStencilFailOp);
	psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12Utility::GetStencilOp(depthStencilState.BackStencilZFailOp);
	psoDesc.DepthStencilState.BackFace.StencilPassOp = D3D12Utility::GetStencilOp(depthStencilState.BackStencilPassOp);
	psoDesc.DepthStencilState.BackFace.StencilFunc = D3D12Utility::GetComparisonFunc(depthStencilState.BackStencilComparisonFunc);

	// Output configuration, all of it baked into the variant key
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.SampleDesc.Count = key.SampleCount;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.PrimitiveTopologyType = key.TopologyType;
	psoDesc.NumRenderTargets = key.RenderTargetCount;
	psoDesc.DSVFormat = key.DepthStencilFormat;

	for (u32 renderTargetIndex = 0; renderTargetIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; renderTargetIndex++)
		psoDesc.RTVFormats[renderTargetIndex] = renderTargetIndex < key.RenderTargetCount ? key.RenderTargetFormats[renderTargetIndex] : DXGI_FORMAT_UNKNOWN;

	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	ComPtr<ID3D12PipelineState> pipelineState;
	const HRESULT hr = d3d12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create graphics pipeline state (hr={0})", (u32)hr);
		device.LogDebugLayerMessages();
	}

	if (pipelineState == nullptr)
		return nullptr;

	return device.GetResourceManager().Create<D3D12Pipeline>(std::move(pipelineState), "D3D12 graphics pipeline");
}

D3D12GpuComputePipelineState::D3D12GpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuDevice& device)
	: GpuComputePipelineState(device, createInformation)
{
}

D3D12GpuComputePipelineState::~D3D12GpuComputePipelineState()
{
	if (mPipelineState != nullptr)
		mPipelineState->Destroy();
}

void D3D12GpuComputePipelineState::Initialize()
{
	GpuComputePipelineState::Initialize();

	CreatePipelineState();
}

void D3D12GpuComputePipelineState::CreatePipelineState()
{
	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	D3D12RootSignature* rootSignature = GetRootSignature();
	psoDesc.pRootSignature = rootSignature != nullptr ? rootSignature->Get() : nullptr;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	if (mData.Program != nullptr)
		psoDesc.CS = static_cast<D3D12GpuProgram*>(mData.Program.get())->GetShaderBytecode();

	ComPtr<ID3D12PipelineState> pipelineState;
#if B3D_BUILD_TYPE_DEVELOPMENT
	const Timer compileTimer;
#endif
	const HRESULT hr = d3d12Device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));

#if B3D_BUILD_TYPE_DEVELOPMENT
	if (gGpuDumpPipelineTimings.Get() && SUCCEEDED(hr))
		B3D_LOG(Info, LogRenderBackend, "[PIPELINE] compute {0} ms {1}", compileTimer.GetMicroseconds() / 1000.0, mData.Program != nullptr ? mData.Program->GetName() : String());
#endif

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create compute pipeline state (hr={0})", (u32)hr);
		device.LogDebugLayerMessages();
	}
	else
	{
		mPipelineState = device.GetResourceManager().Create<D3D12Pipeline>(std::move(pipelineState), "D3D12 compute pipeline");
		B3D_LOG(Verbose, LogRenderBackend, "Created compute pipeline state");
	}
}

D3D12RootSignature* D3D12GpuComputePipelineState::GetRootSignature() const
{
	D3D12GpuPipelineParameterLayout* parameterLayout = GetD3D12ParameterLayout();
	return parameterLayout != nullptr ? parameterLayout->GetRootSignature() : nullptr;
}

D3D12GpuPipelineParameterLayout* D3D12GpuComputePipelineState::GetD3D12ParameterLayout() const
{
	return static_cast<D3D12GpuPipelineParameterLayout*>(mParameterLayout.get());
}


D3D12GpuRayTracingPipelineState::D3D12GpuRayTracingPipelineState(const GpuRayTracingPipelineStateCreateInformation& createInformation, GpuDevice& device)
	: GpuRayTracingPipelineState(device, createInformation)
{ }

D3D12GpuRayTracingPipelineState::~D3D12GpuRayTracingPipelineState()
{
	if(mStateObject != nullptr)
		mStateObject->Destroy();
}

void D3D12GpuRayTracingPipelineState::Initialize()
{
	GpuRayTracingPipelineState::Initialize();

	u32 programCount = (u32)(mData.RayGenerationPrograms.size() + mData.MissPrograms.size() + mData.CallablePrograms.size());
	programCount += (u32)mData.HitGroups.size() * 3;

	// Every program is a library of its own with one export, renamed after its place in the pipeline: entry points of different programs
	// often share a name, and export names must be unique within a state object. Listing the export leaves out every other function of the
	// library. The arrays are reserved, so the descriptions can point into them.
	Vector<WString> exportNames;
	Vector<WString> entryPoints;
	Vector<D3D12_EXPORT_DESC> exports;
	Vector<D3D12_DXIL_LIBRARY_DESC> libraries;
	exportNames.reserve(programCount);
	entryPoints.reserve(programCount);
	exports.reserve(programCount);
	libraries.reserve(programCount);

	// Like compute pipelines, a pipeline with a program that didn't compile is never created, and dispatches with it fail
	bool areProgramsCompiled = true;
	const auto fnAddProgram = [&](const TShared<GpuProgram>& program) -> const wchar_t*
	{
		if(program == nullptr)
			return nullptr;

		const D3D12GpuProgram& d3d12Program = static_cast<const D3D12GpuProgram&>(*program);
		const D3D12_SHADER_BYTECODE& bytecode = d3d12Program.GetShaderBytecode();
		if(!d3d12Program.IsCompiled() || bytecode.BytecodeLength == 0)
		{
			areProgramsCompiled = false;
			return nullptr;
		}

		exportNames.push_back(ToWideString(StringUtility::Format("Program{0}", exportNames.size())));
		entryPoints.push_back(ToWideString(d3d12Program.GetEntryPoint()));

		D3D12_EXPORT_DESC& exportDescription = exports.emplace_back();
		exportDescription.Name = exportNames.back().c_str();
		exportDescription.ExportToRename = entryPoints.back().c_str();
		exportDescription.Flags = D3D12_EXPORT_FLAG_NONE;

		D3D12_DXIL_LIBRARY_DESC& library = libraries.emplace_back();
		library.DXILLibrary = bytecode;
		library.NumExports = 1;
		library.pExports = &exportDescription;

		return exportDescription.Name;
	};

	// Export of every shader group, in the order GetShaderGroupHandle() numbers them
	Vector<const wchar_t*> groupExports;
	for(const TShared<GpuProgram>& program : mData.RayGenerationPrograms)
		groupExports.push_back(fnAddProgram(program));

	for(const TShared<GpuProgram>& program : mData.MissPrograms)
		groupExports.push_back(fnAddProgram(program));

	Vector<WString> hitGroupNames;
	Vector<D3D12_HIT_GROUP_DESC> hitGroups;
	hitGroupNames.reserve(mData.HitGroups.size());
	hitGroups.reserve(mData.HitGroups.size());
	for(const GpuRayTracingHitGroup& hitGroup : mData.HitGroups)
	{
		hitGroupNames.push_back(ToWideString(StringUtility::Format("HitGroup{0}", hitGroupNames.size())));

		D3D12_HIT_GROUP_DESC& hitGroupDescription = hitGroups.emplace_back();
		hitGroupDescription.HitGroupExport = hitGroupNames.back().c_str();
		hitGroupDescription.Type = hitGroup.IntersectionProgram != nullptr ? D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE : D3D12_HIT_GROUP_TYPE_TRIANGLES;
		hitGroupDescription.ClosestHitShaderImport = fnAddProgram(hitGroup.ClosestHitProgram);
		hitGroupDescription.AnyHitShaderImport = fnAddProgram(hitGroup.AnyHitProgram);
		hitGroupDescription.IntersectionShaderImport = fnAddProgram(hitGroup.IntersectionProgram);

		groupExports.push_back(hitGroupDescription.HitGroupExport);
	}

	for(const TShared<GpuProgram>& program : mData.CallablePrograms)
		groupExports.push_back(fnAddProgram(program));

	D3D12RootSignature* const rootSignature = GetRootSignature();
	if(!areProgramsCompiled || rootSignature == nullptr)
		return;

	// Without association subobjects, the configurations and the global root signature apply to every export. Programs have no local
	// root signatures, since records hold only shader identifiers.
	D3D12_RAYTRACING_SHADER_CONFIG shaderConfiguration;
	shaderConfiguration.MaxPayloadSizeInBytes = mMaximumPayloadSize;
	shaderConfiguration.MaxAttributeSizeInBytes = mMaximumAttributeSize;

	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfiguration;
	pipelineConfiguration.MaxTraceRecursionDepth = mData.MaximumRecursionDepth;

	D3D12_GLOBAL_ROOT_SIGNATURE globalRootSignature;
	globalRootSignature.pGlobalRootSignature = rootSignature->Get();

	Vector<D3D12_STATE_SUBOBJECT> subobjects;
	subobjects.reserve(libraries.size() + hitGroups.size() + 3);
	for(const D3D12_DXIL_LIBRARY_DESC& library : libraries)
		subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &library });

	for(const D3D12_HIT_GROUP_DESC& hitGroupDescription : hitGroups)
		subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &hitGroupDescription });

	subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &shaderConfiguration });
	subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, &pipelineConfiguration });
	subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, &globalRootSignature });

	D3D12_STATE_OBJECT_DESC stateObjectDescription;
	stateObjectDescription.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
	stateObjectDescription.NumSubobjects = (u32)subobjects.size();
	stateObjectDescription.pSubobjects = subobjects.data();

	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
	const String& pipelineName = mData.RayGenerationPrograms[0]->GetName();

#if B3D_BUILD_TYPE_DEVELOPMENT
	const Timer compileTimer;
#endif

	// No collections or pipeline library: one state object with every program, created now
	ComPtr<ID3D12StateObject> stateObject;
	const HRESULT result = device.GetD3D12Device10()->CreateStateObject(&stateObjectDescription, IID_PPV_ARGS(&stateObject));
	if(FAILED(result))
	{
		// The debug layer names the subobject it rejected
		B3D_LOG(Error, LogRenderBackend, "Failed to create the ray tracing pipeline of '{0}' (hr={1}).", pipelineName, (u32)result);
		device.LogDebugLayerMessages();
		return;
	}

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(gGpuDumpPipelineTimings.Get())
		B3D_LOG(Info, LogRenderBackend, "[PIPELINE] ray tracing {0} ms {1}", compileTimer.GetMicroseconds() / 1000.0, pipelineName);
#endif

	// Read once, for every shader binding table created from the pipeline
	ComPtr<ID3D12StateObjectProperties> properties;
	const HRESULT propertiesResult = stateObject.As(&properties);
	B3D_ASSERT(SUCCEEDED(propertiesResult));
	(void)propertiesResult;

	mShaderIdentifiers.resize((size_t)D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES * groupExports.size());
	for(u32 groupIndex = 0; groupIndex < (u32)groupExports.size(); groupIndex++)
	{
		const void* const identifier = properties->GetShaderIdentifier(groupExports[groupIndex]);
		B3D_ASSERT(identifier != nullptr);

		std::memcpy(mShaderIdentifiers.data() + (size_t)groupIndex * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, identifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
	}

	mStateObject = device.GetResourceManager().Create<D3D12StateObject>(std::move(stateObject), "D3D12 ray tracing pipeline");
	B3D_LOG(Verbose, LogRenderBackend, "Created ray tracing pipeline state");
}

D3D12RootSignature* D3D12GpuRayTracingPipelineState::GetRootSignature() const
{
	D3D12GpuPipelineParameterLayout* parameterLayout = GetD3D12ParameterLayout();
	return parameterLayout != nullptr ? parameterLayout->GetRootSignature() : nullptr;
}

D3D12GpuPipelineParameterLayout* D3D12GpuRayTracingPipelineState::GetD3D12ParameterLayout() const
{
	return static_cast<D3D12GpuPipelineParameterLayout*>(mParameterLayout.get());
}
