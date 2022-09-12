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

namespace bs { namespace ct
{
	struct ClearVertex
	{
		Vector3 pos;
		UINT32 col;
	};

	D3D11RenderUtility::D3D11RenderUtility(D3D11Device* device)
		:mDevice(device), mClearQuadIB(nullptr), mClearQuadVB(nullptr),
		mClearQuadIL(nullptr), mClearQuadVS(nullptr), mClearQuadPS(nullptr)
	{
		initClearQuadResources();
	}

	D3D11RenderUtility::~D3D11RenderUtility()
	{
		SAFE_RELEASE(mClearQuadPS);
		SAFE_RELEASE(mClearQuadVS);
		SAFE_RELEASE(mClearQuadIL);
		SAFE_RELEASE(mClearQuadIB);
		SAFE_RELEASE(mClearQuadVB);
	}

	void D3D11RenderUtility::DrawClearQuad(UINT32 clearBuffers, const Color& color, float depth, UINT16 stencil)
	{
		// Set states
		if((clearBuffers & FBT_COLOR) != 0)
		{
			D3D11BlendState* d3d11BlendState = static_cast<D3D11BlendState*>(const_cast<BlendState*>(mClearQuadBlendStateYesC.Get()));
			mDevice->GetImmediateContext()->OMSetBlendState(d3d11BlendState->getInternal(), nullptr, 0xFFFFFFFF);
		}
		else
		{
			D3D11BlendState* d3d11BlendState = static_cast<D3D11BlendState*>(const_cast<BlendState*>(mClearQuadBlendStateNoC.Get()));
			mDevice->GetImmediateContext()->OMSetBlendState(d3d11BlendState->getInternal(), nullptr, 0xFFFFFFFF);
		}

		D3D11RasterizerState* d3d11RasterizerState = static_cast<D3D11RasterizerState*>(const_cast<RasterizerState*>(mClearQuadRasterizerState.Get()));
		mDevice->GetImmediateContext()->RSSetState(d3d11RasterizerState->getInternal());

		if((clearBuffers & FBT_DEPTH) != 0)
		{
			if((clearBuffers & FBT_STENCIL) != 0)
			{
				D3D11DepthStencilState* d3d11DepthStencilState = static_cast<D3D11DepthStencilState*>(const_cast<DepthStencilState*>(mClearQuadDSStateYesD_YesS.Get()));
				mDevice->GetImmediateContext()->OMSetDepthStencilState(d3d11DepthStencilState->getInternal(), stencil);
			}
			else
			{
				D3D11DepthStencilState* d3d11DepthStencilState = static_cast<D3D11DepthStencilState*>(const_cast<DepthStencilState*>(mClearQuadDSStateYesD_NoS.Get()));
				mDevice->GetImmediateContext()->OMSetDepthStencilState(d3d11DepthStencilState->getInternal(), stencil);
			}
		}
		else
		{
			if((clearBuffers & FBT_STENCIL) != 0)
			{
				D3D11DepthStencilState* d3d11DepthStencilState = static_cast<D3D11DepthStencilState*>(const_cast<DepthStencilState*>(mClearQuadDSStateNoD_YesS.Get()));
				mDevice->GetImmediateContext()->OMSetDepthStencilState(d3d11DepthStencilState->getInternal(), stencil);
			}
			else
			{
				D3D11DepthStencilState* d3d11DepthStencilState = static_cast<D3D11DepthStencilState*>(const_cast<DepthStencilState*>(mClearQuadDSStateNoD_NoS.Get()));
				mDevice->GetImmediateContext()->OMSetDepthStencilState(d3d11DepthStencilState->getInternal(), stencil);
			}
		}

		// TODO - How smart it is to update buffer right before drawing it!? (cache the clip area)
		ClearVertex vertexData[4];
		vertexData[0].pos = Vector3(-1.0f, 1.0f, depth);
		vertexData[1].pos = Vector3(1.0f, 1.0f, depth);
		vertexData[2].pos = Vector3(-1.0f, -1.0f, depth);
		vertexData[3].pos = Vector3(1.0f, -1.0f, depth);

		vertexData[0].col = color.GetAsRGBA();
		vertexData[1].col = color.GetAsRGBA();
		vertexData[2].col = color.GetAsRGBA();
		vertexData[3].col = color.GetAsRGBA();

		mDevice->GetImmediateContext()->UpdateSubresource(mClearQuadVB, 0, nullptr, vertexData, 0, sizeof(ClearVertex) * 4);

		mDevice->GetImmediateContext()->VSSetShader(mClearQuadVS, nullptr, 0);
		mDevice->GetImmediateContext()->PSSetShader(mClearQuadPS, nullptr, 0);

		ID3D11Buffer* buffers[1];
		buffers[0] = mClearQuadVB;

		UINT32 strides[1] = { sizeof(ClearVertex) };
		UINT32 offsets[1] = { 0 };

		mDevice->GetImmediateContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mDevice->GetImmediateContext()->IASetIndexBuffer(mClearQuadIB, DXGI_FORMAT_R16_UINT, 0);
		mDevice->GetImmediateContext()->IASetVertexBuffers(0, 1, buffers, strides, offsets);
		mDevice->GetImmediateContext()->IASetInputLayout(mClearQuadIL);

		mDevice->GetImmediateContext()->DrawIndexed(6, 0, 0);
	}

	void D3D11RenderUtility::InitClearQuadResources()
	{
		BLEND_STATE_DESC blendStateDescYesC;
		mClearQuadBlendStateYesC = RenderStateManager::instance().CreateBlendState(blendStateDescYesC);

		BLEND_STATE_DESC blendStateDescNoC;
		for(int i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
			blendStateDescNoC.renderTargetDesc[i].renderTargetWriteMask = 0;

		mClearQuadBlendStateNoC = RenderStateManager::instance().CreateBlendState(blendStateDescNoC);

		DEPTH_STENCIL_STATE_DESC depthStateDescNoD_NoS;
		depthStateDescNoD_NoS.depthReadEnable = false;
		depthStateDescNoD_NoS.depthWriteEnable = false;
		depthStateDescNoD_NoS.depthComparisonFunc = CMPF_ALWAYS_PASS;
		mClearQuadDSStateNoD_NoS = RenderStateManager::instance().CreateDepthStencilState(depthStateDescNoD_NoS);

		DEPTH_STENCIL_STATE_DESC depthStateDescYesD_NoS;
		depthStateDescYesD_NoS.depthReadEnable = false;
		depthStateDescYesD_NoS.depthWriteEnable = true;
		depthStateDescYesD_NoS.depthComparisonFunc = CMPF_ALWAYS_PASS;
		mClearQuadDSStateYesD_NoS = RenderStateManager::instance().CreateDepthStencilState(depthStateDescYesD_NoS);

		DEPTH_STENCIL_STATE_DESC depthStateDescYesD_YesS;
		depthStateDescYesD_YesS.depthReadEnable = false;
		depthStateDescYesD_YesS.depthWriteEnable = true;
		depthStateDescYesD_YesS.depthComparisonFunc = CMPF_ALWAYS_PASS;
		depthStateDescYesD_YesS.stencilEnable = true;
		depthStateDescYesD_YesS.frontStencilComparisonFunc = CMPF_ALWAYS_PASS;
		depthStateDescYesD_YesS.frontStencilPassOp = SOP_REPLACE;
		mClearQuadDSStateYesD_YesS = RenderStateManager::instance().CreateDepthStencilState(depthStateDescYesD_YesS);

		DEPTH_STENCIL_STATE_DESC depthStateDescNoD_YesS;
		depthStateDescNoD_YesS.depthReadEnable = false;
		depthStateDescNoD_YesS.depthWriteEnable = false;
		depthStateDescNoD_YesS.depthComparisonFunc = CMPF_ALWAYS_PASS;
		depthStateDescNoD_YesS.stencilEnable = true;
		depthStateDescNoD_YesS.frontStencilComparisonFunc = CMPF_ALWAYS_PASS;
		mClearQuadDSStateNoD_YesS = RenderStateManager::instance().CreateDepthStencilState(depthStateDescNoD_YesS);

		RASTERIZER_STATE_DESC rasterizerStateDesc;
		mClearQuadRasterizerState = RenderStateManager::instance().CreateRasterizerState(rasterizerStateDesc);

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
		hr = D3DCompile(psShaderCode.c_str(), psShaderCode.size(), nullptr, nullptr, nullptr, "main", "ps_4_0",
			D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR, 0, &microcode, &errors);

		if (FAILED(hr))
		{
			String message = "Cannot assemble D3D11 high-level shader. Errors:\n" +
				String(static_cast<const char*>(errors->GetBufferPointer()));

			SAFE_RELEASE(errors);
			BS_EXCEPT(RenderingAPIException, message);
		}

		SAFE_RELEASE(errors);

		hr = mDevice->GetD3D11Device()->CreatePixelShader(static_cast<DWORD*>(microcode->GetBufferPointer()),
			microcode->GetBufferSize(), mDevice->GetClassLinkage(), &mClearQuadPS);

		if (FAILED(hr))
		{
			SAFE_RELEASE(microcode);
			BS_EXCEPT(RenderingAPIException, "Cannot create D3D11 high-level shader.");
		}

		// Compile vertex shader
		hr = D3DCompile(vsShaderCode.c_str(), vsShaderCode.size(), nullptr, nullptr, nullptr, "main", "vs_4_0",
			D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR, 0, &microcode, &errors);

		if (FAILED(hr))
		{
			String message = "Cannot assemble D3D11 high-level shader. Errors:\n" +
				String(static_cast<const char*>(errors->GetBufferPointer()));

			SAFE_RELEASE(errors);
			BS_EXCEPT(RenderingAPIException, message);
		}

		SAFE_RELEASE(errors);

		hr = mDevice->GetD3D11Device()->CreateVertexShader(static_cast<DWORD*>(microcode->GetBufferPointer()),
			microcode->GetBufferSize(), mDevice->GetClassLinkage(), &mClearQuadVS);

		if (FAILED(hr))
		{
			SAFE_RELEASE(microcode);
			BS_EXCEPT(RenderingAPIException, "Cannot create D3D11 high-level shader.");
		}

		// Create input layout
		D3D11_INPUT_ELEMENT_DESC* declElements = bs_newN<D3D11_INPUT_ELEMENT_DESC>(2);
		ZeroMemory(declElements, sizeof(D3D11_INPUT_ELEMENT_DESC) * 2);

		declElements[0].SemanticName			= "POSITION";
		declElements[0].SemanticIndex			= 0;
		declElements[0].Format					= DXGI_FORMAT_R32G32B32_FLOAT;
		declElements[0].InputSlot				= 0;
		declElements[0].AlignedByteOffset		= 0;
		declElements[0].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
		declElements[0].InstanceDataStepRate	= 0;

		declElements[1].SemanticName			= "COLOR";
		declElements[1].SemanticIndex			= 0;
		declElements[1].Format					= DXGI_FORMAT_R8G8B8A8_UNORM;
		declElements[1].InputSlot				= 0;
		declElements[1].AlignedByteOffset		= sizeof(float) * 3;
		declElements[1].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
		declElements[1].InstanceDataStepRate	= 0;

		hr = mDevice->GetD3D11Device()->CreateInputLayout(declElements, 2, microcode->GetBufferPointer(),
			microcode->GetBufferSize(), &mClearQuadIL);

		bs_deleteN(declElements, 2);

		if (FAILED(hr))
		{
			BS_EXCEPT(RenderingAPIException, "Cannot create D3D11 input layout.");
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
		vertexData[0].pos = Vector3(0, 0, 0);
		vertexData[1].pos = Vector3(0, 0, 0);
		vertexData[2].pos = Vector3(0, 0, 0);
		vertexData[3].pos = Vector3(0, 0, 0);

		vertexData[0].col = Color::White.GetAsBGRA();
		vertexData[1].col = Color::White.GetAsBGRA();
		vertexData[2].col = Color::White.GetAsBGRA();
		vertexData[3].col = Color::White.GetAsBGRA();

		D3D11_SUBRESOURCE_DATA vertexSubresourceData;
		vertexSubresourceData.pSysMem = vertexData;

		hr = mDevice->GetD3D11Device()->CreateBuffer(&mVBDesc, &vertexSubresourceData, &mClearQuadVB);
		if (FAILED(hr) || mDevice->HasError())
		{
			String msg = mDevice->GetErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create D3D11 buffer: " + msg);
		}

		// Create index buffer
		D3D11_BUFFER_DESC mIBDesc;

		mIBDesc.ByteWidth = sizeof(UINT16) * 6;
		mIBDesc.MiscFlags = 0;
		mIBDesc.StructureByteStride = 0;

		mIBDesc.Usage = D3D11_USAGE_DEFAULT;
		mIBDesc.CPUAccessFlags = 0;
		mIBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		UINT16 indexData[6];
		indexData[0] = 0;
		indexData[1] = 1;
		indexData[2] = 2;
		indexData[3] = 1;
		indexData[4] = 3;
		indexData[5] = 2;

		D3D11_SUBRESOURCE_DATA indexSubresourceData;
		indexSubresourceData.pSysMem = indexData;

		hr = mDevice->GetD3D11Device()->CreateBuffer(&mIBDesc, &indexSubresourceData, &mClearQuadIB);
		if (FAILED(hr) || mDevice->HasError())
		{
			String msg = mDevice->GetErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create D3D11 buffer: " + msg);
		}
	}
}}
