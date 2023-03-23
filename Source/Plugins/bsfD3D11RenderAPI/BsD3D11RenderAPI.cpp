//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11RenderAPI.h"
#include "BsD3D11DriverList.h"
#include "BsD3D11Driver.h"
#include "BsD3D11Device.h"
#include "BsD3D11TextureManager.h"
#include "BsD3D11Texture.h"
#include "BsD3D11HardwareBufferManager.h"
#include "BsD3D11RenderWindowManager.h"
#include "BsD3D11HLSLProgramFactory.h"
#include "BsD3D11BlendState.h"
#include "BsD3D11RasterizerState.h"
#include "BsD3D11DepthStencilState.h"
#include "BsD3D11SamplerState.h"
#include "BsD3D11GpuProgram.h"
#include "BsD3D11Mappings.h"
#include "BsD3D11VertexBuffer.h"
#include "BsD3D11IndexBuffer.h"
#include "BsD3D11RenderStateManager.h"
#include "BsD3D11GpuParamBlockBuffer.h"
#include "BsD3D11InputLayoutManager.h"
#include "BsD3D11TextureView.h"
#include "BsD3D11RenderUtility.h"
#include "RenderAPI/BsGpuParams.h"
#include "CoreThread/BsCoreThread.h"
#include "BsD3D11QueryManager.h"
#include "Debug/BsDebug.h"
#include "Error/BsException.h"
#include "Profiling/BsRenderStats.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "BsD3D11GpuBuffer.h"
#include "BsD3D11CommandBuffer.h"
#include "BsD3D11CommandBufferManager.h"
#include "BsD3D11RenderWindow.h"

using namespace bs;
using namespace bs::ct;

const StringID& D3D11RenderAPI::GetName() const
{
	static StringID strName("D3D11RenderAPI");
	return strName;
}

void D3D11RenderAPI::Initialize()
{
	THROW_IF_NOT_CORE_THREAD;

	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&mDXGIFactory);
	if(FAILED(hr))
		B3D_EXCEPT(RenderingAPIException, "Failed to create Direct3D11 DXGIFactory");

	mDriverList = B3DNew<D3D11DriverList>(mDXGIFactory);
	mActiveD3DDriver = mDriverList->Item(0); // TODO: Always get first driver, for now
	mVideoModeInfo = mActiveD3DDriver->GetVideoModeInfo();

	GPUInfo gpuInfo;
	gpuInfo.NumGpUs = std::min(5U, mDriverList->Count());

	for(u32 i = 0; i < gpuInfo.NumGpUs; i++)
		gpuInfo.Names[i] = mDriverList->Item(i)->GetDriverName();

	PlatformUtility::SetGPUInfoInternal(gpuInfo);

	IDXGIAdapter* selectedAdapter = mActiveD3DDriver->GetDeviceAdapter();

	D3D_FEATURE_LEVEL requestedLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	const u32 numRequestedLevels = sizeof(requestedLevels) / sizeof(requestedLevels[0]);

	u32 deviceFlags = 0;
#if B3D_DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	ID3D11Device* device;
	hr = D3D11CreateDevice(selectedAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, deviceFlags, requestedLevels, numRequestedLevels, D3D11_SDK_VERSION, &device, &mFeatureLevel, nullptr);

	// This will fail on Win 7 due to lack of 11.1, so re-try again without it
	if(hr == E_INVALIDARG)
	{
		hr = D3D11CreateDevice(selectedAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, deviceFlags, &requestedLevels[1], numRequestedLevels - 1, D3D11_SDK_VERSION, &device, &mFeatureLevel, nullptr);
	}

	if(FAILED(hr))
		B3D_EXCEPT(RenderingAPIException, "Failed to create Direct3D11 object. D3D11CreateDeviceN returned this error code: " + ToString(hr));

	mDevice = B3DNew<D3D11Device>(device);

	CommandBufferManager::StartUp<D3D11CommandBufferManager>();

	// Create the texture manager for use by others
	bs::TextureManager::StartUp<bs::D3D11TextureManager>();
	TextureManager::StartUp<D3D11TextureManager>();

	// Create hardware buffer manager
	bs::HardwareBufferManager::StartUp();
	HardwareBufferManager::StartUp<D3D11HardwareBufferManager>(std::ref(*mDevice));

	// Create render window manager
	bs::RenderWindowManager::StartUp<bs::D3D11RenderWindowManager>(this);
	RenderWindowManager::StartUp();

	// Create & register HLSL factory
	mHLSLFactory = B3DNew<D3D11HLSLProgramFactory>();

	// Create render state manager
	RenderStateManager::StartUp<D3D11RenderStateManager>();

	// Create main command buffer
	mMainCommandBuffer = std::static_pointer_cast<D3D11CommandBuffer>(CommandBuffer::Create(GQT_GRAPHICS));

	mNumDevices = 1;
	mCurrentCapabilities = B3DNewMultiple<GpuDeviceCapabilities>(mNumDevices);
	InitCapabilites(selectedAdapter, mCurrentCapabilities[0]);

	GpuProgramManager::Instance().AddFactory("hlsl", mHLSLFactory);

	mIAManager = B3DNew<D3D11InputLayoutManager>();

	RenderAPI::Initialize();
}

void D3D11RenderAPI::InitializeWithWindow(const SPtr<RenderWindow>& primaryWindow)
{
	D3D11RenderUtility::StartUp(mDevice);

	QueryManager::StartUp<D3D11QueryManager>();

	RenderAPI::InitializeWithWindow(primaryWindow);
}

void D3D11RenderAPI::DestroyCore()
{
	THROW_IF_NOT_CORE_THREAD;

	// Ensure that all GPU commands finish executing before shutting down the device. If we don't do this a crash
	// on shutdown may occurr as the driver is still executing the commands, and we unload this library.
	mDevice->GetImmediateContext()->Flush();
	SPtr<EventQuery> query = EventQuery::Create();
	query->Begin();
	while(!query->IsReady())
	{
		// Spin
	}
	query = nullptr;

	QueryManager::ShutDown();
	D3D11RenderUtility::ShutDown();

	if(mIAManager != nullptr)
	{
		B3DDelete(mIAManager);
		mIAManager = nullptr;
	}

	if(mHLSLFactory != nullptr)
	{
		B3DDelete(mHLSLFactory);
		mHLSLFactory = nullptr;
	}

	mActiveVertexDeclaration = nullptr;
	mActiveVertexShader = nullptr;
	mActiveRenderTarget = nullptr;
	mActiveDepthStencilState = nullptr;
	mMainCommandBuffer = nullptr;

	RenderStateManager::ShutDown();
	RenderWindowManager::ShutDown();
	bs::RenderWindowManager::ShutDown();
	HardwareBufferManager::ShutDown();
	bs::HardwareBufferManager::ShutDown();
	TextureManager::ShutDown();
	bs::TextureManager::ShutDown();
	CommandBufferManager::ShutDown();

	SAFE_RELEASE(mDXGIFactory);

	if(mDevice != nullptr)
	{
		B3DDelete(mDevice);
		mDevice = nullptr;
	}

	if(mDriverList != nullptr)
	{
		B3DDelete(mDriverList);
		mDriverList = nullptr;
	}

	mActiveD3DDriver = nullptr;

	RenderAPI::DestroyCore();
}

void D3D11RenderAPI::SetGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](const SPtr<GraphicsPipelineState>& pipelineState)
	{
		THROW_IF_NOT_CORE_THREAD;

		D3D11BlendState* d3d11BlendState;
		D3D11RasterizerState* d3d11RasterizerState;

		D3D11GpuFragmentProgram* d3d11FragmentProgram;
		D3D11GpuGeometryProgram* d3d11GeometryProgram;
		D3D11GpuDomainProgram* d3d11DomainProgram;
		D3D11GpuHullProgram* d3d11HullProgram;

		if(pipelineState != nullptr)
		{
			d3d11BlendState = static_cast<D3D11BlendState*>(pipelineState->GetBlendState().get());
			d3d11RasterizerState = static_cast<D3D11RasterizerState*>(pipelineState->GetRasterizerState().get());
			mActiveDepthStencilState = std::static_pointer_cast<D3D11DepthStencilState>(pipelineState->GetDepthStencilState());

			mActiveVertexShader = std::static_pointer_cast<D3D11GpuVertexProgram>(pipelineState->GetVertexProgram());
			d3d11FragmentProgram = static_cast<D3D11GpuFragmentProgram*>(pipelineState->GetFragmentProgram().get());
			d3d11GeometryProgram = static_cast<D3D11GpuGeometryProgram*>(pipelineState->GetGeometryProgram().get());
			d3d11DomainProgram = static_cast<D3D11GpuDomainProgram*>(pipelineState->GetDomainProgram().get());
			d3d11HullProgram = static_cast<D3D11GpuHullProgram*>(pipelineState->GetHullProgram().get());

			if(d3d11BlendState == nullptr)
				d3d11BlendState = static_cast<D3D11BlendState*>(BlendState::GetDefault().get());

			if(d3d11RasterizerState == nullptr)
				d3d11RasterizerState = static_cast<D3D11RasterizerState*>(RasterizerState::GetDefault().get());

			if(mActiveDepthStencilState == nullptr)
				mActiveDepthStencilState = std::static_pointer_cast<D3D11DepthStencilState>(DepthStencilState::GetDefault());
		}
		else
		{
			d3d11BlendState = static_cast<D3D11BlendState*>(BlendState::GetDefault().get());
			d3d11RasterizerState = static_cast<D3D11RasterizerState*>(RasterizerState::GetDefault().get());
			mActiveDepthStencilState = std::static_pointer_cast<D3D11DepthStencilState>(DepthStencilState::GetDefault());

			mActiveVertexShader = nullptr;
			d3d11FragmentProgram = nullptr;
			d3d11GeometryProgram = nullptr;
			d3d11DomainProgram = nullptr;
			d3d11HullProgram = nullptr;
		}

		ID3D11DeviceContext* d3d11Context = mDevice->GetImmediateContext();
		d3d11Context->OMSetBlendState(d3d11BlendState->GetInternal(), nullptr, 0xFFFFFFFF);
		d3d11Context->RSSetState(d3d11RasterizerState->GetInternal());
		d3d11Context->OMSetDepthStencilState(mActiveDepthStencilState->GetInternal(), mStencilRef);

		if(mActiveVertexShader != nullptr)
		{
			D3D11GpuVertexProgram* vertexProgram = static_cast<D3D11GpuVertexProgram*>(mActiveVertexShader.get());
			d3d11Context->VSSetShader(vertexProgram->GetVertexShader(), nullptr, 0);
		}
		else
			d3d11Context->VSSetShader(nullptr, nullptr, 0);

		if(d3d11FragmentProgram != nullptr)
			d3d11Context->PSSetShader(d3d11FragmentProgram->GetPixelShader(), nullptr, 0);
		else
			d3d11Context->PSSetShader(nullptr, nullptr, 0);

		if(d3d11GeometryProgram != nullptr)
			d3d11Context->GSSetShader(d3d11GeometryProgram->GetGeometryShader(), nullptr, 0);
		else
			d3d11Context->GSSetShader(nullptr, nullptr, 0);

		if(d3d11DomainProgram != nullptr)
			d3d11Context->DSSetShader(d3d11DomainProgram->GetDomainShader(), nullptr, 0);
		else
			d3d11Context->DSSetShader(nullptr, nullptr, 0);

		if(d3d11HullProgram != nullptr)
			d3d11Context->HSSetShader(d3d11HullProgram->GetHullShader(), nullptr, 0);
		else
			d3d11Context->HSSetShader(nullptr, nullptr, 0);
	};

	auto execute = [=]()
	{ executeRef(pipelineState); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	B3D_INCREMENT_RENDER_STATISTIC(NumPipelineStateChanges);
}

void D3D11RenderAPI::SetComputePipeline(const SPtr<ComputePipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](const SPtr<ComputePipelineState>& pipelineState)
	{
		THROW_IF_NOT_CORE_THREAD;

		SPtr<GpuProgram> program;
		if(pipelineState != nullptr)
			program = pipelineState->GetProgram();

		if(program != nullptr && program->GetType() == GPT_COMPUTE_PROGRAM)
		{
			D3D11GpuComputeProgram* d3d11ComputeProgram = static_cast<D3D11GpuComputeProgram*>(program.get());
			mDevice->GetImmediateContext()->CSSetShader(d3d11ComputeProgram->GetComputeShader(), nullptr, 0);
		}
		else
			mDevice->GetImmediateContext()->CSSetShader(nullptr, nullptr, 0);
	};

	auto execute = [=]()
	{ executeRef(pipelineState); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	B3D_INCREMENT_RENDER_STATISTIC(NumPipelineStateChanges);
}

void D3D11RenderAPI::SetGpuParams(const SPtr<GpuParams>& gpuParams, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](const SPtr<GpuParams>& gpuParams)
	{
		THROW_IF_NOT_CORE_THREAD;

		ID3D11DeviceContext* context = mDevice->GetImmediateContext();

		// Clear any previously bound UAVs (otherwise shaders attempting to read resources viewed by those views will
		// be unable to)
		if(mPSUAVsBound || mCSUAVsBound)
		{
			ID3D11UnorderedAccessView* emptyUAVs[D3D11_PS_CS_UAV_REGISTER_COUNT];
			B3DZeroOut(emptyUAVs);

			if(mPSUAVsBound)
			{
				context->OMSetRenderTargetsAndUnorderedAccessViews(
					D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, 0,
					D3D11_PS_CS_UAV_REGISTER_COUNT, emptyUAVs, nullptr);

				mPSUAVsBound = false;
			}

			if(mCSUAVsBound)
			{
				context->CSSetUnorderedAccessViews(0, D3D11_PS_CS_UAV_REGISTER_COUNT, emptyUAVs, nullptr);

				mCSUAVsBound = false;
			}
		}

		B3DMarkAllocatorFrame();
		{
			FrameVector<ID3D11ShaderResourceView*> srvs(8);
			FrameVector<ID3D11UnorderedAccessView*> uavs(8);
			FrameVector<ID3D11Buffer*> constBuffers(8);
			FrameVector<ID3D11SamplerState*> samplers(8);

			auto populateViews = [&](GpuProgramType type)
			{
				srvs.clear();
				uavs.clear();
				constBuffers.clear();
				samplers.clear();

				SPtr<GpuParamDesc> paramDesc = gpuParams->GetParameterInformation(type);
				if(paramDesc == nullptr)
					return;

				for(auto iter = paramDesc->Textures.begin(); iter != paramDesc->Textures.end(); ++iter)
				{
					u32 slot = iter->second.Slot;

					SPtr<Texture> texture = gpuParams->GetSampledTexture(iter->second.Set, slot);
					const TextureSurface& surface = gpuParams->GetTextureSurface(iter->second.Set, slot);

					while(slot >= (u32)srvs.size())
						srvs.push_back(nullptr);

					if(texture != nullptr)
					{
						SPtr<TextureView> texView = texture->RequestView(surface, GVU_DEFAULT);

						D3D11TextureView* d3d11texView = static_cast<D3D11TextureView*>(texView.get());
						srvs[slot] = d3d11texView->GetSrv();
					}
				}

				for(auto iter = paramDesc->Buffers.begin(); iter != paramDesc->Buffers.end(); ++iter)
				{
					u32 slot = iter->second.Slot;
					SPtr<GpuBuffer> buffer = gpuParams->GetStorageBuffer(iter->second.Set, slot);

					bool isLoadStore = iter->second.Type != GPOT_BYTE_BUFFER &&
						iter->second.Type != GPOT_STRUCTURED_BUFFER;

					if(!isLoadStore)
					{
						while(slot >= (u32)srvs.size())
							srvs.push_back(nullptr);

						if(buffer != nullptr)
						{
							D3D11GpuBuffer* d3d11buffer = static_cast<D3D11GpuBuffer*>(buffer.get());
							srvs[slot] = d3d11buffer->GetSrv();
						}
					}
					else
					{
						while(slot >= (u32)uavs.size())
							uavs.push_back(nullptr);

						if(buffer != nullptr)
						{
							D3D11GpuBuffer* d3d11buffer = static_cast<D3D11GpuBuffer*>(buffer.get());
							uavs[slot] = d3d11buffer->GetUav();
						}
					}
				}

				for(auto iter = paramDesc->LoadStoreTextures.begin(); iter != paramDesc->LoadStoreTextures.end(); ++iter)
				{
					u32 slot = iter->second.Slot;

					SPtr<Texture> texture = gpuParams->GetStorageTexture(iter->second.Set, slot);
					const TextureSurface& surface = gpuParams->GetStorageTextureSurface(iter->second.Set, slot);

					while(slot >= (u32)uavs.size())
						uavs.push_back(nullptr);

					if(texture != nullptr)
					{
						SPtr<TextureView> texView = texture->RequestView(TextureSurface(surface.MipLevel, 1, surface.Face, surface.FaceCount), GVU_RANDOMWRITE);

						D3D11TextureView* d3d11texView = static_cast<D3D11TextureView*>(texView.get());
						uavs[slot] = d3d11texView->GetUav();
					}
					else
					{
						uavs[slot] = nullptr;
					}
				}

				for(auto iter = paramDesc->Samplers.begin(); iter != paramDesc->Samplers.end(); ++iter)
				{
					u32 slot = iter->second.Slot;
					SPtr<SamplerState> samplerState = gpuParams->GetSamplerState(iter->second.Set, slot);

					while(slot >= (u32)samplers.size())
						samplers.push_back(nullptr);

					if(samplerState == nullptr)
						samplerState = SamplerState::GetDefault();

					D3D11SamplerState* d3d11SamplerState =
						static_cast<D3D11SamplerState*>(const_cast<SamplerState*>(samplerState.get()));
					samplers[slot] = d3d11SamplerState->GetInternal();
				}

				for(auto iter = paramDesc->ParamBlocks.begin(); iter != paramDesc->ParamBlocks.end(); ++iter)
				{
					u32 slot = iter->second.Slot;
					SPtr<GpuBuffer> buffer = gpuParams->GetUniformBuffer(iter->second.Set, slot);

					while(slot >= (u32)constBuffers.size())
						constBuffers.push_back(nullptr);

					if(buffer != nullptr)
					{
						buffer->FlushToGpu();

						const D3D11GpuParamBlockBuffer* d3d11paramBlockBuffer =
							static_cast<const D3D11GpuParamBlockBuffer*>(buffer.get());
						constBuffers[slot] = d3d11paramBlockBuffer->GetD3D11Buffer();
					}
				}
			};

			u32 numSRVs = 0;
			u32 numUAVs = 0;
			u32 numConstBuffers = 0;
			u32 numSamplers = 0;

			populateViews(GPT_VERTEX_PROGRAM);
			numSRVs = (u32)srvs.size();
			numConstBuffers = (u32)constBuffers.size();
			numSamplers = (u32)samplers.size();

			if(numSRVs > 0)
				context->VSSetShaderResources(0, numSRVs, srvs.data());

			if(numConstBuffers > 0)
				context->VSSetConstantBuffers(0, numConstBuffers, constBuffers.data());

			if(numSamplers > 0)
				context->VSSetSamplers(0, numSamplers, samplers.data());

			populateViews(GPT_FRAGMENT_PROGRAM);
			numSRVs = (u32)srvs.size();
			numUAVs = (u32)uavs.size();
			numConstBuffers = (u32)constBuffers.size();
			numSamplers = (u32)samplers.size();

			if(numSRVs > 0)
				context->PSSetShaderResources(0, numSRVs, srvs.data());

			if(numUAVs > 0)
			{
				context->OMSetRenderTargetsAndUnorderedAccessViews(
					D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, 0, numUAVs, uavs.data(), nullptr);
				mPSUAVsBound = true;
			}

			if(numConstBuffers > 0)
				context->PSSetConstantBuffers(0, numConstBuffers, constBuffers.data());

			if(numSamplers > 0)
				context->PSSetSamplers(0, numSamplers, samplers.data());

			populateViews(GPT_GEOMETRY_PROGRAM);
			numSRVs = (u32)srvs.size();
			numConstBuffers = (u32)constBuffers.size();
			numSamplers = (u32)samplers.size();

			if(numSRVs > 0)
				context->GSSetShaderResources(0, numSRVs, srvs.data());

			if(numConstBuffers > 0)
				context->GSSetConstantBuffers(0, numConstBuffers, constBuffers.data());

			if(numSamplers > 0)
				context->GSSetSamplers(0, numSamplers, samplers.data());

			populateViews(GPT_HULL_PROGRAM);
			numSRVs = (u32)srvs.size();
			numConstBuffers = (u32)constBuffers.size();
			numSamplers = (u32)samplers.size();

			if(numSRVs > 0)
				context->HSSetShaderResources(0, numSRVs, srvs.data());

			if(numConstBuffers > 0)
				context->HSSetConstantBuffers(0, numConstBuffers, constBuffers.data());

			if(numSamplers > 0)
				context->HSSetSamplers(0, numSamplers, samplers.data());

			populateViews(GPT_DOMAIN_PROGRAM);
			numSRVs = (u32)srvs.size();
			numConstBuffers = (u32)constBuffers.size();
			numSamplers = (u32)samplers.size();

			if(numSRVs > 0)
				context->DSSetShaderResources(0, numSRVs, srvs.data());

			if(numConstBuffers > 0)
				context->DSSetConstantBuffers(0, numConstBuffers, constBuffers.data());

			if(numSamplers > 0)
				context->DSSetSamplers(0, numSamplers, samplers.data());

			populateViews(GPT_COMPUTE_PROGRAM);
			numSRVs = (u32)srvs.size();
			numUAVs = (u32)uavs.size();
			numConstBuffers = (u32)constBuffers.size();
			numSamplers = (u32)samplers.size();

			if(numSRVs > 0)
				context->CSSetShaderResources(0, numSRVs, srvs.data());

			if(numUAVs > 0)
			{
				context->CSSetUnorderedAccessViews(0, numUAVs, uavs.data(), nullptr);
				mCSUAVsBound = true;
			}

			if(numConstBuffers > 0)
				context->CSSetConstantBuffers(0, numConstBuffers, constBuffers.data());

			if(numSamplers > 0)
				context->CSSetSamplers(0, numSamplers, samplers.data());
		}
		B3DClearAllocatorFrame();

		if(mDevice->HasError())
			B3D_EXCEPT(RenderingAPIException, "Failed to set GPU parameters: " + mDevice->GetErrorDescription());
	};

	auto execute = [=]()
	{ executeRef(gpuParams); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	B3D_INCREMENT_RENDER_STATISTIC(NumGpuParamBinds);
}

void D3D11RenderAPI::SetViewport(const Rect2& vp, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](const Rect2& vp)
	{
		THROW_IF_NOT_CORE_THREAD;

		mViewportNorm = vp;
		ApplyViewport();
	};

	auto execute = [=]()
	{ executeRef(vp); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);
}

void D3D11RenderAPI::SetVertexBuffers(u32 index, SPtr<VertexBuffer>* buffers, u32 numBuffers, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](u32 index, const SmallVector<SPtr<VertexBuffer>, 8>& buffers, u32 numBuffers)
	{
		THROW_IF_NOT_CORE_THREAD;

		u32 maxBoundVertexBuffers = mCurrentCapabilities[0].MaxBoundVertexBuffers;
		if(index < 0 || (index + numBuffers) >= maxBoundVertexBuffers)
		{
			B3D_EXCEPT(InvalidParametersException, "Invalid vertex index: " + ToString(index) + ". Valid range is 0 .. " + ToString(maxBoundVertexBuffers - 1));
		}

		ID3D11Buffer* dx11buffers[BS_MAX_BOUND_VERTEX_BUFFERS];
		u32 strides[BS_MAX_BOUND_VERTEX_BUFFERS];
		u32 offsets[BS_MAX_BOUND_VERTEX_BUFFERS];

		for(u32 i = 0; i < numBuffers; i++)
		{
			SPtr<D3D11VertexBuffer> vertexBuffer = std::static_pointer_cast<D3D11VertexBuffer>(buffers[i]);
			const VertexBufferProperties& vbProps = vertexBuffer->GetProperties();

			dx11buffers[i] = vertexBuffer->GetD3DVertexBuffer();

			strides[i] = vbProps.GetVertexSize();
			offsets[i] = 0;
		}

		mDevice->GetImmediateContext()->IASetVertexBuffers(index, numBuffers, dx11buffers, strides, offsets);
	};

	SmallVector<SPtr<VertexBuffer>, 8> _buffers;
	for(u32 i = 0; i < numBuffers; i++)
		_buffers.Add(buffers[i]);

	auto execute = [executeRef, index, buffers = std::move(_buffers), numBuffers]()
	{ executeRef(index, buffers, numBuffers); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	B3D_INCREMENT_RENDER_STATISTIC(NumVertexBufferBinds);
}

void D3D11RenderAPI::SetIndexBuffer(const SPtr<IndexBuffer>& buffer, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](const SPtr<IndexBuffer>& buffer)
	{
		THROW_IF_NOT_CORE_THREAD;

		SPtr<D3D11IndexBuffer> indexBuffer = std::static_pointer_cast<D3D11IndexBuffer>(buffer);

		DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
		if(indexBuffer->GetProperties().GetType() == IT_16BIT)
			indexFormat = DXGI_FORMAT_R16_UINT;
		else if(indexBuffer->GetProperties().GetType() == IT_32BIT)
			indexFormat = DXGI_FORMAT_R32_UINT;
		else
			B3D_EXCEPT(InternalErrorException, "Unsupported index format: " + ToString(indexBuffer->GetProperties().GetType()));

		mDevice->GetImmediateContext()->IASetIndexBuffer(indexBuffer->GetD3DIndexBuffer(), indexFormat, 0);
	};

	auto execute = [=]()
	{ executeRef(buffer); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	B3D_INCREMENT_RENDER_STATISTIC(NumIndexBufferBinds);
}

void D3D11RenderAPI::SetVertexDescription(const SPtr<VertexDeclaration>& vertexDeclaration, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](const SPtr<VertexDeclaration>& vertexDeclaration)
	{
		THROW_IF_NOT_CORE_THREAD;

		mActiveVertexDeclaration = vertexDeclaration;
	};

	auto execute = [=]()
	{ executeRef(vertexDeclaration); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);
}

void D3D11RenderAPI::SetDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](DrawOperationType op)
	{
		THROW_IF_NOT_CORE_THREAD;

		mDevice->GetImmediateContext()->IASetPrimitiveTopology(D3D11Mappings::GetPrimitiveType(op));
		mActiveDrawOp = op;
	};

	auto execute = [=]()
	{ executeRef(op); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);
}

void D3D11RenderAPI::Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](u32 vertexOffset, u32 vertexCount, u32 instanceCount)
	{
		THROW_IF_NOT_CORE_THREAD;

		ApplyInputLayout();

		if(instanceCount <= 1)
			mDevice->GetImmediateContext()->Draw(vertexCount, vertexOffset);
		else
			mDevice->GetImmediateContext()->DrawInstanced(vertexCount, instanceCount, vertexOffset, 0);

		NotifyRenderTargetModified();

#if B3D_DEBUG
		if(mDevice->HasError())
			B3D_LOG(Warning, RenderBackend, mDevice->GetErrorDescription());
#endif
	};

	auto execute = [=]()
	{ executeRef(vertexOffset, vertexCount, instanceCount); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	u32 primCount = VertexCountToPrimCount(mActiveDrawOp, vertexCount);

	B3D_INCREMENT_RENDER_STATISTIC(NumDrawCalls);
	B3D_ADD_RENDER_STATISTIC(NumVertices, vertexCount);
	B3D_ADD_RENDER_STATISTIC(NumPrimitives, primCount);
}

void D3D11RenderAPI::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount,
						  u32 instanceCount)
	{
		THROW_IF_NOT_CORE_THREAD;

		ApplyInputLayout();

		if(instanceCount <= 1)
			mDevice->GetImmediateContext()->DrawIndexed(indexCount, startIndex, vertexOffset);
		else
			mDevice->GetImmediateContext()->DrawIndexedInstanced(indexCount, instanceCount, startIndex, vertexOffset, 0);

		NotifyRenderTargetModified();

#if B3D_DEBUG
		if(mDevice->HasError())
			B3D_LOG(Warning, RenderBackend, mDevice->GetErrorDescription());
#endif
	};

	auto execute = [=]()
	{ executeRef(startIndex, indexCount, vertexOffset, vertexCount, instanceCount); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	u32 primCount = VertexCountToPrimCount(mActiveDrawOp, indexCount);

	B3D_INCREMENT_RENDER_STATISTIC(NumDrawCalls);
	B3D_ADD_RENDER_STATISTIC(NumVertices, vertexCount);
	B3D_ADD_RENDER_STATISTIC(NumPrimitives, primCount);
}

void D3D11RenderAPI::DispatchCompute(u32 numGroupsX, u32 numGroupsY, u32 numGroupsZ, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](u32 numGroupsX, u32 numGroupsY, u32 numGroupsZ)
	{
		THROW_IF_NOT_CORE_THREAD;

		mDevice->GetImmediateContext()->Dispatch(numGroupsX, numGroupsY, numGroupsZ);

#if B3D_DEBUG
		if(mDevice->HasError())
			B3D_LOG(Warning, RenderBackend, mDevice->GetErrorDescription());
#endif
	};

	auto execute = [=]()
	{ executeRef(numGroupsX, numGroupsY, numGroupsZ); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	B3D_INCREMENT_RENDER_STATISTIC(NumComputeCalls);
}

void D3D11RenderAPI::SetScissorRect(u32 left, u32 top, u32 right, u32 bottom, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](u32 left, u32 top, u32 right, u32 bottom)
	{
		THROW_IF_NOT_CORE_THREAD;

		mScissorRect.left = static_cast<LONG>(left);
		mScissorRect.top = static_cast<LONG>(top);
		mScissorRect.bottom = static_cast<LONG>(bottom);
		mScissorRect.right = static_cast<LONG>(right);

		mDevice->GetImmediateContext()->RSSetScissorRects(1, &mScissorRect);
	};

	auto execute = [=]()
	{ executeRef(left, top, right, bottom); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);
}

void D3D11RenderAPI::SetStencilRef(u32 value, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](u32 value)
	{
		THROW_IF_NOT_CORE_THREAD;

		mStencilRef = value;

		if(mActiveDepthStencilState != nullptr)
			mDevice->GetImmediateContext()->OMSetDepthStencilState(mActiveDepthStencilState->GetInternal(), mStencilRef);
		else
			mDevice->GetImmediateContext()->OMSetDepthStencilState(nullptr, mStencilRef);
	};

	auto execute = [=]()
	{ executeRef(value); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);
}

void D3D11RenderAPI::ClearViewport(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(mActiveRenderTarget == nullptr)
			return;

		const RenderTargetProperties& rtProps = mActiveRenderTarget->GetProperties();

		Rect2I clearArea((int)mViewport.TopLeftX, (int)mViewport.TopLeftY, (int)mViewport.Width, (int)mViewport.Height);

		bool clearEntireTarget = clearArea.Width == 0 || clearArea.Height == 0;
		clearEntireTarget |= (clearArea.X == 0 && clearArea.Y == 0 && clearArea.Width == rtProps.Width && clearArea.Height == rtProps.Height);

		if(!clearEntireTarget)
		{
			// TODO - Ignoring targetMask here
			D3D11RenderUtility::Instance().DrawClearQuad(buffers, color, depth, stencil);
			B3D_INCREMENT_RENDER_STATISTIC(NumClears);

			NotifyRenderTargetModified();
		}
		else
			ClearRenderTarget(buffers, color, depth, stencil, targetMask);
	};

	auto execute = [=]()
	{ executeRef(buffers, color, depth, stencil, targetMask); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);
}

void D3D11RenderAPI::ClearRenderTarget(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(mActiveRenderTarget == nullptr)
			return;

		// Clear render surfaces
		if(buffers & FBT_COLOR)
		{
			u32 maxRenderTargets = mCurrentCapabilities[0].NumMultiRenderTargets;

			ID3D11RenderTargetView** views = B3DNewMultiple<ID3D11RenderTargetView*>(maxRenderTargets);
			memset(views, 0, sizeof(ID3D11RenderTargetView*) * maxRenderTargets);

			mActiveRenderTarget->GetCustomAttribute("RTV", views);
			if(!views[0])
			{
				B3DDeleteMultiple(views, maxRenderTargets);
				return;
			}

			float clearColor[4];
			clearColor[0] = color.R;
			clearColor[1] = color.G;
			clearColor[2] = color.B;
			clearColor[3] = color.A;

			for(u32 i = 0; i < maxRenderTargets; i++)
			{
				if(views[i] != nullptr && ((1 << i) & targetMask) != 0)
					mDevice->GetImmediateContext()->ClearRenderTargetView(views[i], clearColor);
			}

			B3DDeleteMultiple(views, maxRenderTargets);
		}

		// Clear depth stencil
		if((buffers & FBT_DEPTH) != 0 || (buffers & FBT_STENCIL) != 0)
		{
			ID3D11DepthStencilView* depthStencilView = nullptr;
			mActiveRenderTarget->GetCustomAttribute("DSV", &depthStencilView);

			D3D11_CLEAR_FLAG clearFlag;

			if((buffers & FBT_DEPTH) != 0 && (buffers & FBT_STENCIL) != 0)
				clearFlag = (D3D11_CLEAR_FLAG)(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);
			else if((buffers & FBT_STENCIL) != 0)
				clearFlag = D3D11_CLEAR_STENCIL;
			else
				clearFlag = D3D11_CLEAR_DEPTH;

			if(depthStencilView != nullptr)
				mDevice->GetImmediateContext()->ClearDepthStencilView(depthStencilView, clearFlag, depth, (u8)stencil);
		}

		NotifyRenderTargetModified();
	};

	auto execute = [=]()
	{ executeRef(buffers, color, depth, stencil, targetMask); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	B3D_INCREMENT_RENDER_STATISTIC(NumClears);
}

void D3D11RenderAPI::SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags, RenderSurfaceMask loadMask, const SPtr<CommandBuffer>& commandBuffer)
{
	auto executeRef = [&](const SPtr<RenderTarget>& target, u32 readOnlyFlags)
	{
		THROW_IF_NOT_CORE_THREAD;

		mActiveRenderTarget = target;
		mActiveRenderTargetModified = false;

		u32 maxRenderTargets = mCurrentCapabilities[0].NumMultiRenderTargets;
		ID3D11RenderTargetView** views = B3DNewMultiple<ID3D11RenderTargetView*>(maxRenderTargets);
		memset(views, 0, sizeof(ID3D11RenderTargetView*) * maxRenderTargets);

		ID3D11DepthStencilView* depthStencilView = nullptr;

		if(target != nullptr)
		{
			target->GetCustomAttribute("RTV", views);

			if((readOnlyFlags & FBT_DEPTH) == 0)
			{
				if((readOnlyFlags & FBT_STENCIL) == 0)
					target->GetCustomAttribute("DSV", &depthStencilView);
				else
					target->GetCustomAttribute("WDROSV", &depthStencilView);
			}
			else
			{
				if((readOnlyFlags & FBT_STENCIL) == 0)
					target->GetCustomAttribute("RODWSV", &depthStencilView);
				else
					target->GetCustomAttribute("RODSV", &depthStencilView);
			}
		}

		// Bind render targets
		mDevice->GetImmediateContext()->OMSetRenderTargets(maxRenderTargets, views, depthStencilView);
		if(mDevice->HasError())
			B3D_EXCEPT(RenderingAPIException, "Failed to setRenderTarget : " + mDevice->GetErrorDescription());

		B3DDeleteMultiple(views, maxRenderTargets);
		ApplyViewport();
	};

	auto execute = [=]()
	{ executeRef(target, readOnlyFlags); };

	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->QueueCommand(execute);

	B3D_INCREMENT_RENDER_STATISTIC(NumRenderTargetChanges);
}

void D3D11RenderAPI::SwapBuffers(const SPtr<RenderTarget>& target, u32 syncMask)
{
	THROW_IF_NOT_CORE_THREAD;

	if(target == nullptr || !target->GetProperties().IsWindow)
		return;

	D3D11RenderWindow *const window = static_cast<D3D11RenderWindow*>(target.get());

	SubmitCommandBuffer(mMainCommandBuffer, syncMask);
	window->SwapBuffers();

	B3D_INCREMENT_RENDER_STATISTIC(NumPresents);
}

void D3D11RenderAPI::AddCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary)
{
	// We're not supporting this as we don't support command buffer command queuing at all (i.e. they are executed
	// straight away).
	B3D_LOG(Error, RenderBackend, "Secondary command buffers not supported on DirectX 11.");
}

void D3D11RenderAPI::SubmitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, u32 syncMask)
{
	SPtr<D3D11CommandBuffer> cb = GetCb(commandBuffer);
	cb->ExecuteCommands();

	if(cb == mMainCommandBuffer)
		mMainCommandBuffer = std::static_pointer_cast<D3D11CommandBuffer>(CommandBuffer::Create(GQT_GRAPHICS));
}

SPtr<CommandBuffer> D3D11RenderAPI::GetMainCommandBuffer() const
{
	return mMainCommandBuffer;
}

SPtr<D3D11CommandBuffer> D3D11RenderAPI::GetCb(const SPtr<CommandBuffer>& buffer)
{
	if(buffer != nullptr)
		return std::static_pointer_cast<D3D11CommandBuffer>(buffer);

	return std::static_pointer_cast<D3D11CommandBuffer>(mMainCommandBuffer);
}

void D3D11RenderAPI::ApplyViewport()
{
	if(mActiveRenderTarget == nullptr)
		return;

	const RenderTargetProperties& rtProps = mActiveRenderTarget->GetProperties();

	// Set viewport dimensions
	mViewport.TopLeftX = (FLOAT)(rtProps.Width * mViewportNorm.X);
	mViewport.TopLeftY = (FLOAT)(rtProps.Height * mViewportNorm.Y);
	mViewport.Width = (FLOAT)(rtProps.Width * mViewportNorm.Width);
	mViewport.Height = (FLOAT)(rtProps.Height * mViewportNorm.Height);

	if(rtProps.RequiresTextureFlipping)
	{
		// Convert "top-left" to "bottom-left"
		mViewport.TopLeftY = rtProps.Height - mViewport.Height - mViewport.TopLeftY;
	}

	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;

	mDevice->GetImmediateContext()->RSSetViewports(1, &mViewport);
}

void D3D11RenderAPI::NotifyRenderTargetModified()
{
	if(mActiveRenderTarget == nullptr || mActiveRenderTargetModified)
		return;

	mActiveRenderTarget->TickUpdateCountInternal();
	mActiveRenderTargetModified = true;
}

void D3D11RenderAPI::InitCapabilites(IDXGIAdapter* adapter, GpuDeviceCapabilities& caps) const
{
	THROW_IF_NOT_CORE_THREAD;

	// This must query for DirectX 10 interface as this is unsupported for DX11
	LARGE_INTEGER driverVersionNum;
	DriverVersion driverVersion;
	if(SUCCEEDED(adapter->CheckInterfaceSupport(IID_ID3D10Device, &driverVersionNum)))
	{
		driverVersion.Major = HIWORD(driverVersionNum.HighPart);
		driverVersion.Minor = LOWORD(driverVersionNum.HighPart);
		driverVersion.Release = HIWORD(driverVersionNum.LowPart);
		driverVersion.Build = LOWORD(driverVersionNum.LowPart);
	}

	caps.DriverVersion = driverVersion;
	caps.DeviceName = mActiveD3DDriver->GetDriverDescription();
	caps.RenderApiName = GetName();

	caps.SetCapability(RSC_TEXTURE_COMPRESSION_BC);
	caps.SetCapability(RSC_TEXTURE_VIEWS);
	caps.SetCapability(RSC_BYTECODE_CACHING);
	caps.SetCapability(RSC_RENDER_TARGET_LAYERS);

	caps.AddShaderProfile("hlsl");

	if(mFeatureLevel >= D3D_FEATURE_LEVEL_10_1)
		caps.MaxBoundVertexBuffers = 32;
	else
		caps.MaxBoundVertexBuffers = 16;

	if(mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
	{
		caps.SetCapability(RSC_GEOMETRY_PROGRAM);

		caps.NumTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
		caps.NumTextureUnitsPerStage[GPT_VERTEX_PROGRAM] = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
		caps.NumTextureUnitsPerStage[GPT_GEOMETRY_PROGRAM] = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;

		caps.NumCombinedTextureUnits = caps.NumTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] + caps.NumTextureUnitsPerStage[GPT_VERTEX_PROGRAM] + caps.NumTextureUnitsPerStage[GPT_GEOMETRY_PROGRAM];

		caps.NumGpuParamBlockBuffersPerStage[GPT_FRAGMENT_PROGRAM] = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
		caps.NumGpuParamBlockBuffersPerStage[GPT_VERTEX_PROGRAM] = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
		caps.NumGpuParamBlockBuffersPerStage[GPT_GEOMETRY_PROGRAM] = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;

		caps.NumCombinedParamBlockBuffers = caps.NumGpuParamBlockBuffersPerStage[GPT_FRAGMENT_PROGRAM] + caps.NumGpuParamBlockBuffersPerStage[GPT_VERTEX_PROGRAM] + caps.NumGpuParamBlockBuffersPerStage[GPT_GEOMETRY_PROGRAM];
	}

	if(mFeatureLevel >= D3D_FEATURE_LEVEL_11_0)
	{
		caps.SetCapability(RSC_TESSELLATION_PROGRAM);
		caps.SetCapability(RSC_COMPUTE_PROGRAM);
		caps.SetCapability(RSC_LOAD_STORE);

		caps.NumTextureUnitsPerStage[GPT_HULL_PROGRAM] = D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT;
		caps.NumTextureUnitsPerStage[GPT_DOMAIN_PROGRAM] = D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT;
		caps.NumTextureUnitsPerStage[GPT_COMPUTE_PROGRAM] = D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT;

		caps.NumCombinedTextureUnits = caps.NumTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] + caps.NumTextureUnitsPerStage[GPT_VERTEX_PROGRAM] + caps.NumTextureUnitsPerStage[GPT_GEOMETRY_PROGRAM] + caps.NumTextureUnitsPerStage[GPT_HULL_PROGRAM] + caps.NumTextureUnitsPerStage[GPT_DOMAIN_PROGRAM] + caps.NumTextureUnitsPerStage[GPT_COMPUTE_PROGRAM];

		caps.NumGpuParamBlockBuffersPerStage[GPT_HULL_PROGRAM] = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
		caps.NumGpuParamBlockBuffersPerStage[GPT_DOMAIN_PROGRAM] = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
		caps.NumGpuParamBlockBuffersPerStage[GPT_COMPUTE_PROGRAM] = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;

		caps.NumCombinedParamBlockBuffers = caps.NumGpuParamBlockBuffersPerStage[GPT_FRAGMENT_PROGRAM] + caps.NumGpuParamBlockBuffersPerStage[GPT_VERTEX_PROGRAM] + caps.NumGpuParamBlockBuffersPerStage[GPT_GEOMETRY_PROGRAM] + caps.NumGpuParamBlockBuffersPerStage[GPT_HULL_PROGRAM] + caps.NumGpuParamBlockBuffersPerStage[GPT_DOMAIN_PROGRAM] + caps.NumGpuParamBlockBuffersPerStage[GPT_COMPUTE_PROGRAM];

		caps.NumLoadStoreTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] = D3D11_PS_CS_UAV_REGISTER_COUNT;
		caps.NumLoadStoreTextureUnitsPerStage[GPT_COMPUTE_PROGRAM] = D3D11_PS_CS_UAV_REGISTER_COUNT;

		caps.NumCombinedLoadStoreTextureUnits = caps.NumLoadStoreTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] + caps.NumLoadStoreTextureUnitsPerStage[GPT_COMPUTE_PROGRAM];
	}

	// Adapter details
	const DXGI_ADAPTER_DESC& adapterID = mActiveD3DDriver->GetAdapterIdentifier();

	// Determine vendor
	switch(adapterID.VendorId)
	{
	case 0x10DE:
		caps.DeviceVendor = GPU_NVIDIA;
		break;
	case 0x1002:
		caps.DeviceVendor = GPU_AMD;
		break;
	case 0x163C:
	case 0x8086:
		caps.DeviceVendor = GPU_INTEL;
		break;
	default:
		caps.DeviceVendor = GPU_UNKNOWN;
		break;
	};

	caps.NumMultiRenderTargets = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
}

void D3D11RenderAPI::DetermineMultisampleSettings(u32 multisampleCount, DXGI_FORMAT format, DXGI_SAMPLE_DESC* outputSampleDesc)
{
	if(multisampleCount == 0 || multisampleCount == 1)
	{
		outputSampleDesc->Count = 1;
		outputSampleDesc->Quality = 0;

		return;
	}

	bool tryCSAA = false; // Note: Disabled for now, but leaving the code for later so it might be useful

	enum CSAAMode
	{
		CSAA_Normal,
		CSAA_Quality
	};

	CSAAMode csaaMode = CSAA_Normal;

	bool foundValid = false;
	size_t origNumSamples = multisampleCount;
	while(!foundValid)
	{
		// Deal with special cases
		if(tryCSAA)
		{
			switch(multisampleCount)
			{
			case 8:
				if(csaaMode == CSAA_Quality)
				{
					outputSampleDesc->Count = 8;
					outputSampleDesc->Quality = 8;
				}
				else
				{
					outputSampleDesc->Count = 4;
					outputSampleDesc->Quality = 8;
				}
				break;
			case 16:
				if(csaaMode == CSAA_Quality)
				{
					outputSampleDesc->Count = 8;
					outputSampleDesc->Quality = 16;
				}
				else
				{
					outputSampleDesc->Count = 4;
					outputSampleDesc->Quality = 16;
				}
				break;
			}
		}
		else // !CSAA
		{
			outputSampleDesc->Count = multisampleCount == 0 ? 1 : multisampleCount;
			outputSampleDesc->Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
		}

		HRESULT hr;
		UINT outQuality;
		hr = mDevice->GetD3D11Device()->CheckMultisampleQualityLevels(format, outputSampleDesc->Count, &outQuality);

		if(SUCCEEDED(hr) && (!tryCSAA || outQuality > outputSampleDesc->Quality))
		{
			foundValid = true;
		}
		else
		{
			// Downgrade
			if(tryCSAA && multisampleCount == 8)
			{
				// For CSAA, we'll try downgrading with quality mode at all samples.
				// then try without quality, then drop CSAA
				if(csaaMode == CSAA_Quality)
				{
					// Drop quality first
					csaaMode = CSAA_Normal;
				}
				else
				{
					// Drop CSAA entirely
					tryCSAA = false;
				}

				// Return to original requested samples
				multisampleCount = static_cast<u32>(origNumSamples);
			}
			else
			{
				// Drop samples
				multisampleCount--;

				if(multisampleCount == 1)
				{
					// Ran out of options, no multisampling
					multisampleCount = 0;
					foundValid = true;
				}
			}
		}
	}
}

void D3D11RenderAPI::ConvertProjectionMatrix(const Matrix4& matrix, Matrix4& dest)
{
	dest = matrix;

	// Convert depth range from [-1,+1] to [0,1]
	dest[2][0] = (dest[2][0] + dest[3][0]) / 2;
	dest[2][1] = (dest[2][1] + dest[3][1]) / 2;
	dest[2][2] = (dest[2][2] + dest[3][2]) / 2;
	dest[2][3] = (dest[2][3] + dest[3][3]) / 2;
}

GpuParameterBlockInformation D3D11RenderAPI::GenerateParamBlockDesc(const String& name, Vector<GpuDataParameterInformation>& params)
{
	GpuParameterBlockInformation block;
	block.BlockSize = 0;
	block.IsShareable = true;
	block.Name = name;
	block.Slot = 0;
	block.Set = 0;

	for(auto& param : params)
	{
		const GpuDataParameterTypeInformation& typeInfo = bs::GpuParams::kParamSizes.Lookup[param.Type];

		if(param.ArraySize > 1)
		{
			// Arrays perform no packing and their elements are always padded and aligned to four component vectors
			u32 size;
			if(param.Type == GPDT_STRUCT)
				size = Math::DivideAndRoundUp(param.ElementSize, 16U) * 4;
			else
				size = Math::DivideAndRoundUp(typeInfo.Size, 16U) * 4;

			block.BlockSize = Math::DivideAndRoundUp(block.BlockSize, 4U) * 4;

			param.ElementSize = size;
			param.ArrayElementStride = size;
			param.CpuMemOffset = block.BlockSize;
			param.GpuMemOffset = 0;

			// Last array element isn't rounded up to four component vectors unless it's a struct
			if(param.Type != GPDT_STRUCT)
			{
				block.BlockSize += size * (param.ArraySize - 1);
				block.BlockSize += typeInfo.Size / 4;
			}
			else
				block.BlockSize += param.ArraySize * size;
		}
		else
		{
			u32 size;
			if(param.Type == GPDT_STRUCT)
			{
				// Structs are always aligned and arounded up to 4 component vectors
				size = Math::DivideAndRoundUp(param.ElementSize, 16U) * 4;
				block.BlockSize = Math::DivideAndRoundUp(block.BlockSize, 4U) * 4;
			}
			else
			{
				size = typeInfo.BaseTypeSize * (typeInfo.NumRows * typeInfo.NumColumns) / 4;

				// Pack everything as tightly as possible as long as the data doesn't cross 16 byte boundary
				u32 alignOffset = block.BlockSize % 4;
				if(alignOffset != 0 && size > (4 - alignOffset))
				{
					u32 padding = (4 - alignOffset);
					block.BlockSize += padding;
				}
			}

			param.ElementSize = size;
			param.ArrayElementStride = size;
			param.CpuMemOffset = block.BlockSize;
			param.GpuMemOffset = 0;

			block.BlockSize += size;
		}

		param.ParamBlockSlot = 0;
		param.ParamBlockSet = 0;
	}

	// Constant buffer size must always be a multiple of 16
	if(block.BlockSize % 4 != 0)
		block.BlockSize += (4 - (block.BlockSize % 4));

	return block;
}

/************************************************************************/
/* 								PRIVATE		                     		*/
/************************************************************************/

void D3D11RenderAPI::ApplyInputLayout()
{
	if(mActiveVertexDeclaration == nullptr)
	{
		B3D_LOG(Warning, RenderBackend, "Cannot apply input layout without a vertex declaration. Set vertex declaration "
									   "before calling this method.");
		return;
	}

	if(mActiveVertexShader == nullptr)
	{
		B3D_LOG(Warning, RenderBackend, "Cannot apply input layout without a vertex shader. Set vertex shader before "
									   "calling this method.");
		return;
	}

	ID3D11InputLayout* ia = mIAManager->RetrieveInputLayout(mActiveVertexShader->GetVertexInputDescription(), mActiveVertexDeclaration, *mActiveVertexShader);
	mDevice->GetImmediateContext()->IASetInputLayout(ia);
}
