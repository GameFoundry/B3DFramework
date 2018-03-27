//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11GpuProgram.h"
#include "BsD3D11Device.h"
#include "Error/BsException.h"
#include "Debug/BsDebug.h"
#include "RenderAPI/BsGpuParams.h"
#include "BsD3D11RenderAPI.h"
#include "Managers/BsGpuProgramManager.h"
#include "Managers/BsHardwareBufferManager.h"
#include "BsD3D11HLSLParamParser.h"
#include "Profiling/BsRenderStats.h"
#include <regex>

namespace bs { namespace ct
{
	UINT32 D3D11GpuProgram::GlobalProgramId = 0;

	D3D11GpuProgram::D3D11GpuProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuProgram(desc, deviceMask), mProgramId(0)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on DirectX 11.");
	}

	D3D11GpuProgram::~D3D11GpuProgram()
	{
		mInputDeclaration = nullptr;

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuProgram);
	}

	void D3D11GpuProgram::initialize()
	{
		if (!isSupported())
		{
			mIsCompiled = false;
			mCompileMessages = "Specified program is not supported by the current render system.";

			GpuProgram::initialize();
			return;
		}

		if(!mBytecode || mBytecode->compilerId != DIRECTX_COMPILER_ID)
		{
			GPU_PROGRAM_DESC desc;
			desc.type = mType;
			desc.entryPoint = mEntryPoint;
			desc.source = mSource;
			desc.language = "hlsl";

			mBytecode = compileBytecode(desc);
		}

		mCompileMessages = mBytecode->messages;
		mIsCompiled = mBytecode->instructions.data != nullptr;

		if(mIsCompiled)
		{
			mParametersDesc = mBytecode->paramDesc;

			D3D11RenderAPI* rapi = static_cast<D3D11RenderAPI*>(RenderAPI::instancePtr());
			loadFromMicrocode(rapi->getPrimaryDevice(), mBytecode->instructions);

			if(mType == GPT_VERTEX_PROGRAM)
				mInputDeclaration = HardwareBufferManager::instance().createVertexDeclaration(mBytecode->vertexInput);
			
		}

		mProgramId = GlobalProgramId++;

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuProgram);

		GpuProgram::initialize();
	}

	D3D11GpuVertexProgram::D3D11GpuVertexProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		: D3D11GpuProgram(desc, deviceMask), mVertexShader(nullptr)
	{ }

	D3D11GpuVertexProgram::~D3D11GpuVertexProgram()
	{
		SAFE_RELEASE(mVertexShader);
	}

	void D3D11GpuVertexProgram::loadFromMicrocode(D3D11Device& device, const DataBlob& microcode)
	{
		HRESULT hr = device.getD3D11Device()->CreateVertexShader( 
			microcode.data, microcode.size, device.getClassLinkage(), &mVertexShader);

		if (FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, 
				"Cannot create D3D11 vertex shader from microcode\nError Description:" + errorDescription);

		}
	}

	ID3D11VertexShader * D3D11GpuVertexProgram::getVertexShader() const
	{
		return mVertexShader;
	}

	D3D11GpuFragmentProgram::D3D11GpuFragmentProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		: D3D11GpuProgram(desc, deviceMask), mPixelShader(nullptr)
	{ }

	D3D11GpuFragmentProgram::~D3D11GpuFragmentProgram()
	{
		SAFE_RELEASE(mPixelShader);
	}

	void D3D11GpuFragmentProgram::loadFromMicrocode(D3D11Device& device, const DataBlob& microcode)
	{
		HRESULT hr = device.getD3D11Device()->CreatePixelShader(
			microcode.data, microcode.size, device.getClassLinkage(), &mPixelShader);

		if (FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, 
				"Cannot create D3D11 pixel shader from microcode.\nError Description:" + errorDescription);
		}
	}

	ID3D11PixelShader * D3D11GpuFragmentProgram::getPixelShader() const
	{
		return mPixelShader;
	}


	D3D11GpuGeometryProgram::D3D11GpuGeometryProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		: D3D11GpuProgram(desc, deviceMask), mGeometryShader(nullptr)
	{ }

	D3D11GpuGeometryProgram::~D3D11GpuGeometryProgram()
	{ 
		SAFE_RELEASE(mGeometryShader);
	}

	void D3D11GpuGeometryProgram::loadFromMicrocode(D3D11Device& device, const DataBlob& microcode)
	{
		HRESULT hr = device.getD3D11Device()->CreateGeometryShader(
			microcode.data, microcode.size, device.getClassLinkage(), &mGeometryShader);

		if (FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, 
				"Cannot create D3D11 geometry shader from microcode.\nError Description:" + errorDescription);
		}
	}

	ID3D11GeometryShader * D3D11GpuGeometryProgram::getGeometryShader() const
	{
		return mGeometryShader;
	}

	D3D11GpuDomainProgram::D3D11GpuDomainProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		: D3D11GpuProgram(desc, deviceMask), mDomainShader(nullptr)
	{ }

	D3D11GpuDomainProgram::~D3D11GpuDomainProgram()
	{ 
		SAFE_RELEASE(mDomainShader);
	}

	void D3D11GpuDomainProgram::loadFromMicrocode(D3D11Device& device, const DataBlob& microcode)
	{
		HRESULT hr = device.getD3D11Device()->CreateDomainShader(
			microcode.data, microcode.size, device.getClassLinkage(), &mDomainShader);

		if (FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, 
				"Cannot create D3D11 domain shader from microcode.\nError Description:" + errorDescription);
		}
	}

	ID3D11DomainShader * D3D11GpuDomainProgram::getDomainShader() const
	{
		return mDomainShader;
	}

	D3D11GpuHullProgram::D3D11GpuHullProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		: D3D11GpuProgram(desc, deviceMask), mHullShader(nullptr)
	{ }

	D3D11GpuHullProgram::~D3D11GpuHullProgram()
	{ 
		SAFE_RELEASE(mHullShader);
	}

	void D3D11GpuHullProgram::loadFromMicrocode(D3D11Device& device, const DataBlob& microcode)
	{
		// Create the shader
		HRESULT hr = device.getD3D11Device()->CreateHullShader(
			microcode.data, microcode.size, device.getClassLinkage(), &mHullShader);

		if (FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException, 
				"Cannot create D3D11 hull shader from microcode.\nError Description:" + errorDescription);
		}
	}

	ID3D11HullShader* D3D11GpuHullProgram::getHullShader() const
	{
		return mHullShader;
	}


	D3D11GpuComputeProgram::D3D11GpuComputeProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		: D3D11GpuProgram(desc, deviceMask), mComputeShader(nullptr)
	{ }

	D3D11GpuComputeProgram::~D3D11GpuComputeProgram()
	{
		SAFE_RELEASE(mComputeShader);
	}

	void D3D11GpuComputeProgram::loadFromMicrocode(D3D11Device& device, const DataBlob& microcode)
	{
		HRESULT hr = device.getD3D11Device()->CreateComputeShader(
			microcode.data, microcode.size, device.getClassLinkage(), &mComputeShader);

		if (FAILED(hr) || device.hasError())
		{
			String errorDescription = device.getErrorDescription();
			BS_EXCEPT(RenderingAPIException,
				"Cannot create D3D11 compute shader from microcode.\nError Description:" + errorDescription);
		}
	}

	ID3D11ComputeShader* D3D11GpuComputeProgram::getComputeShader() const
	{
		return mComputeShader;
	}
}}
