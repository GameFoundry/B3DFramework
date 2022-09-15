//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRendererMaterialManager.h"
#include "Renderer/BsRendererMaterial.h"
#include "Resources/BsBuiltinResources.h"
#include "CoreThread/BsCoreThread.h"
#include "Material/BsShader.h"

namespace bs
{
	RendererMaterialManager::RendererMaterialManager()
	{
		BuiltinResources& br = BuiltinResources::Instance();

		// Note: Ideally I want to avoid loading all materials, and instead just load those that are used.
		Vector<RendererMaterialData>& materials = GetMaterials();
		Vector<SPtr<ct::Shader>> shaders;
		for (auto& material : materials)
		{
			HShader shader = br.GetShader(material.shaderPath);
			if (shader.IsLoaded())
				shaders.push_back(shader->GetCore());
			else
				shaders.push_back(nullptr);
		}

		gCoreThread().QueueCommand(std::bind(&RendererMaterialManager::InitOnCore, shaders), CTQF_InternalQueue);
	}

	RendererMaterialManager::~RendererMaterialManager()
	{
		gCoreThread().QueueCommand(std::bind(&RendererMaterialManager::DestroyOnCore));
	}

	void RendererMaterialManager::RegisterMaterialInternal(ct::RendererMaterialMetaData* metaData, const char* shaderPath)
	{
		Lock lock(GetMutex());

		Vector<RendererMaterialData>& materials = GetMaterials();
		materials.push_back({ metaData, shaderPath });
	}

	void RendererMaterialManager::InitOnCore(const Vector<SPtr<ct::Shader>>& shaders)
	{
		Lock lock(GetMutex());

		Vector<RendererMaterialData>& materials = GetMaterials();
		for (UINT32 i = 0; i < materials.size(); i++)
		{
			materials[i].metaData->shaderPath = materials[i].shaderPath;
			materials[i].metaData->shader = shaders[i];

			if(!shaders[i])
			{
				BS_LOG(Error, Renderer, "Failed to load renderer material: {0}", materials[i].shaderPath);
				continue;
			}

			// Note: Making the assumption here that all the techniques are generated due to shader variations
			Vector<SPtr<ct::Technique>> techniques = shaders[i]->GetCompatibleTechniques();
			materials[i].metaData->instances.Resize((UINT32)techniques.size());

			for(auto& entry : techniques)
				materials[i].metaData->variations.Add(entry->GetVariation());

#if BS_PROFILING_ENABLED
			const String& filename = materials[i].shaderPath.GetFilename(false);
			materials[i].metaData->profilerSampleName = ProfilerString("RM: ") +
				ProfilerString(filename.data(), filename.size());
#endif
		}
	}

	ShaderDefines RendererMaterialManager::GetDefinesInternal(const Path& shaderPath)
	{
		ShaderDefines output;

		Vector<RendererMaterialData>& materials = GetMaterials();
		for (auto& entry : materials)
		{
			if (entry.shaderPath == shaderPath)
				return entry.metaData->defines;
		}

		return output;
	}

	void RendererMaterialManager::DestroyOnCore()
	{
		Lock lock(GetMutex());

		Vector<RendererMaterialData>& materials = GetMaterials();
		for (UINT32 i = 0; i < materials.size(); i++)
		{
			materials[i].metaData->shader = nullptr;
			materials[i].metaData->overrideShader = nullptr;

			for (auto& entry : materials[i].metaData->instances)
			{
				if(entry != nullptr)
					bs_delete(entry);
			}

			materials[i].metaData->instances.Clear();
		}
	}

	Vector<RendererMaterialManager::RendererMaterialData>& RendererMaterialManager::GetMaterials()
	{
		static Vector<RendererMaterialData> materials;
		return materials;
	}

	Mutex& RendererMaterialManager::GetMutex()
	{
		static Mutex mutex;
		return mutex;
	}
}
