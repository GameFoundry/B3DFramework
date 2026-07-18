//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuPipelineState.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuProgram.h"
#include "B3DD3D12Utility.h"
#include "B3DD3D12GpuPipelineParameterLayout.h"
#include "Managers/B3DD3D12VertexInputManager.h"
#include "GpuBackend/B3DVertexDescription.h"

using namespace b3d;
using namespace b3d::render;

D3D12GpuGraphicsPipelineState::D3D12GpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuDevice& device)
	: GpuGraphicsPipelineState(device, createInformation)
{
}

D3D12GpuGraphicsPipelineState::~D3D12GpuGraphicsPipelineState()
{
	mPipelines.clear();
	mRootSignature.Reset();
}

void D3D12GpuGraphicsPipelineState::Initialize()
{
	GpuGraphicsPipelineState::Initialize();

	// The vertex shader's input declaration; paired with the vertex buffer description bound on the command
	// buffer to build the input layout of each pipeline variant
	if (mData.VertexProgram != nullptr)
		mVertexDescription = mData.VertexProgram->GetVertexInputDescription();

	// Get root signature from parameter layout. Actual pipeline objects are created lazily per render target
	// format / topology / vertex input combination, see FindOrCreatePipeline().
	D3D12GpuPipelineParameterLayout* d3d12ParamLayout = static_cast<D3D12GpuPipelineParameterLayout*>(mParameterLayout.get());
	if (d3d12ParamLayout)
		mRootSignature = d3d12ParamLayout->GetRootSignature();
}

D3D12GpuPipelineParameterLayout* D3D12GpuGraphicsPipelineState::GetD3D12ParameterLayout() const
{
	return static_cast<D3D12GpuPipelineParameterLayout*>(mParameterLayout.get());
}

D3D12GpuPipelineParameterLayout* D3D12GpuComputePipelineState::GetD3D12ParameterLayout() const
{
	return static_cast<D3D12GpuPipelineParameterLayout*>(mParameterLayout.get());
}

bool D3D12PipelineVariantKey::operator==(const D3D12PipelineVariantKey& other) const
{
	if (RenderTargetCount != other.RenderTargetCount || DepthStencilFormat != other.DepthStencilFormat ||
		SampleCount != other.SampleCount || TopologyType != other.TopologyType ||
		VertexInputId != other.VertexInputId)
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

ID3D12PipelineState* D3D12GpuGraphicsPipelineState::FindOrCreatePipeline(const D3D12PipelineVariantKey& key, const D3D12VertexInput& vertexInput)
{
	auto found = mPipelines.find(key);
	if (found != mPipelines.end())
		return found->second.Get();

	ComPtr<ID3D12PipelineState> pipeline = CreatePipelineState(key, vertexInput);
	mPipelines[key] = pipeline;

	return pipeline.Get();
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> D3D12GpuGraphicsPipelineState::CreatePipelineState(const D3D12PipelineVariantKey& key, const D3D12VertexInput& vertexInput)
{
	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	// Graphics pipeline state descriptor
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	// Root signature
	psoDesc.pRootSignature = mRootSignature.Get();

	// Shader stages
	if (mData.VertexProgram)
	{
		D3D12GpuProgram* vsProgram = static_cast<D3D12GpuProgram*>(mData.VertexProgram.get());
		psoDesc.VS = vsProgram->GetShaderBytecode();
	}

	if (mData.FragmentProgram)
	{
		D3D12GpuProgram* psProgram = static_cast<D3D12GpuProgram*>(mData.FragmentProgram.get());
		psoDesc.PS = psProgram->GetShaderBytecode();
	}

	if (mData.GeometryProgram)
	{
		D3D12GpuProgram* gsProgram = static_cast<D3D12GpuProgram*>(mData.GeometryProgram.get());
		psoDesc.GS = gsProgram->GetShaderBytecode();
	}

	if (mData.HullProgram)
	{
		D3D12GpuProgram* hsProgram = static_cast<D3D12GpuProgram*>(mData.HullProgram.get());
		psoDesc.HS = hsProgram->GetShaderBytecode();
	}

	if (mData.DomainProgram)
	{
		D3D12GpuProgram* dsProgram = static_cast<D3D12GpuProgram*>(mData.DomainProgram.get());
		psoDesc.DS = dsProgram->GetShaderBytecode();
	}

	// Input layout, mapping the bound vertex buffer layout to the vertex shader's inputs
	psoDesc.InputLayout = vertexInput.GetLayoutDesc();

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

	for (u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		u32 rtIdx = blendState.EnableIndependantBlend ? i : 0;
		const RenderTargetBlendStateInformation& rtBlendState = blendState.RenderTargets[rtIdx];

		psoDesc.BlendState.RenderTarget[i].BlendEnable = rtBlendState.BlendEnable;
		psoDesc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
		psoDesc.BlendState.RenderTarget[i].SrcBlend = D3D12Utility::GetBlend(rtBlendState.ColorSourceFactor);
		psoDesc.BlendState.RenderTarget[i].DestBlend = D3D12Utility::GetBlend(rtBlendState.ColorDestinationFactor);
		psoDesc.BlendState.RenderTarget[i].BlendOp = D3D12Utility::GetBlendOp(rtBlendState.ColorBlendOperation);
		psoDesc.BlendState.RenderTarget[i].SrcBlendAlpha = D3D12Utility::GetBlend(rtBlendState.AlphaSourceFactor);
		psoDesc.BlendState.RenderTarget[i].DestBlendAlpha = D3D12Utility::GetBlend(rtBlendState.AlphaDestinationFactor);
		psoDesc.BlendState.RenderTarget[i].BlendOpAlpha = D3D12Utility::GetBlendOp(rtBlendState.AlphaBlendOperation);
		psoDesc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
		psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = rtBlendState.RenderTargetWriteMask & 0xF;
	}

	// Depth-stencil state
	const DepthStencilStateInformation& depthStencilState = GetDepthStencilState();
	psoDesc.DepthStencilState.DepthEnable = depthStencilState.DepthReadEnable;
	psoDesc.DepthStencilState.DepthWriteMask = depthStencilState.DepthWriteEnable ?
		D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
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

	// Sample description
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.SampleDesc.Count = key.SampleCount;
	psoDesc.SampleDesc.Quality = 0;

	// Primitive topology type
	psoDesc.PrimitiveTopologyType = key.TopologyType;

	// Render target formats
	psoDesc.NumRenderTargets = key.RenderTargetCount;
	for (u32 i = 0; i < 8; i++)
		psoDesc.RTVFormats[i] = i < key.RenderTargetCount ? key.RenderTargetFormats[i] : DXGI_FORMAT_UNKNOWN;

	psoDesc.DSVFormat = key.DepthStencilFormat;

	// Stream output (not used)
	psoDesc.StreamOutput.pSODeclaration = nullptr;
	psoDesc.StreamOutput.NumEntries = 0;
	psoDesc.StreamOutput.pBufferStrides = nullptr;
	psoDesc.StreamOutput.NumStrides = 0;
	psoDesc.StreamOutput.RasterizedStream = 0;

	// Cache and node mask
	psoDesc.CachedPSO.pCachedBlob = nullptr;
	psoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	psoDesc.NodeMask = 0;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	// Create pipeline state
	ComPtr<ID3D12PipelineState> pipelineState;
	HRESULT hr = d3d12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create graphics pipeline state (hr={0})", (u32)hr);
		device.LogDebugLayerMessages();
	}

	return pipelineState;
}

D3D12GpuComputePipelineState::D3D12GpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuDevice& device)
	: GpuComputePipelineState(device, createInformation)
{
}

D3D12GpuComputePipelineState::~D3D12GpuComputePipelineState()
{
	mPipelineState.Reset();
	mRootSignature.Reset();
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

	// Get root signature from parameter layout
	D3D12GpuPipelineParameterLayout* d3d12ParamLayout = static_cast<D3D12GpuPipelineParameterLayout*>(mParameterLayout.get());
	if (d3d12ParamLayout)
		mRootSignature = d3d12ParamLayout->GetRootSignature();

	// Compute pipeline state descriptor
	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = mRootSignature.Get();

	// Compute shader
	if (mData.Program)
	{
		D3D12GpuProgram* csProgram = static_cast<D3D12GpuProgram*>(mData.Program.get());
		psoDesc.CS = csProgram->GetShaderBytecode();
	}

	// Cache and node mask
	psoDesc.CachedPSO.pCachedBlob = nullptr;
	psoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
	psoDesc.NodeMask = 0;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	// Create pipeline state
	HRESULT hr = d3d12Device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState));

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create compute pipeline state (hr={0})", (u32)hr);
		device.LogDebugLayerMessages();
	}
	else
	{
		B3D_LOG(Info, LogRenderBackend, "Created compute pipeline state");
	}
}
