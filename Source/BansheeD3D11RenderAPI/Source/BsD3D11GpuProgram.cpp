//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11GpuProgram.h"
#include "BsD3D11Device.h"
#include "BsException.h"
#include "BsDebug.h"
#include "BsGpuParams.h"
#include "BsD3D11RenderAPI.h"
#include "BsGpuProgramManager.h"
#include "BsHardwareBufferManager.h"
#include "BsD3D11HLSLParamParser.h"
#include "BsRenderStats.h"
#include <regex>

namespace bs { namespace ct
{
	UINT32 D3D11GpuProgram::GlobalProgramId = 0;

	D3D11GpuProgram::D3D11GpuProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuProgram(desc, deviceMask),
		mEnableBackwardsCompatibility(false), mProgramId(0)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on DirectX 11.");
	}

	D3D11GpuProgram::~D3D11GpuProgram()
	{
		mMicrocode.clear();
		mInputDeclaration = nullptr;

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuProgram);
	}

	void D3D11GpuProgram::initialize()
	{
		if (!isSupported())
		{
			mIsCompiled = false;
			mCompileError = "Specified program is not supported by the current render system.";

			GpuProgram::initialize();
			return;
		}

		String hlslProfile;
		switch(mProperties.getType())
		{
		case GPT_FRAGMENT_PROGRAM:
			hlslProfile = "ps_5_0";
			break;
		case GPT_VERTEX_PROGRAM:
			hlslProfile = "vs_5_0";
			break;
		case GPT_GEOMETRY_PROGRAM:
			hlslProfile = "gs_5_0";
			break;
		case GPT_COMPUTE_PROGRAM:
			hlslProfile = "cs_5_0";
			break;
		case GPT_HULL_PROGRAM:
			hlslProfile = "hs_5_0";
			break;
		case GPT_DOMAIN_PROGRAM:
			hlslProfile = "ds_5_0";
			break;
		}

		D3D11RenderAPI* rapi = static_cast<D3D11RenderAPI*>(RenderAPI::instancePtr());

		ID3DBlob* microcode = compileMicrocode(hlslProfile);
		if (microcode != nullptr)
		{
			mMicrocode.resize(microcode->GetBufferSize());
			memcpy(&mMicrocode[0], microcode->GetBufferPointer(), microcode->GetBufferSize());

			populateParametersAndConstants(microcode);
			loadFromMicrocode(rapi->getPrimaryDevice(), microcode);

			SAFE_RELEASE(microcode);
		}

		mProgramId = GlobalProgramId++;

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuProgram);

		GpuProgram::initialize();
	}

	UINT32 D3D11GpuProgram::parseErrorMessage(const char* message)
	{
		if (message == nullptr)
			return 0;

		String pattern = R"(\(([0-9]*),.*\))";
		std::regex regex(pattern);

		std::cmatch results;
		if (std::regex_search(message, results, regex))
		{
			std::string result = results[1].str();

			return strtol(result.c_str(), nullptr, 10) - 1;
		}

		return 0;
	}

	ID3DBlob* D3D11GpuProgram::compileMicrocode(const String& profile)
	{
		UINT compileFlags = 0;
#if defined(BS_DEBUG_MODE)
		compileFlags |= D3DCOMPILE_DEBUG;
		compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

		if (mEnableBackwardsCompatibility)
			compileFlags |= D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;

		ID3DBlob* microCode = nullptr;
		ID3DBlob* errors = nullptr;

		const String& source = mProperties.getSource();
		const String& entryPoint = mProperties.getEntryPoint();

		const D3D_SHADER_MACRO defines[] = 
		{ 
			{ "HLSL", "1" },
			{ nullptr, nullptr }
		};

		HRESULT hr = D3DCompile(
			source.c_str(),		// [in] Pointer to the shader in memory. 
			source.size(),		// [in] Size of the shader in memory.  
			nullptr,			// [in] The name of the file that contains the shader code. 
			defines,			// [in] Optional. Pointer to a NULL-terminated array of macro definitions. See D3D_SHADER_MACRO. If not used, set this to NULL. 
			nullptr,			// [in] Optional. Pointer to an ID3DInclude Interface interface for handling include files. Setting this to NULL will cause a compile error if a shader contains a #include. 
			entryPoint.c_str(),	// [in] Name of the shader-entrypoint function where shader execution begins. 
			profile.c_str(),// [in] A string that specifies the shader model; can be any profile in shader model 4 or higher. 
			compileFlags,		// [in] Effect compile flags - no D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY at the first try...
			0,					// [in] Effect compile flags
			&microCode,			// [out] A pointer to an ID3DBlob Interface which contains the compiled shader, as well as any embedded debug and symbol-table information. 
			&errors				// [out] A pointer to an ID3DBlob Interface which contains a listing of errors and warnings that occurred during compilation. These errors and warnings are identical to the the debug output from a debugger.
			);

		if (FAILED(hr))
		{
			const char* errorMessage = static_cast<const char*>(errors->GetBufferPointer());
			UINT32 errorLineIdx = parseErrorMessage(errorMessage);

			Vector<String> sourceLines = StringUtil::split(source, "\n");
			String sourceLine;
			if (errorLineIdx < sourceLines.size())
				sourceLine = sourceLines[errorLineIdx];

			mIsCompiled = false;
			mCompileError = "Cannot compile D3D11 high-level shader. Errors:\n" +
				String(errorMessage) + "\n" +
				"\n" +
				"Line " + toString(errorLineIdx) + ": " + sourceLine;

			SAFE_RELEASE(microCode);
			SAFE_RELEASE(errors);
			return nullptr;
		}
		else
		{
			mIsCompiled = true;
			mCompileError = "";

			SAFE_RELEASE(errors);
			return microCode;
		}
	}

	void D3D11GpuProgram::populateParametersAndConstants(ID3DBlob* microcode)
	{
		assert(microcode != nullptr);

		D3D11HLSLParamParser parser;
		if (mProperties.getType() == GPT_VERTEX_PROGRAM)
		{
			List<VertexElement> inputParams;
			parser.parse(microcode, mProperties.getType(), *mParametersDesc, &inputParams);

			mInputDeclaration = HardwareBufferManager::instance().createVertexDeclaration(inputParams);
		}
		else
		{
			parser.parse(microcode, mProperties.getType(), *mParametersDesc, nullptr);
		}
	}

	D3D11GpuVertexProgram::D3D11GpuVertexProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		: D3D11GpuProgram(desc, deviceMask), mVertexShader(nullptr)
	{ }

	D3D11GpuVertexProgram::~D3D11GpuVertexProgram()
	{
		SAFE_RELEASE(mVertexShader);
	}

	void D3D11GpuVertexProgram::loadFromMicrocode(D3D11Device& device, ID3D10Blob*  microcode)
	{
		HRESULT hr = device.getD3D11Device()->CreateVertexShader( 
			static_cast<DWORD*>(microcode->GetBufferPointer()), microcode->GetBufferSize(),
			device.getClassLinkage(), &mVertexShader);

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

	void D3D11GpuFragmentProgram::loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode)
	{
		HRESULT hr = device.getD3D11Device()->CreatePixelShader(
			static_cast<DWORD*>(microcode->GetBufferPointer()), microcode->GetBufferSize(),
			device.getClassLinkage(), &mPixelShader);

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

	void D3D11GpuGeometryProgram::loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode)
	{
		HRESULT hr = device.getD3D11Device()->CreateGeometryShader(
			static_cast<DWORD*>(microcode->GetBufferPointer()), microcode->GetBufferSize(),
			device.getClassLinkage(), &mGeometryShader);

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

	void D3D11GpuDomainProgram::loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode)
	{
		HRESULT hr = device.getD3D11Device()->CreateDomainShader(
			static_cast<DWORD*>(microcode->GetBufferPointer()), microcode->GetBufferSize(),
			device.getClassLinkage(), &mDomainShader);

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

	void D3D11GpuHullProgram::loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode)
	{
		// Create the shader
		HRESULT hr = device.getD3D11Device()->CreateHullShader(
			static_cast<DWORD*>(microcode->GetBufferPointer()), microcode->GetBufferSize(),
			device.getClassLinkage(), &mHullShader);

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

	void D3D11GpuComputeProgram::loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode)
	{
		HRESULT hr = device.getD3D11Device()->CreateComputeShader(
			static_cast<DWORD*>(microcode->GetBufferPointer()), microcode->GetBufferSize(),
			device.getClassLinkage(), &mComputeShader);

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