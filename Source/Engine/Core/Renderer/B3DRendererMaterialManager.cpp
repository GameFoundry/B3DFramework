//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DRendererMaterialManager.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Resources/B3DBuiltinResources.h"
#include "CoreObject/B3DRenderThread.h"
#include "Material/B3DShader.h"

using namespace b3d;

RendererMaterialManager::RendererMaterialManager()
{
	GetRenderThread().PostCommand([this]() { InitOnRenderThread(); }, "RendererMaterialManager::Initialize");
}

RendererMaterialManager::~RendererMaterialManager()
{
	GetRenderThread().PostCommand([]() { DestroyOnRenderThread(); }, "RendererMaterialManager::Destroy");
}

void RendererMaterialManager::RegisterMaterial(render::RendererMaterialMetaData* metaData, const char* shaderPath)
{
	Lock lock(GetMutex());

	Vector<RendererMaterialData>& materials = GetMaterials();
	materials.push_back({ metaData, shaderPath });
}

void RendererMaterialManager::InitOnRenderThread()
{
	Lock lock(GetMutex());

	Vector<RendererMaterialData>& materials = GetMaterials();
	for(u32 i = 0; i < materials.size(); i++)
	{
		materials[i].MetaData->ShaderPath = materials[i].ShaderPath;

#if B3D_PROFILING_ENABLED
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

void RendererMaterialManager::DestroyOnRenderThread()
{
	Lock lock(GetMutex());

	Vector<RendererMaterialData>& materials = GetMaterials();
	for(u32 i = 0; i < materials.size(); i++)
	{
		render::RendererMaterialMetaData* metaData = materials[i].MetaData;

		Lock stateLock(metaData->StateMutex);

		metaData->Shader = nullptr;
		metaData->VariationInformation.Clear();
		metaData->VariationParameterSet.Clear();
		metaData->ShaderState = render::RendererMaterialShaderState::NotInitialized;
		metaData->ShaderInitializeOperation = TAsyncOp<TShared<render::Shader>>(AsyncOpEmpty());
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
