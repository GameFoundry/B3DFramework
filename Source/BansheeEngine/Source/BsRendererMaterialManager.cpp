//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRendererMaterialManager.h"
#include "BsRendererMaterial.h"
#include "BsBuiltinResources.h"
#include "BsCoreThread.h"
#include "BsShader.h"

namespace bs
{
	RendererMaterialManager::RendererMaterialManager()
	{
		BuiltinResources& br = BuiltinResources::instance();

		// Note: Ideally I want to avoid loading all materials, and instead just load those that are used. This might be a
		// problem on lower end systems that don't support all renderer features.
		Vector<RendererMaterialData>& materials = getMaterials();
		Vector<SPtr<ct::Shader>> shaders;
		for (auto& material : materials)
		{
			HShader shader = br.getShader(material.resourcePath);
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

	void RendererMaterialManager::_registerMaterial(ct::RendererMaterialMetaData* metaData, const Path& shaderPath)
	{
		Lock lock(getMutex());

		Vector<RendererMaterialData>& materials = getMaterials();

		const SmallVector<ct::ShaderVariation, 4>& variations = metaData->variations.getVariations();

		if(variations.empty())
		{
			metaData->shaders.resize(1);
			metaData->instances.resize(1);

			Path resourcePath = _getVariationPath(shaderPath, 0);
			materials.push_back({ metaData, shaderPath, resourcePath, 0 });
		}
		else
		{
			metaData->shaders.resize(variations.size());
			metaData->instances.resize(variations.size());

			UINT32 variationIdx = 0;
			for (auto& variation : variations)
			{
				assert(variation.getIdx() == variationIdx);

				Path resourcePath = _getVariationPath(shaderPath, variationIdx);
				materials.push_back({ metaData, shaderPath, resourcePath, variationIdx });

				variationIdx++;
			}
		}
	}

	Vector<ShaderDefines> RendererMaterialManager::_getVariations(const Path& shaderPath)
	{
		Vector<ShaderDefines> output;

		Vector<RendererMaterialData>& materials = getMaterials();
		for (auto& entry : materials)
		{
			if (entry.shaderPath == shaderPath)
			{
				if(entry.metaData->variations.getVariations().empty())
				{
					output.push_back(ShaderDefines());
				}
				else
				{
					const ct::ShaderVariation& variation = entry.metaData->variations.get(entry.variationIdx);
					output.push_back(variation.getDefines());
				}
			}
		}

		return output;
	}

	Path RendererMaterialManager::_getVariationPath(const Path& shaderPath, UINT32 variationIdx)
	{
		if (variationIdx == 0)
		{
			return shaderPath;
		}
		else
		{
			Path uniquePath = shaderPath;
			uniquePath.setFilename(shaderPath.getWFilename(false) + L"_" + toWString(variationIdx) +
				shaderPath.getWExtension());

			return uniquePath;
		}
	}

	void RendererMaterialManager::initOnCore(const Vector<SPtr<ct::Shader>>& shaders)
	{
		Lock lock(getMutex());

		Vector<RendererMaterialData>& materials = getMaterials();
		for (UINT32 i = 0; i < materials.size(); i++)
			materials[i].metaData->shaders[materials[i].variationIdx] = shaders[i];
	}

	void RendererMaterialManager::destroyOnCore()
	{
		Lock lock(getMutex());

		Vector<RendererMaterialData>& materials = getMaterials();
		for (UINT32 i = 0; i < materials.size(); i++)
		{
			materials[i].metaData->shaders.clear();

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
