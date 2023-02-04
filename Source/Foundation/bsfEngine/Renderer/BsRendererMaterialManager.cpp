//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRendererMaterialManager.h"
#include "Renderer/BsRendererMaterial.h"
#include "Resources/BsBuiltinResources.h"
#include "CoreThread/BsCoreThread.h"
#include "Material/BsShader.h"

using namespace bs;

RendererMaterialManager::RendererMaterialManager()
{
	GetCoreThread().QueueCommand([this]() { InitOnCore(); }, CTQF_InternalQueue);
}

RendererMaterialManager::~RendererMaterialManager()
{
	GetCoreThread().QueueCommand(std::bind(&RendererMaterialManager::DestroyOnCore));
}

void RendererMaterialManager::RegisterMaterial(ct::RendererMaterialMetaData* metaData, const char* shaderPath)
{
	Lock lock(GetMutex());

	Vector<RendererMaterialData>& materials = GetMaterials();
	materials.push_back({ metaData, shaderPath });
}

void RendererMaterialManager::InitOnCore()
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

void RendererMaterialManager::QueueOnRenderThread(Function<void()> callback)
{
	Lock lock(mAsyncCompilationMutex);
	mQueuedOperationsOnWorkerThread.push(std::move(callback));
}

void RendererMaterialManager::BlockUntilQueueEmpty()
{
	THROW_IF_NOT_CORE_THREAD

	{
		Lock lock(mAsyncCompilationMutex);

		B3D_ASSERT(mQueuedOperationsOnRenderThread.empty());
		std::swap(mQueuedOperationsOnWorkerThread, mQueuedOperationsOnRenderThread);
	}

	while(!mQueuedOperationsOnRenderThread.empty())
	{
		Function<void()> callback = std::move(mQueuedOperationsOnRenderThread.front());
		mQueuedOperationsOnRenderThread.pop();

		if(callback != nullptr)
			callback();
	}
}

void RendererMaterialManager::Update()
{
	auto fnUpdateOnRenderThread = [this]() {
		BlockUntilQueueEmpty();
	};

	GetCoreThread().QueueCommand(fnUpdateOnRenderThread, CTQF_InternalQueue);
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

		for(auto& entry : materials[i].MetaData->VariationInformation)
		{
			if(entry.RendererMaterialInstance != nullptr)
				B3DDelete(entry.RendererMaterialInstance);
		}

		materials[i].MetaData->VariationInformation.Clear();
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
