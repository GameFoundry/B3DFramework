//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11HLSLProgramFactory.h"
#include "BsD3D11GpuProgram.h"
#include "RenderAPI/BsVertexDeclaration.h"
#include "BsD3D11HLSLParamParser.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include <regex>

namespace bs { namespace ct
{
	D3D11HLSLProgramFactory::D3D11HLSLProgramFactory()
	{ }

	D3D11HLSLProgramFactory::~D3D11HLSLProgramFactory()
	{ }

	SPtr<GpuProgram> D3D11HLSLProgramFactory::create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
	{
		SPtr<GpuProgram> gpuProg;

		switch (desc.type)
		{
		case GPT_VERTEX_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuVertexProgram>(new (bs_alloc<D3D11GpuVertexProgram>())
				D3D11GpuVertexProgram(desc, deviceMask));
			break;
		case GPT_FRAGMENT_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuFragmentProgram>(new (bs_alloc<D3D11GpuFragmentProgram>())
				D3D11GpuFragmentProgram(desc, deviceMask));
			break;
		case GPT_HULL_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuHullProgram>(new (bs_alloc<D3D11GpuHullProgram>())
				D3D11GpuHullProgram(desc, deviceMask));
			break;
		case GPT_DOMAIN_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuDomainProgram>(new (bs_alloc<D3D11GpuDomainProgram>())
				D3D11GpuDomainProgram(desc, deviceMask));
			break;
		case GPT_GEOMETRY_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuGeometryProgram>(new (bs_alloc<D3D11GpuGeometryProgram>())
				D3D11GpuGeometryProgram(desc, deviceMask));
			break;
		case GPT_COMPUTE_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuComputeProgram>(new (bs_alloc<D3D11GpuComputeProgram>())
				D3D11GpuComputeProgram(desc, deviceMask));
			break;
		}

		if (gpuProg != nullptr)
			gpuProg->_setThisPtr(gpuProg);

		return gpuProg;
	}

	SPtr<GpuProgram> D3D11HLSLProgramFactory::create(GpuProgramType type, GpuDeviceFlags deviceMask)
	{
		SPtr<GpuProgram> gpuProg;

		GPU_PROGRAM_DESC desc;
		desc.type = type;

		switch (type)
		{
		case GPT_VERTEX_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuVertexProgram>(new (bs_alloc<D3D11GpuVertexProgram>())
				D3D11GpuVertexProgram(desc, deviceMask));
			break;
		case GPT_FRAGMENT_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuFragmentProgram>(new (bs_alloc<D3D11GpuFragmentProgram>())
				D3D11GpuFragmentProgram(desc, deviceMask));
			break;
		case GPT_HULL_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuHullProgram>(new (bs_alloc<D3D11GpuHullProgram>())
				D3D11GpuHullProgram(desc, deviceMask));
			break;
		case GPT_DOMAIN_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuDomainProgram>(new (bs_alloc<D3D11GpuDomainProgram>())
				D3D11GpuDomainProgram(desc, deviceMask));
			break;
		case GPT_GEOMETRY_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuGeometryProgram>(new (bs_alloc<D3D11GpuGeometryProgram>())
				D3D11GpuGeometryProgram(desc, deviceMask));
			break;
		case GPT_COMPUTE_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuComputeProgram>(new (bs_alloc<D3D11GpuComputeProgram>())
				D3D11GpuComputeProgram(desc, deviceMask));
			break;
		}

		if (gpuProg != nullptr)
			gpuProg->_setThisPtr(gpuProg);

		return gpuProg;
	}

	UINT32 parseErrorMessage(const char* message)
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

	SPtr<GpuProgramBytecode> D3D11HLSLProgramFactory::compileBytecode(const GPU_PROGRAM_DESC& desc)
	{
		String hlslProfile;
		switch(desc.type)
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
		default:
			break;
		}

		UINT compileFlags = 0;
#if defined(BS_DEBUG_MODE)
		compileFlags |= D3DCOMPILE_DEBUG;
		compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

		ID3DBlob* microcode = nullptr;
		ID3DBlob* messages = nullptr;

		const String& source = desc.source;
		const String& entryPoint = desc.entryPoint;

		const D3D_SHADER_MACRO defines[] = 
		{ 
			{ "HLSL", "1" },
			{ nullptr, nullptr }
		};

		HRESULT hr = D3DCompile(
			source.c_str(),		// [in] Pointer to the shader in memory. 
			source.size(),		// [in] Size of the shader in memory.  
			nullptr,			// [in] The name of the file that contains the shader code. 
			defines,			// [in] Optional. Pointer to a NULL-terminated array of macro definitions. 
								//		See D3D_SHADER_MACRO. If not used, set this to NULL. 
			nullptr,			// [in] Optional. Pointer to an ID3DInclude Interface interface for handling include files. 
								//		Setting this to NULL will cause a compile error if a shader contains a #include. 
			entryPoint.c_str(),	// [in] Name of the shader-entrypoint function where shader execution begins. 
			hlslProfile.c_str(),// [in] A string that specifies the shader model; can be any profile in shader model 4 or higher. 
			compileFlags,		// [in] Effect compile flags - no D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY at the first try...
			0,					// [in] Effect compile flags
			&microcode,			// [out] A pointer to an ID3DBlob Interface which contains the compiled shader, as well as
								//		 any embedded debug and symbol-table information. 
			&messages			// [out] A pointer to an ID3DBlob Interface which contains a listing of errors and warnings
								//		 that occurred during compilation. These errors and warnings are identical to the 
								//		 debug output from a debugger.
			);

		String compileMessage;
		if (messages != nullptr)
		{
			const char* message = static_cast<const char*>(messages->GetBufferPointer());
			UINT32 lineIdx = parseErrorMessage(message);

			Vector<String> sourceLines = StringUtil::split(source, "\n");
			String sourceLine;
			if (lineIdx < sourceLines.size())
				sourceLine = sourceLines[lineIdx];

			compileMessage =
				String(message) + "\n" +
				"\n" +
				"Line " + toString(lineIdx) + ": " + sourceLine;

			SAFE_RELEASE(messages);
		}

		SPtr<GpuProgramBytecode> bytecode = bs_shared_ptr_new<GpuProgramBytecode>();
		bytecode->compilerId = DIRECTX_COMPILER_ID;
		bytecode->messages = compileMessage;

		if (FAILED(hr))
		{
			SAFE_RELEASE(microcode);
			return bytecode;
		}

		if (microcode != nullptr)
		{
			bytecode->instructions.size = (UINT32)microcode->GetBufferSize();
			bytecode->instructions.data = (UINT8*)bs_alloc(bytecode->instructions.size);

			memcpy(bytecode->instructions.data, microcode->GetBufferPointer(), bytecode->instructions.size);

			D3D11HLSLParamParser parser;
			bytecode->paramDesc = bs_shared_ptr_new<GpuParamDesc>();

			if (desc.type == GPT_VERTEX_PROGRAM)
				parser.parse(microcode, desc.type, *bytecode->paramDesc, &bytecode->vertexInput);
			else
				parser.parse(microcode, desc.type, *bytecode->paramDesc, nullptr);
		}
		
		SAFE_RELEASE(microcode);
		return bytecode;
	}
}}
