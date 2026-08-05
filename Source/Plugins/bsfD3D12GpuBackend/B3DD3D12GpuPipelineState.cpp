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

	// Only the root signature is available upfront. Actual pipeline objects are created lazily per render target
	// format / topology / vertex input combination, see FindOrCreatePipeline().
	if (D3D12GpuPipelineParameterLayout* d3d12ParameterLayout = GetD3D12ParameterLayout())
		mRootSignature = d3d12ParameterLayout->GetRootSignature();
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

ComPtr<ID3D12PipelineState> D3D12GpuGraphicsPipelineState::CreatePipelineState(const D3D12PipelineVariantKey& key, const D3D12VertexInput& vertexInput)
{
	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	auto fnGetShaderBytecode = [](const TShared<GpuProgram>& program)
	{
		return program != nullptr ? static_cast<D3D12GpuProgram*>(program.get())->GetShaderBytecode() : D3D12_SHADER_BYTECODE{};
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = mRootSignature.Get();

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

		// MIN/MAX blend operations ignore the blend factors (treated as ONE); set them explicitly so the debug
		// layer doesn't warn about the unused values
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

	if (D3D12GpuPipelineParameterLayout* d3d12ParameterLayout = GetD3D12ParameterLayout())
		mRootSignature = d3d12ParameterLayout->GetRootSignature();

	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	if (mData.Program != nullptr)
		psoDesc.CS = static_cast<D3D12GpuProgram*>(mData.Program.get())->GetShaderBytecode();

	const HRESULT hr = d3d12Device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState));

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create compute pipeline state (hr={0})", (u32)hr);
		device.LogDebugLayerMessages();
	}
	else
	{
		B3D_LOG(Verbose, LogRenderBackend, "Created compute pipeline state");
	}
}
