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

using namespace bs;
using namespace bs::ct;

u32 D3D11GpuProgram::GlobalProgramId = 0;

D3D11GpuProgram::D3D11GpuProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask)
	: GpuProgram(desc, deviceMask)
{
	B3D_ASSERT((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on DirectX 11.");
}

D3D11GpuProgram::~D3D11GpuProgram()
{
	mInputDeclaration = nullptr;

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_GpuProgram);
}

void D3D11GpuProgram::Initialize()
{
	if(!IsSupported())
	{
		mIsCompiled = false;
		mCompileMessages = "Specified program is not supported by the current render system.";

		GpuProgram::Initialize();
		return;
	}

	if(!mBytecode || mBytecode->CompilerId != DIRECTX_COMPILER_ID)
	{
		GpuProgramCreateInformation desc;
		desc.Name = mName;
		desc.Type = mType;
		desc.EntryPoint = mEntryPoint;
		desc.Source = mSource;
		desc.Language = "hlsl";

		mBytecode = CompileBytecode(desc);
	}

	mCompileMessages = mBytecode->Messages;
	mIsCompiled = mBytecode->Instructions.Data != nullptr;

	if(mIsCompiled)
	{
		mParametersDesc = mBytecode->ParamDesc;

		D3D11RenderAPI* rapi = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
		LoadFromMicrocode(rapi->GetPrimaryDevice(), mBytecode->Instructions);

		if(mType == GPT_VERTEX_PROGRAM)
			mInputDeclaration = HardwareBufferManager::Instance().CreateVertexDeclaration(mBytecode->VertexInput);
	}

	mProgramId = GlobalProgramId++;

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_GpuProgram);

	GpuProgram::Initialize();
}

D3D11GpuVertexProgram::D3D11GpuVertexProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask)
	: D3D11GpuProgram(desc, deviceMask), mVertexShader(nullptr)
{}

D3D11GpuVertexProgram::~D3D11GpuVertexProgram()
{
	SAFE_RELEASE(mVertexShader);
}

void D3D11GpuVertexProgram::LoadFromMicrocode(D3D11Device& device, const DataBlob& microcode)
{
	HRESULT hr = device.GetD3D11Device()->CreateVertexShader(
		microcode.Data, microcode.Size, device.GetClassLinkage(), &mVertexShader);

	if(FAILED(hr) || device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "Cannot create D3D11 vertex shader from microcode\nError Description:" + errorDescription);
	}
}

ID3D11VertexShader* D3D11GpuVertexProgram::GetVertexShader() const
{
	return mVertexShader;
}

D3D11GpuFragmentProgram::D3D11GpuFragmentProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask)
	: D3D11GpuProgram(desc, deviceMask), mPixelShader(nullptr)
{}

D3D11GpuFragmentProgram::~D3D11GpuFragmentProgram()
{
	SAFE_RELEASE(mPixelShader);
}

void D3D11GpuFragmentProgram::LoadFromMicrocode(D3D11Device& device, const DataBlob& microcode)
{
	HRESULT hr = device.GetD3D11Device()->CreatePixelShader(
		microcode.Data, microcode.Size, device.GetClassLinkage(), &mPixelShader);

	if(FAILED(hr) || device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "Cannot create D3D11 pixel shader from microcode.\nError Description:" + errorDescription);
	}
}

ID3D11PixelShader* D3D11GpuFragmentProgram::GetPixelShader() const
{
	return mPixelShader;
}

D3D11GpuGeometryProgram::D3D11GpuGeometryProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask)
	: D3D11GpuProgram(desc, deviceMask), mGeometryShader(nullptr)
{}

D3D11GpuGeometryProgram::~D3D11GpuGeometryProgram()
{
	SAFE_RELEASE(mGeometryShader);
}

void D3D11GpuGeometryProgram::LoadFromMicrocode(D3D11Device& device, const DataBlob& microcode)
{
	HRESULT hr = device.GetD3D11Device()->CreateGeometryShader(
		microcode.Data, microcode.Size, device.GetClassLinkage(), &mGeometryShader);

	if(FAILED(hr) || device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "Cannot create D3D11 geometry shader from microcode.\nError Description:" + errorDescription);
	}
}

ID3D11GeometryShader* D3D11GpuGeometryProgram::GetGeometryShader() const
{
	return mGeometryShader;
}

D3D11GpuDomainProgram::D3D11GpuDomainProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask)
	: D3D11GpuProgram(desc, deviceMask), mDomainShader(nullptr)
{}

D3D11GpuDomainProgram::~D3D11GpuDomainProgram()
{
	SAFE_RELEASE(mDomainShader);
}

void D3D11GpuDomainProgram::LoadFromMicrocode(D3D11Device& device, const DataBlob& microcode)
{
	HRESULT hr = device.GetD3D11Device()->CreateDomainShader(
		microcode.Data, microcode.Size, device.GetClassLinkage(), &mDomainShader);

	if(FAILED(hr) || device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "Cannot create D3D11 domain shader from microcode.\nError Description:" + errorDescription);
	}
}

ID3D11DomainShader* D3D11GpuDomainProgram::GetDomainShader() const
{
	return mDomainShader;
}

D3D11GpuHullProgram::D3D11GpuHullProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask)
	: D3D11GpuProgram(desc, deviceMask), mHullShader(nullptr)
{}

D3D11GpuHullProgram::~D3D11GpuHullProgram()
{
	SAFE_RELEASE(mHullShader);
}

void D3D11GpuHullProgram::LoadFromMicrocode(D3D11Device& device, const DataBlob& microcode)
{
	// Create the shader
	HRESULT hr = device.GetD3D11Device()->CreateHullShader(
		microcode.Data, microcode.Size, device.GetClassLinkage(), &mHullShader);

	if(FAILED(hr) || device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "Cannot create D3D11 hull shader from microcode.\nError Description:" + errorDescription);
	}
}

ID3D11HullShader* D3D11GpuHullProgram::GetHullShader() const
{
	return mHullShader;
}

D3D11GpuComputeProgram::D3D11GpuComputeProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask)
	: D3D11GpuProgram(desc, deviceMask), mComputeShader(nullptr)
{}

D3D11GpuComputeProgram::~D3D11GpuComputeProgram()
{
	SAFE_RELEASE(mComputeShader);
}

void D3D11GpuComputeProgram::LoadFromMicrocode(D3D11Device& device, const DataBlob& microcode)
{
	HRESULT hr = device.GetD3D11Device()->CreateComputeShader(
		microcode.Data, microcode.Size, device.GetClassLinkage(), &mComputeShader);

	if(FAILED(hr) || device.HasError())
	{
		String errorDescription = device.GetErrorDescription();
		B3D_EXCEPT(RenderingAPIException, "Cannot create D3D11 compute shader from microcode.\nError Description:" + errorDescription);
	}
}

ID3D11ComputeShader* D3D11GpuComputeProgram::GetComputeShader() const
{
	return mComputeShader;
}
