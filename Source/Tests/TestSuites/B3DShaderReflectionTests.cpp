//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DCoreTestSuite.h"
#include "Material/B3DShader.h"
#include "Material/B3DMaterial.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "CoreObject/B3DRenderThread.h"
#include "Material/B3DShaderReflection.h"
#include "Material/B3DShaderCompiler.h"
#include "Material/B3DShaderRegistry.h"
#include "Resources/B3DPackage.h"
#include "Material/B3DPass.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "Serialization/B3DBinarySerializer.h"
#include "FileSystem/B3DDataStream.h"

using namespace b3d;

void CoreTestSuite::TestShaderRegistryCacheVersion()
{
	B3D_TEST_ASSERT(ShaderRegistry::GetShaderCacheName("Renderer/", "Test") == "Renderer/Test/")

	const TShared<Package> package = Package::Create("ShaderRegistryCacheVersionTest");
	B3D_TEST_ASSERT(!ShaderRegistry::DoesPrebuiltStoreVersionMatch(*package))
	package->SetPackageMetaData(B3DMakeShared<PackageMetaData>());
	B3D_TEST_ASSERT(!ShaderRegistry::DoesPrebuiltStoreVersionMatch(*package))

	const Path shaderPath = ShaderRegistry::GetShaderMetaDataPath("Renderer/Test/", "vksl");
	package->AddResource(shaderPath, PrebuiltShader::Create(nullptr));
	for(u32 version : { 0u, ShaderRegistry::kCacheVersion - 1, ShaderRegistry::kCacheVersion, ShaderRegistry::kCacheVersion + 1 })
	{
		const TShared<ShaderRegistryMetaData> metaData = B3DMakeShared<ShaderRegistryMetaData>();
		metaData->Version = version;
		metaData->IncludePackageNameInVirtualPath = true;
		package->SetPackageMetaData(metaData);

		// Check compatibility from package metadata alone, without deserializing shader resources.
		const TShared<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
		B3D_TEST_ASSERT(package->Save(stream, SavePackageOptions()))
		stream->Seek(0);
		const TShared<Package> restoredPackage = Package::Load(stream);
		B3D_TEST_ASSERT(restoredPackage != nullptr)
		if(restoredPackage == nullptr)
			return;

		B3D_TEST_ASSERT(ShaderRegistry::DoesPrebuiltStoreVersionMatch(*restoredPackage) == (version == ShaderRegistry::kCacheVersion))
		const TShared<ShaderRegistryMetaData> restoredMetaData = B3DRTTICast<ShaderRegistryMetaData>(restoredPackage->GetPackageMetaData());
		B3D_TEST_ASSERT(restoredMetaData != nullptr)
		if(restoredMetaData == nullptr)
			return;

		B3D_TEST_ASSERT(restoredMetaData->Version == version)
		B3D_TEST_ASSERT(restoredMetaData->IncludePackageNameInVirtualPath)
		B3D_TEST_ASSERT(restoredPackage->Contains(shaderPath))
	}
}

#if !B3D_PLATFORM_PS5
void CoreTestSuite::TestMaterialParameterAdapterCompilation()
{
	const TShared<IShaderCompiler> compiler = ShaderCompilers::Instance().GetCompiler("bsl");
	B3D_TEST_ASSERT(compiler != nullptr)
	if(compiler == nullptr)
		return;

	const String source = R"(
shader MaterialAdapterCompilationTest
{
	pass { };
	code
	{
		float4 vsmain(float3 position : POSITION) : SV_Position { return float4(position, 1); }
		float4 fsmain() : SV_Target0 { return float4(1, 1, 1, 1); }
	};
};
)";
	TShared<Shader> shader;
	const String language = ShaderCompilers::Instance().DetectActiveShadingLanguage();
	const ShaderCompilerResult result = compiler->Compile("MaterialAdapterCompilationTest", source, {}, { language }, true, shader);
	B3D_TEST_ASSERT_MSG(result.ErrorMessage.empty(), result.ErrorMessage)
	B3D_TEST_ASSERT(shader != nullptr)
	if(shader == nullptr)
		return;

	GetRenderThread().PostCommand([this, renderShader = B3DGetRenderProxy(shader)]()
	{
		const TShared<render::Material> material = render::Material::Create(renderShader);
		const TShared<render::Variation>& variation = material->GetVariation(0);
		B3D_TEST_ASSERT(!variation->IsCompiled())
		{
			LoggingScope logs(*this);
			logs.ExpectError("compile the variation first");
			B3D_TEST_ASSERT(material->CreateParameterAdapter() == nullptr)
		}
		B3D_TEST_ASSERT(!variation->IsCompiled())

		const TAsyncOp<bool> compilation = variation->Compile();
		compilation.BlockUntilComplete();
		B3D_TEST_ASSERT(compilation.GetReturnValue())
		B3D_TEST_ASSERT(material->CreateParameterAdapter() != nullptr)
	}, "Test explicit material variation compilation", true);
}

void CoreTestSuite::TestShaderReflectionCompilation()
{
	const TShared<IShaderCompiler> compiler = ShaderCompilers::Instance().GetCompiler("bsl");
	B3D_TEST_ASSERT(compiler != nullptr)
	if(compiler == nullptr)
		return;

	const String source = R"(
shader ShaderReflectionTest
{
	pass { };
	pass { };
	code
	{
		[dynamicOffset] cbuffer Parameters : register(c3, space2) { float Value = 0.25; };
		cbuffer StaticParameters : register(c1, space2) { float Scale; };
		[internal] [dynamicOffset] cbuffer InternalParameters : register(c0, space1) { float Bias; };
		RWStructuredBuffer<float> Output;
		[numthreads(8, 4, 2)]
		void csmain(uint3 id : SV_DispatchThreadID) { Output[id.x] = Value * Scale + Bias; }
	};
};
)";
	Vector<String> languages = { "vksl" };
#if B3D_PLATFORM_WIN32
	languages.push_back("hlsl");
#elif B3D_PLATFORM_MACOS
	languages.push_back("msl");
#endif

	TShared<Shader> shader;
	const ShaderCompilerResult result = compiler->Compile("ShaderReflectionTest", source, {}, languages, true, shader);
	B3D_TEST_ASSERT_MSG(result.ErrorMessage.empty(), result.ErrorMessage)
	B3D_TEST_ASSERT(shader != nullptr)
	if(shader == nullptr)
		return;

	const ShaderDataParameterInformation* parameter = shader->GetDataParameterDescription("Value");
	B3D_TEST_ASSERT(parameter != nullptr && parameter->DefaultValueIndex != ~0u)
	if(parameter != nullptr && parameter->DefaultValueIndex != ~0u)
	{
		float value;
		memcpy(&value, shader->GetDefaultValue(parameter->DefaultValueIndex), sizeof(value));
		B3D_TEST_ASSERT(value == 0.25f)
	}
	B3D_TEST_ASSERT(shader->GetPrecompiledData()->Description->Parameters->GetDataDefaultValues().size() == sizeof(float))
	B3D_TEST_ASSERT(shader->GetVariations().size() == languages.size())
	for(const TShared<Variation>& variation : shader->GetVariations())
	{
		B3D_TEST_ASSERT(variation->GetPassCount() == 2)
		for(u32 passIndex = 0; passIndex < variation->GetPassCount(); passIndex++)
		{
			const GpuProgramCreateInformation& program = variation->GetPass(passIndex)->GetGpuProgramCreateInformation(GPT_COMPUTE_PROGRAM);
			B3D_TEST_ASSERT(program.ShaderReflection != nullptr)
			B3D_TEST_ASSERT(program.Bytecode != nullptr && program.Bytecode->Instructions.Data != nullptr)
			if(program.Bytecode == nullptr || program.Bytecode->ParameterDescription == nullptr)
				continue;

			const ShaderEntryPointReflection& entry = program.GetEntryPointReflection();
			const Array<u32, 3> expectedThreadGroupSize = { 8, 4, 2 };
			B3D_TEST_ASSERT(entry.Type == GPT_COMPUTE_PROGRAM)
			B3D_TEST_ASSERT(entry.ThreadGroupSize == expectedThreadGroupSize)
			// Vulkan uses the workgroup size encoded in SPIR-V without populating this bytecode metadata.
			if(program.Language != "vksl")
				B3D_TEST_ASSERT(program.Bytecode->ThreadGroupSize == expectedThreadGroupSize)
			const auto& parameters = *program.ShaderReflection->Parameters;
			B3D_TEST_ASSERT(parameters.GetUniformBuffers().size() == 3)
			for(const auto& uniformBuffer : parameters.GetUniformBuffers())
			{
				const ShaderUniformBufferInformation& binding = uniformBuffer.second;
				const auto found = program.Bytecode->ParameterDescription->UniformBuffers.find(binding.Name);
				B3D_TEST_ASSERT(found != program.Bytecode->ParameterDescription->UniformBuffers.end())
				if(found != program.Bytecode->ParameterDescription->UniformBuffers.end())
				{
					B3D_TEST_ASSERT(found->second.Set == binding.Set)
					// HLSL source bindings are register numbers; native reflection additionally encodes the register class.
					if(program.Language != "hlsl")
						B3D_TEST_ASSERT(found->second.Slot == binding.Slot)
					B3D_TEST_ASSERT(binding.UsesDynamicOffset == (binding.Name != "StaticParameters"))
					B3D_TEST_ASSERT(found->second.UsesDynamicOffset == binding.UsesDynamicOffset)
				}
			}

			BinarySerializer serializer;
			const TShared<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
			serializer.Encode(program.Bytecode.get(), stream);
			stream->Seek(0);
			const auto decoded = B3DRTTICast<GpuProgramBytecode>(serializer.Decode(stream, (u32)stream->Size()));
			B3D_TEST_ASSERT(decoded != nullptr && decoded->ParameterDescription != nullptr)
			if(decoded != nullptr && decoded->ParameterDescription != nullptr)
			{
				B3D_TEST_ASSERT(decoded->ParameterDescription->UniformBuffers.at("Parameters").UsesDynamicOffset)
				B3D_TEST_ASSERT(decoded->ParameterDescription->UniformBuffers.at("InternalParameters").UsesDynamicOffset)
				B3D_TEST_ASSERT(!decoded->ParameterDescription->UniformBuffers.at("StaticParameters").UsesDynamicOffset)
			}
		}
	}

	const String graphicsSource = R"(
shader SharedStageReflectionTest
{
	variations { MODE = { 0, 1 }; };
	code
	{
		cbuffer VertexParameters { float4 Position; };
		cbuffer FragmentParameters { float4 Color; };
		[internal] cbuffer InternalParameters { float4 InternalValue; };
		[internal] Texture2D InternalTexture;
		float4 vsmain() : SV_Position { return Position; }
		float4 fsmain() : SV_Target0
		{
			#if MODE
			return Color * 0.5;
			#else
			return Color;
			#endif
		}
	};
};
)";
	TShared<Shader> graphicsShader;
	const ShaderCompilerResult graphicsResult = compiler->Compile("SharedStageReflectionTest", graphicsSource, {}, languages, true, graphicsShader);
	B3D_TEST_ASSERT_MSG(graphicsResult.ErrorMessage.empty(), graphicsResult.ErrorMessage)
	if(graphicsShader == nullptr)
		return;

	B3D_TEST_ASSERT(graphicsShader->GetUniformBuffers().count("InternalParameters") == 0)
	B3D_TEST_ASSERT(graphicsShader->GetTextureParameters().count("InternalTexture") == 0)
	for(const auto& variation : graphicsShader->GetVariations())
	{
		const auto& vertex = variation->GetPass(0)->GetGpuProgramCreateInformation(GPT_VERTEX_PROGRAM);
		const auto& fragment = variation->GetPass(0)->GetGpuProgramCreateInformation(GPT_FRAGMENT_PROGRAM);
		B3D_TEST_ASSERT(vertex.ShaderReflection->Parameters == fragment.ShaderReflection->Parameters)
		B3D_TEST_ASSERT(vertex.ShaderReflection->ParameterLayoutHash == fragment.ShaderReflection->ParameterLayoutHash)
		B3D_TEST_ASSERT(vertex.ShaderReflection->Parameters->GetUniformBuffers().size() == 3)
		B3D_TEST_ASSERT(vertex.ShaderReflection->Parameters->GetDataParameters().count("InternalValue") == 1)
		B3D_TEST_ASSERT(vertex.ShaderReflection->Parameters->GetTextureParameters().count("InternalTexture") == 1)
		B3D_TEST_ASSERT(vertex.ShaderReflection->Parameters->GetUniformBuffers().at("InternalParameters").IsInternal)
		B3D_TEST_ASSERT(vertex.ShaderReflection->Parameters->GetDataParameters().at("InternalValue").IsInternal)
		B3D_TEST_ASSERT(vertex.ShaderReflection->Parameters->GetTextureParameters().at("InternalTexture").IsInternal)
		B3D_TEST_ASSERT(vertex.Bytecode != nullptr && vertex.Bytecode->Instructions.Data != nullptr)
		B3D_TEST_ASSERT(fragment.Bytecode != nullptr && fragment.Bytecode->Instructions.Data != nullptr)
	}

	// Renderer and low-level variations may differ; only public Material interfaces must remain compatible.
	u32 declarationIndex = 0;
	for(const String& changedDeclaration : {
		String("#if MODE\n[internal] cbuffer Conditional { float4 Value; };\n[internal] Buffer<float4> InternalBuffer;\n[internal] SamplerState InternalSampler;\n#endif\n"),
		String("#if MODE\ncbuffer Conditional { float4 Value; };\n#else\ncbuffer Conditional { float2 Value; };\n#endif\n"),
		String("#if MODE\ncbuffer Conditional { float Value[2]; };\n#else\ncbuffer Conditional { float Value[3]; };\n#endif\n"),
		String("#if MODE\ncbuffer Conditional { float Value; };\n#endif\n"),
		String("#if MODE\nstruct Data { float4 First; };\n#else\nstruct Data { float2 First; };\n#endif\ncbuffer Conditional { Data Value; };\n"),
		String("#if MODE\nTexture2D Resource;\n#else\nStructuredBuffer<float4> Resource;\n#endif\n") })
	{
		String variationSource = graphicsSource;
		variationSource.insert(variationSource.find("cbuffer VertexParameters"), changedDeclaration);
		TShared<Shader> variationShader;
		const ShaderCompilerResult variationCompileResult = compiler->Compile("VariationInterfaces", variationSource, {}, languages, true, variationShader);
		B3D_TEST_ASSERT_MSG(variationCompileResult.ErrorMessage.empty(), variationCompileResult.ErrorMessage)
		if(variationShader == nullptr)
			return;

		u32 incompatibleVariations = 0;
		for(const auto& variation : variationShader->GetVariations())
		{
			const auto& program = variation->GetPass(0)->GetGpuProgramCreateInformation(GPT_VERTEX_PROGRAM);

			// Cached source declarations retain visibility and validation, independent of optimized bytecode.
			BinarySerializer serializer;
			const TShared<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
			serializer.Encode(program.ShaderReflection.get(), stream);
			stream->Seek(0);
			const auto decoded = B3DRTTICast<ShaderReflection>(serializer.Decode(stream, (u32)stream->Size()));
			const Result compatibility = variationShader->GetParameterDescription()->ValidateMaterialInterface({ decoded->Parameters.get() });
			if(!compatibility.IsSuccessful())
				incompatibleVariations++;
		}
		B3D_TEST_ASSERT(incompatibleVariations == (declarationIndex == 0 ? 0 : languages.size()))

		// Exercise the actual Material boundary, not just the declaration comparison.
		GetRenderThread().PostCommand([this, renderShader = B3DGetRenderProxy(variationShader)]()
		{
			const TShared<render::Material> material = render::Material::Create(renderShader);
			const auto compatibleVariations = renderShader->GetCompatibleVariations();
			for(u32 variationIndex = 0; variationIndex < compatibleVariations.size(); variationIndex++)
			{
				const TAsyncOp<bool> compilation = compatibleVariations[variationIndex]->Compile();
				compilation.BlockUntilComplete();
				B3D_TEST_ASSERT(compilation.GetReturnValue())

				const auto& program = compatibleVariations[variationIndex]->GetPass(0)->GetGpuProgramCreateInformation(GPT_VERTEX_PROGRAM);
				const bool matches = renderShader->GetParameterDescription()->ValidateMaterialInterface({ program.ShaderReflection->Parameters.get() }).IsSuccessful();
				LoggingScope logs(*this);
				if(!matches)
					logs.ExpectError("Cannot create material parameter adapter");

				B3D_TEST_ASSERT((material->CreateParameterAdapter(variationIndex) != nullptr) == matches)
			}
		}, "Test material variation interfaces", true);

		TShared<Shader> lazyShader;
		const ShaderCompilerResult lazyResult = compiler->Compile("LazyVariationInterfaces", variationSource, {}, languages, false, lazyShader);
		B3D_TEST_ASSERT_MSG(lazyResult.ErrorMessage.empty(), lazyResult.ErrorMessage)
		if(lazyShader == nullptr)
			return;

		u32 variationIndex = 0;
		for(const auto& variation : lazyShader->GetVariations())
		{
			const ShaderCompilerResult variationResult = compiler->CompileVariation(*lazyShader, variation->GetVariationParameters(), languages[variationIndex++ % languages.size()], *variation);
			B3D_TEST_ASSERT_MSG(variationResult.ErrorMessage.empty(), variationResult.ErrorMessage)
		}
		declarationIndex++;
	}
}
#endif

void CoreTestSuite::TestShaderParameterDescription()
{
	ShaderParameterDescription parameters;
	const float initialValue = 0.5f;
	parameters.AddParameter(ShaderDataParameterInformation("Initial", "Initial", GPDT_FLOAT1), TArrayView<const u8>((const u8*)&initialValue, sizeof(initialValue)));
	parameters.AddParameter(ShaderObjectParameterInformation("Texture", "FirstTexture", GPOT_TEXTURE2D), ShaderDefaultTextureType::White);

	ShaderParameterDescription incoming;
	const float values[] = { 1.0f, 2.0f, 3.0f };
	incoming.AddParameter(ShaderDataParameterInformation("Values", "Values", GPDT_FLOAT1, StringID::kNone, 3), TArrayView<const u8>((const u8*)values, sizeof(values)));
	incoming.AddParameter(ShaderObjectParameterInformation("Texture", "SecondTexture", GPOT_TEXTURE2D), ShaderDefaultTextureType::White);

	ShaderParameterAttribute name;
	name.Type = ShaderParamAttributeType::Name;
	name.Value = "Original name";
	incoming.SetParameterAttribute("Values", name);
	ShaderParameterAttribute hidden;
	hidden.Type = ShaderParamAttributeType::HideInInspector;
	incoming.SetParameterAttribute("Values", hidden);

	// Replacing an attribute must preserve the rest of its chain.
	hidden.Value = "true";
	incoming.SetParameterAttribute("Values", hidden);
	name.Value = "New name";
	incoming.SetParameterAttribute("Values", name);
	B3D_TEST_ASSERT(parameters.TryCombine(incoming).IsSuccessful())
	const ShaderDataParameterInformation& valueParameter = parameters.GetDataParameters().at("Values");
	B3D_TEST_ASSERT(valueParameter.DefaultValueIndex == sizeof(initialValue))
	B3D_TEST_ASSERT(parameters.GetDataDefaultValues().size() == sizeof(initialValue) + sizeof(values))
	B3D_TEST_ASSERT(memcmp(parameters.GetDataDefaultValues().data() + valueParameter.DefaultValueIndex, values, sizeof(values)) == 0)
	B3D_TEST_ASSERT(parameters.GetTextureParameters().at("Texture").GpuVariableNames.size() == 2)
	B3D_TEST_ASSERT(parameters.GetTextureDefaultValues().size() == 1)

	u32 attributeCount = 0;
	for(u32 index = valueParameter.AttributeIndex; index != ~0u; index = parameters.GetParameterAttributes()[index].NextParameterIndex)
	{
		B3D_TEST_ASSERT(index < parameters.GetParameterAttributes().size())
		if(index >= parameters.GetParameterAttributes().size() || ++attributeCount > 2)
			break;
	}
	B3D_TEST_ASSERT(attributeCount == 2)
	B3D_TEST_ASSERT(parameters.TryCombine(incoming).IsSuccessful())
	B3D_TEST_ASSERT(parameters.GetDataDefaultValues().size() == sizeof(initialValue) + sizeof(values))
	B3D_TEST_ASSERT(parameters.GetParameterAttributes().size() == 2)

	// A conflict discovered after a new declaration must not partially append that declaration.
	ShaderParameterDescription conflicting;
	conflicting.AddParameter(ShaderDataParameterInformation("Added", "Added", GPDT_FLOAT1));
	conflicting.AddParameter(ShaderDataParameterInformation("Values", "Values", GPDT_FLOAT2));
	B3D_TEST_ASSERT(!parameters.TryCombine(conflicting).IsSuccessful())
	B3D_TEST_ASSERT(parameters.GetDataParameters().count("Added") == 0)
	B3D_TEST_ASSERT(parameters.GetDataParameters().at("Values").Type == GPDT_FLOAT1)

	ShaderParameterDescription differentDefault;
	differentDefault.AddParameter(ShaderObjectParameterInformation("Texture", "ThirdTexture", GPOT_TEXTURE2D), ShaderDefaultTextureType::Black);
	B3D_TEST_ASSERT(!parameters.TryCombine(differentDefault).IsSuccessful())
	B3D_TEST_ASSERT(parameters.GetTextureParameters().at("Texture").GpuVariableNames.size() == 2)

	// A material interface spans passes and ignores bindings, internal resources and native optimization.
	ShaderParameterDescription materialInterface;
	materialInterface.AddParameter(ShaderDataParameterInformation("Value", "SourceValue", GPDT_FLOAT1));
	materialInterface.AddParameter(ShaderObjectParameterInformation("Texture", "SourceTexture", GPOT_TEXTURE2D));
	ShaderParameterDescription firstPass;
	firstPass.AddParameter(ShaderDataParameterInformation("SourceValue", "SourceValue", GPDT_FLOAT1));
	firstPass.SetParameterBinding("SourceValue", 3, 9);
	firstPass.AddParameter(ShaderDataParameterInformation("InternalValue", "InternalValue", GPDT_FLOAT4));
	firstPass.SetParameterInternal("InternalValue", true);
	ShaderParameterDescription secondPass;
	secondPass.AddParameter(ShaderObjectParameterInformation("SourceTexture", "SourceTexture", GPOT_TEXTURE2D));
	secondPass.SetParameterBinding("SourceTexture", 4, 12);
	B3D_TEST_ASSERT(materialInterface.ValidateMaterialInterface({ &firstPass, &secondPass }).IsSuccessful())
	B3D_TEST_ASSERT(!materialInterface.ValidateMaterialInterface({ &firstPass }).IsSuccessful())
	secondPass.SetParameterInternal("SourceTexture", true);
	B3D_TEST_ASSERT(!materialInterface.ValidateMaterialInterface({ &firstPass, &secondPass }).IsSuccessful())
	secondPass.SetParameterInternal("SourceTexture", false);
	secondPass.AddParameter(ShaderObjectParameterInformation("AddedTexture", "AddedTexture", GPOT_TEXTURE2D));
	B3D_TEST_ASSERT(!materialInterface.ValidateMaterialInterface({ &firstPass, &secondPass }).IsSuccessful())
}

void CoreTestSuite::TestShaderReflection()
{
	ShaderCreateInformation shaderInformation;
	shaderInformation.Description->QueuePriority = 42;
	const float defaultValue = 0.25f;
	shaderInformation.Description->Parameters->AddParameter(ShaderDataParameterInformation("Value", "Value", GPDT_FLOAT1), TArrayView<const u8>((const u8*)&defaultValue, sizeof(defaultValue)));

	const render::ShaderCreateInformation renderInformation = ShaderCreateInformation::ConvertToRenderProxy(shaderInformation);
	B3D_TEST_ASSERT(renderInformation.Description == shaderInformation.Description)
	B3D_TEST_ASSERT(renderInformation.Description->Parameters == shaderInformation.Description->Parameters)
	B3D_TEST_ASSERT(renderInformation.Description->Parameters->GetDataDefaultValues().data() == shaderInformation.Description->Parameters->GetDataDefaultValues().data())

	const TShared<Shader> shader = Shader::CreateShared("SharedDescriptionTest", shaderInformation);
	const TShared<PrecompiledShaderData> cached = shader->GetPrecompiledData();
	B3D_TEST_ASSERT(cached->Description == shaderInformation.Description)

	GpuProgramCreateInformation program;
	program.EntryPoint = "csmain";
	program.Type = GPT_COMPUTE_PROGRAM;
	program.ShaderReflection = B3DMakeShared<ShaderReflection>();
	program.ShaderReflection->Parameters = shaderInformation.Description->Parameters;
	ShaderEntryPointReflection& entry = program.ShaderReflection->EntryPoints[program.EntryPoint];
	entry.Type = program.Type;
	entry.ThreadGroupSize = { 8, 4, 2 };
	entry.PushConstantBufferSize = 12;
	ShaderUniformBufferInformation binding;
	binding.Name = "PerDraw";
	binding.Set = 2;
	binding.Slot = 3;
	binding.UsesDynamicOffset = true;
	binding.IsInternal = true;
	program.ShaderReflection->Parameters->AddUniformBuffer(binding);
	program.ShaderReflection->Parameters->SetParameterBinding("Value", 2, 3);
	program.ShaderReflection->ParameterLayoutHash = { 123, 456 };

	// Native resource names may differ; source attributes are matched by binding coordinates only.
	GpuProgramParameterDescription nativeParameters;
	GpuUniformBufferInformation nativeBuffer;
	nativeBuffer.Name = "RenamedPerDraw";
	nativeBuffer.Set = 2;
	nativeBuffer.Slot = 3;
	nativeBuffer.Size = 16;
	nativeParameters.UniformBuffers[nativeBuffer.Name] = nativeBuffer;
	nativeParameters.UniformBuffers["Alias"] = nativeBuffer;
	nativeBuffer.Name = "PerDraw";
	nativeBuffer.Set = 1;
	nativeParameters.UniformBuffers[nativeBuffer.Name] = nativeBuffer;
	nativeBuffer.Set = 2;
	nativeBuffer.Slot = 4;
	nativeBuffer.UsesDynamicOffset = true;
	nativeParameters.UniformBuffers["Unmatched"] = nativeBuffer;

	u32 matchCount = 0;
	auto fnCopyDynamicOffset = [&matchCount](const auto& source, auto& outTarget)
	{
		outTarget.UsesDynamicOffset = source.UsesDynamicOffset;
		matchCount++;
	};
	ShaderParameterDescription::IterateMatching(program.ShaderReflection->Parameters->GetUniformBuffers(), nativeParameters.UniformBuffers, fnCopyDynamicOffset);
	B3D_TEST_ASSERT(matchCount == 2)
	B3D_TEST_ASSERT(nativeParameters.UniformBuffers.at("RenamedPerDraw").UsesDynamicOffset)
	B3D_TEST_ASSERT(nativeParameters.UniformBuffers.at("RenamedPerDraw").Size == 16)
	B3D_TEST_ASSERT(nativeParameters.UniformBuffers.at("Alias").UsesDynamicOffset)
	B3D_TEST_ASSERT(!nativeParameters.UniformBuffers.at("PerDraw").UsesDynamicOffset)
	B3D_TEST_ASSERT(nativeParameters.UniformBuffers.at("Unmatched").UsesDynamicOffset)

	// Slot conversion only affects matching, not the source bindings or unmatched target values.
	nativeParameters.UniformBuffers.at("RenamedPerDraw").Slot = 12;
	nativeParameters.UniformBuffers.at("RenamedPerDraw").UsesDynamicOffset = false;
	nativeParameters.UniformBuffers.at("Alias").UsesDynamicOffset = false;
	matchCount = 0;
	ShaderParameterDescription::IterateMatching(program.ShaderReflection->Parameters->GetUniformBuffers(), nativeParameters.UniformBuffers, fnCopyDynamicOffset, [](u32 slot) { return slot * 4; });
	B3D_TEST_ASSERT(matchCount == 1)
	B3D_TEST_ASSERT(nativeParameters.UniformBuffers.at("RenamedPerDraw").UsesDynamicOffset)
	B3D_TEST_ASSERT(!nativeParameters.UniformBuffers.at("Alias").UsesDynamicOffset)
	B3D_TEST_ASSERT(nativeParameters.UniformBuffers.at("Unmatched").UsesDynamicOffset)
	B3D_TEST_ASSERT(program.ShaderReflection->Parameters->GetUniformBuffers().at("PerDraw").Slot == 3)

	BinarySerializer serializer;
	const TShared<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
	serializer.Encode(&program, stream);
	stream->Seek(0);
	const TShared<GpuProgramCreateInformation> decoded = B3DRTTICast<GpuProgramCreateInformation>(serializer.Decode(stream, (u32)stream->Size()));
	B3D_TEST_ASSERT(decoded != nullptr && decoded->ShaderReflection != nullptr)
	if(decoded == nullptr || decoded->ShaderReflection == nullptr)
		return;

	const ShaderEntryPointReflection& decodedEntry = decoded->GetEntryPointReflection();
	B3D_TEST_ASSERT(decodedEntry.Type == GPT_COMPUTE_PROGRAM)
	B3D_TEST_ASSERT(decodedEntry.ThreadGroupSize == entry.ThreadGroupSize)
	B3D_TEST_ASSERT(decodedEntry.PushConstantBufferSize == 12)
	B3D_TEST_ASSERT(decoded->ShaderReflection->Parameters != nullptr)
	if(decoded->ShaderReflection->Parameters == nullptr)
		return;

	const auto& uniformBuffers = decoded->ShaderReflection->Parameters->GetUniformBuffers();
	B3D_TEST_ASSERT(uniformBuffers.size() == 1)
	if(uniformBuffers.size() != 1)
		return;

	B3D_TEST_ASSERT(uniformBuffers.at("PerDraw").Set == 2 && uniformBuffers.at("PerDraw").Slot == 3)
	B3D_TEST_ASSERT(uniformBuffers.at("PerDraw").UsesDynamicOffset)
	B3D_TEST_ASSERT(uniformBuffers.at("PerDraw").IsInternal)
	B3D_TEST_ASSERT(decoded->ShaderReflection->ParameterLayoutHash == program.ShaderReflection->ParameterLayoutHash)
	B3D_TEST_ASSERT(decoded->ShaderReflection->Parameters->GetDataParameters().at("Value").Set == 2)
	B3D_TEST_ASSERT(decoded->ShaderReflection->Parameters->GetDataParameters().at("Value").Slot == 3)
	if(decoded->ShaderReflection->Parameters != nullptr)
	{
		const auto& values = decoded->ShaderReflection->Parameters->GetDataDefaultValues();
		B3D_TEST_ASSERT(values.size() == sizeof(defaultValue) && memcmp(values.data(), &defaultValue, sizeof(defaultValue)) == 0)
	}
}
