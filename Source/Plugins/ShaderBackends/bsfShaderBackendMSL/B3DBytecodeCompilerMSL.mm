//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DBytecodeCompilerMSL.h"

#if B3D_PLATFORM_MACOS

#include "B3DMetalShaderABI.h"
#include "FileSystem/B3DFileSystem.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "Math/B3DMath.h"
#include "Utility/B3DScopeGuard.h"

#include <Metal/Metal.h>
#include <algorithm>
#include <cerrno>
#include <crt_externs.h>
#include <cstring>
#include <fcntl.h>
#include <limits>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace b3d;
using namespace b3d::render;

namespace
{
	struct ReflectedTable
	{
		u32 Set = 0;
		u32 SizeInBytes = 0;
		Vector<GpuDescriptorTableEntry> Entries;
	};

	String ReadTextFile(const Path& path)
	{
		const TShared<DataStream> stream = FileSystem::OpenFile(path);
		return stream != nullptr ? stream->GetAsString() : String();
	}

	bool RunXcrun(const Vector<String>& arguments, const Path& logPath, String& outDiagnostics)
	{
		Vector<char*> nativeArguments;
		for(const String& argument : arguments)
			nativeArguments.push_back(const_cast<char*>(argument.c_str()));
		nativeArguments.push_back(nullptr);

		posix_spawn_file_actions_t actions;
		const int actionsResult = posix_spawn_file_actions_init(&actions);
		if(actionsResult != 0)
		{
			outDiagnostics = StringUtility::Format("Could not initialize xcrun process actions: {0}", std::strerror(actionsResult));
			return false;
		}

		const String logPathString = logPath.ToString();
		const int openResult = posix_spawn_file_actions_addopen(&actions, STDERR_FILENO, logPathString.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
		const int duplicateResult = openResult == 0 ? posix_spawn_file_actions_adddup2(&actions, STDERR_FILENO, STDOUT_FILENO) : openResult;
		if(duplicateResult != 0)
		{
			posix_spawn_file_actions_destroy(&actions);
			outDiagnostics = StringUtility::Format("Could not redirect xcrun diagnostics: {0}", std::strerror(duplicateResult));
			return false;
		}

		pid_t process = 0;
		const int spawnResult = posix_spawn(&process, "/usr/bin/xcrun", &actions, nullptr, nativeArguments.data(), *_NSGetEnviron());
		posix_spawn_file_actions_destroy(&actions);
		if(spawnResult != 0)
		{
			outDiagnostics = StringUtility::Format("Could not launch xcrun: {0}", std::strerror(spawnResult));
			return false;
		}

		int status = 0;
		while(waitpid(process, &status, 0) < 0)
		{
			if(errno == EINTR)
				continue;

			outDiagnostics = StringUtility::Format("Could not wait for xcrun: {0}", std::strerror(errno));
			return false;
		}

		outDiagnostics = ReadTextFile(logPath);
		return WIFEXITED(status) && WEXITSTATUS(status) == 0;
	}

	GpuDataParameterType MapDataType(MTLDataType type)
	{
		switch(type)
		{
		case MTLDataTypeBool: return GPDT_BOOL;
		case MTLDataTypeFloat: return GPDT_FLOAT1;
		case MTLDataTypeFloat2: return GPDT_FLOAT2;
		case MTLDataTypeFloat3: return GPDT_FLOAT3;
		case MTLDataTypeFloat4: return GPDT_FLOAT4;
		case MTLDataTypeHalf: return GPDT_HALF1;
		case MTLDataTypeHalf2: return GPDT_HALF2;
		case MTLDataTypeHalf3: return GPDT_HALF3;
		case MTLDataTypeHalf4: return GPDT_HALF4;
		case MTLDataTypeInt: return GPDT_INT1;
		case MTLDataTypeInt2: return GPDT_INT2;
		case MTLDataTypeInt3: return GPDT_INT3;
		case MTLDataTypeInt4: return GPDT_INT4;
		case MTLDataTypeUInt: return GPDT_UINT1;
		case MTLDataTypeUInt2: return GPDT_UINT2;
		case MTLDataTypeUInt3: return GPDT_UINT3;
		case MTLDataTypeUInt4: return GPDT_UINT4;
		case MTLDataTypeFloat2x2: return GPDT_MATRIX_2X2;
		case MTLDataTypeFloat2x3: return GPDT_MATRIX_3X2;
		case MTLDataTypeFloat2x4: return GPDT_MATRIX_4X2;
		case MTLDataTypeFloat3x2: return GPDT_MATRIX_2X3;
		case MTLDataTypeFloat3x3: return GPDT_MATRIX_3X3;
		case MTLDataTypeFloat3x4: return GPDT_MATRIX_4X3;
		case MTLDataTypeFloat4x2: return GPDT_MATRIX_2X4;
		case MTLDataTypeFloat4x3: return GPDT_MATRIX_3X4;
		case MTLDataTypeFloat4x4: return GPDT_MATRIX_4X4;
		case MTLDataTypeHalf2x2: return GPDT_HALF_MATRIX_2X2;
		case MTLDataTypeHalf2x3: return GPDT_HALF_MATRIX_3X2;
		case MTLDataTypeHalf2x4: return GPDT_HALF_MATRIX_4X2;
		case MTLDataTypeHalf3x2: return GPDT_HALF_MATRIX_2X3;
		case MTLDataTypeHalf3x3: return GPDT_HALF_MATRIX_3X3;
		case MTLDataTypeHalf3x4: return GPDT_HALF_MATRIX_4X3;
		case MTLDataTypeHalf4x2: return GPDT_HALF_MATRIX_2X4;
		case MTLDataTypeHalf4x3: return GPDT_HALF_MATRIX_3X4;
		case MTLDataTypeHalf4x4: return GPDT_HALF_MATRIX_4X4;
		case MTLDataTypeStruct: return GPDT_STRUCT;
		default: return GPDT_UNKNOWN;
		}
	}

	GpuBufferFormat MapElementFormat(MTLDataType type)
	{
		switch(type)
		{
		case MTLDataTypeFloat: return BF_32X1F;
		case MTLDataTypeFloat2: return BF_32X2F;
		case MTLDataTypeFloat3: return BF_32X3F;
		case MTLDataTypeFloat4: return BF_32X4F;
		case MTLDataTypeHalf: return BF_16X1F;
		case MTLDataTypeHalf2: return BF_16X2F;
		case MTLDataTypeHalf4: return BF_16X4F;
		case MTLDataTypeInt: return BF_32X1S;
		case MTLDataTypeInt2: return BF_32X2S;
		case MTLDataTypeInt3: return BF_32X3S;
		case MTLDataTypeInt4: return BF_32X4S;
		case MTLDataTypeUInt: return BF_32X1U;
		case MTLDataTypeUInt2: return BF_32X2U;
		case MTLDataTypeUInt3: return BF_32X3U;
		case MTLDataTypeUInt4: return BF_32X4U;
		default: return BF_UNKNOWN;
		}
	}

	VertexElementType MapVertexType(MTLDataType type)
	{
		switch(type)
		{
		case MTLDataTypeFloat: return VET_FLOAT1;
		case MTLDataTypeFloat2: return VET_FLOAT2;
		case MTLDataTypeFloat3: return VET_FLOAT3;
		case MTLDataTypeFloat4: return VET_FLOAT4;
		case MTLDataTypeHalf: return VET_HALF1;
		case MTLDataTypeHalf2: return VET_HALF2;
		case MTLDataTypeHalf3: return VET_HALF3;
		case MTLDataTypeHalf4: return VET_HALF4;
		case MTLDataTypeInt: return VET_INT1;
		case MTLDataTypeInt2: return VET_INT2;
		case MTLDataTypeInt3: return VET_INT3;
		case MTLDataTypeInt4: return VET_INT4;
		case MTLDataTypeUInt: return VET_UINT1;
		case MTLDataTypeUInt2: return VET_UINT2;
		case MTLDataTypeUInt3: return VET_UINT3;
		case MTLDataTypeUInt4: return VET_UINT4;
		default: return VET_UNKNOWN;
		}
	}

	GpuParameterObjectType MapTextureType(MTLTextureType type, bool writable)
	{
		switch(type)
		{
		case MTLTextureType1D: return writable ? GPOT_RWTEXTURE1D : GPOT_TEXTURE1D;
		case MTLTextureType1DArray: return writable ? GPOT_RWTEXTURE1DARRAY : GPOT_TEXTURE1DARRAY;
		case MTLTextureType2D: return writable ? GPOT_RWTEXTURE2D : GPOT_TEXTURE2D;
		case MTLTextureType2DArray: return writable ? GPOT_RWTEXTURE2DARRAY : GPOT_TEXTURE2DARRAY;
		case MTLTextureType2DMultisample: return writable ? GPOT_RWTEXTURE2DMS : GPOT_TEXTURE2DMS;
		case MTLTextureType2DMultisampleArray: return writable ? GPOT_RWTEXTURE2DMSARRAY : GPOT_TEXTURE2DMSARRAY;
		case MTLTextureType3D: return writable ? GPOT_RWTEXTURE3D : GPOT_TEXTURE3D;
		case MTLTextureTypeCube: return GPOT_TEXTURECUBE;
		case MTLTextureTypeCubeArray: return GPOT_TEXTURECUBEARRAY;
		default: return GPOT_UNKNOWN;
		}
	}

	u32 GetDataTypeSize(MTLDataType dataType, u32 fallbackSize)
	{
		const GpuDataParameterType type = MapDataType(dataType);
		if(type == GPDT_STRUCT || type == GPDT_UNKNOWN)
			return fallbackSize;

		return b3d::GpuParameterSet::kParamSizes.Lookup[(u32)type].Size;
	}

	bool ParseVertexSemantic(const String& inputName, VertexElementSemantic& outSemantic, u16& outIndex)
	{
		struct SemanticName
		{
			String Name;
			VertexElementSemantic Semantic;
		};

		static const SemanticName names[] =
		{
			{ "bs_position", VES_POSITION }, { "bs_normal", VES_NORMAL },
			{ "bs_tangent", VES_TANGENT }, { "bs_bitangent", VES_BITANGENT },
			{ "bs_texcoord", VES_TEXCOORD }, { "bs_color", VES_COLOR },
			{ "bs_blendweights", VES_BLEND_WEIGHTS }, { "bs_blendindices", VES_BLEND_INDICES },
			{ "POSITION", VES_POSITION }, { "NORMAL", VES_NORMAL }, { "TANGENT", VES_TANGENT },
			{ "BITANGENT", VES_BITANGENT }, { "TEXCOORD", VES_TEXCOORD }, { "COLOR", VES_COLOR },
			{ "BLENDWEIGHT", VES_BLEND_WEIGHTS }, { "BLENDINDICES", VES_BLEND_INDICES }
		};

		static const String inputPrefix = "in_var_";
		String name = inputName;
		if(StringUtility::StartsWith(name, inputPrefix, true))
			name = name.substr(inputPrefix.size());

		for(const SemanticName& semanticName : names)
		{
			if(!StringUtility::StartsWith(name, semanticName.Name, false))
				continue;

			const String index = name.substr(semanticName.Name.size());
			outIndex = index.empty() ? 0 : (u16)ParseI32(index);
			outSemantic = semanticName.Semantic;
			return true;
		}

		return false;
	}

	bool ReflectUniformMembers(MTLStructType* structType, u32 structSize, u32 set, u32 slot, GpuProgramParameterDescription& outDescription, String& outMessages)
	{
		bool reflectionValid = true;
		NSArray<MTLStructMember*>* members = structType.members;
		const NSUInteger memberCount = members.count;
		for(NSUInteger memberIndex = 0; memberIndex < memberCount; memberIndex++)
		{
			MTLStructMember* member = members[memberIndex];
			const u32 offset = (u32)member.offset;
			const u32 nextOffset = memberIndex + 1 < memberCount ? (u32)members[memberIndex + 1].offset : structSize;
			if(offset % 4 != 0 || nextOffset < offset)
			{
				outMessages += StringUtility::Format("Metal reflection reported an unsupported uniform member layout for '{0}'.\n", member.name.UTF8String);
				reflectionValid = false;

				continue;
			}

			MTLDataType dataType = member.dataType;
			u32 arraySize = 1;
			u32 arrayStride = nextOffset - offset;
			if(dataType == MTLDataTypeArray)
			{
				MTLArrayType* arrayType = [member arrayType];
				if(arrayType == nil)
				{
					outMessages += StringUtility::Format("Metal reflection could not describe uniform array '{0}'.\n", member.name.UTF8String);
					reflectionValid = false;

					continue;
				}

				dataType = arrayType.elementType;
				arraySize = (u32)arrayType.arrayLength;
				arrayStride = (u32)arrayType.stride;
			}

			const u32 reflectedSize = GetDataTypeSize(dataType, nextOffset - offset);
			if(arrayStride % 4 != 0 || reflectedSize == 0)
			{
				outMessages += StringUtility::Format("Metal reflection reported an unsupported uniform member size for '{0}'.\n", member.name.UTF8String);
				reflectionValid = false;

				continue;
			}

			GpuUniformBufferMemberInformation information;
			information.Name = member.name.UTF8String;
			information.Type = MapDataType(dataType);
			information.ParentUniformBufferSet = set;
			information.ParentUniformBufferSlot = slot;
			information.ElementSize = Math::DivideAndRoundUp(reflectedSize, 4u);
			information.ArraySize = arraySize;
			information.ArrayElementStride = arrayStride / 4;
			information.CpuOffset = offset / 4;
			information.GpuOffset = offset / 4;

			if(information.Type == GPDT_UNKNOWN)
			{
				outMessages += StringUtility::Format("Metal reflection could not map uniform member '{0}'.\n", information.Name);
				reflectionValid = false;

				continue;
			}

			outDescription.UniformBufferMembers[information.Name] = std::move(information);
		}

		return reflectionValid;
	}

	bool ReflectArgumentMember(MTLStructMember* member, id<MTLBufferBinding> tableBinding, GpuProgramParameterDescription& outDescription, GpuDescriptorTableEntry& outEntry, String& outMessages)
	{
		GpuParameterType parameterType = GpuParameterType::Unknown;
		u32 set = 0;
		u32 slot = 0;
		String name;
		if(!DecodeMetalResourceName(member.name.UTF8String, parameterType, set, slot, name))
		{
			outMessages += StringUtility::Format("Metal reflection found an argument-buffer member without engine binding identity: '{0}'.\n", member.name != nil ? member.name.UTF8String : "<unnamed>");
			return false;
		}

		if(set != (u32)tableBinding.index)
		{
			outMessages += StringUtility::Format("Metal reflection set mismatch for resource '{0}'.\n", name);
			return false;
		}

		MTLDataType dataType = member.dataType;
		u32 arrayLength = 1;
		u32 descriptorSize = (u32)sizeof(MTLResourceID);
		MTLPointerType* pointerType = nil;
		MTLTextureReferenceType* textureType = nil;
		if(dataType == MTLDataTypeArray)
		{
			MTLArrayType* arrayType = [member arrayType];
			if(arrayType == nil)
			{
				outMessages += StringUtility::Format("Metal reflection could not describe resource array '{0}'.\n", name);
				return false;
			}

			const NSUInteger reflectedArrayLength = arrayType.arrayLength;
			const NSUInteger reflectedDescriptorSize = arrayType.stride;
			if(reflectedArrayLength == 0 || reflectedArrayLength > std::numeric_limits<u32>::max()
				|| reflectedDescriptorSize < sizeof(MTLResourceID)
				|| reflectedDescriptorSize > std::numeric_limits<u32>::max())
			{
				outMessages += StringUtility::Format("Metal reflection reported an invalid resource-array layout for '{0}'.\n", name);
				return false;
			}

			dataType = arrayType.elementType;
			arrayLength = (u32)reflectedArrayLength;
			descriptorSize = (u32)reflectedDescriptorSize;
			pointerType = [arrayType elementPointerType];
			textureType = [arrayType elementTextureReferenceType];
		}
		else
		{
			pointerType = [member pointerType];
			textureType = [member textureReferenceType];
		}

		outEntry.Kind = GpuDescriptorEntryKind::Resource;
		outEntry.OffsetInBytes = (u32)member.offset;
		outEntry.Type = parameterType;
		outEntry.Slot = slot;
		outEntry.DescriptorCount = arrayLength;
		outEntry.DescriptorSizeInBytes = descriptorSize;

		if(parameterType == GpuParameterType::UniformBuffer)
		{
			if(pointerType == nil || pointerType.dataSize == 0 || pointerType.dataSize % 4 != 0)
			{
				outMessages += StringUtility::Format("Metal reflection reported an invalid uniform-buffer size for '{0}'.\n", name);
				return false;
			}

			GpuUniformBufferInformation information;
			information.Name = name;
			information.Set = set;
			information.Slot = slot;
			information.Size = Math::CeilToMultiple((u32)pointerType.dataSize / 4, 4u);
			information.IsShareable = true;
			outDescription.UniformBuffers[name] = information;

			MTLStructType* uniformStruct = [pointerType elementStructType];
			if(uniformStruct == nil)
			{
				outMessages += StringUtility::Format("Metal reflection did not report struct metadata for uniform buffer '{0}'.\n", name);
				return false;
			}

			return ReflectUniformMembers(uniformStruct, (u32)pointerType.dataSize, set, slot, outDescription, outMessages);
		}

		GpuObjectParameterInformation information;
		information.Name = name;
		information.Set = set;
		information.Slot = slot;
		information.ArraySize = arrayLength;
		if(parameterType == GpuParameterType::Sampler)
		{
			information.Type = GPOT_SAMPLER2D;
			outDescription.Samplers[name] = information;

			return true;
		}

		if(parameterType == GpuParameterType::StorageBuffer)
		{
			if(pointerType == nil)
			{
				outMessages += StringUtility::Format("Metal reflection did not report pointer metadata for storage buffer '{0}'.\n", name);
				return false;
			}

			information.Type = pointerType.access == MTLBindingAccessReadOnly ? GPOT_STRUCTURED_BUFFER : GPOT_RWSTRUCTURED_BUFFER;
			information.ElementType = MapElementFormat(pointerType.elementType);
			outDescription.Buffers[name] = information;

			return true;
		}

		if(textureType == nil)
		{
			outMessages += StringUtility::Format("Metal reflection did not report texture metadata for resource '{0}'.\n", name);
			return false;
		}

		const bool writable = parameterType == GpuParameterType::StorageTexture;
		information.Type = MapTextureType(textureType.textureType, writable);
		information.ElementType = MapElementFormat(textureType.textureDataType);
		if(information.Type == GPOT_UNKNOWN)
			return false;

		if(writable)
			outDescription.StorageTextures[name] = information;
		else
			outDescription.SampledTextures[name] = information;

		return true;
	}

	bool ReflectLibrary(id<MTLLibrary> library, NSString* entryPointName, GpuProgramType type, GpuProgramBytecode& outBytecode, id<MTLFunction> __strong& outFunction)
	{
		outFunction = [library newFunctionWithName:entryPointName];
		if(outFunction == nil)
		{
			outBytecode.Messages += "The compiled Metal library does not contain the requested entry point.\n";
			return false;
		}

		MTLFunctionReflection* reflection = [library reflectionForFunctionWithName:entryPointName];
		if(reflection == nil)
		{
			outBytecode.Messages += "Metal did not return function reflection. The shader library must target macOS 13 or later.\n";
			return false;
		}

		outBytecode.ParameterDescription = B3DMakeShared<GpuProgramParameterDescription>();
		outBytecode.ResourceTableLayout = B3DMakeShared<GpuResourceTableLayout>();

		Vector<ReflectedTable> reflectedTables;
		UnorderedSet<u32> reflectedSets;
		bool reflectionValid = true;
		for(id<MTLBinding> binding in reflection.bindings)
		{
			if(binding.type != MTLBindingTypeBuffer)
			{
				outBytecode.Messages += StringUtility::Format("Metal reflection found unsupported top-level binding '{0}'.\n", binding.name != nil ? binding.name.UTF8String : "<unnamed>");
				reflectionValid = false;
				continue;
			}

			id<MTLBufferBinding> bufferBinding = (id<MTLBufferBinding>)binding;
			MTLStructType* tableStruct = bufferBinding.bufferStructType;
			if(tableStruct == nil)
			{
				outBytecode.Messages += StringUtility::Format("Metal reflection found a buffer binding that is not a parameter-set argument buffer: '{0}'.\n", binding.name != nil ? binding.name.UTF8String : "<unnamed>");
				reflectionValid = false;
				continue;
			}

			if((u32)binding.index > kMetalMaximumParameterSetIndex || !reflectedSets.insert((u32)binding.index).second)
			{
				outBytecode.Messages += StringUtility::Format("Metal reflection reported an invalid or duplicate parameter-set argument buffer at index {0}.\n", (u32)binding.index);
				reflectionValid = false;
				continue;
			}

			ReflectedTable table;
			table.Set = (u32)binding.index;
			table.SizeInBytes = (u32)bufferBinding.bufferDataSize;
			for(MTLStructMember* member in tableStruct.members)
			{
				GpuDescriptorTableEntry entry;
				if(ReflectArgumentMember(member, bufferBinding, *outBytecode.ParameterDescription, entry, outBytecode.Messages))
					table.Entries.push_back(entry);
				else
					reflectionValid = false;
			}

			if(table.SizeInBytes == 0)
			{
				outBytecode.Messages += StringUtility::Format("Metal reflection reported a zero-sized parameter-set argument buffer at index {0}.\n", table.Set);
				reflectionValid = false;
			}
			else
			{
				std::sort(table.Entries.begin(), table.Entries.end(), [](const auto& lhs, const auto& rhs)
				{
					return lhs.OffsetInBytes < rhs.OffsetInBytes;
				});

				reflectedTables.push_back(std::move(table));
			}
		}

		if(!reflectionValid)
			return false;

		if(!reflectedTables.empty())
		{
			// Concrete parameter types: in a dependent context `.Set <` would parse as the b3d::Set template
			std::sort(reflectedTables.begin(), reflectedTables.end(), [](const ReflectedTable& lhs, const ReflectedTable& rhs)
			{
				return lhs.Set < rhs.Set;
			});

			GpuResourceTableLayout& layout = *outBytecode.ResourceTableLayout;
			GpuDescriptorTable root;
			root.FirstEntry = 0;
			root.EntryCount = (u32)reflectedTables.size();
			layout.Tables.push_back(root);

			for(u32 tableIndex = 0; tableIndex < (u32)reflectedTables.size(); tableIndex++)
			{
				GpuDescriptorTableEntry entry;
				entry.Kind = GpuDescriptorEntryKind::SubTable;
				entry.TableIndex = tableIndex + 1;
				layout.Entries.push_back(entry);
			}

			for(const ReflectedTable& reflectedTable : reflectedTables)
			{
				GpuDescriptorTable table;
				table.Set = reflectedTable.Set;
				table.SizeInBytes = reflectedTable.SizeInBytes;
				table.FirstEntry = (u32)layout.Entries.size();
				table.EntryCount = (u32)reflectedTable.Entries.size();
				layout.Tables.push_back(table);
				layout.Entries.insert(layout.Entries.end(), reflectedTable.Entries.begin(), reflectedTable.Entries.end());
			}
		}

		if(type == GPT_VERTEX_PROGRAM)
		{
			for(MTLVertexAttribute* attribute in outFunction.vertexAttributes)
			{
				if(!attribute.active)
					continue;

				const char* attributeName = attribute.name.UTF8String;
				VertexElementSemantic semantic = VES_POSITION;
				u16 semanticIndex = 0;
				if(attributeName == nullptr || !ParseVertexSemantic(attributeName, semantic, semanticIndex))
				{
					outBytecode.Messages += StringUtility::Format("Metal reflection could not map vertex attribute '{0}'.\n", attributeName != nullptr ? attributeName : "<unnamed>");
					reflectionValid = false;
					continue;
				}

				const VertexElementType elementType = MapVertexType(attribute.attributeType);
				if(elementType == VET_UNKNOWN)
				{
					outBytecode.Messages += StringUtility::Format("Metal reflection reported an unsupported type for vertex attribute '{0}'.\n", attributeName);
					reflectionValid = false;
					continue;
				}

				outBytecode.VertexInput.push_back(VertexElement(elementType, semantic, semanticIndex, 0, 0,
					(u32)attribute.attributeIndex));
			}
		}

		return reflectionValid;
	}
}

TShared<IGpuBytecodeCompiler> render::CreateBytecodeCompilermsl()
{
	return B3DMakeShared<BytecodeCompilerMSL>();
}

bool BytecodeCompilerMSL::IsUpToDate(const GpuProgramBytecode& bytecode) const
{
	return bytecode.CompilerId == kMetalCompilerId && bytecode.CompilerVersion == kMetalCompilerVersion
		&& bytecode.Instructions.Data != nullptr && bytecode.Instructions.Size > 0
		&& bytecode.ParameterDescription != nullptr && bytecode.ResourceTableLayout != nullptr;
}

TShared<GpuProgramBytecode> BytecodeCompilerMSL::CompileBytecode(const GpuProgramCreateInformation& createInformation)
{
	TShared<GpuProgramBytecode> bytecode = B3DMakeShared<GpuProgramBytecode>();
	bytecode->CompilerId = kMetalCompilerId;
	bytecode->CompilerVersion = kMetalCompilerVersion;
	bytecode->ThreadGroupSize = createInformation.ThreadGroupSize;

	if(!IsProgramTypeSupported(createInformation.Type))
	{
		bytecode->Messages = "The macOS Metal backend does not support geometry or tessellation shader stages.";
		return bytecode;
	}

	if(createInformation.Bytecode != nullptr && IsUpToDate(*createInformation.Bytecode))
		return createInformation.Bytecode;

	if(createInformation.Type == GPT_COMPUTE_PROGRAM && (createInformation.ThreadGroupSize[0] == 0 || createInformation.ThreadGroupSize[1] == 0 || createInformation.ThreadGroupSize[2] == 0))
	{
		bytecode->Messages = "Metal compute threadgroup dimensions must all be greater than zero.";
		return bytecode;
	}

	const Path uniqueTemporaryPath = FileSystem::GetUniqueTemporaryFilePath();
	Path directory = uniqueTemporaryPath.GetParent();
	directory.PushDirectory(uniqueTemporaryPath.GetFilename());
	if(!FileSystem::CreateFolder(directory))
	{
		bytecode->Messages = StringUtility::Format("Could not create temporary directory '{0}' for Metal compilation.", directory.ToString());
		return bytecode;
	}

	Path sourcePath = directory;
	sourcePath.SetFilename("shader.metal");

	Path airPath = directory;
	airPath.SetFilename("shader.air");

	Path libraryPath = directory;
	libraryPath.SetFilename("shader.metallib");

	Path logPath = directory;
	logPath.SetFilename("compiler.log");

	ScopeGuard cleanup([&]()
	{
		FileSystem::Remove(directory);
	});

	{
		const TShared<DataStream> sourceStream = FileSystem::CreateAndOpenFile(sourcePath);
		if(sourceStream == nullptr)
		{
			bytecode->Messages = "Could not write the temporary MSL source file.";
			return bytecode;
		}

		const size_t sourceSize = createInformation.Source.size();
		const bool sourceWritten = sourceStream->Write(createInformation.Source.data(), sourceSize) == sourceSize;
		if(!sourceStream->Close() || !sourceWritten)
		{
			bytecode->Messages = "Could not finish writing the temporary MSL source file.";
			return bytecode;
		}
	}

	String diagnostics;
	// TODO: Batch shader permutations into fewer metal/metallib process invocations during offline cooking.
	// TODO: Source the MSL version and deployment target from the configured macOS toolchain.
	const String sourcePathString = sourcePath.ToString();
	const String airPathString = airPath.ToString();
	const String libraryPathString = libraryPath.ToString();
	const Vector<String> metalArguments =
	{
		"xcrun", "-sdk", "macosx", "metal", "-std=macos-metal3.0", "-mmacosx-version-min=13.0",
		"-ffast-math", "-c", sourcePathString, "-o", airPathString
	};

	if(!RunXcrun(metalArguments, logPath, diagnostics))
	{
		bytecode->Messages = StringUtility::Format("Metal source compilation failed.\n{0}", diagnostics);
		return bytecode;
	}

	if(!diagnostics.empty())
		bytecode->Messages += StringUtility::Format("Metal source compilation diagnostics:\n{0}", diagnostics);

	const Vector<String> libraryArguments =
	{
		"xcrun", "-sdk", "macosx", "metallib", airPathString, "-o", libraryPathString
	};

	if(!RunXcrun(libraryArguments, logPath, diagnostics))
	{
		bytecode->Messages += StringUtility::Format("Metal library linking failed.\n{0}", diagnostics);
		return bytecode;
	}

	if(!diagnostics.empty())
		bytecode->Messages += StringUtility::Format("Metal library linking diagnostics:\n{0}", diagnostics);

	Vector<u8> libraryData;
	{
		const TShared<DataStream> libraryStream = FileSystem::OpenFile(libraryPath);
		if(libraryStream == nullptr)
		{
			bytecode->Messages += "Could not read the compiled Metal library.";
			return bytecode;
		}

		const size_t librarySize = libraryStream->Size();
		if(librarySize == 0 || (u64)librarySize > (u64)std::numeric_limits<u32>::max())
		{
			bytecode->Messages += "The compiled Metal library has an invalid size.";
			return bytecode;
		}

		libraryData.resize(librarySize);
		if(libraryStream->Read(libraryData.data(), librarySize) != librarySize)
		{
			bytecode->Messages += "Could not finish reading the compiled Metal library.";
			return bytecode;
		}
	}

	@autoreleasepool
	{
		id<MTLDevice> device = MTLCreateSystemDefaultDevice();
		if(device == nil)
		{
			bytecode->Messages += "Metal reflection requires a Metal-capable Apple Silicon device.";
			return bytecode;
		}

		NSError* error = nil;
		NSString* path = [NSString stringWithUTF8String:libraryPathString.c_str()];
		id<MTLLibrary> library = [device newLibraryWithFile:path error:&error];
		if(library == nil)
		{
			bytecode->Messages += StringUtility::Format("Could not load the compiled Metal library for reflection: {0}.",
				error != nil ? error.localizedDescription.UTF8String : "unknown error");
#if !__has_feature(objc_arc)
			[device release];
#endif
			return bytecode;
		}

		NSString* entryPoint = [NSString stringWithUTF8String:createInformation.EntryPoint.c_str()];
		id<MTLFunction> function = nil;
		const bool reflectionSucceeded = ReflectLibrary(library, entryPoint, createInformation.Type, *bytecode, function);
#if !__has_feature(objc_arc)
		[function release];
		[library release];
		[device release];
#endif

		if(!reflectionSucceeded)
			return bytecode;
	}

	bytecode->Instructions.Size = (u32)libraryData.size();
	bytecode->Instructions.Data = (u8*)B3DAllocate(bytecode->Instructions.Size);
	std::memcpy(bytecode->Instructions.Data, libraryData.data(), bytecode->Instructions.Size);

	return bytecode;
}

#endif // B3D_PLATFORM_MACOS
