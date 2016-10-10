//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
#include "BsGpuParams.h"
#include "BsCoreThread.h"
#include "BsD3D11QueryManager.h"
#include "BsDebug.h"
#include "BsException.h"
#include "BsRenderStats.h"
#include "BsGpuParamDesc.h"
#include "BsD3D11GpuBuffer.h"
#include "BsD3D11CommandBuffer.h"

namespace BansheeEngine
{
	D3D11RenderAPI::D3D11RenderAPI()
		: mDXGIFactory(nullptr), mDevice(nullptr), mDriverList(nullptr), mActiveD3DDriver(nullptr)
		, mFeatureLevel(D3D_FEATURE_LEVEL_11_0), mHLSLFactory(nullptr), mIAManager(nullptr), mStencilRef(0)
		, mActiveDrawOp(DOT_TRIANGLE_LIST), mViewportNorm(0.0f, 0.0f, 1.0f, 1.0f)
	{ }

	D3D11RenderAPI::~D3D11RenderAPI()
	{

	}

	const StringID& D3D11RenderAPI::getName() const
	{
		static StringID strName("D3D11RenderAPI");
		return strName;
	}

	const String& D3D11RenderAPI::getShadingLanguageName() const
	{
		static String strName("hlsl");
		return strName;
	}

	void D3D11RenderAPI::initializePrepare()
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&mDXGIFactory);
		if(FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Failed to create Direct3D11 DXGIFactory");

		mDriverList = bs_new<D3D11DriverList>(mDXGIFactory);
		mActiveD3DDriver = mDriverList->item(0); // TODO: Always get first driver, for now
		mVideoModeInfo = mActiveD3DDriver->getVideoModeInfo();

		IDXGIAdapter* selectedAdapter = mActiveD3DDriver->getDeviceAdapter();

		D3D_FEATURE_LEVEL requestedLevels[] = {
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};

		UINT32 numRequestedLevel = sizeof(requestedLevels) / sizeof(requestedLevels[0]);

		UINT32 deviceFlags = 0;

#if BS_DEBUG_MODE
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		ID3D11Device* device;
		hr = D3D11CreateDevice(selectedAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, deviceFlags, 
			requestedLevels, numRequestedLevel, D3D11_SDK_VERSION, &device, &mFeatureLevel, nullptr);

		if(FAILED(hr))         
			BS_EXCEPT(RenderingAPIException, "Failed to create Direct3D11 object. D3D11CreateDeviceN returned this error code: " + toString(hr));

		mDevice = bs_new<D3D11Device>(device);
		
		// This must query for DirectX 10 interface as this is unsupported for DX11
		LARGE_INTEGER driverVersion; 
		if(SUCCEEDED(selectedAdapter->CheckInterfaceSupport(IID_ID3D10Device, &driverVersion)))
		{
			mDriverVersion.major =  HIWORD(driverVersion.HighPart);
			mDriverVersion.minor = LOWORD(driverVersion.HighPart);
			mDriverVersion.release = HIWORD(driverVersion.LowPart);
			mDriverVersion.build = LOWORD(driverVersion.LowPart);
		}

		// Create the texture manager for use by others		
		TextureManager::startUp<D3D11TextureManager>();
		TextureCoreManager::startUp<D3D11TextureCoreManager>();

		// Create hardware buffer manager		
		HardwareBufferManager::startUp();
		HardwareBufferCoreManager::startUp<D3D11HardwareBufferCoreManager>(std::ref(*mDevice));

		// Create render window manager
		RenderWindowManager::startUp<D3D11RenderWindowManager>(this);
		RenderWindowCoreManager::startUp<D3D11RenderWindowCoreManager>(this);

		// Create & register HLSL factory		
		mHLSLFactory = bs_new<D3D11HLSLProgramFactory>();

		// Create render state manager
		RenderStateCoreManager::startUp<D3D11RenderStateCoreManager>();

		mCurrentCapabilities = createRenderSystemCapabilities();

		mCurrentCapabilities->addShaderProfile("hlsl");
		GpuProgramCoreManager::instance().addFactory(mHLSLFactory);

		mIAManager = bs_new<D3D11InputLayoutManager>();

		RenderAPICore::initializePrepare();
	}

	void D3D11RenderAPI::initializeFinalize(const SPtr<RenderWindowCore>& primaryWindow)
	{
		D3D11RenderUtility::startUp(mDevice);

		QueryManager::startUp<D3D11QueryManager>();

		RenderAPICore::initializeFinalize(primaryWindow);
	}

    void D3D11RenderAPI::destroyCore()
	{
		THROW_IF_NOT_CORE_THREAD;

		for (auto& boundUAV : mBoundUAVs)
		{
			if (boundUAV.second != nullptr)
				boundUAV.first->releaseView(boundUAV.second);

			boundUAV.second = nullptr;
			boundUAV.first = nullptr;
		}

		// Ensure that all GPU commands finish executing before shutting down the device. If we don't do this a crash
		// on shutdown may occurr as the driver is still executing the commands, and we unload this library.
		mDevice->getImmediateContext()->Flush();
		SPtr<EventQuery> query = EventQuery::create();
		query->begin();
		while(!query->isReady())
		{
			// Spin
		}
		query = nullptr;

		QueryManager::shutDown();
		D3D11RenderUtility::shutDown();

		if(mIAManager != nullptr)
		{
			bs_delete(mIAManager);
			mIAManager = nullptr;
		}

		if(mHLSLFactory != nullptr)
		{
			bs_delete(mHLSLFactory);
			mHLSLFactory = nullptr;
		}

		mActiveVertexDeclaration = nullptr;
		mActiveVertexShader = nullptr;
		mActiveRenderTarget = nullptr;
		mActiveDepthStencilState = nullptr;

		RenderStateCoreManager::shutDown();
		RenderWindowCoreManager::shutDown();
		RenderWindowManager::shutDown();
		HardwareBufferCoreManager::shutDown();
		HardwareBufferManager::shutDown();
		TextureCoreManager::shutDown();
		TextureManager::shutDown();

		SAFE_RELEASE(mDXGIFactory);

		if(mDevice != nullptr)
		{
			bs_delete(mDevice);
			mDevice = nullptr;
		}

		if(mDriverList != nullptr)
		{
			bs_delete(mDriverList);
			mDriverList = nullptr;
		}

		mActiveD3DDriver = nullptr;

		RenderAPICore::destroyCore();
	}

	void D3D11RenderAPI::setSamplerState(GpuProgramType gptype, UINT16 texUnit, const SPtr<SamplerStateCore>& samplerState,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](GpuProgramType gptype, UINT16 texUnit, const SPtr<SamplerStateCore>& samplerState)
		{
			THROW_IF_NOT_CORE_THREAD;

			ID3D11SamplerState* samplerArray[1];
			D3D11SamplerStateCore* d3d11SamplerState = static_cast<D3D11SamplerStateCore*>(const_cast<SamplerStateCore*>(samplerState.get()));
			samplerArray[0] = d3d11SamplerState->getInternal();

			switch (gptype)
			{
			case GPT_VERTEX_PROGRAM:
				mDevice->getImmediateContext()->VSSetSamplers(texUnit, 1, samplerArray);
				break;
			case GPT_FRAGMENT_PROGRAM:
				mDevice->getImmediateContext()->PSSetSamplers(texUnit, 1, samplerArray);
				break;
			case GPT_GEOMETRY_PROGRAM:
				mDevice->getImmediateContext()->GSSetSamplers(texUnit, 1, samplerArray);
				break;
			case GPT_DOMAIN_PROGRAM:
				mDevice->getImmediateContext()->DSSetSamplers(texUnit, 1, samplerArray);
				break;
			case GPT_HULL_PROGRAM:
				mDevice->getImmediateContext()->HSSetSamplers(texUnit, 1, samplerArray);
				break;
			case GPT_COMPUTE_PROGRAM:
				mDevice->getImmediateContext()->CSSetSamplers(texUnit, 1, samplerArray);
				break;
			default:
				BS_EXCEPT(InvalidParametersException, "Unsupported gpu program type: " + toString(gptype));
			}

		};

		if (commandBuffer == nullptr)
			executeRef(gptype, texUnit, samplerState);
		else
		{
			auto execute = [=]() { executeRef(gptype, texUnit, samplerState); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumSamplerBinds);
	}

	void D3D11RenderAPI::setGraphicsPipeline(const SPtr<GpuPipelineStateCore>& pipelineState,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](const SPtr<GpuPipelineStateCore>& pipelineState)
		{
			THROW_IF_NOT_CORE_THREAD;

			D3D11BlendStateCore* d3d11BlendState;
			D3D11RasterizerStateCore* d3d11RasterizerState;

			D3D11GpuFragmentProgramCore* d3d11FragmentProgram;
			D3D11GpuGeometryProgramCore* d3d11GeometryProgram;
			D3D11GpuDomainProgramCore* d3d11DomainProgram;
			D3D11GpuHullProgramCore* d3d11HullProgram;

			if(pipelineState != nullptr)
			{
				d3d11BlendState = static_cast<D3D11BlendStateCore*>(pipelineState->getBlendState().get());
				d3d11RasterizerState = static_cast<D3D11RasterizerStateCore*>(pipelineState->getRasterizerState().get());
				mActiveDepthStencilState = std::static_pointer_cast<D3D11DepthStencilStateCore>(pipelineState->getDepthStencilState());

				mActiveVertexShader = std::static_pointer_cast<D3D11GpuVertexProgramCore>(pipelineState->getVertexProgram());
				d3d11FragmentProgram = static_cast<D3D11GpuFragmentProgramCore*>(pipelineState->getFragmentProgram().get());
				d3d11GeometryProgram = static_cast<D3D11GpuGeometryProgramCore*>(pipelineState->getGeometryProgram().get());
				d3d11DomainProgram = static_cast<D3D11GpuDomainProgramCore*>(pipelineState->getDomainProgram().get());
				d3d11HullProgram = static_cast<D3D11GpuHullProgramCore*>(pipelineState->getHullProgram().get());

				if (d3d11BlendState == nullptr)
					d3d11BlendState = static_cast<D3D11BlendStateCore*>(BlendStateCore::getDefault().get());

				if (d3d11RasterizerState == nullptr)
					d3d11RasterizerState = static_cast<D3D11RasterizerStateCore*>(RasterizerStateCore::getDefault().get());

				if (mActiveDepthStencilState == nullptr)
					mActiveDepthStencilState = std::static_pointer_cast<D3D11DepthStencilStateCore>(DepthStencilStateCore::getDefault());
			}
			else
			{
				d3d11BlendState = static_cast<D3D11BlendStateCore*>(BlendStateCore::getDefault().get());
				d3d11RasterizerState = static_cast<D3D11RasterizerStateCore*>(RasterizerStateCore::getDefault().get());
				mActiveDepthStencilState = std::static_pointer_cast<D3D11DepthStencilStateCore>(DepthStencilStateCore::getDefault());

				mActiveVertexShader = nullptr;
				d3d11FragmentProgram = nullptr;
				d3d11GeometryProgram = nullptr;
				d3d11DomainProgram = nullptr;
				d3d11HullProgram = nullptr;
			}

			ID3D11DeviceContext* d3d11Context = mDevice->getImmediateContext();
			d3d11Context->OMSetBlendState(d3d11BlendState->getInternal(), nullptr, 0xFFFFFFFF);
			d3d11Context->RSSetState(d3d11RasterizerState->getInternal());
			d3d11Context->OMSetDepthStencilState(mActiveDepthStencilState->getInternal(), mStencilRef);

			if (mActiveVertexShader != nullptr)
			{
				D3D11GpuVertexProgramCore* vertexProgram = static_cast<D3D11GpuVertexProgramCore*>(mActiveVertexShader.get());
				d3d11Context->VSSetShader(vertexProgram->getVertexShader(), nullptr, 0);
			}
			else
				d3d11Context->VSSetShader(nullptr, nullptr, 0);

			if(d3d11FragmentProgram != nullptr)
				d3d11Context->PSSetShader(d3d11FragmentProgram->getPixelShader(), nullptr, 0);
			else
				d3d11Context->PSSetShader(nullptr, nullptr, 0);

			if (d3d11GeometryProgram != nullptr)
				d3d11Context->GSSetShader(d3d11GeometryProgram->getGeometryShader(), nullptr, 0);
			else
				d3d11Context->GSSetShader(nullptr, nullptr, 0);

			if (d3d11DomainProgram != nullptr)
				d3d11Context->DSSetShader(d3d11DomainProgram->getDomainShader(), nullptr, 0);
			else
				d3d11Context->DSSetShader(nullptr, nullptr, 0);

			if (d3d11HullProgram != nullptr)
				d3d11Context->HSSetShader(d3d11HullProgram->getHullShader(), nullptr, 0);
			else
				d3d11Context->HSSetShader(nullptr, nullptr, 0);
			
		};

		if (commandBuffer == nullptr)
			executeRef(pipelineState);
		else
		{
			auto execute = [=]() { executeRef(pipelineState); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumPipelineStateChanges);
	}

	void D3D11RenderAPI::setComputePipeline(const SPtr<GpuProgramCore>& computeProgram,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](const SPtr<GpuProgramCore>& computeProgram)
		{
			THROW_IF_NOT_CORE_THREAD;

			if (computeProgram != nullptr)
			{
				D3D11GpuComputeProgramCore *d3d11ComputeProgram = static_cast<D3D11GpuComputeProgramCore*>(computeProgram.get());
				mDevice->getImmediateContext()->CSSetShader(d3d11ComputeProgram->getComputeShader(), nullptr, 0);
			}
			else
				mDevice->getImmediateContext()->CSSetShader(nullptr, nullptr, 0);
		};

		if (commandBuffer == nullptr)
			executeRef(computeProgram);
		else
		{
			auto execute = [=]() { executeRef(computeProgram); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumPipelineStateChanges);
	}

	void D3D11RenderAPI::setTexture(GpuProgramType gptype, UINT16 unit, const SPtr<TextureCore>& texPtr, 
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](GpuProgramType gptype, UINT16 unit, const SPtr<TextureCore>& texPtr)
		{
			THROW_IF_NOT_CORE_THREAD;

			ID3D11ShaderResourceView* viewArray[1];
			if (texPtr != nullptr)
			{
				D3D11TextureCore* d3d11Texture = static_cast<D3D11TextureCore*>(texPtr.get());
				viewArray[0] = d3d11Texture->getSRV();
			}
			else
				viewArray[0] = nullptr;

			switch (gptype)
			{
			case GPT_VERTEX_PROGRAM:
				mDevice->getImmediateContext()->VSSetShaderResources(unit, 1, viewArray);
				break;
			case GPT_FRAGMENT_PROGRAM:
				mDevice->getImmediateContext()->PSSetShaderResources(unit, 1, viewArray);
				break;
			case GPT_GEOMETRY_PROGRAM:
				mDevice->getImmediateContext()->GSSetShaderResources(unit, 1, viewArray);
				break;
			case GPT_DOMAIN_PROGRAM:
				mDevice->getImmediateContext()->DSSetShaderResources(unit, 1, viewArray);
				break;
			case GPT_HULL_PROGRAM:
				mDevice->getImmediateContext()->HSSetShaderResources(unit, 1, viewArray);
				break;
			case GPT_COMPUTE_PROGRAM:
				mDevice->getImmediateContext()->CSSetShaderResources(unit, 1, viewArray);
				break;
			default:
				BS_EXCEPT(InvalidParametersException, "Unsupported gpu program type: " + toString(gptype));
			}
		};

		if (commandBuffer == nullptr)
			executeRef(gptype, unit, texPtr);
		else
		{
			auto execute = [=]() { executeRef(gptype, unit, texPtr); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumTextureBinds);
	}

	void D3D11RenderAPI::setLoadStoreTexture(GpuProgramType gptype, UINT16 unit, const SPtr<TextureCore>& texPtr,
		const TextureSurface& surface, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](GpuProgramType gptype, UINT16 unit, const SPtr<TextureCore>& texPtr,
			const TextureSurface& surface)
		{
			THROW_IF_NOT_CORE_THREAD;

			ID3D11UnorderedAccessView* viewArray[1];
			if (texPtr != nullptr)
			{
				D3D11TextureCore* d3d11Texture = static_cast<D3D11TextureCore*>(texPtr.get());
				SPtr<TextureView> texView = TextureCore::requestView(texPtr, surface.mipLevel, 1,
					surface.arraySlice, surface.numArraySlices, GVU_RANDOMWRITE);

				D3D11TextureView* d3d11texView = static_cast<D3D11TextureView*>(texView.get());
				viewArray[0] = d3d11texView->getUAV();

				if (mBoundUAVs[unit].second != nullptr)
					mBoundUAVs[unit].first->releaseView(mBoundUAVs[unit].second);

				mBoundUAVs[unit] = std::make_pair(texPtr, texView);
			}
			else
			{
				viewArray[0] = nullptr;

				if (mBoundUAVs[unit].second != nullptr)
					mBoundUAVs[unit].first->releaseView(mBoundUAVs[unit].second);

				mBoundUAVs[unit] = std::pair<SPtr<TextureCore>, SPtr<TextureView>>();
			}

			if (gptype == GPT_FRAGMENT_PROGRAM)
			{
				mDevice->getImmediateContext()->OMSetRenderTargetsAndUnorderedAccessViews(
					D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, unit, 1, viewArray, nullptr);
			}
			else if (gptype == GPT_COMPUTE_PROGRAM)
			{
				mDevice->getImmediateContext()->CSSetUnorderedAccessViews(unit, 1, viewArray, nullptr);
			}
			else
				LOGERR("Unsupported gpu program type: " + toString(gptype));
		};

		if (commandBuffer == nullptr)
			executeRef(gptype, unit, texPtr, surface);
		else
		{
			auto execute = [=]() { executeRef(gptype, unit, texPtr, surface); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumTextureBinds);
	}

	void D3D11RenderAPI::setBuffer(GpuProgramType gptype, UINT16 unit, const SPtr<GpuBufferCore>& buffer, bool loadStore, 
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](GpuProgramType gptype, UINT16 unit, const SPtr<GpuBufferCore>& buffer, bool loadStore)
		{
			THROW_IF_NOT_CORE_THREAD;

			if (!loadStore)
			{
				ID3D11ShaderResourceView* viewArray[1];
				if (buffer != nullptr)
				{
					D3D11GpuBufferCore* d3d11buffer = static_cast<D3D11GpuBufferCore*>(buffer.get());
					viewArray[0] = d3d11buffer->getSRV();
				}
				else
					viewArray[0] = nullptr;

				switch (gptype)
				{
				case GPT_VERTEX_PROGRAM:
					mDevice->getImmediateContext()->VSSetShaderResources(unit, 1, viewArray);
					break;
				case GPT_FRAGMENT_PROGRAM:
					mDevice->getImmediateContext()->PSSetShaderResources(unit, 1, viewArray);
					break;
				case GPT_GEOMETRY_PROGRAM:
					mDevice->getImmediateContext()->GSSetShaderResources(unit, 1, viewArray);
					break;
				case GPT_DOMAIN_PROGRAM:
					mDevice->getImmediateContext()->DSSetShaderResources(unit, 1, viewArray);
					break;
				case GPT_HULL_PROGRAM:
					mDevice->getImmediateContext()->HSSetShaderResources(unit, 1, viewArray);
					break;
				case GPT_COMPUTE_PROGRAM:
					mDevice->getImmediateContext()->CSSetShaderResources(unit, 1, viewArray);
					break;
				default:
					BS_EXCEPT(InvalidParametersException, "Unsupported gpu program type: " + toString(gptype));
				}
			}
			else
			{
				ID3D11UnorderedAccessView* viewArray[1];
				if (buffer != nullptr)
				{
					D3D11GpuBufferCore* d3d11buffer = static_cast<D3D11GpuBufferCore*>(buffer.get());
					viewArray[0] = d3d11buffer->getUAV();
				}
				else
					viewArray[0] = nullptr;

				if (gptype == GPT_FRAGMENT_PROGRAM)
				{
					mDevice->getImmediateContext()->OMSetRenderTargetsAndUnorderedAccessViews(
						D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, unit, 1, viewArray, nullptr);
				}
				else if (gptype == GPT_COMPUTE_PROGRAM)
				{
					mDevice->getImmediateContext()->CSSetUnorderedAccessViews(unit, 1, viewArray, nullptr);
				}
				else
					BS_EXCEPT(InvalidParametersException, "Unsupported gpu program type: " + toString(gptype));
			}
		};

		if (commandBuffer == nullptr)
			executeRef(gptype, unit, buffer, loadStore);
		else
		{
			auto execute = [=]() { executeRef(gptype, unit, buffer, loadStore); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumTextureBinds);
	}

	void D3D11RenderAPI::beginFrame(const SPtr<CommandBuffer>& commandBuffer)
	{
		// Not used
	}

	void D3D11RenderAPI::endFrame(const SPtr<CommandBuffer>& commandBuffer)
	{
		// Not used
	}

	void D3D11RenderAPI::setViewport(const Rect2& vp, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](const Rect2& vp)
		{
			THROW_IF_NOT_CORE_THREAD;

			mViewportNorm = vp;
			applyViewport();
		};

		if (commandBuffer == nullptr)
			executeRef(vp);
		else
		{
			auto execute = [=]() { executeRef(vp); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}
	}

	void D3D11RenderAPI::setVertexBuffers(UINT32 index, SPtr<VertexBufferCore>* buffers, UINT32 numBuffers, 
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](UINT32 index, SPtr<VertexBufferCore>* buffers, UINT32 numBuffers)
		{
			THROW_IF_NOT_CORE_THREAD;

			UINT32 maxBoundVertexBuffers = mCurrentCapabilities->getMaxBoundVertexBuffers();
			if (index < 0 || (index + numBuffers) >= maxBoundVertexBuffers)
			{
				BS_EXCEPT(InvalidParametersException, "Invalid vertex index: " + toString(index) +
					". Valid range is 0 .. " + toString(maxBoundVertexBuffers - 1));
			}

			ID3D11Buffer* dx11buffers[MAX_BOUND_VERTEX_BUFFERS];
			UINT32 strides[MAX_BOUND_VERTEX_BUFFERS];
			UINT32 offsets[MAX_BOUND_VERTEX_BUFFERS];

			for (UINT32 i = 0; i < numBuffers; i++)
			{
				SPtr<D3D11VertexBufferCore> vertexBuffer = std::static_pointer_cast<D3D11VertexBufferCore>(buffers[i]);
				const VertexBufferProperties& vbProps = vertexBuffer->getProperties();

				dx11buffers[i] = vertexBuffer->getD3DVertexBuffer();

				strides[i] = vbProps.getVertexSize();
				offsets[i] = 0;
			}

			mDevice->getImmediateContext()->IASetVertexBuffers(index, numBuffers, dx11buffers, strides, offsets);
		};

		if (commandBuffer == nullptr)
			executeRef(index, buffers, numBuffers);
		else
		{
			auto execute = [=]() { executeRef(index, buffers, numBuffers); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumVertexBufferBinds);
	}

	void D3D11RenderAPI::setIndexBuffer(const SPtr<IndexBufferCore>& buffer, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](const SPtr<IndexBufferCore>& buffer)
		{
			THROW_IF_NOT_CORE_THREAD;

			SPtr<D3D11IndexBufferCore> indexBuffer = std::static_pointer_cast<D3D11IndexBufferCore>(buffer);

			DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
			if (indexBuffer->getProperties().getType() == IT_16BIT)
				indexFormat = DXGI_FORMAT_R16_UINT;
			else if (indexBuffer->getProperties().getType() == IT_32BIT)
				indexFormat = DXGI_FORMAT_R32_UINT;
			else
				BS_EXCEPT(InternalErrorException, "Unsupported index format: " + toString(indexBuffer->getProperties().getType()));

			mDevice->getImmediateContext()->IASetIndexBuffer(indexBuffer->getD3DIndexBuffer(), indexFormat, 0);
		};

		if (commandBuffer == nullptr)
			executeRef(buffer);
		else
		{
			auto execute = [=]() { executeRef(buffer); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumIndexBufferBinds);
	}

	void D3D11RenderAPI::setVertexDeclaration(const SPtr<VertexDeclarationCore>& vertexDeclaration, 
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](const SPtr<VertexDeclarationCore>& vertexDeclaration)
		{
			THROW_IF_NOT_CORE_THREAD;

			mActiveVertexDeclaration = vertexDeclaration;
		};

		if (commandBuffer == nullptr)
			executeRef(vertexDeclaration);
		else
		{
			auto execute = [=]() { executeRef(vertexDeclaration); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}
	}

	void D3D11RenderAPI::setDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](DrawOperationType op)
		{
			THROW_IF_NOT_CORE_THREAD;

			mDevice->getImmediateContext()->IASetPrimitiveTopology(D3D11Mappings::getPrimitiveType(op));
			mActiveDrawOp = op;
		};

		if (commandBuffer == nullptr)
			executeRef(op);
		else
		{
			auto execute = [=]() { executeRef(op); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);

			cb->mActiveDrawOp = op;
		}
	}

	void D3D11RenderAPI::setParamBuffer(GpuProgramType gptype, UINT32 slot, const SPtr<GpuParamBlockBufferCore>& buffer, 
		const SPtr<GpuParamDesc>& paramDesc, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](GpuProgramType gptype, UINT32 slot, const SPtr<GpuParamBlockBufferCore>& buffer,
			const SPtr<GpuParamDesc>& paramDesc)
		{
			THROW_IF_NOT_CORE_THREAD;

			ID3D11Buffer* bufferArray[1];

			if (buffer != nullptr)
			{
				const D3D11GpuParamBlockBufferCore* d3d11paramBlockBuffer =
					static_cast<const D3D11GpuParamBlockBufferCore*>(buffer.get());
				bufferArray[0] = d3d11paramBlockBuffer->getD3D11Buffer();
			}
			else
				bufferArray[0] = nullptr;

			switch (gptype)
			{
			case GPT_VERTEX_PROGRAM:
				mDevice->getImmediateContext()->VSSetConstantBuffers(slot, 1, bufferArray);
				break;
			case GPT_FRAGMENT_PROGRAM:
				mDevice->getImmediateContext()->PSSetConstantBuffers(slot, 1, bufferArray);
				break;
			case GPT_GEOMETRY_PROGRAM:
				mDevice->getImmediateContext()->GSSetConstantBuffers(slot, 1, bufferArray);
				break;
			case GPT_HULL_PROGRAM:
				mDevice->getImmediateContext()->HSSetConstantBuffers(slot, 1, bufferArray);
				break;
			case GPT_DOMAIN_PROGRAM:
				mDevice->getImmediateContext()->DSSetConstantBuffers(slot, 1, bufferArray);
				break;
			case GPT_COMPUTE_PROGRAM:
				mDevice->getImmediateContext()->CSSetConstantBuffers(slot, 1, bufferArray);
				break;
			}

			if (mDevice->hasError())
				BS_EXCEPT(RenderingAPIException, "Failed to setParamBuffer: " + mDevice->getErrorDescription());
		};

		if (commandBuffer == nullptr)
			executeRef(gptype, slot,buffer, paramDesc);
		else
		{
			auto execute = [=]() { executeRef(gptype, slot, buffer, paramDesc); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumGpuParamBufferBinds);
	}

	void D3D11RenderAPI::draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount, 
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount)
		{
			THROW_IF_NOT_CORE_THREAD;

			applyInputLayout();

			if (instanceCount <= 1)
				mDevice->getImmediateContext()->Draw(vertexCount, vertexOffset);
			else
				mDevice->getImmediateContext()->DrawInstanced(vertexCount, instanceCount, vertexOffset, 0);

#if BS_DEBUG_MODE
			if (mDevice->hasError())
				LOGWRN(mDevice->getErrorDescription());
#endif
		};

		UINT32 primCount;
		if (commandBuffer == nullptr)
		{
			executeRef(vertexOffset, vertexCount, instanceCount);
			primCount = vertexCountToPrimCount(mActiveDrawOp, vertexCount);
		}
		else
		{
			auto execute = [=]() { executeRef(vertexOffset, vertexCount, instanceCount); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);

			primCount = vertexCountToPrimCount(cb->mActiveDrawOp, vertexCount);
		}

		BS_INC_RENDER_STAT(NumDrawCalls);
		BS_ADD_RENDER_STAT(NumVertices, vertexCount);
		BS_ADD_RENDER_STAT(NumPrimitives, primCount);
	}

	void D3D11RenderAPI::drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount, 
		UINT32 instanceCount, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount,
			UINT32 instanceCount)
		{
			THROW_IF_NOT_CORE_THREAD;

			applyInputLayout();

			if (instanceCount <= 1)
				mDevice->getImmediateContext()->DrawIndexed(indexCount, startIndex, vertexOffset);
			else
				mDevice->getImmediateContext()->DrawIndexedInstanced(indexCount, instanceCount, startIndex, vertexOffset, 0);

#if BS_DEBUG_MODE
			if (mDevice->hasError())
				LOGWRN(mDevice->getErrorDescription());
#endif
		};

		UINT32 primCount;
		if (commandBuffer == nullptr)
		{
			executeRef(startIndex, indexCount, vertexOffset, vertexCount, instanceCount);
			primCount = vertexCountToPrimCount(mActiveDrawOp, vertexCount);
		}
		else
		{
			auto execute = [=]() { executeRef(startIndex, indexCount, vertexOffset, vertexCount, instanceCount); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);

			primCount = vertexCountToPrimCount(cb->mActiveDrawOp, vertexCount);
		}

		BS_INC_RENDER_STAT(NumDrawCalls);
		BS_ADD_RENDER_STAT(NumVertices, vertexCount);
		BS_ADD_RENDER_STAT(NumPrimitives, primCount);
	}

	void D3D11RenderAPI::dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ, 
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ)
		{
			THROW_IF_NOT_CORE_THREAD;

			mDevice->getImmediateContext()->Dispatch(numGroupsX, numGroupsY, numGroupsZ);

#if BS_DEBUG_MODE
			if (mDevice->hasError())
				LOGWRN(mDevice->getErrorDescription());
#endif
		};

		if (commandBuffer == nullptr)
			executeRef(numGroupsX, numGroupsY, numGroupsZ);
		else
		{
			auto execute = [=]() { executeRef(numGroupsX, numGroupsY, numGroupsZ); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumComputeCalls);
	}

	void D3D11RenderAPI::setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom, 
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](UINT32 left, UINT32 top, UINT32 right, UINT32 bottom)
		{
			THROW_IF_NOT_CORE_THREAD;

			mScissorRect.left = static_cast<LONG>(left);
			mScissorRect.top = static_cast<LONG>(top);
			mScissorRect.bottom = static_cast<LONG>(bottom);
			mScissorRect.right = static_cast<LONG>(right);

			mDevice->getImmediateContext()->RSSetScissorRects(1, &mScissorRect);
		};

		if (commandBuffer == nullptr)
			executeRef(left, top, right, bottom);
		else
		{
			auto execute = [=]() { executeRef(left, top, right, bottom); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}
	}

	void D3D11RenderAPI::setStencilRef(UINT32 value, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](UINT32 value)
		{
			THROW_IF_NOT_CORE_THREAD;

			mStencilRef = value;

			if(mActiveDepthStencilState != nullptr)
				mDevice->getImmediateContext()->OMSetDepthStencilState(mActiveDepthStencilState->getInternal(), mStencilRef);
			else
				mDevice->getImmediateContext()->OMSetDepthStencilState(nullptr, mStencilRef);
		};

		if (commandBuffer == nullptr)
			executeRef(value);
		else
		{
			auto execute = [=]() { executeRef(value); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}
	}

	void D3D11RenderAPI::clearViewport(UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask, 
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask)
		{
			THROW_IF_NOT_CORE_THREAD;

			if (mActiveRenderTarget == nullptr)
				return;

			const RenderTargetProperties& rtProps = mActiveRenderTarget->getProperties();

			Rect2I clearArea((int)mViewport.TopLeftX, (int)mViewport.TopLeftY, (int)mViewport.Width, (int)mViewport.Height);

			bool clearEntireTarget = clearArea.width == 0 || clearArea.height == 0;
			clearEntireTarget |= (clearArea.x == 0 && clearArea.y == 0 && clearArea.width == rtProps.getWidth() && 
				clearArea.height == rtProps.getHeight());

			if (!clearEntireTarget)
			{
				// TODO - Ignoring targetMask here
				D3D11RenderUtility::instance().drawClearQuad(buffers, color, depth, stencil);
				BS_INC_RENDER_STAT(NumClears);
			}
			else
				clearRenderTarget(buffers, color, depth, stencil, targetMask);
		};

		if (commandBuffer == nullptr)
			executeRef(buffers, color, depth, stencil, targetMask);
		else
		{
			auto execute = [=]() { executeRef(buffers, color, depth, stencil, targetMask); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}
	}

	void D3D11RenderAPI::clearRenderTarget(UINT32 buffers, const Color& color, float depth, UINT16 stencil, 
		UINT8 targetMask, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask)
		{
			THROW_IF_NOT_CORE_THREAD;

			if (mActiveRenderTarget == nullptr)
				return;

			// Clear render surfaces
			if (buffers & FBT_COLOR)
			{
				UINT32 maxRenderTargets = mCurrentCapabilities->getNumMultiRenderTargets();

				ID3D11RenderTargetView** views = bs_newN<ID3D11RenderTargetView*>(maxRenderTargets);
				memset(views, 0, sizeof(ID3D11RenderTargetView*) * maxRenderTargets);

				mActiveRenderTarget->getCustomAttribute("RTV", views);
				if (!views[0])
				{
					bs_deleteN(views, maxRenderTargets);
					return;
				}

				float clearColor[4];
				clearColor[0] = color.r;
				clearColor[1] = color.g;
				clearColor[2] = color.b;
				clearColor[3] = color.a;

				for (UINT32 i = 0; i < maxRenderTargets; i++)
				{
					if (views[i] != nullptr && ((1 << i) & targetMask) != 0)
						mDevice->getImmediateContext()->ClearRenderTargetView(views[i], clearColor);
				}

				bs_deleteN(views, maxRenderTargets);
			}

			// Clear depth stencil
			if ((buffers & FBT_DEPTH) != 0 || (buffers & FBT_STENCIL) != 0)
			{
				ID3D11DepthStencilView* depthStencilView = nullptr;
				mActiveRenderTarget->getCustomAttribute("DSV", &depthStencilView);

				D3D11_CLEAR_FLAG clearFlag;

				if ((buffers & FBT_DEPTH) != 0 && (buffers & FBT_STENCIL) != 0)
					clearFlag = (D3D11_CLEAR_FLAG)(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);
				else if ((buffers & FBT_STENCIL) != 0)
					clearFlag = D3D11_CLEAR_STENCIL;
				else
					clearFlag = D3D11_CLEAR_DEPTH;

				if (depthStencilView != nullptr)
					mDevice->getImmediateContext()->ClearDepthStencilView(depthStencilView, clearFlag, depth, (UINT8)stencil);
			}
		};

		if (commandBuffer == nullptr)
			executeRef(buffers, color, depth, stencil, targetMask);
		else
		{
			auto execute = [=]() { executeRef(buffers, color, depth, stencil, targetMask); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumClears);
	}

	void D3D11RenderAPI::setRenderTarget(const SPtr<RenderTargetCore>& target, bool readOnlyDepthStencil, 
		const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](const SPtr<RenderTargetCore>& target, bool readOnlyDepthStencil)
		{
			THROW_IF_NOT_CORE_THREAD;

			mActiveRenderTarget = target;

			UINT32 maxRenderTargets = mCurrentCapabilities->getNumMultiRenderTargets();
			ID3D11RenderTargetView** views = bs_newN<ID3D11RenderTargetView*>(maxRenderTargets);
			memset(views, 0, sizeof(ID3D11RenderTargetView*) * maxRenderTargets);

			ID3D11DepthStencilView* depthStencilView = nullptr;

			if (target != nullptr)
			{
				target->getCustomAttribute("RTV", views);

				if (readOnlyDepthStencil)
					target->getCustomAttribute("RODSV", &depthStencilView);
				else
					target->getCustomAttribute("DSV", &depthStencilView);
			}

			// Bind render targets
			mDevice->getImmediateContext()->OMSetRenderTargets(maxRenderTargets, views, depthStencilView);
			if (mDevice->hasError())
				BS_EXCEPT(RenderingAPIException, "Failed to setRenderTarget : " + mDevice->getErrorDescription());

			bs_deleteN(views, maxRenderTargets);
			applyViewport();
		};

		if (commandBuffer == nullptr)
			executeRef(target, readOnlyDepthStencil);
		else
		{
			auto execute = [=]() { executeRef(target, readOnlyDepthStencil); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumRenderTargetChanges);
	}

	void D3D11RenderAPI::swapBuffers(const SPtr<RenderTargetCore>& target, const SPtr<CommandBuffer>& commandBuffer)
	{
		auto executeRef = [&](const SPtr<RenderTargetCore>& target)
		{
			THROW_IF_NOT_CORE_THREAD;
			target->swapBuffers();
		};

		if (commandBuffer == nullptr)
			executeRef(target);
		else
		{
			auto execute = [=]() { executeRef(target); };

			SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
			cb->queueCommand(execute);
		}

		BS_INC_RENDER_STAT(NumPresents);
	}

	void D3D11RenderAPI::addCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary)
	{
		SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
		SPtr<D3D11CommandBuffer> secondaryCb = std::static_pointer_cast<D3D11CommandBuffer>(secondary);

		cb->appendSecondary(secondaryCb);
	}

	void D3D11RenderAPI::executeCommands(const SPtr<CommandBuffer>& commandBuffer)
	{
		SPtr<D3D11CommandBuffer> cb = std::static_pointer_cast<D3D11CommandBuffer>(commandBuffer);
		cb->executeCommands();
		cb->clear();
	}

	void D3D11RenderAPI::applyViewport()
	{
		if (mActiveRenderTarget == nullptr)
			return;

		const RenderTargetProperties& rtProps = mActiveRenderTarget->getProperties();

		// Set viewport dimensions
		mViewport.TopLeftX = (FLOAT)(rtProps.getWidth() * mViewportNorm.x);
		mViewport.TopLeftY = (FLOAT)(rtProps.getHeight() * mViewportNorm.y);
		mViewport.Width = (FLOAT)(rtProps.getWidth() * mViewportNorm.width);
		mViewport.Height = (FLOAT)(rtProps.getHeight() * mViewportNorm.height);

		if (rtProps.requiresTextureFlipping())
		{
			// Convert "top-left" to "bottom-left"
			mViewport.TopLeftY = rtProps.getHeight() - mViewport.Height - mViewport.TopLeftY;
		}

		mViewport.MinDepth = 0.0f;
		mViewport.MaxDepth = 1.0f;

		mDevice->getImmediateContext()->RSSetViewports(1, &mViewport);
	}

	RenderAPICapabilities* D3D11RenderAPI::createRenderSystemCapabilities() const
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderAPICapabilities* rsc = bs_new<RenderAPICapabilities>();

		rsc->setDriverVersion(mDriverVersion);
		rsc->setDeviceName(mActiveD3DDriver->getDriverDescription());
		rsc->setRenderAPIName(getName());

		rsc->setStencilBufferBitDepth(8);

		rsc->setCapability(RSC_ANISOTROPY);
		rsc->setCapability(RSC_AUTOMIPMAP);

		// Cube map
		rsc->setCapability(RSC_CUBEMAPPING);

		// We always support compression, D3DX will decompress if device does not support
		rsc->setCapability(RSC_TEXTURE_COMPRESSION);
		rsc->setCapability(RSC_TEXTURE_COMPRESSION_DXT);
		rsc->setCapability(RSC_TWO_SIDED_STENCIL);
		rsc->setCapability(RSC_STENCIL_WRAP);
		rsc->setCapability(RSC_HWOCCLUSION);
		rsc->setCapability(RSC_HWOCCLUSION_ASYNCHRONOUS);

		if(mFeatureLevel >= D3D_FEATURE_LEVEL_10_1)
			rsc->setMaxBoundVertexBuffers(32);
		else
			rsc->setMaxBoundVertexBuffers(16);

		if(mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
		{
			rsc->addShaderProfile("ps_4_0");
			rsc->addShaderProfile("vs_4_0");
			rsc->addShaderProfile("gs_4_0");

			rsc->addGpuProgramProfile(GPP_FS_4_0, "ps_4_0");
			rsc->addGpuProgramProfile(GPP_VS_4_0, "vs_4_0");
			rsc->addGpuProgramProfile(GPP_GS_4_0, "gs_4_0");

			rsc->setNumTextureUnits(GPT_FRAGMENT_PROGRAM, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
			rsc->setNumTextureUnits(GPT_VERTEX_PROGRAM, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
			rsc->setNumTextureUnits(GPT_GEOMETRY_PROGRAM, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);

			rsc->setNumCombinedTextureUnits(rsc->getNumTextureUnits(GPT_FRAGMENT_PROGRAM)
				+ rsc->getNumTextureUnits(GPT_VERTEX_PROGRAM) + rsc->getNumTextureUnits(GPT_VERTEX_PROGRAM));

			rsc->setNumGpuParamBlockBuffers(GPT_FRAGMENT_PROGRAM, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
			rsc->setNumGpuParamBlockBuffers(GPT_VERTEX_PROGRAM, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
			rsc->setNumGpuParamBlockBuffers(GPT_GEOMETRY_PROGRAM, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);

			rsc->setNumCombinedGpuParamBlockBuffers(rsc->getNumGpuParamBlockBuffers(GPT_FRAGMENT_PROGRAM)
				+ rsc->getNumGpuParamBlockBuffers(GPT_VERTEX_PROGRAM) + rsc->getNumGpuParamBlockBuffers(GPT_VERTEX_PROGRAM));
		}

		if(mFeatureLevel >= D3D_FEATURE_LEVEL_10_1)
		{
			rsc->addShaderProfile("ps_4_1");
			rsc->addShaderProfile("vs_4_1");
			rsc->addShaderProfile("gs_4_1");

			rsc->addGpuProgramProfile(GPP_FS_4_1, "ps_4_1");
			rsc->addGpuProgramProfile(GPP_VS_4_1, "vs_4_1");
			rsc->addGpuProgramProfile(GPP_GS_4_1, "gs_4_1");
		}

		if(mFeatureLevel >= D3D_FEATURE_LEVEL_11_0)
		{
			rsc->addShaderProfile("ps_5_0");
			rsc->addShaderProfile("vs_5_0");
			rsc->addShaderProfile("gs_5_0");
			rsc->addShaderProfile("cs_5_0");
			rsc->addShaderProfile("hs_5_0");
			rsc->addShaderProfile("ds_5_0");

			rsc->addGpuProgramProfile(GPP_FS_5_0, "ps_5_0");
			rsc->addGpuProgramProfile(GPP_VS_5_0, "vs_5_0");
			rsc->addGpuProgramProfile(GPP_GS_5_0, "gs_5_0");
			rsc->addGpuProgramProfile(GPP_CS_5_0, "cs_5_0");
			rsc->addGpuProgramProfile(GPP_HS_5_0, "hs_5_0");
			rsc->addGpuProgramProfile(GPP_DS_5_0, "ds_5_0");

			rsc->setNumTextureUnits(GPT_HULL_PROGRAM, D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
			rsc->setNumTextureUnits(GPT_DOMAIN_PROGRAM, D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);
			rsc->setNumTextureUnits(GPT_COMPUTE_PROGRAM, D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT);

			rsc->setNumCombinedTextureUnits(rsc->getNumTextureUnits(GPT_FRAGMENT_PROGRAM)
				+ rsc->getNumTextureUnits(GPT_VERTEX_PROGRAM) + rsc->getNumTextureUnits(GPT_VERTEX_PROGRAM)
				+ rsc->getNumTextureUnits(GPT_HULL_PROGRAM) + rsc->getNumTextureUnits(GPT_DOMAIN_PROGRAM)
				+ rsc->getNumTextureUnits(GPT_COMPUTE_PROGRAM));

			rsc->setNumGpuParamBlockBuffers(GPT_HULL_PROGRAM, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
			rsc->setNumGpuParamBlockBuffers(GPT_DOMAIN_PROGRAM, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
			rsc->setNumGpuParamBlockBuffers(GPT_COMPUTE_PROGRAM, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);

			rsc->setNumCombinedGpuParamBlockBuffers(rsc->getNumGpuParamBlockBuffers(GPT_FRAGMENT_PROGRAM)
				+ rsc->getNumGpuParamBlockBuffers(GPT_VERTEX_PROGRAM) + rsc->getNumGpuParamBlockBuffers(GPT_VERTEX_PROGRAM)
				+ rsc->getNumGpuParamBlockBuffers(GPT_HULL_PROGRAM) + rsc->getNumGpuParamBlockBuffers(GPT_DOMAIN_PROGRAM)
				+ rsc->getNumGpuParamBlockBuffers(GPT_COMPUTE_PROGRAM));

			rsc->setNumLoadStoreTextureUnits(GPT_FRAGMENT_PROGRAM, D3D11_PS_CS_UAV_REGISTER_COUNT);
			rsc->setNumLoadStoreTextureUnits(GPT_COMPUTE_PROGRAM, D3D11_PS_CS_UAV_REGISTER_COUNT);

			rsc->setNumCombinedLoadStoreTextureUnits(rsc->getNumLoadStoreTextureUnits(GPT_FRAGMENT_PROGRAM)
				+ rsc->getNumLoadStoreTextureUnits(GPT_COMPUTE_PROGRAM));

			rsc->setCapability(RSC_SHADER_SUBROUTINE);
		}

		rsc->setCapability(RSC_USER_CLIP_PLANES);
		rsc->setCapability(RSC_VERTEX_FORMAT_UBYTE4);

		// Adapter details
		const DXGI_ADAPTER_DESC& adapterID = mActiveD3DDriver->getAdapterIdentifier();

		// Determine vendor
		switch(adapterID.VendorId)
		{
		case 0x10DE:
			rsc->setVendor(GPU_NVIDIA);
			break;
		case 0x1002:
			rsc->setVendor(GPU_AMD);
			break;
		case 0x163C:
		case 0x8086:
			rsc->setVendor(GPU_INTEL);
			break;
		default:
			rsc->setVendor(GPU_UNKNOWN);
			break;
		};

		rsc->setCapability(RSC_INFINITE_FAR_PLANE);

		rsc->setCapability(RSC_TEXTURE_3D);
		rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
		rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);
		rsc->setCapability(RSC_TEXTURE_FLOAT);

		rsc->setNumMultiRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
		rsc->setCapability(RSC_MRT_DIFFERENT_BIT_DEPTHS);

		rsc->setCapability(RSC_POINT_SPRITES);
		rsc->setCapability(RSC_POINT_EXTENDED_PARAMETERS);
		rsc->setMaxPointSize(256);

		rsc->setCapability(RSC_VERTEX_TEXTURE_FETCH);

		rsc->setCapability(RSC_MIPMAP_LOD_BIAS);

		rsc->setCapability(RSC_PERSTAGECONSTANT);

		return rsc;
	}

	void D3D11RenderAPI::determineMultisampleSettings(UINT32 multisampleCount, DXGI_FORMAT format, DXGI_SAMPLE_DESC* outputSampleDesc)
	{
		bool tryCSAA = false; // Note: Disabled for now, but leaving the code for later so it might be useful
		enum CSAAMode { CSAA_Normal, CSAA_Quality };
		CSAAMode csaaMode = CSAA_Normal;

		bool foundValid = false;
		size_t origNumSamples = multisampleCount;
		while (!foundValid)
		{
			// Deal with special cases
			if (tryCSAA)
			{
				switch(multisampleCount)
				{
				case 8:
					if (csaaMode == CSAA_Quality)
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
					if (csaaMode == CSAA_Quality)
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
			hr = mDevice->getD3D11Device()->CheckMultisampleQualityLevels(format, outputSampleDesc->Count, &outQuality);

			if (SUCCEEDED(hr) && (!tryCSAA || outQuality > outputSampleDesc->Quality))
			{
				foundValid = true;
			}
			else
			{
				// Downgrade
				if (tryCSAA && multisampleCount == 8)
				{
					// For CSAA, we'll try downgrading with quality mode at all samples.
					// then try without quality, then drop CSAA
					if (csaaMode == CSAA_Quality)
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
					multisampleCount = static_cast<UINT32>(origNumSamples);
				}
				else
				{
					// Drop samples
					multisampleCount--;

					if (multisampleCount == 1)
					{
						// Ran out of options, no multisampling
						multisampleCount = 0;
						foundValid = true;
					}
				}
			}
		} 
	}

	void D3D11RenderAPI::convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest)
	{
		dest = matrix;

		// Convert depth range from [-1,+1] to [0,1]
		dest[2][0] = (dest[2][0] + dest[3][0]) / 2;
		dest[2][1] = (dest[2][1] + dest[3][1]) / 2;
		dest[2][2] = (dest[2][2] + dest[3][2]) / 2;
		dest[2][3] = (dest[2][3] + dest[3][3]) / 2;
	}

	const RenderAPIInfo& D3D11RenderAPI::getAPIInfo() const
	{
		static RenderAPIInfo info(0.0f, 0.0f, 0.0f, 1.0f, VET_COLOR_ABGR, false, true, false, false);

		return info;
	}

	GpuParamBlockDesc D3D11RenderAPI::generateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params)
	{
		GpuParamBlockDesc block;
		block.blockSize = 0;
		block.isShareable = true;
		block.name = name;
		block.slot = 0;

		for (auto& param : params)
		{
			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[param.type];
			UINT32 size = typeInfo.size / 4;

			if (param.arraySize > 1)
			{
				// Arrays perform no packing and their elements are always padded and aligned to four component vectors
				UINT32 alignOffset = size % typeInfo.baseTypeSize;
				if (alignOffset != 0)
				{
					UINT32 padding = (typeInfo.baseTypeSize - alignOffset);
					size += padding;
				}

				alignOffset = block.blockSize % typeInfo.baseTypeSize;
				if (alignOffset != 0)
				{
					UINT32 padding = (typeInfo.baseTypeSize - alignOffset);
					block.blockSize += padding;
				}

				param.elementSize = size;
				param.arrayElementStride = size;
				param.cpuMemOffset = block.blockSize;
				param.gpuMemOffset = 0;

				block.blockSize += size * param.arraySize;
			}
			else
			{
				// Pack everything as tightly as possible as long as the data doesn't cross 16 byte boundary
				UINT32 alignOffset = block.blockSize % 4;
				if (alignOffset != 0 && size > (4 - alignOffset))
				{
					UINT32 padding = (4 - alignOffset);
					block.blockSize += padding;
				}

				param.elementSize = size;
				param.arrayElementStride = size;
				param.cpuMemOffset = block.blockSize;
				param.gpuMemOffset = 0;

				block.blockSize += size;
			}

			param.paramBlockSlot = 0;
		}

		// Constant buffer size must always be a multiple of 16
		if (block.blockSize % 4 != 0)
			block.blockSize += (4 - (block.blockSize % 4));

		return block;
	}

	/************************************************************************/
	/* 								PRIVATE		                     		*/
	/************************************************************************/

	void D3D11RenderAPI::applyInputLayout()
	{
		if(mActiveVertexDeclaration == nullptr)
		{
			LOGWRN("Cannot apply input layout without a vertex declaration. Set vertex declaration before calling this method.");
			return;
		}

		if(mActiveVertexShader == nullptr)
		{
			LOGWRN("Cannot apply input layout without a vertex shader. Set vertex shader before calling this method.");
			return;
		}

		ID3D11InputLayout* ia = mIAManager->retrieveInputLayout(mActiveVertexShader->getInputDeclaration(), mActiveVertexDeclaration, *mActiveVertexShader);

		mDevice->getImmediateContext()->IASetInputLayout(ia);
	}
}