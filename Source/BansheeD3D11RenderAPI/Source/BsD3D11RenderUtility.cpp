//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11RenderUtility.h"
#include "BsD3D11Device.h"
#include "BsVector3.h"
#include "BsColor.h"
#include "BsRect2I.h"
#include "BsD3D11BlendState.h"
#include "BsD3D11RasterizerState.h"
#include "BsD3D11DepthStencilState.h"
#include "BsRenderStateManager.h"

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

	void D3D11RenderUtility::drawClearQuad(UINT32 clearBuffers, const Color& color, float depth, UINT16 stencil)
	{
		// Set states
		if((clearBuffers & FBT_COLOR) != 0)
		{
			D3D11BlendState* d3d11BlendState = static_cast<D3D11BlendState*>(const_cast<BlendStateCore*>(mClearQuadBlendStateYesC.get()));
			mDevice->getImmediateContext()->OMSetBlendState(d3d11BlendState->getInternal(), nullptr, 0xFFFFFFFF);
		}
		else
		{
			D3D11BlendState* d3d11BlendState = static_cast<D3D11BlendState*>(const_cast<BlendStateCore*>(mClearQuadBlendStateNoC.get()));
			mDevice->getImmediateContext()->OMSetBlendState(d3d11BlendState->getInternal(), nullptr, 0xFFFFFFFF);
		}

		D3D11RasterizerState* d3d11RasterizerState = static_cast<D3D11RasterizerState*>(const_cast<RasterizerStateCore*>(mClearQuadRasterizerState.get()));
		mDevice->getImmediateContext()->RSSetState(d3d11RasterizerState->getInternal());

		if((clearBuffers & FBT_DEPTH) != 0)
		{
			if((clearBuffers & FBT_STENCIL) != 0)
			{
				D3D11DepthStencilState* d3d11RasterizerState = static_cast<D3D11DepthStencilState*>(const_cast<DepthStencilStateCore*>(mClearQuadDSStateYesD_YesS.get()));
				mDevice->getImmediateContext()->OMSetDepthStencilState(d3d11RasterizerState->getInternal(), stencil);
			}
			else
			{
				D3D11DepthStencilState* d3d11RasterizerState = static_cast<D3D11DepthStencilState*>(const_cast<DepthStencilStateCore*>(mClearQuadDSStateYesD_NoS.get()));
				mDevice->getImmediateContext()->OMSetDepthStencilState(d3d11RasterizerState->getInternal(), stencil);
			}
		}
		else
		{
			if((clearBuffers & FBT_STENCIL) != 0)
			{
				D3D11DepthStencilState* d3d11RasterizerState = static_cast<D3D11DepthStencilState*>(const_cast<DepthStencilStateCore*>(mClearQuadDSStateNoD_YesS.get()));
				mDevice->getImmediateContext()->OMSetDepthStencilState(d3d11RasterizerState->getInternal(), stencil);
			}
			else
			{
				D3D11DepthStencilState* d3d11RasterizerState = static_cast<D3D11DepthStencilState*>(const_cast<DepthStencilStateCore*>(mClearQuadDSStateNoD_NoS.get()));
				mDevice->getImmediateContext()->OMSetDepthStencilState(d3d11RasterizerState->getInternal(), stencil);
			}
		}

		// TODO - How smart it is to update buffer right before drawing it!? (cache the clip area)
		ClearVertex vertexData[4];
		vertexData[0].pos = Vector3(-1.0f, 1.0f, depth);
		vertexData[1].pos = Vector3(1.0f, 1.0f, depth);
		vertexData[2].pos = Vector3(-1.0f, -1.0f, depth);
		vertexData[3].pos = Vector3(1.0f, -1.0f, depth);

		vertexData[0].col = color.getAsRGBA();
		vertexData[1].col = color.getAsRGBA();
		vertexData[2].col = color.getAsRGBA();
		vertexData[3].col = color.getAsRGBA();

		mDevice->getImmediateContext()->UpdateSubresource(mClearQuadVB, 0, nullptr, vertexData, 0, sizeof(ClearVertex) * 4);

		mDevice->getImmediateContext()->VSSetShader(mClearQuadVS, nullptr, 0);
		mDevice->getImmediateContext()->PSSetShader(mClearQuadPS, nullptr, 0);

		ID3D11Buffer* buffers[1];
		buffers[0] = mClearQuadVB;

		UINT32 strides[1] = { sizeof(ClearVertex) };
		UINT32 offsets[1] = { 0 };

		mDevice->getImmediateContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mDevice->getImmediateContext()->IASetIndexBuffer(mClearQuadIB, DXGI_FORMAT_R16_UINT, 0);
		mDevice->getImmediateContext()->IASetVertexBuffers(0, 1, buffers, strides, offsets);
		mDevice->getImmediateContext()->IASetInputLayout(mClearQuadIL);

		mDevice->getImmediateContext()->DrawIndexed(6, 0, 0);
	}

	void D3D11RenderUtility::initClearQuadResources()
	{
		BLEND_STATE_DESC blendStateDescYesC;
		mClearQuadBlendStateYesC = RenderStateManager::instance().createBlendState(blendStateDescYesC);

		BLEND_STATE_DESC blendStateDescNoC;
		for(int i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
			blendStateDescNoC.renderTargetDesc[i].renderTargetWriteMask = 0;

		mClearQuadBlendStateNoC = RenderStateManager::instance().createBlendState(blendStateDescNoC);

		DEPTH_STENCIL_STATE_DESC depthStateDescNoD_NoS;
		depthStateDescNoD_NoS.depthReadEnable = false;
		depthStateDescNoD_NoS.depthWriteEnable = false;
		depthStateDescNoD_NoS.depthComparisonFunc = CMPF_ALWAYS_PASS;
		mClearQuadDSStateNoD_NoS = RenderStateManager::instance().createDepthStencilState(depthStateDescNoD_NoS);

		DEPTH_STENCIL_STATE_DESC depthStateDescYesD_NoS;
		depthStateDescYesD_NoS.depthReadEnable = false;
		depthStateDescYesD_NoS.depthWriteEnable = false; // TODO - Set to true
		//depthStateDescYesD_NoS.depthComparisonFunc = CMPF_ALWAYS_PASS;
		mClearQuadDSStateYesD_NoS = RenderStateManager::instance().createDepthStencilState(depthStateDescYesD_NoS);

		DEPTH_STENCIL_STATE_DESC depthStateDescYesD_YesS;
		depthStateDescYesD_YesS.depthReadEnable = false;
		depthStateDescYesD_YesS.depthWriteEnable = true;
		depthStateDescYesD_YesS.depthComparisonFunc = CMPF_ALWAYS_PASS;
		depthStateDescYesD_YesS.stencilEnable = true;
		depthStateDescYesD_YesS.frontStencilComparisonFunc = CMPF_ALWAYS_PASS;
		depthStateDescYesD_YesS.frontStencilPassOp = SOP_REPLACE;
		mClearQuadDSStateYesD_YesS = RenderStateManager::instance().createDepthStencilState(depthStateDescYesD_YesS);

		DEPTH_STENCIL_STATE_DESC depthStateDescNoD_YesS;
		depthStateDescNoD_YesS.depthReadEnable = false;
		depthStateDescNoD_YesS.depthWriteEnable = false;
		depthStateDescNoD_YesS.depthComparisonFunc = CMPF_ALWAYS_PASS;
		depthStateDescNoD_YesS.stencilEnable = true;
		depthStateDescNoD_YesS.frontStencilComparisonFunc = CMPF_ALWAYS_PASS;
		mClearQuadDSStateNoD_YesS = RenderStateManager::instance().createDepthStencilState(depthStateDescNoD_YesS);

		RASTERIZER_STATE_DESC rasterizerStateDesc;
		mClearQuadRasterizerState = RenderStateManager::instance().createRasterizerState(rasterizerStateDesc);

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

		hr = mDevice->getD3D11Device()->CreatePixelShader(static_cast<DWORD*>(microcode->GetBufferPointer()), 
			microcode->GetBufferSize(), mDevice->getClassLinkage(), &mClearQuadPS);

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

		hr = mDevice->getD3D11Device()->CreateVertexShader(static_cast<DWORD*>(microcode->GetBufferPointer()), 
			microcode->GetBufferSize(), mDevice->getClassLinkage(), &mClearQuadVS);

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

		hr = mDevice->getD3D11Device()->CreateInputLayout(declElements, 2, microcode->GetBufferPointer(), 
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

		vertexData[0].col = Color::White.getAsBGRA();
		vertexData[1].col = Color::White.getAsBGRA();
		vertexData[2].col = Color::White.getAsBGRA();
		vertexData[3].col = Color::White.getAsBGRA();

		D3D11_SUBRESOURCE_DATA vertexSubresourceData;
		vertexSubresourceData.pSysMem = vertexData;

		hr = mDevice->getD3D11Device()->CreateBuffer(&mVBDesc, &vertexSubresourceData, &mClearQuadVB);
		if (FAILED(hr) || mDevice->hasError())
		{
			String msg = mDevice->getErrorDescription();
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

		hr = mDevice->getD3D11Device()->CreateBuffer(&mIBDesc, &indexSubresourceData, &mClearQuadIB);
		if (FAILED(hr) || mDevice->hasError())
		{
			String msg = mDevice->getErrorDescription();
			BS_EXCEPT(RenderingAPIException, "Cannot create D3D11 buffer: " + msg);
		}
	}
}}