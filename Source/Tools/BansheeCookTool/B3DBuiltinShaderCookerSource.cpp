//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DBuiltinShaderCookerSource.h"

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

BuiltinShaderCookerSource::BuiltinShaderCookerSource(Path shaderFolder)
	: mShaderFolder(std::move(shaderFolder))
{
}

void BuiltinShaderCookerSource::GetItems(Vector<ShaderCookItem>& outItems)
{
	// Build the renderer-material lookup once: map each renderer-material shader (keyed by filename, which is what the
	// runtime cache key is derived from) to the distinct define sets it is registered with. This requires the renderer
	// materials to have registered, which is why the cook tool runs the real renderer over the Null GPU backend.
	Vector<RendererMaterialManager::RendererMaterialShaderInfo> rendererMaterialShaders;
	RendererMaterialManager::GetRegisteredMaterialShaders(rendererMaterialShaders);

	UnorderedMap<String, Vector<ShaderDefines>> rendererMaterialsByName;
	for(const RendererMaterialManager::RendererMaterialShaderInfo& info : rendererMaterialShaders)
		rendererMaterialsByName[info.ShaderPath.GetFilename(false)].push_back(info.Defines);

	// Glob the top-level *.bsl files. GetChildren is non-recursive, so the Includes sub-folder (which holds shared
	// shader code, not standalone shaders) is naturally excluded.
	Vector<Path> files;
	Vector<Path> directories;
	FileSystem::GetChildren(mShaderFolder, files, directories);

	Set<String> emittedRendererMaterialNames;

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

			emittedRendererMaterialNames.insert(name);
		}

		outItems.push_back(std::move(item));
	}

	// A registered renderer material with no matching source file in this folder would silently miss the prebuilt store
	// at runtime, so surface it loudly.
	for(const auto& entry : rendererMaterialsByName)
	{
		if(emittedRendererMaterialNames.find(entry.first) == emittedRendererMaterialNames.end())
			B3D_LOG(Warning, LogResources, "Renderer-material shader \"{0}\" is registered but no matching \"{0}.bsl\" was found in \"{1}\"; it will not be cooked.", entry.first, mShaderFolder.ToString());
	}
}
