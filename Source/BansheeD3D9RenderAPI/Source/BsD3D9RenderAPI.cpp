//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D9RenderAPI.h"
#include "BsD3D9Prerequisites.h"
#include "BsD3D9DriverList.h"
#include "BsD3D9Driver.h"
#include "BsD3D9RenderWindow.h"
#include "BsD3D9TextureManager.h"
#include "BsD3D9Texture.h"
#include "BsMath.h"
#include "BsD3D9HardwareBufferManager.h"
#include "BsD3D9IndexBuffer.h"
#include "BsD3D9VertexBuffer.h"
#include "BsD3D9VertexDeclaration.h"
#include "BsD3D9GpuProgram.h"
#include "BsD3D9HLSLProgramFactory.h"
#include "BsD3D9OcclusionQuery.h"
#include "BsD3D9DeviceManager.h"
#include "BsD3D9ResourceManager.h"
#include "BsD3D9RenderWindowManager.h"
#include "BsGpuProgramManager.h"
#include "BsRenderStateManager.h"
#include "BsAsyncOp.h"
#include "BsBlendState.h"
#include "BsRasterizerState.h"
#include "BsDepthStencilState.h"
#include "BsGpuParams.h"
#include "BsGpuParamDesc.h"
#include "BsGpuParamBlockBuffer.h"
#include "BsCoreThread.h"
#include "BsD3D9QueryManager.h"
#include "BsDebug.h"
#include "BsRenderStats.h"

namespace BansheeEngine 
{
	D3D9RenderAPI* D3D9RenderAPI::msD3D9RenderSystem = nullptr;

	D3D9RenderAPI::D3D9RenderAPI(HINSTANCE hInstance)
		: mTexStageDesc(nullptr), mNumTexStages(0), mCurrentDrawOperation(DOT_TRIANGLE_LIST), 
		mViewportLeft(0), mViewportTop(0), mViewportWidth(0), mViewportHeight(0),
		mIsFrameInProgress(false), mRestoreFrameOnReset(false), mhInstance(hInstance),
		mpD3D(nullptr), mDriverList(nullptr), mActiveD3DDriver(nullptr), mHLSLProgramFactory(nullptr),
		mDeviceManager(nullptr), mResourceManager(nullptr), mViewportNorm(0.0f, 0.0f, 1.0f, 1.0f)
	{
		msD3D9RenderSystem = this;

		mScissorRect.left = 0;
		mScissorRect.right = 1280;
		mScissorRect.top = 0;
		mScissorRect.bottom = 720;
	}

	D3D9RenderAPI::~D3D9RenderAPI()
	{

	}

	const StringID& D3D9RenderAPI::getName() const
	{
		static StringID strName("D3D9RenderAPI");
		return strName;
	}

	const String& D3D9RenderAPI::getShadingLanguageName() const
	{
		static String strName("hlsl9");
		return strName;
	}

	void D3D9RenderAPI::initializePrepare()
	{
		THROW_IF_NOT_CORE_THREAD;

		// Create the resource manager.
		mResourceManager = bs_new<D3D9ResourceManager>();

		// Create our Direct3D object
		if((mpD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
			BS_EXCEPT(InternalErrorException, "Failed to create Direct3D9 object");

		// Init using current settings
		mActiveD3DDriver = getDirect3DDrivers()->item(0); // TODO - We always use the first driver

		if(mActiveD3DDriver == nullptr)
			BS_EXCEPT(InvalidParametersException, "Problems finding requested Direct3D driver!" );

		// get driver version
		mDriverVersion.major = HIWORD(mActiveD3DDriver->getAdapterIdentifier().DriverVersion.HighPart);
		mDriverVersion.minor = LOWORD(mActiveD3DDriver->getAdapterIdentifier().DriverVersion.HighPart);
		mDriverVersion.release = HIWORD(mActiveD3DDriver->getAdapterIdentifier().DriverVersion.LowPart);
		mDriverVersion.build = LOWORD(mActiveD3DDriver->getAdapterIdentifier().DriverVersion.LowPart);

		mVideoModeInfo = getDirect3DDrivers()->getVideoModeInfo();

		// Create the device manager.
		mDeviceManager = bs_new<D3D9DeviceManager>();

		// Also create hardware buffer manager		
		HardwareBufferManager::startUp();
		HardwareBufferCoreManager::startUp<D3D9HardwareBufferCoreManager>();

		// Create & register HLSL factory		
		mHLSLProgramFactory = bs_new<D3D9HLSLProgramFactory>();

		// Create render window manager
		RenderWindowManager::startUp<D3D9RenderWindowManager>(this);
		RenderWindowCoreManager::startUp<D3D9RenderWindowCoreManager>(this);

		// Create render state manager
		RenderStateCoreManager::startUp();

		RenderAPICore::initializePrepare();
	}

	void D3D9RenderAPI::initializeFinalize(const SPtr<RenderWindowCore>& primaryWindow)
	{
		D3D9RenderWindowCore* d3d9window = static_cast<D3D9RenderWindowCore*>(primaryWindow.get());
		updateRenderSystemCapabilities(d3d9window);

		// Create the texture manager for use by others		
		TextureManager::startUp<D3D9TextureManager>();
		TextureCoreManager::startUp<D3D9TextureCoreManager>();

		QueryManager::startUp<D3D9QueryManager>();

		RenderAPICore::initializeFinalize(primaryWindow);
	}

	void D3D9RenderAPI::destroyCore()
	{
		if(mTexStageDesc != nullptr)
		{
			bs_deleteN(mTexStageDesc, mNumTexStages);
			mTexStageDesc = nullptr;
		}

		RenderAPICore::destroyCore();

		if(mDeviceManager != nullptr)
		{
			bs_delete(mDeviceManager);
			mDeviceManager = nullptr;
		}

		if(mDriverList != nullptr)
		{
			bs_delete(mDriverList);
			mDriverList = nullptr;
		}
		mActiveD3DDriver = NULL;	

		QueryManager::shutDown();
		TextureCoreManager::shutDown();
		TextureManager::shutDown();
		HardwareBufferCoreManager::shutDown();
		HardwareBufferManager::shutDown();
		RenderWindowCoreManager::shutDown();
		RenderWindowManager::shutDown();
		RenderStateCoreManager::shutDown();

		// Deleting the HLSL program factory
		if (mHLSLProgramFactory)
		{
			GpuProgramCoreManager::instance().removeFactory(mHLSLProgramFactory);
			bs_delete(mHLSLProgramFactory);
			mHLSLProgramFactory = 0;
		}

		SAFE_RELEASE(mpD3D);

		if(mResourceManager != nullptr)
		{
			bs_delete(mResourceManager);
			mResourceManager = nullptr;
		}

		msD3D9RenderSystem = NULL;
	}

	void D3D9RenderAPI::registerWindow(RenderWindowCore& renderWindow)
	{		
		THROW_IF_NOT_CORE_THREAD;

		D3D9RenderWindowCore* d3d9renderWindow = static_cast<D3D9RenderWindowCore*>(&renderWindow);

		String msg;

		mResourceManager->lockDeviceAccess();
		mDeviceManager->linkRenderWindow(d3d9renderWindow);

		mResourceManager->unlockDeviceAccess();
	}	

	void D3D9RenderAPI::bindGpuProgram(const SPtr<GpuProgramCore>& prg)
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;
		switch (prg->getProperties().getType())
		{
		case GPT_VERTEX_PROGRAM:
			hr = getActiveD3D9Device()->SetVertexShader(
				static_cast<D3D9GpuVertexProgramCore*>(prg.get())->getVertexShader());
			if (FAILED(hr))
			{
				BS_EXCEPT(RenderingAPIException, "Error calling SetVertexShader");
			}
			break;
		case GPT_FRAGMENT_PROGRAM:
			hr = getActiveD3D9Device()->SetPixelShader(
				static_cast<D3D9GpuFragmentProgramCore*>(prg.get())->getPixelShader());
			if (FAILED(hr))
			{
				BS_EXCEPT(RenderingAPIException, "Error calling SetPixelShader");
			}
			break;
		};

		// Make sure texcoord index is equal to stage value, As SDK Doc suggests:
		// "When rendering using vertex shaders, each stage's texture coordinate index must be set to its default value."
		// This solves such an errors when working with the Debug runtime -
		// "Direct3D9: (ERROR) :Stage 1 - Texture coordinate index in the stage must be equal to the stage index when programmable vertex pipeline is used".
		for (unsigned int nStage=0; nStage < 8; ++nStage)
			setTextureStageState(nStage, D3DTSS_TEXCOORDINDEX, nStage);

		BS_INC_RENDER_STAT(NumGpuProgramBinds);

		RenderAPICore::bindGpuProgram(prg);
	}

	void D3D9RenderAPI::unbindGpuProgram(GpuProgramType gptype)
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;
		switch(gptype)
		{
		case GPT_VERTEX_PROGRAM:
			hr = getActiveD3D9Device()->SetVertexShader(NULL);
			if (FAILED(hr))
			{
				BS_EXCEPT(RenderingAPIException, "Error resetting SetVertexShader to NULL");
			}
			break;
		case GPT_FRAGMENT_PROGRAM:
			hr = getActiveD3D9Device()->SetPixelShader(NULL);
			if (FAILED(hr))
			{
				BS_EXCEPT(RenderingAPIException, "Error resetting SetPixelShader to NULL");
			}
			break;
		};

		BS_INC_RENDER_STAT(NumGpuProgramBinds);

		RenderAPICore::unbindGpuProgram(gptype);
	}

	void D3D9RenderAPI::setConstantBuffers(GpuProgramType gptype, const SPtr<GpuParamsCore>& bindableParams)
	{
		THROW_IF_NOT_CORE_THREAD;

		bindableParams->updateHardwareBuffers();
		const GpuParamDesc& paramDesc = bindableParams->getParamDesc();

		// Read all the buffer data so we can assign it. Not the most efficient way of accessing data
		// but it is required in order to have standardized buffer interface.
		UnorderedMap<UINT32, UINT8*> bufferData;

		for(auto& curParam : paramDesc.params)
		{
			UINT32 paramBlockSlot = curParam.second.paramBlockSlot;
			auto iterFind = bufferData.find(paramBlockSlot);

			if(iterFind == bufferData.end())
			{
				SPtr<GpuParamBlockBufferCore> paramBlock = bindableParams->getParamBlockBuffer(paramBlockSlot);

				UINT8* data = (UINT8*)bs_alloc(paramBlock->getSize());
				paramBlock->readFromGPU(data);

				bufferData[paramBlockSlot] = data;
			}
		}

		HRESULT hr;

		switch(gptype)
		{
		case GPT_VERTEX_PROGRAM:
			{
				for(auto iter = paramDesc.params.begin(); iter != paramDesc.params.end(); ++iter)
				{
					const GpuParamDataDesc& paramDesc = iter->second;

					const UINT8* ptrData = bufferData[paramDesc.paramBlockSlot] + paramDesc.cpuMemOffset * sizeof(UINT32);

					switch(paramDesc.type)
					{
					case GPDT_FLOAT1:
					case GPDT_FLOAT2:
					case GPDT_FLOAT3:
					case GPDT_FLOAT4:
					case GPDT_MATRIX_2X2:
					case GPDT_MATRIX_2X3:
					case GPDT_MATRIX_2X4:
					case GPDT_MATRIX_3X2:
					case GPDT_MATRIX_3X3:
					case GPDT_MATRIX_3X4:
					case GPDT_MATRIX_4X2:
					case GPDT_MATRIX_4X3:
					case GPDT_MATRIX_4X4:
						{
							UINT32 slotCount = (paramDesc.elementSize / 4) * paramDesc.arraySize;
							assert (paramDesc.elementSize % 4 == 0 && "Should not have any elements less than 4 wide for D3D9");

							if (FAILED(hr = getActiveD3D9Device()->SetVertexShaderConstantF(paramDesc.gpuMemOffset, (const float*)ptrData, slotCount))) 
								BS_EXCEPT(RenderingAPIException, "Unable to upload vertex shader float parameters.");
							break;
						}
					case GPDT_INT1:
					case GPDT_INT2:
					case GPDT_INT3:
					case GPDT_INT4:
						{
							UINT32 slotCount = (paramDesc.elementSize / 4) * paramDesc.arraySize;
							assert (paramDesc.elementSize % 4 == 0 && "Should not have any elements less than 4 wide for D3D9");

							if (FAILED(hr = getActiveD3D9Device()->SetVertexShaderConstantI(paramDesc.gpuMemOffset, (const INT32*)ptrData, slotCount))) 
								BS_EXCEPT(RenderingAPIException, "Unable to upload vertex shader int parameters.");
							break;
						}
					case GPDT_BOOL:
						if (FAILED(hr = getActiveD3D9Device()->SetVertexShaderConstantB(paramDesc.gpuMemOffset, (const BOOL*)ptrData, paramDesc.arraySize))) 
							BS_EXCEPT(RenderingAPIException, "Unable to upload vertex shader bool parameters.");
						break;
					}
				}
			}
			break;
		case GPT_FRAGMENT_PROGRAM:
			{
				for(auto iter = paramDesc.params.begin(); iter != paramDesc.params.end(); ++iter)
				{
					const GpuParamDataDesc& paramDesc = iter->second;

					const UINT8* ptrData = bufferData[paramDesc.paramBlockSlot] + paramDesc.cpuMemOffset * sizeof(UINT32);

					switch(paramDesc.type)
					{
					case GPDT_FLOAT1:
					case GPDT_FLOAT2:
					case GPDT_FLOAT3:
					case GPDT_FLOAT4:
					case GPDT_MATRIX_2X2:
					case GPDT_MATRIX_2X3:
					case GPDT_MATRIX_2X4:
					case GPDT_MATRIX_3X2:
					case GPDT_MATRIX_3X3:
					case GPDT_MATRIX_3X4:
					case GPDT_MATRIX_4X2:
					case GPDT_MATRIX_4X3:
					case GPDT_MATRIX_4X4:
						{
							UINT32 slotCount = (paramDesc.elementSize / 4) * paramDesc.arraySize;
							assert (paramDesc.elementSize % 4 == 0 && "Should not have any elements less than 4 wide for D3D9");

							if (FAILED(hr = getActiveD3D9Device()->SetPixelShaderConstantF(paramDesc.gpuMemOffset, (const float*)ptrData, slotCount))) 
								BS_EXCEPT(RenderingAPIException, "Unable to upload pixel shader float parameters.");
							break;
						}
					case GPDT_INT1:
					case GPDT_INT2:
					case GPDT_INT3:
					case GPDT_INT4:
						{
							UINT32 slotCount = (paramDesc.elementSize / 4) * paramDesc.arraySize;
							assert (paramDesc.elementSize % 4 == 0 && "Should not have any elements less than 4 wide for D3D9");

							if (FAILED(hr = getActiveD3D9Device()->SetPixelShaderConstantI(paramDesc.gpuMemOffset, (const INT32*)ptrData, slotCount))) 
								BS_EXCEPT(RenderingAPIException, "Unable to upload pixel shader int parameters.");
							break;
						}
					case GPDT_BOOL:
						if (FAILED(hr = getActiveD3D9Device()->SetPixelShaderConstantB(paramDesc.gpuMemOffset, (const BOOL*)ptrData, paramDesc.arraySize))) 
							BS_EXCEPT(RenderingAPIException, "Unable to upload pixel shader bool parameters.");
						break;
					}
				}
			}
			break;
		};

		for(auto& curBufferData : bufferData)
		{
			bs_free(curBufferData.second);
		}

		BS_INC_RENDER_STAT(NumGpuParamBufferBinds);
	}

	void D3D9RenderAPI::setTexture(GpuProgramType gptype, UINT16 unit, bool enabled, const SPtr<TextureCore>& tex)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (tex != nullptr && !tex->isBindableAsShaderResource())
			BS_EXCEPT(InvalidParametersException, "Texture you have specified cannot be bound to a shader.");

		if(gptype != GPT_FRAGMENT_PROGRAM && gptype != GPT_VERTEX_PROGRAM)
		{
			LOGWRN("D3D9 cannot assign textures to this gpu program type: " + toString(gptype));
			return;
		}

		if(gptype == GPT_VERTEX_PROGRAM)
		{
			unit = D3DVERTEXTEXTURESAMPLER0 + unit; // Vertex stage uses special samplers
		}

		HRESULT hr;
		SPtr<D3D9TextureCore> dt = std::static_pointer_cast<D3D9TextureCore>(tex);
		if (enabled && (dt != nullptr))
		{
			IDirect3DBaseTexture9 *pTex = dt->getTexture_internal();
			if (mTexStageDesc[unit].pTex != pTex)
			{
				hr = getActiveD3D9Device()->SetTexture(static_cast<DWORD>(unit), pTex);
				if( hr != S_OK )
				{
					String str = "Unable to set texture in D3D9";
					BS_EXCEPT(RenderingAPIException, str);
				}

				// set stage desc.
				mTexStageDesc[unit].pTex = pTex;
				mTexStageDesc[unit].texType = D3D9Mappings::get(dt->getProperties().getTextureType());

				// Set gamma now too
				if (dt->isHardwareGammaReadToBeUsed())
				{
					setSamplerState(static_cast<DWORD>(unit), D3DSAMP_SRGBTEXTURE, TRUE);
				}
				else
				{
					setSamplerState(static_cast<DWORD>(unit), D3DSAMP_SRGBTEXTURE, FALSE);
				}

				BS_INC_RENDER_STAT(NumTextureBinds);
				BS_INC_RENDER_STAT(NumSamplerBinds);
			}
		}
		else
		{
			if (mTexStageDesc[unit].pTex != 0)
			{
				hr = getActiveD3D9Device()->SetTexture(static_cast<DWORD>(unit), 0);
				if( hr != S_OK )
				{
					String str = "Unable to disable texture '" + toString(unit) + "' in D3D9";
					BS_EXCEPT(RenderingAPIException, str);
				}

				BS_INC_RENDER_STAT(NumTextureBinds);
			}

			hr = setTextureStageState(static_cast<DWORD>(unit), D3DTSS_COLOROP, D3DTOP_DISABLE);
			if( hr != S_OK )
			{
				String str = "Unable to disable texture '" + toString(unit) + "' in D3D9";
				BS_EXCEPT(RenderingAPIException, str);
			}

			// set stage desc. to defaults
			mTexStageDesc[unit].pTex = 0;
			mTexStageDesc[unit].coordIndex = 0;
			mTexStageDesc[unit].texType = D3D9Mappings::D3D_TEX_TYPE_NORMAL;
		}
	}

	void D3D9RenderAPI::setLoadStoreTexture(GpuProgramType gptype, UINT16 unit, bool enabled, const SPtr<TextureCore>& texPtr,
		const TextureSurface& surface)
	{
		THROW_IF_NOT_CORE_THREAD;

		LOGWRN("Texture random load/store not supported on DX9.");
	}

	void D3D9RenderAPI::setBuffer(GpuProgramType gptype, UINT16 unit, const SPtr<GpuBufferCore>& buffer, bool loadStore)
	{
		THROW_IF_NOT_CORE_THREAD;

		LOGWRN("Generic GPU buffers not supported on DX9.");
	}

	void D3D9RenderAPI::setSamplerState(GpuProgramType gptype, UINT16 unit, const SPtr<SamplerStateCore>& state)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(gptype != GPT_FRAGMENT_PROGRAM && gptype != GPT_VERTEX_PROGRAM)
		{
			LOGWRN("D3D9 doesn't support this gpu program type: " + toString(gptype));
			return;
		}

		if(gptype == GPT_VERTEX_PROGRAM)
		{
			unit = D3DVERTEXTEXTURESAMPLER0 + unit; // Vertex stage uses special samplers
		}

		const SamplerProperties& sampProps = state->getProperties();

		// Set texture layer filtering
		setTextureFiltering(unit, FT_MIN, sampProps.getTextureFiltering(FT_MIN));
		setTextureFiltering(unit, FT_MAG, sampProps.getTextureFiltering(FT_MAG));
		setTextureFiltering(unit, FT_MIP, sampProps.getTextureFiltering(FT_MIP));

		// Set texture layer filtering
		if (sampProps.getTextureAnisotropy() > 0)
			setTextureAnisotropy(unit, sampProps.getTextureAnisotropy());

		// Set mipmap biasing
		setTextureMipmapBias(unit, sampProps.getTextureMipmapBias());

		// Texture addressing mode
		const UVWAddressingMode& uvw = sampProps.getTextureAddressingMode();
		setTextureAddressingMode(unit, uvw);

		// Set border color
		setTextureBorderColor(unit, sampProps.getBorderColor());

		BS_INC_RENDER_STAT(NumSamplerBinds);
	}

	void D3D9RenderAPI::setBlendState(const SPtr<BlendStateCore>& blendState)
	{
		THROW_IF_NOT_CORE_THREAD;

		const BlendProperties& stateProps = blendState->getProperties();

		// Alpha to coverage
		setAlphaToCoverage(stateProps.getAlphaToCoverageEnabled());

		// Blend states
		// DirectX 9 doesn't allow us to specify blend state per render target, so we just use the first one.
		if (stateProps.getBlendEnabled(0))
		{
			setSceneBlending(stateProps.getSrcBlend(0), stateProps.getDstBlend(0), stateProps.getAlphaSrcBlend(0), stateProps.getAlphaDstBlend(0)
				, stateProps.getBlendOperation(0), stateProps.getAlphaBlendOperation(0));
		}
		else
		{
			setSceneBlending(BF_ONE, BF_ZERO, BO_ADD);
		}

		// Color write mask
		UINT8 writeMask = stateProps.getRenderTargetWriteMask(0);
		setColorBufferWriteEnabled((writeMask & 0x1) != 0, (writeMask & 0x2) != 0, (writeMask & 0x4) != 0, (writeMask & 0x8) != 0);

		BS_INC_RENDER_STAT(NumBlendStateChanges);
	}

	void D3D9RenderAPI::setRasterizerState(const SPtr<RasterizerStateCore>& rasterizerState)
	{
		THROW_IF_NOT_CORE_THREAD;

		const RasterizerProperties& stateProps = rasterizerState->getProperties();

		setDepthBias(stateProps.getDepthBias(), stateProps.getSlopeScaledDepthBias());

		setCullingMode(stateProps.getCullMode());

		setPolygonMode(stateProps.getPolygonMode());

		setScissorTestEnable(stateProps.getScissorEnable());

		setMultisampleAntialiasEnable(stateProps.getMultisampleEnable());

		setAntialiasedLineEnable(stateProps.getAntialiasedLineEnable());

		BS_INC_RENDER_STAT(NumRasterizerStateChanges);
	}

	void D3D9RenderAPI::setDepthStencilState(const SPtr<DepthStencilStateCore>& depthStencilState, UINT32 stencilRefValue)
	{
		THROW_IF_NOT_CORE_THREAD;

		const DepthStencilProperties& stateProps = depthStencilState->getProperties();

		// Set stencil buffer options
		setStencilCheckEnabled(stateProps.getStencilEnable());

		setStencilBufferOperations(stateProps.getStencilFrontFailOp(), stateProps.getStencilFrontZFailOp(), stateProps.getStencilFrontPassOp(), true);
		setStencilBufferFunc(stateProps.getStencilFrontCompFunc(), true);

		setStencilBufferOperations(stateProps.getStencilBackFailOp(), stateProps.getStencilBackZFailOp(), stateProps.getStencilBackPassOp(), false);
		setStencilBufferFunc(stateProps.getStencilBackCompFunc(), false);

		setStencilBufferReadMask(stateProps.getStencilReadMask());
		setStencilBufferWriteMask(stateProps.getStencilWriteMask());

		// Set depth buffer options
		setDepthBufferCheckEnabled(stateProps.getDepthReadEnable());
		setDepthBufferWriteEnabled(stateProps.getDepthWriteEnable());
		setDepthBufferFunction(stateProps.getDepthComparisonFunc());

		// Set stencil ref value
		setStencilRefValue(stencilRefValue);

		BS_INC_RENDER_STAT(NumDepthStencilStateChanges);
	}

	void D3D9RenderAPI::setTextureMipmapBias(UINT16 unit, float bias)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mCurrentCapabilities->hasCapability(RSC_MIPMAP_LOD_BIAS))
		{
			// ugh - have to pass float data through DWORD with no conversion
			HRESULT hr = setSamplerState(static_cast<DWORD>(unit), D3DSAMP_MIPMAPLODBIAS, 
				*(DWORD*)&bias);
			if(FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Unable to set texture mipmap bias");
		}
	}

	void D3D9RenderAPI::setTextureAddressingMode( UINT16 stage, 
		const UVWAddressingMode& uvw )
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;
		if( FAILED( hr = setSamplerState( static_cast<DWORD>(stage), D3DSAMP_ADDRESSU, D3D9Mappings::get(uvw.u, mDeviceManager->getActiveDevice()->getD3D9DeviceCaps()) ) ) )
			BS_EXCEPT(RenderingAPIException, "Failed to set texture addressing mode for U" );
		if( FAILED( hr = setSamplerState( static_cast<DWORD>(stage), D3DSAMP_ADDRESSV, D3D9Mappings::get(uvw.v, mDeviceManager->getActiveDevice()->getD3D9DeviceCaps()) ) ) )
			BS_EXCEPT(RenderingAPIException, "Failed to set texture addressing mode for V");
		if( FAILED( hr = setSamplerState( static_cast<DWORD>(stage), D3DSAMP_ADDRESSW, D3D9Mappings::get(uvw.w, mDeviceManager->getActiveDevice()->getD3D9DeviceCaps()) ) ) )
			BS_EXCEPT(RenderingAPIException, "Failed to set texture addressing mode for W");
	}

	void D3D9RenderAPI::setTextureBorderColor(UINT16 stage, const Color& colour)
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;
		if( FAILED( hr = setSamplerState( static_cast<DWORD>(stage), D3DSAMP_BORDERCOLOR, colour.getAsBGRA()) ) )
			BS_EXCEPT(RenderingAPIException, "Failed to set texture border colour");
	}

	void D3D9RenderAPI::setSceneBlending( BlendFactor sourceFactor, BlendFactor destFactor, BlendOperation op )
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;
		if( sourceFactor == BF_ONE && destFactor == BF_ZERO)
		{
			if (FAILED(hr = setRenderState(D3DRS_ALPHABLENDENABLE, FALSE)))
				BS_EXCEPT(RenderingAPIException, "Failed to set alpha blending option");
		}
		else
		{
			if (FAILED(hr = setRenderState(D3DRS_ALPHABLENDENABLE, TRUE)))
				BS_EXCEPT(RenderingAPIException, "Failed to set alpha blending option");
			if (FAILED(hr = setRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE)))
				BS_EXCEPT(RenderingAPIException, "Failed to set separate alpha blending option");
			if( FAILED( hr = setRenderState( D3DRS_SRCBLEND, D3D9Mappings::get(sourceFactor) ) ) )
				BS_EXCEPT(RenderingAPIException, "Failed to set source blend");
			if( FAILED( hr = setRenderState( D3DRS_DESTBLEND, D3D9Mappings::get(destFactor) ) ) )
				BS_EXCEPT(RenderingAPIException, "Failed to set destination blend");
		}

		if (FAILED(hr = setRenderState(D3DRS_BLENDOP, D3D9Mappings::get(op))))
			BS_EXCEPT(RenderingAPIException, "Failed to set scene blending operation option");
		if (FAILED(hr = setRenderState(D3DRS_BLENDOPALPHA, D3D9Mappings::get(op))))
			BS_EXCEPT(RenderingAPIException, "Failed to set scene blending operation option");
	}

	void D3D9RenderAPI::setSceneBlending( BlendFactor sourceFactor, BlendFactor destFactor, BlendFactor sourceFactorAlpha, 
		BlendFactor destFactorAlpha, BlendOperation op, BlendOperation alphaOp )
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;
		if( sourceFactor == BF_ONE && destFactor == BF_ZERO && 
			sourceFactorAlpha == BF_ONE && destFactorAlpha == BF_ZERO)
		{
			if (FAILED(hr = setRenderState(D3DRS_ALPHABLENDENABLE, FALSE)))
				BS_EXCEPT(RenderingAPIException, "Failed to set alpha blending option");
		}
		else
		{
			if (FAILED(hr = setRenderState(D3DRS_ALPHABLENDENABLE, TRUE)))
				BS_EXCEPT(RenderingAPIException, "Failed to set alpha blending option");
			if (FAILED(hr = setRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE)))
				BS_EXCEPT(RenderingAPIException, "Failed to set separate alpha blending option");
			if( FAILED( hr = setRenderState( D3DRS_SRCBLEND, D3D9Mappings::get(sourceFactor) ) ) )
				BS_EXCEPT(RenderingAPIException, "Failed to set source blend");
			if( FAILED( hr = setRenderState( D3DRS_DESTBLEND, D3D9Mappings::get(destFactor) ) ) )
				BS_EXCEPT(RenderingAPIException, "Failed to set destination blend");
			if( FAILED( hr = setRenderState( D3DRS_SRCBLENDALPHA, D3D9Mappings::get(sourceFactorAlpha) ) ) )
				BS_EXCEPT(RenderingAPIException, "Failed to set alpha source blend");
			if( FAILED( hr = setRenderState( D3DRS_DESTBLENDALPHA, D3D9Mappings::get(destFactorAlpha) ) ) )
				BS_EXCEPT(RenderingAPIException, "Failed to set alpha destination blend");
		}

		if (FAILED(hr = setRenderState(D3DRS_BLENDOP, D3D9Mappings::get(op))))
			BS_EXCEPT(RenderingAPIException, "Failed to set scene blending operation option");
		if (FAILED(hr = setRenderState(D3DRS_BLENDOPALPHA, D3D9Mappings::get(alphaOp))))
			BS_EXCEPT(RenderingAPIException, "Failed to set alpha scene blending operation option");
	}

	void D3D9RenderAPI::setAlphaTest(CompareFunction func, unsigned char value)
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;

		if (func != CMPF_ALWAYS_PASS)
		{
			if( FAILED( hr = setRenderState( D3DRS_ALPHATESTENABLE,  TRUE ) ) )
				BS_EXCEPT(RenderingAPIException, "Failed to enable alpha testing");
		}
		else
		{
			if( FAILED( hr = setRenderState( D3DRS_ALPHATESTENABLE,  FALSE ) ) )
				BS_EXCEPT(RenderingAPIException, "Failed to disable alpha testing");
		}
		// Set always just be sure
		if( FAILED( hr = setRenderState( D3DRS_ALPHAFUNC, D3D9Mappings::get(func) ) ) )
			BS_EXCEPT(RenderingAPIException, "Failed to set alpha reject function");
		if( FAILED( hr = setRenderState( D3DRS_ALPHAREF, value ) ) )
			BS_EXCEPT(RenderingAPIException, "Failed to set render state D3DRS_ALPHAREF");
	}

	void D3D9RenderAPI::setAlphaToCoverage(bool enable)
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;
		static bool lasta2c = false;

		// Alpha to coverage
		if (getCapabilities()->hasCapability(RSC_ALPHA_TO_COVERAGE))
		{
			// Vendor-specific hacks on renderstate, gotta love 'em
			if (getCapabilities()->getVendor() == GPU_NVIDIA)
			{
				if (enable)
				{
					if( FAILED( hr = setRenderState( D3DRS_ADAPTIVETESS_Y,  (D3DFORMAT)MAKEFOURCC('A', 'T', 'O', 'C') ) ) )
						BS_EXCEPT(RenderingAPIException, "Failed to set alpha to coverage option");
				}
				else
				{
					if( FAILED( hr = setRenderState( D3DRS_ADAPTIVETESS_Y,  D3DFMT_UNKNOWN ) ) )
						BS_EXCEPT(RenderingAPIException, "Failed to set alpha to coverage option");
				}

			}
			else if ((getCapabilities()->getVendor() == GPU_AMD))
			{
				if (enable)
				{
					if( FAILED( hr = setRenderState( D3DRS_POINTSIZE,  MAKEFOURCC('A','2','M','1') ) ) )
						BS_EXCEPT(RenderingAPIException, "Failed to set alpha to coverage option");
				}
				else
				{
					// discovered this through trial and error, seems to work
					if( FAILED( hr = setRenderState( D3DRS_POINTSIZE,  MAKEFOURCC('A','2','M','0') ) ) )
						BS_EXCEPT(RenderingAPIException, "Failed to set alpha to coverage option");
				}
			}

			lasta2c = enable;
		}
	}

	void D3D9RenderAPI::setCullingMode(CullingMode mode)
	{
		THROW_IF_NOT_CORE_THREAD;

		mCullingMode = mode;
		HRESULT hr;

		if( FAILED (hr = setRenderState(D3DRS_CULLMODE, 
			D3D9Mappings::get(mode, false))) )
			BS_EXCEPT(RenderingAPIException, "Failed to set culling mode");
	}

	void D3D9RenderAPI::setDepthBufferCheckEnabled(bool enabled)
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;

		if( enabled )
			hr = setRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		else
			hr = setRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

		if(FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Error setting depth buffer test state");
	}

	void D3D9RenderAPI::setDepthBufferWriteEnabled(bool enabled)
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;

		if( FAILED( hr = setRenderState( D3DRS_ZWRITEENABLE, enabled ) ) )
			BS_EXCEPT(RenderingAPIException, "Error setting depth buffer write state");
	}

	void D3D9RenderAPI::setDepthBufferFunction(CompareFunction func)
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;
		if( FAILED( hr = setRenderState( D3DRS_ZFUNC, D3D9Mappings::get(func) ) ) )
			BS_EXCEPT(RenderingAPIException, "Error setting depth buffer test function");
	}

	void D3D9RenderAPI::setDepthBias(float constantBias, float slopeScaleBias)
	{
		THROW_IF_NOT_CORE_THREAD;

		if ((mDeviceManager->getActiveDevice()->getD3D9DeviceCaps().RasterCaps & D3DPRASTERCAPS_DEPTHBIAS) != 0)
		{
			// Negate bias since D3D is backward
			constantBias = -constantBias;
			HRESULT hr = setRenderState(D3DRS_DEPTHBIAS, *((DWORD*)&constantBias));
			if (FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Error setting constant depth bias");
		}

		if ((mDeviceManager->getActiveDevice()->getD3D9DeviceCaps().RasterCaps & D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS) != 0)
		{
			// Negate bias since D3D is backward
			slopeScaleBias = -slopeScaleBias;
			HRESULT hr = setRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *((DWORD*)&slopeScaleBias));
			if (FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Error setting slope scale depth bias");
		}


	}

	void D3D9RenderAPI::setColorBufferWriteEnabled(bool red, bool green, bool blue, bool alpha)
	{
		THROW_IF_NOT_CORE_THREAD;

		DWORD val = 0;
		if (red) 
			val |= D3DCOLORWRITEENABLE_RED;
		if (green)
			val |= D3DCOLORWRITEENABLE_GREEN;
		if (blue)
			val |= D3DCOLORWRITEENABLE_BLUE;
		if (alpha)
			val |= D3DCOLORWRITEENABLE_ALPHA;

		HRESULT hr = setRenderState(D3DRS_COLORWRITEENABLE, val); 
		if (FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Error setting colour write enable flags");
	}

	void D3D9RenderAPI::setPolygonMode(PolygonMode level)
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr = setRenderState(D3DRS_FILLMODE, D3D9Mappings::get(level));
		if (FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Error setting polygon mode.");
	}

	void D3D9RenderAPI::setStencilCheckEnabled(bool enabled)
	{
		THROW_IF_NOT_CORE_THREAD;

		// Allow stencilling
		HRESULT hr = setRenderState(D3DRS_STENCILENABLE, enabled);
		if (FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Error enabling / disabling stencilling.");

		if (mCurrentCapabilities->hasCapability(RSC_TWO_SIDED_STENCIL))
		{
			hr = setRenderState(D3DRS_TWOSIDEDSTENCILMODE, TRUE);

			if (FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Error setting 2-sided stencil mode.");
		}
		else
		{
			hr = setRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE);

			if (FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Error setting 1-sided stencil mode.");
		}
	}

	void D3D9RenderAPI::setStencilBufferOperations(StencilOperation stencilFailOp, StencilOperation depthFailOp, StencilOperation passOp, bool ccw)
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;

		// 2-sided operation
		if (ccw)
		{
			// fail op
			hr = setRenderState(D3DRS_CCW_STENCILFAIL, D3D9Mappings::get(stencilFailOp));
			if (FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Error setting stencil fail operation (ccw).");

			// depth fail op
			hr = setRenderState(D3DRS_CCW_STENCILZFAIL, D3D9Mappings::get(depthFailOp));
			if (FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Error setting stencil depth fail operation (ccw).");

			// pass op
			hr = setRenderState(D3DRS_CCW_STENCILPASS, D3D9Mappings::get(passOp));
			if (FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Error setting stencil pass operation (ccw).");
		}
		else
		{
			// fail op
			hr = setRenderState(D3DRS_STENCILFAIL, D3D9Mappings::get(stencilFailOp, true));
			if (FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Error setting stencil fail operation (cw).");

			// depth fail op
			hr = setRenderState(D3DRS_STENCILZFAIL, D3D9Mappings::get(depthFailOp, true));
			if (FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Error setting stencil depth fail operation (cw).");

			// pass op
			hr = setRenderState(D3DRS_STENCILPASS, D3D9Mappings::get(passOp, true));
			if (FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Error setting stencil pass operation (cw).");
		}
	}

	void D3D9RenderAPI::setStencilBufferFunc(CompareFunction func, bool ccw)
	{
		HRESULT hr;
		
		if(ccw)
			hr = setRenderState(D3DRS_CCW_STENCILFUNC, D3D9Mappings::get(func));
		else
			hr = setRenderState(D3DRS_STENCILFUNC, D3D9Mappings::get(func));

		if (FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Error setting stencil buffer test function.");
	}

	void D3D9RenderAPI::setStencilBufferReadMask(UINT32 mask)
	{
		HRESULT hr = setRenderState(D3DRS_STENCILMASK, mask);

		if (FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Error setting stencil buffer mask.");
	}

	void D3D9RenderAPI::setStencilBufferWriteMask(UINT32 mask)
	{
		HRESULT hr = setRenderState(D3DRS_STENCILWRITEMASK, mask);

		if (FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Error setting stencil buffer write mask.");
	}

	void D3D9RenderAPI::setStencilRefValue(UINT32 refValue)
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr = setRenderState(D3DRS_STENCILREF, refValue);
		if (FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Error setting stencil buffer reference value.");
	}

	void D3D9RenderAPI::setTextureFiltering(UINT16 unit, FilterType ftype, FilterOptions filter)
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;
		D3D9Mappings::D3DTexType texType = mTexStageDesc[unit].texType;
		hr = setSamplerState( static_cast<DWORD>(unit), D3D9Mappings::get(ftype), 
			D3D9Mappings::get(ftype, filter, mDeviceManager->getActiveDevice()->getD3D9DeviceCaps(), texType));

		if (FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Failed to set texture filter ");
	}


	void D3D9RenderAPI::setTextureAnisotropy(UINT16 unit, unsigned int maxAnisotropy)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (static_cast<DWORD>(maxAnisotropy) > mDeviceManager->getActiveDevice()->getD3D9DeviceCaps().MaxAnisotropy)
			maxAnisotropy = mDeviceManager->getActiveDevice()->getD3D9DeviceCaps().MaxAnisotropy;

		if (getCurrentAnisotropy(unit) != maxAnisotropy)
			setSamplerState( static_cast<DWORD>(unit), D3DSAMP_MAXANISOTROPY, maxAnisotropy );
	}

	void D3D9RenderAPI::setRenderTarget(const SPtr<RenderTargetCore>& target, bool readOnlyDepthStencil)
	{
		THROW_IF_NOT_CORE_THREAD;

		mActiveRenderTarget = target;
		const RenderTargetProperties& rtProps = target->getProperties();

		HRESULT hr;

		// Possibly change device if the target is a window
		if (rtProps.isWindow())
		{
			D3D9RenderWindowCore* window = static_cast<D3D9RenderWindowCore*>(target.get());
			mDeviceManager->setActiveRenderTargetDevice(window->_getDevice());
			window->_validateDevice();
		}

		// Retrieve render surfaces
		UINT32 maxRenderTargets = mCurrentCapabilities->getNumMultiRenderTargets();
		IDirect3DSurface9** pBack = bs_newN<IDirect3DSurface9*>(maxRenderTargets);
		memset(pBack, 0, sizeof(IDirect3DSurface9*) * maxRenderTargets);
		target->getCustomAttribute("DDBACKBUFFER", pBack);
		if (!pBack[0])
		{
			bs_deleteN(pBack, maxRenderTargets);
			return;
		}

		IDirect3DSurface9* pDepth = NULL;

		if (!pDepth)
			target->getCustomAttribute("D3DZBUFFER", &pDepth);
		
		// Bind render targets
		for(UINT32 x = 0; x < maxRenderTargets; ++x)
		{
			hr = getActiveD3D9Device()->SetRenderTarget(x, pBack[x]);
			if (FAILED(hr))
			{
				String msg = DXGetErrorDescription(hr);
				BS_EXCEPT(RenderingAPIException, "Failed to setRenderTarget : " + msg);
			}
		}

		bs_deleteN(pBack, maxRenderTargets);

		hr = getActiveD3D9Device()->SetDepthStencilSurface(pDepth);
		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription(hr);
			BS_EXCEPT(RenderingAPIException, "Failed to setDepthStencil : " + msg);
		}

		// Set sRGB write mode
		setRenderState(D3DRS_SRGBWRITEENABLE, rtProps.isHwGammaEnabled());
		applyViewport();

		BS_INC_RENDER_STAT(NumRenderTargetChanges);
	}

	void D3D9RenderAPI::setViewport(const Rect2& vp)
	{
		THROW_IF_NOT_CORE_THREAD;

		mViewportNorm = vp;
		applyViewport();
	}

	void D3D9RenderAPI::beginFrame()
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;
		if(FAILED(hr = getActiveD3D9Device()->BeginScene()))
		{
			String msg = DXGetErrorDescription(hr);
			BS_EXCEPT(RenderingAPIException, "Error beginning frame :" + msg);
		}

 		mDeviceManager->getActiveDevice()->clearDeviceStreams();
		mIsFrameInProgress = true;
	}

	void D3D9RenderAPI::endFrame()
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;
		if(FAILED(hr = getActiveD3D9Device()->EndScene()))
			BS_EXCEPT(RenderingAPIException, "Error ending frame");

		mIsFrameInProgress = false;
	}

	void D3D9RenderAPI::setVertexDeclaration(const SPtr<VertexDeclarationCore>& decl)
	{
		THROW_IF_NOT_CORE_THREAD;

		SPtr<D3D9VertexDeclarationCore> d3ddecl = std::static_pointer_cast<D3D9VertexDeclarationCore>(decl);

		HRESULT hr;
		if (FAILED(hr = getActiveD3D9Device()->SetVertexDeclaration(d3ddecl->getD3DVertexDeclaration())))
		{
			BS_EXCEPT(RenderingAPIException, "Unable to set D3D9 vertex declaration");
		}
	}

	void D3D9RenderAPI::setVertexBuffers(UINT32 index, SPtr<VertexBufferCore>* buffers, UINT32 numBuffers)
	{
		THROW_IF_NOT_CORE_THREAD;

		UINT32 maxBoundVertexBuffers = mCurrentCapabilities->getMaxBoundVertexBuffers();
		if(index < 0 || (index + numBuffers) > maxBoundVertexBuffers)
			BS_EXCEPT(InvalidParametersException, "Invalid vertex index: " + toString(index) + ". Valid range is 0 .. " + toString(maxBoundVertexBuffers - 1));

		HRESULT hr;

		for(UINT32 i = 0; i < numBuffers; i++)
		{
			if(buffers[i] != nullptr)
			{
				SPtr<D3D9VertexBufferCore> d3d9buf = std::static_pointer_cast<D3D9VertexBufferCore>(buffers[i]);
				const VertexBufferProperties& vbProps = d3d9buf->getProperties();

				hr = getActiveD3D9Device()->SetStreamSource(
					static_cast<UINT>(index + i),
					d3d9buf->getD3D9VertexBuffer(),
					0,
					static_cast<UINT>(vbProps.getVertexSize()) // stride
					);
			}
			else
			{
				hr = getActiveD3D9Device()->SetStreamSource(static_cast<UINT>(index + i), nullptr, 0, 0);
			}

			if (FAILED(hr))
				BS_EXCEPT(RenderingAPIException, "Unable to set D3D9 stream source for buffer binding");

			BS_INC_RENDER_STAT(NumVertexBufferBinds);
		}
	}

	void D3D9RenderAPI::setIndexBuffer(const SPtr<IndexBufferCore>& buffer)
	{
		THROW_IF_NOT_CORE_THREAD;

		SPtr<D3D9IndexBufferCore> d3dIdxBuf = std::static_pointer_cast<D3D9IndexBufferCore>(buffer);

		HRESULT hr = getActiveD3D9Device()->SetIndices( d3dIdxBuf->getD3DIndexBuffer() );
		if (FAILED(hr))
			BS_EXCEPT(RenderingAPIException, "Failed to set index buffer");

		BS_INC_RENDER_STAT(NumIndexBufferBinds);
	}

	void D3D9RenderAPI::setDrawOperation(DrawOperationType op)
	{
		THROW_IF_NOT_CORE_THREAD;

		mCurrentDrawOperation = op;
	}

	void D3D9RenderAPI::draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount)
	{
		// Note: Not supporting instanced drawing

		UINT32 primCount = vertexCountToPrimCount(mCurrentDrawOperation, vertexCount);

		HRESULT hr = getActiveD3D9Device()->DrawPrimitive(getD3D9PrimitiveType(), static_cast<UINT>(vertexOffset), static_cast<UINT>(primCount)); 

		if(FAILED(hr))
		{
			String msg = DXGetErrorDescription(hr);
			BS_EXCEPT(RenderingAPIException, "Failed to DrawPrimitive : " + msg);
		}

		BS_INC_RENDER_STAT(NumDrawCalls);
		BS_ADD_RENDER_STAT(NumVertices, vertexCount);
		BS_ADD_RENDER_STAT(NumPrimitives, primCount);
	}

	void D3D9RenderAPI::drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount, 
		UINT32 instanceCount)
	{
		// Note: Not supporting instanced drawing

		UINT32 primCount = vertexCountToPrimCount(mCurrentDrawOperation, indexCount);

		// do indexed draw operation
		HRESULT hr = getActiveD3D9Device()->DrawIndexedPrimitive(
			getD3D9PrimitiveType(), 
			static_cast<UINT>(vertexOffset), 
			0, 
			static_cast<UINT>(vertexCount), 
			static_cast<UINT>(startIndex), 
			static_cast<UINT>(primCount)
			);

		if(FAILED(hr))
		{
			String msg = DXGetErrorDescription(hr);
			BS_EXCEPT(RenderingAPIException, "Failed to DrawIndexedPrimitive : " + msg);
		}

		BS_INC_RENDER_STAT(NumDrawCalls);
		BS_ADD_RENDER_STAT(NumVertices, vertexCount);
		BS_ADD_RENDER_STAT(NumPrimitives, primCount);
	}

	void D3D9RenderAPI::dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ)
	{
		// Not supported on DX9
	}

	void D3D9RenderAPI::setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom)
	{
		THROW_IF_NOT_CORE_THREAD;

		mScissorRect.left = static_cast<LONG>(left);
		mScissorRect.top = static_cast<LONG>(top);
		mScissorRect.bottom = static_cast<LONG>(bottom);
		mScissorRect.right = static_cast<LONG>(right);
	}

	void D3D9RenderAPI::setScissorTestEnable(bool enable)
	{
		THROW_IF_NOT_CORE_THREAD;

		HRESULT hr;
		if (enable)
		{
			if (FAILED(hr = setRenderState(D3DRS_SCISSORTESTENABLE, TRUE)))
			{
				BS_EXCEPT(RenderingAPIException, "Unable to enable scissor rendering state; " + getErrorDescription(hr));
			}

			if (FAILED(hr = getActiveD3D9Device()->SetScissorRect(&mScissorRect)))
			{
				BS_EXCEPT(RenderingAPIException, "Unable to set scissor rectangle; " + getErrorDescription(hr));
			}
		}
		else
		{
			if (FAILED(hr = setRenderState(D3DRS_SCISSORTESTENABLE, FALSE)))
			{
				BS_EXCEPT(RenderingAPIException, "Unable to disable scissor rendering state; " + getErrorDescription(hr));
			}
		}
	}

	void D3D9RenderAPI::setMultisampleAntialiasEnable(bool enable)
	{
		HRESULT hr;
		if(enable)
		{
			if (FAILED(hr = setRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE)))
			{
				BS_EXCEPT(RenderingAPIException, "Unable to enable multisample antialiasing. Error description: " + getErrorDescription(hr));
			}
		}
		else
		{
			if (FAILED(hr = setRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE)))
			{
				BS_EXCEPT(RenderingAPIException, "Unable to disable multisample antialiasing. Error description: " + getErrorDescription(hr));
			}
		}
	}

	void D3D9RenderAPI::setAntialiasedLineEnable(bool enable)
	{
		HRESULT hr;
		if(enable)
		{
			if (FAILED(hr = setRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE)))
			{
				BS_EXCEPT(RenderingAPIException, "Unable to enable line antialiasing. Error description: " + getErrorDescription(hr));
			}
		}
		else
		{
			if (FAILED(hr = setRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE)))
			{
				BS_EXCEPT(RenderingAPIException, "Unable to disable line antialiasing. Error description: " + getErrorDescription(hr));
			}
		}
	}

	void D3D9RenderAPI::clearRenderTarget(UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask)
	{
		if(mActiveRenderTarget == nullptr)
			return;

		if (targetMask != 0xFF)
			LOGWRN("DirectX 9 ignoring target mask for clearRenderTarget(). Only 0xFF is supported.");

		const RenderTargetProperties& rtProps = mActiveRenderTarget->getProperties();
		Rect2I clearRect(0, 0, rtProps.getWidth(), rtProps.getHeight());

		clearArea(buffers, color, depth, stencil, clearRect);
	}

	void D3D9RenderAPI::clearViewport(UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask)
	{
		if (targetMask != 0xFF)
			LOGWRN("DirectX 9 ignoring target mask for clearRenderTarget(). Only 0xFF is supported.");

		Rect2I clearRect(mViewportLeft, mViewportTop, mViewportWidth, mViewportHeight);
		clearArea(buffers, color, depth, stencil, clearRect);
	}

	void D3D9RenderAPI::clearArea(UINT32 buffers, const Color& color, float depth, UINT16 stencil, const Rect2I& clearRect)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(mActiveRenderTarget == nullptr)
			return;

		DWORD flags = 0;
		if (buffers & FBT_COLOR)
		{
			flags |= D3DCLEAR_TARGET;
		}
		if (buffers & FBT_DEPTH)
		{
			flags |= D3DCLEAR_ZBUFFER;
		}

		// Only try to clear the stencil buffer if supported
		if (buffers & FBT_STENCIL)
		{
			flags |= D3DCLEAR_STENCIL;
		}

		const RenderTargetProperties& rtProps = mActiveRenderTarget->getProperties();

		bool clearEntireTarget = clearRect.width == 0 || clearRect.height == 0;
		clearEntireTarget |= (clearRect.x == 0 && clearRect.y == 0 && clearRect.width == rtProps.getWidth() && clearRect.height == rtProps.getHeight());

		if(!clearEntireTarget)
		{
			D3DRECT clearD3DRect;
			clearD3DRect.x1 = (LONG)Math::clamp(clearRect.x, 0, (INT32)rtProps.getWidth() - 1);
			clearD3DRect.x2 = (LONG)Math::clamp((INT32)clearD3DRect.x1 + clearRect.width, 0, (INT32)rtProps.getWidth() - 1);

			clearD3DRect.y1 = (LONG)Math::clamp(clearRect.y, 0, (INT32)rtProps.getHeight() - 1);
			clearD3DRect.y2 = (LONG)Math::clamp((INT32)clearD3DRect.y1 + clearRect.height, 0, (INT32)rtProps.getHeight() - 1);

			HRESULT hr;
			if(FAILED(hr = getActiveD3D9Device()->Clear(1, &clearD3DRect, flags, color.getAsBGRA(), depth, stencil)))
			{
				String msg = DXGetErrorDescription(hr);
				BS_EXCEPT(RenderingAPIException, "Error clearing frame buffer : " + msg);
			}
		}
		else
		{
			HRESULT hr;
			if(FAILED(hr = getActiveD3D9Device()->Clear(0, nullptr, flags, color.getAsBGRA(), depth, stencil)))
			{
				String msg = DXGetErrorDescription(hr);
				BS_EXCEPT(RenderingAPIException, "Error clearing frame buffer : " + msg);
			}
		}

		BS_INC_RENDER_STAT(NumClears);
	}

	IDirect3D9*	D3D9RenderAPI::getDirect3D9()
	{
		THROW_IF_NOT_CORE_THREAD;

		return msD3D9RenderSystem->mpD3D;
	}

	UINT D3D9RenderAPI::getResourceCreationDeviceCount()
	{
		THROW_IF_NOT_CORE_THREAD;

		D3D9ResourceCreationPolicy creationPolicy = msD3D9RenderSystem->mResourceManager->getCreationPolicy();

		if (creationPolicy == RCP_CREATE_ON_ACTIVE_DEVICE)
		{
			return 1;
		}
		else if (creationPolicy == RCP_CREATE_ON_ALL_DEVICES)
		{
			return msD3D9RenderSystem->mDeviceManager->getDeviceCount();
		}

		BS_EXCEPT(InvalidParametersException, "Invalid resource creation policy.");

		return 0;
	}

	IDirect3DDevice9* D3D9RenderAPI::getResourceCreationDevice(UINT index)
	{
		THROW_IF_NOT_CORE_THREAD;

		D3D9ResourceCreationPolicy creationPolicy = msD3D9RenderSystem->mResourceManager->getCreationPolicy();
		IDirect3DDevice9* d3d9Device = NULL;

		if (creationPolicy == RCP_CREATE_ON_ACTIVE_DEVICE)
		{
			d3d9Device = msD3D9RenderSystem->getActiveD3D9Device();
		}
		else if (creationPolicy == RCP_CREATE_ON_ALL_DEVICES) 
		{
			d3d9Device = msD3D9RenderSystem->mDeviceManager->getDevice(index)->getD3D9Device();
		}
		else
		{
			BS_EXCEPT(InvalidParametersException, "Invalid resource creation policy.");
		}

		return d3d9Device;
	}

	IDirect3DDevice9* D3D9RenderAPI::getActiveD3D9Device()
	{	
		THROW_IF_NOT_CORE_THREAD;

		D3D9Device* activeDevice = msD3D9RenderSystem->mDeviceManager->getActiveDevice();
		IDirect3DDevice9* d3d9Device;

		d3d9Device = activeDevice->getD3D9Device();

		if (d3d9Device == nullptr)
			BS_EXCEPT(InvalidParametersException, "Current d3d9 device is null.");

		return d3d9Device;
	}	

	D3D9ResourceManager* D3D9RenderAPI::getResourceManager()
	{
		// No need to check if we're on core thread as this is synced up internally

		return msD3D9RenderSystem->mResourceManager;
	}

	D3D9DeviceManager* D3D9RenderAPI::getDeviceManager()
	{
		THROW_IF_NOT_CORE_THREAD;

		return msD3D9RenderSystem->mDeviceManager;
	}

	/************************************************************************/
	/* 								UTILITY METHODS                    		*/
	/************************************************************************/

	void D3D9RenderAPI::convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest)
	{
		dest = matrix;

		// Convert depth range from [-1,+1] to [0,1]
		dest[2][0] = (dest[2][0] + dest[3][0]) / 2;
		dest[2][1] = (dest[2][1] + dest[3][1]) / 2;
		dest[2][2] = (dest[2][2] + dest[3][2]) / 2;
		dest[2][3] = (dest[2][3] + dest[3][3]) / 2;
	}

	const RenderAPIInfo& D3D9RenderAPI::getAPIInfo() const
	{
		static RenderAPIInfo info(-0.5f, 0.5f, 0.0f, 1.0f, VET_COLOR_ARGB, true, true, false);

		return info;
	}

	GpuParamBlockDesc D3D9RenderAPI::generateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params)
	{
		GpuParamBlockDesc block;
		block.blockSize = 0;
		block.isShareable = true;
		block.name = name;
		block.slot = 0;

		// DX9 doesn't natively support parameter blocks but Banshee's emulation expects everything to be 16 byte aligned.
		// Iterate in reverse order because DX9's shader reflection API reports the variables in reverse order then they
		// appear in code, and we want to match the auto-generated buffers that result from that layout.
		for (auto riter = params.rbegin(); riter != params.rend(); ++riter)
		{
			GpuParamDataDesc& param = *riter;

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[param.type];
			UINT32 size = typeInfo.size / 4;

			UINT32 alignOffset = size % 4;
			if (alignOffset != 0)
				size += 4 - alignOffset;

			param.elementSize = size;
			param.arrayElementStride = size;
			param.cpuMemOffset = block.blockSize;
			param.gpuMemOffset = 0;
			param.paramBlockSlot = 0;

			if (param.arraySize > 0)
				block.blockSize += size * param.arraySize;
			else
				block.blockSize += size;
		}

		return block;
	}

	/************************************************************************/
	/* 								PRIVATE		                     		*/
	/************************************************************************/

	D3D9DriverList* D3D9RenderAPI::getDirect3DDrivers() const
	{
		if( !mDriverList )
			mDriverList = bs_new<D3D9DriverList>();

		return mDriverList;
	}

	D3DPRIMITIVETYPE D3D9RenderAPI::getD3D9PrimitiveType() const
	{
		switch(mCurrentDrawOperation)
		{
		case DOT_POINT_LIST:
			return D3DPT_POINTLIST;
		case DOT_LINE_LIST:
			return D3DPT_LINELIST;
		case DOT_LINE_STRIP:
			return D3DPT_LINESTRIP;
		case DOT_TRIANGLE_LIST:
			return D3DPT_TRIANGLELIST;
		case DOT_TRIANGLE_STRIP:
			return D3DPT_TRIANGLESTRIP;
		case DOT_TRIANGLE_FAN:
			return D3DPT_TRIANGLEFAN;
		}

		return D3DPT_TRIANGLELIST;
	}

	RenderAPICapabilities* D3D9RenderAPI::updateRenderSystemCapabilities(D3D9RenderWindowCore* renderWindow)
	{			
		RenderAPICapabilities* rsc = mCurrentCapabilities;
		if (rsc == nullptr)
			rsc = bs_new<RenderAPICapabilities>();

		rsc->setDriverVersion(mDriverVersion);
		rsc->setDeviceName(mActiveD3DDriver->getDriverDescription());
		rsc->setRenderAPIName(getName());

		// Init caps to maximum.		
		rsc->setCapability(RSC_ANISOTROPY);
		rsc->setCapability(RSC_AUTOMIPMAP);
		rsc->setCapability(RSC_CUBEMAPPING);			
		rsc->setCapability(RSC_TWO_SIDED_STENCIL);		
		rsc->setCapability(RSC_STENCIL_WRAP);
		rsc->setCapability(RSC_HWOCCLUSION);		
		rsc->setCapability(RSC_USER_CLIP_PLANES);			
		rsc->setCapability(RSC_VERTEX_FORMAT_UBYTE4);			
		rsc->setCapability(RSC_TEXTURE_3D);			
		rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
		rsc->setNumMultiRenderTargets(BS_MAX_MULTIPLE_RENDER_TARGETS);
		rsc->setCapability(RSC_MRT_DIFFERENT_BIT_DEPTHS);		
		rsc->setCapability(RSC_POINT_SPRITES);			
		rsc->setCapability(RSC_POINT_EXTENDED_PARAMETERS);								
		rsc->setMaxPointSize(10.0);
		rsc->setCapability(RSC_MIPMAP_LOD_BIAS);				
		rsc->setCapability(RSC_PERSTAGECONSTANT);
		rsc->setStencilBufferBitDepth(8);
		rsc->setCapability(RSC_ADVANCED_BLEND_OPERATIONS);

		for (UINT32 i=0; i < mDeviceManager->getDeviceCount(); ++i)
		{
			D3D9Device* device			 = mDeviceManager->getDevice(i);
			IDirect3DDevice9* d3d9Device = device->getD3D9Device();

			IDirect3DSurface9* pSurf;


			// Check for hardware stencil support
			d3d9Device->GetDepthStencilSurface(&pSurf);

			if (pSurf != nullptr)
			{
				D3DSURFACE_DESC surfDesc;

				pSurf->GetDesc(&surfDesc);
				pSurf->Release();
			}																	

			// Check for hardware occlusion support
			HRESULT hr = d3d9Device->CreateQuery(D3DQUERYTYPE_OCCLUSION,  NULL);

			if (FAILED(hr))
				rsc->unsetCapability(RSC_HWOCCLUSION);
		}

		// Update RS caps using the minimum value found in adapter list.
		for (unsigned int i=0; i < mDriverList->count(); ++i)
		{
			D3D9Driver* pCurDriver       = mDriverList->item(i);			
			const D3DCAPS9& rkCurCaps    = pCurDriver->getD3D9DeviceCaps();

			rsc->setNumTextureUnits(GPT_FRAGMENT_PROGRAM, 16); // We don't support anything lower than SM3, and 16 is the sampler count determined by the specification

			rsc->setMaxBoundVertexBuffers(static_cast<UINT32>(rkCurCaps.MaxStreams));

			// Check for Anisotropy.
			if (rkCurCaps.MaxAnisotropy <= 1)
				rsc->unsetCapability(RSC_ANISOTROPY);

			// Check automatic mipmap generation.
			if ((rkCurCaps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) == 0)
				rsc->unsetCapability(RSC_AUTOMIPMAP);

			// Two-sided stencil
			if ((rkCurCaps.StencilCaps & D3DSTENCILCAPS_TWOSIDED) == 0)
				rsc->unsetCapability(RSC_TWO_SIDED_STENCIL);

			// stencil wrap
			if ((rkCurCaps.StencilCaps & D3DSTENCILCAPS_INCR) == 0 ||
				(rkCurCaps.StencilCaps & D3DSTENCILCAPS_DECR) == 0)
				rsc->unsetCapability(RSC_STENCIL_WRAP);

			// User clip planes
			if (rkCurCaps.MaxUserClipPlanes == 0)			
				rsc->unsetCapability(RSC_USER_CLIP_PLANES);			

			// UBYTE4 type?
			if ((rkCurCaps.DeclTypes & D3DDTCAPS_UBYTE4) == 0)			
				rsc->unsetCapability(RSC_VERTEX_FORMAT_UBYTE4);	

			// Check cube map support.
			if ((rkCurCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP) == 0)
				rsc->unsetCapability(RSC_CUBEMAPPING);

			// 3D textures?
			if ((rkCurCaps.TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP) == 0)			
				rsc->unsetCapability(RSC_TEXTURE_3D);			

			if (rkCurCaps.TextureCaps & D3DPTEXTURECAPS_POW2)
			{
				// Conditional support for non POW2
				if (!(rkCurCaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL))							
					rsc->unsetCapability(RSC_NON_POWER_OF_2_TEXTURES);				
			}	

			// Number of render targets
			if (rkCurCaps.NumSimultaneousRTs < rsc->getNumMultiRenderTargets())
			{
				rsc->setNumMultiRenderTargets(std::min((UINT16)rkCurCaps.NumSimultaneousRTs, (UINT16)BS_MAX_MULTIPLE_RENDER_TARGETS));
			}	

			if((rkCurCaps.PrimitiveMiscCaps & D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS) == 0)
			{
				rsc->unsetCapability(RSC_MRT_DIFFERENT_BIT_DEPTHS);
			}

			// Point sprites 
			if (rkCurCaps.MaxPointSize <= 1.0f)
			{
				rsc->unsetCapability(RSC_POINT_SPRITES);
				// sprites and extended parameters go together in D3D
				rsc->unsetCapability(RSC_POINT_EXTENDED_PARAMETERS);				
			}

			// Take the minimum point size.
			if (rkCurCaps.MaxPointSize < rsc->getMaxPointSize())
				rsc->setMaxPointSize(rkCurCaps.MaxPointSize);	

			// Mipmap LOD biasing?
			if ((rkCurCaps.RasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS) == 0)			
				rsc->unsetCapability(RSC_MIPMAP_LOD_BIAS);			


			// Do we support per-stage src_manual constants?
			// HACK - ATI drivers seem to be buggy and don't support per-stage constants properly?
			// TODO: move this to RSC
			if((rkCurCaps.PrimitiveMiscCaps & D3DPMISCCAPS_PERSTAGECONSTANT) == 0)
				rsc->unsetCapability(RSC_PERSTAGECONSTANT);

			// Advanced blend operations? min max subtract rev 
			if((rkCurCaps.PrimitiveMiscCaps & D3DPMISCCAPS_BLENDOP) == 0)
				rsc->unsetCapability(RSC_ADVANCED_BLEND_OPERATIONS);
		}				

		// We always support compression, D3DX will decompress if device does not support
		rsc->setCapability(RSC_TEXTURE_COMPRESSION);
		rsc->setCapability(RSC_TEXTURE_COMPRESSION_DXT);

		updateVertexShaderCaps(rsc);
		updatePixelShaderCaps(rsc);

		// Adapter details
		const D3DADAPTER_IDENTIFIER9& adapterID = mActiveD3DDriver->getAdapterIdentifier();

		// determine vendor
		// Full list of vendors here: http://www.pcidatabase.com/vendors.php?sort=id
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

		// We always support rendertextures bigger than the frame buffer
		rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);

		// Determine if any floating point texture format is supported
		D3DFORMAT floatFormats[6] = {D3DFMT_R16F, D3DFMT_G16R16F, 
			D3DFMT_A16B16G16R16F, D3DFMT_R32F, D3DFMT_G32R32F, 
			D3DFMT_A32B32G32R32F};
		IDirect3DSurface9* bbSurf;
		renderWindow->getCustomAttribute("DDBACKBUFFER", &bbSurf);
		D3DSURFACE_DESC bbSurfDesc;
		bbSurf->GetDesc(&bbSurfDesc);

		for (int i = 0; i < 6; ++i)
		{
			if (SUCCEEDED(mpD3D->CheckDeviceFormat(mActiveD3DDriver->getAdapterNumber(), 
				D3DDEVTYPE_HAL, bbSurfDesc.Format, 
				0, D3DRTYPE_TEXTURE, floatFormats[i])))
			{
				rsc->setCapability(RSC_TEXTURE_FLOAT);
				break;
			}

		}

		// Vertex textures
		if (rsc->isShaderProfileSupported("vs_3_0"))
		{
			rsc->setCapability(RSC_VERTEX_TEXTURE_FETCH);
			rsc->setNumTextureUnits(GPT_VERTEX_PROGRAM, 4);
			rsc->setNumCombinedTextureUnits(rsc->getNumTextureUnits(GPT_FRAGMENT_PROGRAM) +
				rsc->getNumTextureUnits(GPT_VERTEX_PROGRAM));
		}		
		else
		{
			rsc->setNumCombinedTextureUnits(rsc->getNumTextureUnits(GPT_FRAGMENT_PROGRAM));
		}

		// Check alpha to coverage support
		// this varies per vendor! But at least SM3 is required
		if (rsc->isShaderProfileSupported("ps_3_0"))
		{
			// NVIDIA needs a separate check
			if (rsc->getVendor() == GPU_NVIDIA)
			{
				if (mpD3D->CheckDeviceFormat(
					D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0,D3DRTYPE_SURFACE, 
					(D3DFORMAT)MAKEFOURCC('A', 'T', 'O', 'C')) == S_OK)
				{
					rsc->setCapability(RSC_ALPHA_TO_COVERAGE);
				}

			}
			else if (rsc->getVendor() == GPU_AMD)
			{
				// There is no check on ATI, we have to assume SM3 == support
				rsc->setCapability(RSC_ALPHA_TO_COVERAGE);
			}

			// no other cards have Dx9 hacks for alpha to coverage, as far as I know
		}


		if (mCurrentCapabilities == nullptr)
		{		
			mCurrentCapabilities = rsc;
			mCurrentCapabilities->addShaderProfile("hlsl");

			if (mCurrentCapabilities->isShaderProfileSupported("hlsl"))
				GpuProgramCoreManager::instance().addFactory(mHLSLProgramFactory);

			mNumTexStages = mCurrentCapabilities->getNumCombinedTextureUnits();
			mTexStageDesc = bs_newN<sD3DTextureStageDesc>(mNumTexStages);

			// set stages desc. to defaults
			for (UINT32 n = 0; n < mNumTexStages; n++)
			{
				mTexStageDesc[n].coordIndex = 0;
				mTexStageDesc[n].texType = D3D9Mappings::D3D_TEX_TYPE_NORMAL;
				mTexStageDesc[n].pTex = 0;
				mTexStageDesc[n].pVertexTex = 0;
			}
		}

		return rsc;
	}

	void D3D9RenderAPI::updateVertexShaderCaps(RenderAPICapabilities* rsc) const
	{
		UINT16 major = 0xFF;
		UINT16 minor = 0xFF;
		D3DCAPS9 minVSCaps;

		// Find the device with the lowest vertex shader caps.
		for (unsigned int i=0; i < mDriverList->count(); ++i)
		{
			D3D9Driver* pCurDriver      = mDriverList->item(i);			
			const D3DCAPS9& rkCurCaps   = pCurDriver->getD3D9DeviceCaps();
			UINT16 currMajor			= static_cast<UINT16>((rkCurCaps.VertexShaderVersion & 0x0000FF00) >> 8);
			UINT16 currMinor			= static_cast<UINT16>(rkCurCaps.VertexShaderVersion & 0x000000FF);

			if (currMajor < major)	
			{
				major = currMajor;
				minor = currMinor;
				minVSCaps = rkCurCaps;
			}
			else if (currMajor == major && currMinor < minor)
			{
				minor = currMinor;
				minVSCaps = rkCurCaps;
			}			
		}

		// In case we didn't found any vertex shader support
		// try the IDirect3DDevice9 caps instead of the IDirect3D9
		// software vertex processing is reported there
		if (major == 0 && minor == 0)
		{
			IDirect3DDevice9* lpD3DDevice9 = getActiveD3D9Device();
			D3DCAPS9 d3dDeviceCaps9;
			lpD3DDevice9->GetDeviceCaps(&d3dDeviceCaps9);
			major = static_cast<UINT16>((d3dDeviceCaps9.VertexShaderVersion & 0x0000FF00) >> 8);
			minor = static_cast<UINT16>(d3dDeviceCaps9.VertexShaderVersion & 0x000000FF);
		}

		bool vs2x = false;
		bool vs2a = false;

		// Special case detection for vs_2_x/a support
		if (major >= 2)
		{
			if ((minVSCaps.VS20Caps.Caps & D3DVS20CAPS_PREDICATION) &&
				(minVSCaps.VS20Caps.DynamicFlowControlDepth > 0) &&
				(minVSCaps.VS20Caps.NumTemps >= 12))
			{
				vs2x = true;
			}

			if ((minVSCaps.VS20Caps.Caps & D3DVS20CAPS_PREDICATION) &&
				(minVSCaps.VS20Caps.DynamicFlowControlDepth > 0) &&
				(minVSCaps.VS20Caps.NumTemps >= 13))
			{
				vs2a = true;
			}
		}

		// Populate max param count
		switch (major)
		{
		case 1:
			// No boolean params allowed
			rsc->setVertexProgramConstantBoolCount(0);
			// No integer params allowed
			rsc->setVertexProgramConstantIntCount(0);
			// float params, always 4D
			rsc->setVertexProgramConstantFloatCount(static_cast<UINT16>(minVSCaps.MaxVertexShaderConst));

			break;
		case 2:
			// 16 boolean params allowed
			rsc->setVertexProgramConstantBoolCount(16);
			// 16 integer params allowed, 4D
			rsc->setVertexProgramConstantIntCount(16);
			// float params, always 4D
			rsc->setVertexProgramConstantFloatCount(static_cast<UINT16>(minVSCaps.MaxVertexShaderConst));
			break;
		case 3:
			// 16 boolean params allowed
			rsc->setVertexProgramConstantBoolCount(16);
			// 16 integer params allowed, 4D
			rsc->setVertexProgramConstantIntCount(16);
			// float params, always 4D
			rsc->setVertexProgramConstantFloatCount(static_cast<UINT16>(minVSCaps.MaxVertexShaderConst));
			break;
		}

		// populate syntax codes in program manager (no breaks in this one so it falls through)
		switch(major)
		{
		case 3:
			rsc->addShaderProfile("vs_3_0");
			rsc->addGpuProgramProfile(GPP_VS_3_0, "vs_3_0");
		case 2:
			if (vs2x)
			{
				rsc->addShaderProfile("vs_2_x");
				rsc->addGpuProgramProfile(GPP_VS_2_x, "vs_2_x");
			}
			if (vs2a)
			{
				rsc->addShaderProfile("vs_2_a");
				rsc->addGpuProgramProfile(GPP_VS_2_a, "vs_2_a");
			}

			rsc->addShaderProfile("vs_2_0");
			rsc->addGpuProgramProfile(GPP_VS_2_0, "vs_2_0");
		case 1:
			rsc->addShaderProfile("vs_1_1");
			rsc->addGpuProgramProfile(GPP_VS_1_1, "vs_1_1");
		}
	}

	void D3D9RenderAPI::updatePixelShaderCaps(RenderAPICapabilities* rsc) const
	{
		UINT16 major = 0xFF;
		UINT16 minor = 0xFF;
		D3DCAPS9 minPSCaps;

		// Find the device with the lowest pixel shader caps.
		for (unsigned int i=0; i < mDriverList->count(); ++i)
		{
			D3D9Driver* pCurDriver      = mDriverList->item(i);			
			const D3DCAPS9& currCaps    = pCurDriver->getD3D9DeviceCaps();
			UINT16 currMajor			= static_cast<UINT16>((currCaps.PixelShaderVersion & 0x0000FF00) >> 8);
			UINT16 currMinor			= static_cast<UINT16>(currCaps.PixelShaderVersion & 0x000000FF);

			if (currMajor < major)	
			{
				major = currMajor;
				minor = currMinor;
				minPSCaps = currCaps;
			}
			else if (currMajor == major && currMinor < minor)
			{
				minor = currMinor;
				minPSCaps = currCaps;
			}			
		}

		bool ps2a = false;
		bool ps2b = false;
		bool ps2x = false;

		// Special case detection for ps_2_x/a/b support
		if (major >= 2)
		{
			if ((minPSCaps.PS20Caps.Caps & D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT) &&
				(minPSCaps.PS20Caps.NumTemps >= 32))
			{
				ps2b = true;
			}

			if ((minPSCaps.PS20Caps.Caps & D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT) &&
				(minPSCaps.PS20Caps.Caps & D3DPS20CAPS_NODEPENDENTREADLIMIT) &&
				(minPSCaps.PS20Caps.Caps & D3DPS20CAPS_ARBITRARYSWIZZLE) &&
				(minPSCaps.PS20Caps.Caps & D3DPS20CAPS_GRADIENTINSTRUCTIONS) &&
				(minPSCaps.PS20Caps.Caps & D3DPS20CAPS_PREDICATION) &&
				(minPSCaps.PS20Caps.NumTemps >= 22))
			{
				ps2a = true;
			}

			// Does this enough?
			if (ps2a || ps2b)
			{
				ps2x = true;
			}
		}

		switch (major)
		{
		case 1:
			// no boolean params allowed
			rsc->setFragmentProgramConstantBoolCount(0);
			// no integer params allowed
			rsc->setFragmentProgramConstantIntCount(0);
			// float params, always 4D
			rsc->setFragmentProgramConstantFloatCount(8);
			break;
		case 2:
			// 16 boolean params allowed
			rsc->setFragmentProgramConstantBoolCount(16);
			// 16 integer params allowed, 4D
			rsc->setFragmentProgramConstantIntCount(16);
			// float params, always 4D
			rsc->setFragmentProgramConstantFloatCount(32);
			break;
		case 3:
			// 16 boolean params allowed
			rsc->setFragmentProgramConstantBoolCount(16);
			// 16 integer params allowed, 4D
			rsc->setFragmentProgramConstantIntCount(16);
			// float params, always 4D
			rsc->setFragmentProgramConstantFloatCount(224);
			break;
		}

		// populate syntax codes in program manager (no breaks in this one so it falls through)
		switch(major)
		{
		case 3:
			if (minor > 0)
			{
				rsc->addShaderProfile("ps_3_x");
				rsc->addGpuProgramProfile(GPP_FS_3_x, "ps_3_x");
			}

			rsc->addShaderProfile("ps_3_0");
			rsc->addGpuProgramProfile(GPP_FS_3_0, "ps_3_0");
		case 2:
			if (ps2x)
			{
				rsc->addShaderProfile("ps_2_x");
				rsc->addGpuProgramProfile(GPP_FS_2_x, "ps_2_x");
			}
			if (ps2a)
			{
				rsc->addShaderProfile("ps_2_a");
				rsc->addGpuProgramProfile(GPP_FS_2_a, "ps_2_a");
			}
			if (ps2b)
			{
				rsc->addShaderProfile("ps_2_b");
				rsc->addGpuProgramProfile(GPP_FS_2_b, "ps_2_b");
			}

			rsc->addShaderProfile("ps_2_0");
			rsc->addGpuProgramProfile(GPP_FS_2_0, "ps_2_0");
		case 1:
			if (major > 1 || minor >= 4)
			{
				rsc->addShaderProfile("ps_1_4");
				rsc->addGpuProgramProfile(GPP_FS_1_4, "ps_1_4");
			}
			if (major > 1 || minor >= 3)
			{
				rsc->addShaderProfile("ps_1_3");
				rsc->addGpuProgramProfile(GPP_FS_1_3, "ps_1_3");
			}
			if (major > 1 || minor >= 2)
			{
				rsc->addShaderProfile("ps_1_2");
				rsc->addGpuProgramProfile(GPP_FS_1_2, "ps_1_2");
			}

			rsc->addShaderProfile("ps_1_1");
			rsc->addGpuProgramProfile(GPP_FS_1_1, "ps_1_1");
		}
	}

	String D3D9RenderAPI::getErrorDescription(long errorNumber) const
	{
		const String errMsg = DXGetErrorDescription(errorNumber);
		return errMsg;
	}

	void D3D9RenderAPI::setClipPlane (UINT16 index, float A, float B, float C, float D)
	{
		float plane[4] = { A, B, C, D };
		getActiveD3D9Device()->SetClipPlane (index, plane);
	}

	void D3D9RenderAPI::enableClipPlane (UINT16 index, bool enable)
	{
		DWORD prev;
		getActiveD3D9Device()->GetRenderState(D3DRS_CLIPPLANEENABLE, &prev);
		setRenderState(D3DRS_CLIPPLANEENABLE, enable?
			(prev | (1 << index)) : (prev & ~(1 << index)));
	}

	void D3D9RenderAPI::notifyOnDeviceLost(D3D9Device* device)
	{	
		if (mIsFrameInProgress)
		{
			endFrame();
			mRestoreFrameOnReset = true;
		}
	}

	void D3D9RenderAPI::notifyOnDeviceReset(D3D9Device* device)
	{		
		// Reset state attributes.	
		mVertexProgramBound = false;
		mFragmentProgramBound = false;

		if (mRestoreFrameOnReset)
		{
			beginFrame();
			mRestoreFrameOnReset = false;
		}
	}

	void D3D9RenderAPI::determineMultisampleSettings(IDirect3DDevice9* d3d9Device, UINT32 multisampleCount, D3DFORMAT d3dPixelFormat, 
		bool fullScreen, D3DMULTISAMPLE_TYPE *outMultisampleType, DWORD *outMultisampleQuality) const
	{
		bool tryCSAA = false; // Note: Disabled for now, but leaving the code for later so it might be useful
		enum CSAAMode { CSAA_Normal, CSAA_Quality };
		CSAAMode csaaMode = CSAA_Normal;

		D3D9DriverList* driverList = getDirect3DDrivers();
		D3D9Driver* deviceDriver = mActiveD3DDriver;
		D3D9Device* device = mDeviceManager->getDeviceFromD3D9Device(d3d9Device);

		for (UINT32 i = 0; i < driverList->count(); ++i)
		{
			D3D9Driver* currDriver = driverList->item(i);

			if (currDriver->getAdapterNumber() == device->getAdapterNumber())
			{
				deviceDriver = currDriver;
				break;
			}
		}

		UINT32 origNumSamples = multisampleCount;
		bool foundValid = false;
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
						*outMultisampleType = D3DMULTISAMPLE_8_SAMPLES;
						*outMultisampleQuality = 0;
					}
					else
					{
						*outMultisampleType = D3DMULTISAMPLE_4_SAMPLES;
						*outMultisampleQuality = 2;
					}
					break;
				case 16:
					if (csaaMode == CSAA_Quality)
					{
						*outMultisampleType = D3DMULTISAMPLE_8_SAMPLES;
						*outMultisampleQuality = 2;
					}
					else
					{
						*outMultisampleType = D3DMULTISAMPLE_4_SAMPLES;
						*outMultisampleQuality = 4;
					}
					break;
				}
			}
			else // !CSAA
			{
				*outMultisampleType = D3DMULTISAMPLE_NONE;
				*outMultisampleQuality = 0;
			}


			HRESULT hr;
			DWORD outQuality;
			hr = mpD3D->CheckDeviceMultiSampleType( 
				deviceDriver->getAdapterNumber(), 
				D3DDEVTYPE_HAL, 
				d3dPixelFormat, 
				!fullScreen, 
				*outMultisampleType, 
				&outQuality);

			if (SUCCEEDED(hr) && 
				(!tryCSAA || outQuality > *outMultisampleQuality))
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
					multisampleCount = origNumSamples;
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

	void D3D9RenderAPI::setClipPlanesImpl(const PlaneList& clipPlanes)
	{
		size_t i;
		size_t numClipPlanes;
		D3DXPLANE dx9ClipPlane;
		DWORD mask = 0;
		HRESULT hr;

		numClipPlanes = clipPlanes.size();
		for (i = 0; i < numClipPlanes; ++i)
		{
			const Plane& plane = clipPlanes[i];

			dx9ClipPlane.a = plane.normal.x;
			dx9ClipPlane.b = plane.normal.y;
			dx9ClipPlane.c = plane.normal.z;
			dx9ClipPlane.d = plane.d;

			// TODO Low priority - Transform planes to clip space?

			hr = getActiveD3D9Device()->SetClipPlane(static_cast<DWORD>(i), dx9ClipPlane);
			if (FAILED(hr))
			{
				BS_EXCEPT(RenderingAPIException, "Unable to set clip plane");
			}

			mask |= (1 << i);
		}

		hr = setRenderState(D3DRS_CLIPPLANEENABLE, mask);
		if (FAILED(hr))
		{
			BS_EXCEPT(RenderingAPIException, "Unable to set render state for clip planes");
		}
	}

	HRESULT D3D9RenderAPI::setRenderState(D3DRENDERSTATETYPE state, DWORD value)
	{
		return getActiveD3D9Device()->SetRenderState(state, value);
	}

	HRESULT D3D9RenderAPI::setSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value)
	{
		return getActiveD3D9Device()->SetSamplerState(sampler, type, value);
	}

	HRESULT D3D9RenderAPI::setTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
	{
		if (stage < 8)
			return getActiveD3D9Device()->SetTextureStageState(stage, type, value);
		else
			return D3D_OK;
	}

	DWORD D3D9RenderAPI::getCurrentAnisotropy(UINT32 unit)
	{
		DWORD oldVal;
		getActiveD3D9Device()->GetSamplerState(static_cast<DWORD>(unit), D3DSAMP_MAXANISOTROPY, &oldVal);
		return oldVal;
	}

	void D3D9RenderAPI::applyViewport()
	{
		if (mActiveRenderTarget == nullptr)
			return;

		const RenderTargetProperties& rtProps = mActiveRenderTarget->getProperties();
		D3DVIEWPORT9 d3dvp;
		HRESULT hr;

		setCullingMode(mCullingMode);

		// Set viewport dimensions
		mViewportLeft = (UINT32)(rtProps.getWidth() * mViewportNorm.x);
		mViewportTop = (UINT32)(rtProps.getHeight() * mViewportNorm.y);
		mViewportWidth = (UINT32)(rtProps.getWidth() * mViewportNorm.width);
		mViewportHeight = (UINT32)(rtProps.getHeight() * mViewportNorm.height);

		d3dvp.X = mViewportLeft;
		d3dvp.Y = mViewportTop;
		d3dvp.Width = mViewportWidth;
		d3dvp.Height = mViewportHeight;
		if (rtProps.requiresTextureFlipping())
		{
			// Convert "top-left" to "bottom-left"
			d3dvp.Y = rtProps.getHeight() - d3dvp.Height - d3dvp.Y;
		}

		// Z-values from 0.0 to 1.0 (TODO: standardise with OpenGL)
		d3dvp.MinZ = 0.0f;
		d3dvp.MaxZ = 1.0f;

		if (FAILED(hr = getActiveD3D9Device()->SetViewport(&d3dvp)))
			BS_EXCEPT(RenderingAPIException, "Failed to set viewport.");
	}
}