//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuPipelineState.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuProgram.h"
#include "B3DD3D12Utility.h"
#include "B3DD3D12GpuPipelineParameterLayout.h"
#include "Profiling/B3DRenderStats.h"
#include "RenderAPI/B3DVertexDescription.h"

using namespace b3d;
using namespace b3d::render;

D3D12GpuGraphicsPipelineState::D3D12GpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuDevice& device)
	: GpuGraphicsPipelineState(device, createInformation)
{
}

D3D12GpuGraphicsPipelineState::~D3D12GpuGraphicsPipelineState()
{
	mPipelineState.Reset();
	mRootSignature.Reset();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_PipelineState);
}

void D3D12GpuGraphicsPipelineState::Initialize()
{
	GpuGraphicsPipelineState::Initialize();

	if (mData.VertexProgram != nullptr)
		mVertexDescription = mData.VertexProgram->GetVertexInputDescription();

	CreatePipelineState();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_PipelineState);
}

void D3D12GpuGraphicsPipelineState::CreatePipelineState()
{
	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(mGpuDevice);
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	// Get root signature from parameter layout
	D3D12GpuPipelineParameterLayout* d3d12ParamLayout = static_cast<D3D12GpuPipelineParameterLayout*>(mParameterLayout.get());
	if (d3d12ParamLayout)
		mRootSignature = d3d12ParamLayout->GetRootSignature();

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

	// Input layout from vertex description
	Vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
	if (mVertexDescription)
	{
		const auto& vertexElements = mVertexDescription->GetElements();
		inputElements.reserve(vertexElements.size());

		for (const auto& element : vertexElements)
		{
			D3D12_INPUT_ELEMENT_DESC d3d12Element = {};
			d3d12Element.SemanticName = GetSemanticName(element.Semantic);
			d3d12Element.SemanticIndex = element.SemanticIdx;
			d3d12Element.Format = D3D12Utility::GetDXGIFormat(element.Type);
			d3d12Element.InputSlot = element.StreamIdx;
			d3d12Element.AlignedByteOffset = element.Offset;
			d3d12Element.InputSlotClass = element.InstanceStepRate > 0 ?
				D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA :
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			d3d12Element.InstanceDataStepRate = element.InstanceStepRate;

			inputElements.push_back(d3d12Element);
		}
	}

	psoDesc.InputLayout.pInputElementDescs = inputElements.data();
	psoDesc.InputLayout.NumElements = (UINT)inputElements.size();

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
		const RenderTargetBlendState& rtBlendState = blendState.RenderTargets[rtIdx];

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
	psoDesc.SampleDesc.Count = 1; // TODO: Get from render target
	psoDesc.SampleDesc.Quality = 0;

	// Primitive topology type
	psoDesc.PrimitiveTopologyType = D3D12Utility::GetPrimitiveTopologyType(DOT_TRIANGLE_LIST);
	mPrimitiveTopology = D3D12Utility::GetPrimitiveTopology(DOT_TRIANGLE_LIST);

	// Render target formats (will be set at runtime based on bound render targets)
	// For now, use common defaults
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	for (u32 i = 1; i < 8; i++)
		psoDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;

	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

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
	HRESULT hr = d3d12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState));

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create graphics pipeline state");
	}
	else
	{
		B3D_LOG(Info, LogRenderBackend, "Created graphics pipeline state");
	}
}

const char* D3D12GpuGraphicsPipelineState::GetSemanticName(VertexElementSemantic semantic)
{
	switch (semantic)
	{
	case VES_POSITION:
		return "POSITION";
	case VES_BLEND_WEIGHTS:
		return "BLENDWEIGHT";
	case VES_BLEND_INDICES:
		return "BLENDINDICES";
	case VES_NORMAL:
		return "NORMAL";
	case VES_COLOR:
		return "COLOR";
	case VES_TEXCOORD:
		return "TEXCOORD";
	case VES_BINORMAL:
		return "BINORMAL";
	case VES_TANGENT:
		return "TANGENT";
	default:
		return "TEXCOORD";
	}
}

D3D12GpuComputePipelineState::D3D12GpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuDevice& device)
	: GpuComputePipelineState(device, createInformation)
{
}

D3D12GpuComputePipelineState::~D3D12GpuComputePipelineState()
{
	mPipelineState.Reset();
	mRootSignature.Reset();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_PipelineState);
}

void D3D12GpuComputePipelineState::Initialize()
{
	GpuComputePipelineState::Initialize();

	CreatePipelineState();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_PipelineState);
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
	if (mData.ComputeProgram)
	{
		D3D12GpuProgram* csProgram = static_cast<D3D12GpuProgram*>(mData.ComputeProgram.get());
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
		B3D_LOG(Error, LogRenderBackend, "Failed to create compute pipeline state");
	}
	else
	{
		B3D_LOG(Info, LogRenderBackend, "Created compute pipeline state");
	}
}
