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
		for(auto& material : materials)
		{
			HShader shader = br.GetShader(material.ShaderPath);
			if(shader.IsLoaded())
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
		for(u32 i = 0; i < materials.size(); i++)
		{
			materials[i].MetaData->ShaderPath = materials[i].ShaderPath;
			materials[i].MetaData->Shader = shaders[i];

			if(!shaders[i])
			{
				BS_LOG(Error, Renderer, "Failed to load renderer material: {0}", materials[i].ShaderPath);
				continue;
			}

			// Note: Making the assumption here that all the techniques are generated due to shader variations
			Vector<SPtr<ct::Technique>> techniques = shaders[i]->GetCompatibleTechniques();
			materials[i].MetaData->Instances.Resize((u32)techniques.size());

			for(auto& entry : techniques)
				materials[i].MetaData->Variations.Add(entry->GetVariation());

#if BS_PROFILING_ENABLED
			const String& filename = materials[i].ShaderPath.GetFilename(false);
			materials[i].MetaData->ProfilerSampleName = ProfilerString("RM: ") +
				ProfilerString(filename.data(), filename.size());
#endif
		}
	}

	ShaderDefines RendererMaterialManager::GetDefinesInternal(const Path& shaderPath)
	{
		ShaderDefines output;

		Vector<RendererMaterialData>& materials = GetMaterials();
		for(auto& entry : materials)
		{
			if(entry.ShaderPath == shaderPath)
				return entry.MetaData->Defines;
		}

		return output;
	}

	void RendererMaterialManager::DestroyOnCore()
	{
		Lock lock(GetMutex());

		Vector<RendererMaterialData>& materials = GetMaterials();
		for(u32 i = 0; i < materials.size(); i++)
		{
			materials[i].MetaData->Shader = nullptr;
			materials[i].MetaData->OverrideShader = nullptr;

			for(auto& entry : materials[i].MetaData->Instances)
			{
				if(entry != nullptr)
					bs_delete(entry);
			}

			materials[i].MetaData->Instances.Clear();
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
} // namespace bs
