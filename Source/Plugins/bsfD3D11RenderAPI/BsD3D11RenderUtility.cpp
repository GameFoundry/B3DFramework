//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11RenderUtility.h"
#include "BsD3D11Device.h"
#include "Math/BsVector3.h"
#include "Image/BsColor.h"
#include "Math/BsRect2I.h"
#include "BsD3D11BlendState.h"
#include "BsD3D11RasterizerState.h"
#include "BsD3D11DepthStencilState.h"
#include "Managers/BsRenderStateManager.h"

using namespace bs;
using namespace bs::ct;

struct ClearVertex
{
	Vector3 Pos;
	u32 Col;
};

D3D11RenderUtility::D3D11RenderUtility(D3D11Device* device)
	: mDevice(device), mClearQuadIB(nullptr), mClearQuadVB(nullptr), mClearQuadIL(nullptr), mClearQuadVS(nullptr), mClearQuadPS(nullptr)
{
	InitClearQuadResources();
}

D3D11RenderUtility::~D3D11RenderUtility()
{
	SAFE_RELEASE(mClearQuadPS);
	SAFE_RELEASE(mClearQuadVS);
	SAFE_RELEASE(mClearQuadIL);
	SAFE_RELEASE(mClearQuadIB);
	SAFE_RELEASE(mClearQuadVB);
}

void D3D11RenderUtility::DrawClearQuad(u32 clearBuffers, const Color& color, float depth, u16 stencil)
{
	// Set states
	if((clearBuffers & FBT_COLOR) != 0)
	{
		D3D11BlendState* d3d11BlendState = static_cast<D3D11BlendState*>(const_cast<BlendState*>(mClearQuadBlendStateYesC.get()));
		mDevice->GetImmediateContext()->OMSetBlendState(d3d11BlendState->GetInternal(), nullptr, 0xFFFFFFFF);
	}
	else
	{
		D3D11BlendState* d3d11BlendState = static_cast<D3D11BlendState*>(const_cast<BlendState*>(mClearQuadBlendStateNoC.get()));
		mDevice->GetImmediateContext()->OMSetBlendState(d3d11BlendState->GetInternal(), nullptr, 0xFFFFFFFF);
	}

	D3D11RasterizerState* d3d11RasterizerState = static_cast<D3D11RasterizerState*>(const_cast<RasterizerState*>(mClearQuadRasterizerState.get()));
	mDevice->GetImmediateContext()->RSSetState(d3d11RasterizerState->GetInternal());

	if((clearBuffers & FBT_DEPTH) != 0)
	{
		if((clearBuffers & FBT_STENCIL) != 0)
		{
			D3D11DepthStencilState* d3d11DepthStencilState = static_cast<D3D11DepthStencilState*>(const_cast<DepthStencilState*>(mClearQuadDSStateYesD_YesS.get()));
			mDevice->GetImmediateContext()->OMSetDepthStencilState(d3d11DepthStencilState->GetInternal(), stencil);
		}
		else
		{
			D3D11DepthStencilState* d3d11DepthStencilState = static_cast<D3D11DepthStencilState*>(const_cast<DepthStencilState*>(mClearQuadDSStateYesD_NoS.get()));
			mDevice->GetImmediateContext()->OMSetDepthStencilState(d3d11DepthStencilState->GetInternal(), stencil);
		}
	}
	else
	{
		if((clearBuffers & FBT_STENCIL) != 0)
		{
			D3D11DepthStencilState* d3d11DepthStencilState = static_cast<D3D11DepthStencilState*>(const_cast<DepthStencilState*>(mClearQuadDSStateNoD_YesS.get()));
			mDevice->GetImmediateContext()->OMSetDepthStencilState(d3d11DepthStencilState->GetInternal(), stencil);
		}
		else
		{
			D3D11DepthStencilState* d3d11DepthStencilState = static_cast<D3D11DepthStencilState*>(const_cast<DepthStencilState*>(mClearQuadDSStateNoD_NoS.get()));
			mDevice->GetImmediateContext()->OMSetDepthStencilState(d3d11DepthStencilState->GetInternal(), stencil);
		}
	}

	// TODO - How smart it is to update buffer right before drawing it!? (cache the clip area)
	ClearVertex vertexData[4];
	vertexData[0].Pos = Vector3(-1.0f, 1.0f, depth);
	vertexData[1].Pos = Vector3(1.0f, 1.0f, depth);
	vertexData[2].Pos = Vector3(-1.0f, -1.0f, depth);
	vertexData[3].Pos = Vector3(1.0f, -1.0f, depth);

	vertexData[0].Col = color.GetAsRgba();
	vertexData[1].Col = color.GetAsRgba();
	vertexData[2].Col = color.GetAsRgba();
	vertexData[3].Col = color.GetAsRgba();

	mDevice->GetImmediateContext()->UpdateSubresource(mClearQuadVB, 0, nullptr, vertexData, 0, sizeof(ClearVertex) * 4);

	mDevice->GetImmediateContext()->VSSetShader(mClearQuadVS, nullptr, 0);
	mDevice->GetImmediateContext()->PSSetShader(mClearQuadPS, nullptr, 0);

	ID3D11Buffer* buffers[1];
	buffers[0] = mClearQuadVB;

	u32 strides[1] = { sizeof(ClearVertex) };
	u32 offsets[1] = { 0 };

	mDevice->GetImmediateContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mDevice->GetImmediateContext()->IASetIndexBuffer(mClearQuadIB, DXGI_FORMAT_R16_UINT, 0);
	mDevice->GetImmediateContext()->IASetVertexBuffers(0, 1, buffers, strides, offsets);
	mDevice->GetImmediateContext()->IASetInputLayout(mClearQuadIL);

	mDevice->GetImmediateContext()->DrawIndexed(6, 0, 0);
}

void D3D11RenderUtility::InitClearQuadResources()
{
	BLEND_STATE_DESC blendStateDescYesC;
	mClearQuadBlendStateYesC = RenderStateManager::Instance().CreateBlendState(blendStateDescYesC);

	BLEND_STATE_DESC blendStateDescNoC;
	for(int i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
		blendStateDescNoC.RenderTargetDesc[i].RenderTargetWriteMask = 0;

	mClearQuadBlendStateNoC = RenderStateManager::Instance().CreateBlendState(blendStateDescNoC);

	DEPTH_STENCIL_STATE_DESC depthStateDescNoD_NoS;
	depthStateDescNoD_NoS.DepthReadEnable = false;
	depthStateDescNoD_NoS.DepthWriteEnable = false;
	depthStateDescNoD_NoS.DepthComparisonFunc = CMPF_ALWAYS_PASS;
	mClearQuadDSStateNoD_NoS = RenderStateManager::Instance().CreateDepthStencilState(depthStateDescNoD_NoS);

	DEPTH_STENCIL_STATE_DESC depthStateDescYesD_NoS;
	depthStateDescYesD_NoS.DepthReadEnable = false;
	depthStateDescYesD_NoS.DepthWriteEnable = true;
	depthStateDescYesD_NoS.DepthComparisonFunc = CMPF_ALWAYS_PASS;
	mClearQuadDSStateYesD_NoS = RenderStateManager::Instance().CreateDepthStencilState(depthStateDescYesD_NoS);

	DEPTH_STENCIL_STATE_DESC depthStateDescYesD_YesS;
	depthStateDescYesD_YesS.DepthReadEnable = false;
	depthStateDescYesD_YesS.DepthWriteEnable = true;
	depthStateDescYesD_YesS.DepthComparisonFunc = CMPF_ALWAYS_PASS;
	depthStateDescYesD_YesS.StencilEnable = true;
	depthStateDescYesD_YesS.FrontStencilComparisonFunc = CMPF_ALWAYS_PASS;
	depthStateDescYesD_YesS.FrontStencilPassOp = SOP_REPLACE;
	mClearQuadDSStateYesD_YesS = RenderStateManager::Instance().CreateDepthStencilState(depthStateDescYesD_YesS);

	DEPTH_STENCIL_STATE_DESC depthStateDescNoD_YesS;
	depthStateDescNoD_YesS.DepthReadEnable = false;
	depthStateDescNoD_YesS.DepthWriteEnable = false;
	depthStateDescNoD_YesS.DepthComparisonFunc = CMPF_ALWAYS_PASS;
	depthStateDescNoD_YesS.StencilEnable = true;
	depthStateDescNoD_YesS.FrontStencilComparisonFunc = CMPF_ALWAYS_PASS;
	mClearQuadDSStateNoD_YesS = RenderStateManager::Instance().CreateDepthStencilState(depthStateDescNoD_YesS);

	RASTERIZER_STATE_DESC rasterizerStateDesc;
	mClearQuadRasterizerState = RenderStateManager::Instance().CreateRasterizerState(rasterizerStateDesc);

	String vsShaderCode = "										\
						void main(									\
						in float3 inPos : POSITION,					\
						in float4 color : COLOR0,					\
						out float4 oPosition : SV_Position,			\
						out float4 oColor : COLOR0)					\
						{											\
							oPosition = float4(inPos.xyz, 1);		\
							oColor = color;							\
						}											\
						";

	String psShaderCode = "float4 main(in float4 inPos : SV_Position, float4 color : COLOR0) : SV_Target	\
								{ return color; }";

	HRESULT hr;
	ID3DBlob* microcode = nullptr;
	ID3DBlob* errors = nullptr;

	// Compile pixel shader
	hr = D3DCompile(psShaderCode.c_str(), psShaderCode.size(), nullptr, nullptr, nullptr, "main", "ps_4_0", D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR, 0, &microcode, &errors);

	if(FAILED(hr))
	{
		String message = "Cannot assemble D3D11 high-level shader. Errors:\n" +
			String(static_cast<const char*>(errors->GetBufferPointer()));

		SAFE_RELEASE(errors);
		B3D_EXCEPT(RenderingAPIException, message);
	}

	SAFE_RELEASE(errors);

	hr = mDevice->GetD3D11Device()->CreatePixelShader(static_cast<DWORD*>(microcode->GetBufferPointer()), microcode->GetBufferSize(), mDevice->GetClassLinkage(), &mClearQuadPS);

	if(FAILED(hr))
	{
		SAFE_RELEASE(microcode);
		B3D_EXCEPT(RenderingAPIException, "Cannot create D3D11 high-level shader.");
	}

	// Compile vertex shader
	hr = D3DCompile(vsShaderCode.c_str(), vsShaderCode.size(), nullptr, nullptr, nullptr, "main", "vs_4_0", D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR, 0, &microcode, &errors);

	if(FAILED(hr))
	{
		String message = "Cannot assemble D3D11 high-level shader. Errors:\n" +
			String(static_cast<const char*>(errors->GetBufferPointer()));

		SAFE_RELEASE(errors);
		B3D_EXCEPT(RenderingAPIException, message);
	}

	SAFE_RELEASE(errors);

	hr = mDevice->GetD3D11Device()->CreateVertexShader(static_cast<DWORD*>(microcode->GetBufferPointer()), microcode->GetBufferSize(), mDevice->GetClassLinkage(), &mClearQuadVS);

	if(FAILED(hr))
	{
		SAFE_RELEASE(microcode);
		B3D_EXCEPT(RenderingAPIException, "Cannot create D3D11 high-level shader.");
	}

	// Create input layout
	D3D11_INPUT_ELEMENT_DESC* declElements = B3DNewMultiple<D3D11_INPUT_ELEMENT_DESC>(2);
	ZeroMemory(declElements, sizeof(D3D11_INPUT_ELEMENT_DESC) * 2);

	declElements[0].SemanticName = "POSITION";
	declElements[0].SemanticIndex = 0;
	declElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	declElements[0].InputSlot = 0;
	declElements[0].AlignedByteOffset = 0;
	declElements[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	declElements[0].InstanceDataStepRate = 0;

	declElements[1].SemanticName = "COLOR";
	declElements[1].SemanticIndex = 0;
	declElements[1].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	declElements[1].InputSlot = 0;
	declElements[1].AlignedByteOffset = sizeof(float) * 3;
	declElements[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	declElements[1].InstanceDataStepRate = 0;

	hr = mDevice->GetD3D11Device()->CreateInputLayout(declElements, 2, microcode->GetBufferPointer(), microcode->GetBufferSize(), &mClearQuadIL);

	B3DDeleteMultiple(declElements, 2);

	if(FAILED(hr))
	{
		B3D_EXCEPT(RenderingAPIException, "Cannot create D3D11 input layout.");
	}

	// Create vertex buffer
	D3D11_BUFFER_DESC mVBDesc;

	mVBDesc.ByteWidth = sizeof(ClearVertex) * 4;
	mVBDesc.MiscFlags = 0;
	mVBDesc.StructureByteStride = 0;

	mVBDesc.Usage = D3D11_USAGE_DEFAULT;
	mVBDesc.CPUAccessFlags = 0;
	mVBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	ClearVertex vertexData[4];
	vertexData[0].Pos = Vector3(0, 0, 0);
	vertexData[1].Pos = Vector3(0, 0, 0);
	vertexData[2].Pos = Vector3(0, 0, 0);
	vertexData[3].Pos = Vector3(0, 0, 0);

	vertexData[0].Col = Color::kWhite.GetAsBgra();
	vertexData[1].Col = Color::kWhite.GetAsBgra();
	vertexData[2].Col = Color::kWhite.GetAsBgra();
	vertexData[3].Col = Color::kWhite.GetAsBgra();

	D3D11_SUBRESOURCE_DATA vertexSubresourceData;
	vertexSubresourceData.pSysMem = vertexData;

	hr = mDevice->GetD3D11Device()->CreateBuffer(&mVBDesc, &vertexSubresourceData, &mClearQuadVB);
	if(FAILED(hr) || mDevice->HasError())
	{
		String msg = mDevice->GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "Cannot create D3D11 buffer: " + msg);
	}

	// Create index buffer
	D3D11_BUFFER_DESC mIBDesc;

	mIBDesc.ByteWidth = sizeof(u16) * 6;
	mIBDesc.MiscFlags = 0;
	mIBDesc.StructureByteStride = 0;

	mIBDesc.Usage = D3D11_USAGE_DEFAULT;
	mIBDesc.CPUAccessFlags = 0;
	mIBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	u16 indexData[6];
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;

	D3D11_SUBRESOURCE_DATA indexSubresourceData;
	indexSubresourceData.pSysMem = indexData;

	hr = mDevice->GetD3D11Device()->CreateBuffer(&mIBDesc, &indexSubresourceData, &mClearQuadIB);
	if(FAILED(hr) || mDevice->HasError())
	{
		String msg = mDevice->GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "Cannot create D3D11 buffer: " + msg);
	}
}
