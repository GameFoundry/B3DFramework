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
		BuiltinResources& br = BuiltinResources::instance();

		// Note: Ideally I want to avoid loading all materials, and instead just load those that are used.
		Vector<RendererMaterialData>& materials = getMaterials();
		Vector<SPtr<ct::Shader>> shaders;
		for (auto& material : materials)
		{
			HShader shader = br.getShader(material.shaderPath);
			if (shader.isLoaded())
				shaders.push_back(shader->getCore());
			else
				shaders.push_back(nullptr);
		}

		gCoreThread().queueCommand(std::bind(&RendererMaterialManager::initOnCore, shaders), CTQF_InternalQueue);
	}

	RendererMaterialManager::~RendererMaterialManager()
	{
		gCoreThread().queueCommand(std::bind(&RendererMaterialManager::destroyOnCore));
	}

	void RendererMaterialManager::_registerMaterial(ct::RendererMaterialMetaData* metaData, const char* shaderPath)
	{
		Lock lock(getMutex());

		Vector<RendererMaterialData>& materials = getMaterials();
		materials.push_back({ metaData, shaderPath });
	}

	void RendererMaterialManager::initOnCore(const Vector<SPtr<ct::Shader>>& shaders)
	{
		Lock lock(getMutex());

		Vector<RendererMaterialData>& materials = getMaterials();
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
			Vector<SPtr<ct::Technique>> techniques = shaders[i]->getCompatibleTechniques();
			materials[i].metaData->instances.resize((UINT32)techniques.size());

			for(auto& entry : techniques)
				materials[i].metaData->variations.add(entry->getVariation());

#if BS_PROFILING_ENABLED
			const String& filename = materials[i].shaderPath.getFilename(false);
			materials[i].metaData->profilerSampleName = ProfilerString("RM: ") +
				ProfilerString(filename.data(), filename.size());
#endif
		}
	}

	ShaderDefines RendererMaterialManager::_getDefines(const Path& shaderPath)
	{
		ShaderDefines output;

		Vector<RendererMaterialData>& materials = getMaterials();
		for (auto& entry : materials)
		{
			if (entry.shaderPath == shaderPath)
				return entry.metaData->defines;
		}

		return output;
	}

	void RendererMaterialManager::destroyOnCore()
	{
		Lock lock(getMutex());

		Vector<RendererMaterialData>& materials = getMaterials();
		for (UINT32 i = 0; i < materials.size(); i++)
		{
			materials[i].metaData->shader = nullptr;
			materials[i].metaData->overrideShader = nullptr;

			for (auto& entry : materials[i].metaData->instances)
			{
				if(entry != nullptr)
					bs_delete(entry);
			}

			materials[i].metaData->instances.clear();
		}
	}

	Vector<RendererMaterialManager::RendererMaterialData>& RendererMaterialManager::getMaterials()
	{
		static Vector<RendererMaterialData> materials;
		return materials;
	}

	Mutex& RendererMaterialManager::getMutex()
	{
		static Mutex mutex;
		return mutex;
	}
}
