//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DShaderCookerSource.h"

#include "Renderer/B3DRendererMaterialManager.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Resources/B3DBuiltinResources.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

namespace
{
	/** Builds a deterministic, comparable key for a set of defines (sorted "name=value" lines). */
	String MakeDefinesKey(const ShaderDefines& defines)
	{
		const UnorderedMap<String, String> all = defines.GetAll();
		const Map<String, String> sorted(all.begin(), all.end());

		StringStream stream;
		for(const auto& entry : sorted)
			stream << entry.first << "=" << entry.second << "\n";

		return stream.str();
	}
}

ShaderCookerSource::ShaderCookerSource(Path shaderFolder)
	: mShaderFolder(std::move(shaderFolder))
{
}

void ShaderCookerSource::GetItems(Vector<ShaderCookItem>& outItems)
{
	// Build the renderer-material lookup: map each renderer-material shader (keyed by filename, which is what the
	// runtime cache key is derived from) to the distinct define sets it is registered with. This requires the renderer
	// materials to have registered, which is why the cook tool runs the real renderer over the Null GPU backend.
	Vector<RendererMaterialManager::RendererMaterialShaderInfo> rendererMaterialShaders;
	RendererMaterialManager::GetRegisteredMaterialShaders(rendererMaterialShaders);

	UnorderedMap<String, Vector<ShaderDefines>> rendererMaterialsByName;
	for(const RendererMaterialManager::RendererMaterialShaderInfo& info : rendererMaterialShaders)
		rendererMaterialsByName[info.ShaderPath.GetFilename(false)].push_back(info.Defines);

	// Glob the top-level *.bsl files. GetChildren is non-recursive, so sub-folders (which hold shared shader code, not standalone shaders) are naturally excluded.
	Vector<Path> files;
	Vector<Path> directories;
	FileSystem::GetChildren(mShaderFolder, files, directories);

	for(const Path& file : files)
	{
		if(file.GetExtension() != ".bsl")
			continue;

		const String name = file.GetFilename(false);

		const TShared<DataStream> stream = FileSystem::OpenFile(file);
		if(stream == nullptr)
		{
			B3D_LOG(Warning, LogResources, "Skipping shader \"{0}\": failed to open the source file.", file.ToString());
			continue;
		}

		ShaderCookItem item;
		item.Name = name;
		item.Source = stream->GetAsString();
		item.SourcePath = file;

		const auto found = rendererMaterialsByName.find(name);
		if(found == rendererMaterialsByName.end())
		{
			// Not a renderer material: a surface/builtin shader, compiled with no defines.
			item.CachePrefix = BuiltinResources::kBuiltinShaderCachePrefix;
		}
		else
		{
			// Renderer-material shader. Collapse to the distinct define sets the materials registered with.
			Vector<ShaderDefines> distinctDefines;
			Set<String> seenDefinesKeys;
			for(const ShaderDefines& defines : found->second)
			{
				if(seenDefinesKeys.insert(MakeDefinesKey(defines)).second)
					distinctDefines.push_back(defines);
			}

			// The cache key omits the defines, so several distinct sets for one shader collide on a single key (the same
			// ambiguity the runtime resolver has). Pick the first deterministically and warn rather than silently drop.
			if(distinctDefines.size() > 1)
				B3D_LOG(Warning, LogResources, "Renderer-material shader \"{0}\" is registered with {1} conflicting define sets, but the shader cache key does not include defines. Cooking with the first set only.", name, (u32)distinctDefines.size());

			item.CachePrefix = render::RendererMaterialBase::kRendererMaterialShaderCachePrefix;
			if(!distinctDefines.empty())
				item.Defines = distinctDefines.front();
		}

		outItems.push_back(std::move(item));
	}
}
