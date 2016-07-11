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

namespace BansheeEngine
{
	UINT32 D3D11GpuProgramCore::GlobalProgramId = 0;

	D3D11GpuProgramCore::D3D11GpuProgramCore(const String& source, const String& entryPoint, GpuProgramType gptype,
		GpuProgramProfile profile, bool isAdjacencyInfoRequired)
		: GpuProgramCore(source, entryPoint, gptype, profile, isAdjacencyInfoRequired),
		mEnableBackwardsCompatibility(false), mProgramId(0)
	{

	}

	D3D11GpuProgramCore::~D3D11GpuProgramCore()
	{
		mMicrocode.clear();
		mInputDeclaration = nullptr;

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuProgram);
	}

	void D3D11GpuProgramCore::initialize()
	{
		if (!isSupported())
		{
			mIsCompiled = false;
			mCompileError = "Specified program is not supported by the current render system.";

			GpuProgramCore::initialize();
			return;
		}

		D3D11RenderAPI* rs = static_cast<D3D11RenderAPI*>(RenderAPICore::instancePtr());
		String hlslProfile = rs->getCapabilities()->gpuProgProfileToRSSpecificProfile(mProperties.getProfile());

		ID3DBlob* microcode = compileMicrocode(hlslProfile);

		if (microcode != nullptr)
		{
			mMicrocode.resize(microcode->GetBufferSize());
			memcpy(&mMicrocode[0], microcode->GetBufferPointer(), microcode->GetBufferSize());

			populateParametersAndConstants(microcode);
			loadFromMicrocode(rs->getPrimaryDevice(), microcode);

			SAFE_RELEASE(microcode);
		}

		mProgramId = GlobalProgramId++;

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuProgram);

		GpuProgramCore::initialize();
	}

	UINT32 D3D11GpuProgramCore::parseErrorMessage(const char* message)
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

	ID3DBlob* D3D11GpuProgramCore::compileMicrocode(const String& profile)
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

		HRESULT hr = D3DCompile(
			source.c_str(),		// [in] Pointer to the shader in memory. 
			source.size(),		// [in] Size of the shader in memory.  
			nullptr,			// [in] The name of the file that contains the shader code. 
			nullptr,			// [in] Optional. Pointer to a NULL-terminated array of macro definitions. See D3D_SHADER_MACRO. If not used, set this to NULL. 
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

	void D3D11GpuProgramCore::populateParametersAndConstants(ID3DBlob* microcode)
	{
		assert(microcode != nullptr);

		D3D11HLSLParamParser parser;
		if (mProperties.getType() == GPT_VERTEX_PROGRAM)
		{
			List<VertexElement> inputParams;
			parser.parse(microcode, *mParametersDesc, &inputParams);

			mInputDeclaration = HardwareBufferCoreManager::instance().createVertexDeclaration(inputParams);
		}
		else
		{
			parser.parse(microcode, *mParametersDesc, nullptr);
		}
	}

	D3D11GpuVertexProgramCore::D3D11GpuVertexProgramCore(const String& source, const String& entryPoint,
		GpuProgramProfile profile)
		: D3D11GpuProgramCore(source, entryPoint, GPT_VERTEX_PROGRAM, profile, false)
		, mVertexShader(nullptr)
	{ }

	D3D11GpuVertexProgramCore::~D3D11GpuVertexProgramCore()
	{
		SAFE_RELEASE(mVertexShader);
	}

	void D3D11GpuVertexProgramCore::loadFromMicrocode(D3D11Device& device, ID3D10Blob*  microcode)
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

	ID3D11VertexShader * D3D11GpuVertexProgramCore::getVertexShader() const
	{
		return mVertexShader;
	}

	D3D11GpuFragmentProgramCore::D3D11GpuFragmentProgramCore(const String& source, const String& entryPoint,
		GpuProgramProfile profile)
		: D3D11GpuProgramCore(source, entryPoint, GPT_FRAGMENT_PROGRAM, profile, false)
		, mPixelShader(nullptr)
	{ }

	D3D11GpuFragmentProgramCore::~D3D11GpuFragmentProgramCore()
	{
		SAFE_RELEASE(mPixelShader);
	}

	void D3D11GpuFragmentProgramCore::loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode)
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

	ID3D11PixelShader * D3D11GpuFragmentProgramCore::getPixelShader() const
	{
		return mPixelShader;
	}


	D3D11GpuGeometryProgramCore::D3D11GpuGeometryProgramCore(const String& source, const String& entryPoint,
		GpuProgramProfile profile, bool isAdjacencyInfoRequired)
		: D3D11GpuProgramCore(source, entryPoint, GPT_GEOMETRY_PROGRAM, profile, isAdjacencyInfoRequired)
		, mGeometryShader(nullptr)
	{ }

	D3D11GpuGeometryProgramCore::~D3D11GpuGeometryProgramCore()
	{ 
		SAFE_RELEASE(mGeometryShader);
	}

	void D3D11GpuGeometryProgramCore::loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode)
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

	ID3D11GeometryShader * D3D11GpuGeometryProgramCore::getGeometryShader() const
	{
		return mGeometryShader;
	}

	D3D11GpuDomainProgramCore::D3D11GpuDomainProgramCore(const String& source, const String& entryPoint,
		GpuProgramProfile profile)
		: D3D11GpuProgramCore(source, entryPoint, GPT_DOMAIN_PROGRAM, profile, false)
		, mDomainShader(nullptr)
	{ }

	D3D11GpuDomainProgramCore::~D3D11GpuDomainProgramCore()
	{ 
		SAFE_RELEASE(mDomainShader);
	}

	void D3D11GpuDomainProgramCore::loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode)
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

	ID3D11DomainShader * D3D11GpuDomainProgramCore::getDomainShader() const
	{
		return mDomainShader;
	}

	D3D11GpuHullProgramCore::D3D11GpuHullProgramCore(const String& source, const String& entryPoint,
		GpuProgramProfile profile)
		: D3D11GpuProgramCore(source, entryPoint, GPT_HULL_PROGRAM, profile, false)
		, mHullShader(nullptr)
	{ }

	D3D11GpuHullProgramCore::~D3D11GpuHullProgramCore()
	{ 
		SAFE_RELEASE(mHullShader);
	}

	void D3D11GpuHullProgramCore::loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode)
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

	ID3D11HullShader* D3D11GpuHullProgramCore::getHullShader() const
	{
		return mHullShader;
	}


	D3D11GpuComputeProgramCore::D3D11GpuComputeProgramCore(const String& source, const String& entryPoint,
		GpuProgramProfile profile)
		: D3D11GpuProgramCore(source, entryPoint, GPT_COMPUTE_PROGRAM, profile, false)
		, mComputeShader(nullptr)
	{ }

	D3D11GpuComputeProgramCore::~D3D11GpuComputeProgramCore()
	{
		SAFE_RELEASE(mComputeShader);
	}

	void D3D11GpuComputeProgramCore::loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode)
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

	ID3D11ComputeShader* D3D11GpuComputeProgramCore::getComputeShader() const
	{
		return mComputeShader;
	}
}