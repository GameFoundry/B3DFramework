//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DRendererExtension.h"

#include "B3DRendererScene.h"
#include "CoreObject/B3DCoreObject.h"
#include "CoreObject/B3DRenderThread.h"
#include "Renderer/B3DRendererManager.h"
#include "Renderer/B3DRenderer.h"

using namespace b3d;

void RendererExtension::QueueInitializeOnRenderThread(RendererExtension* extension, const Any& data, const SPtr<RendererScene>& scene)
{
	auto renderThreadInitializer = [extension, data, scene = B3DGetRenderProxy(scene)]()
	{
		if(scene != nullptr)
			scene->AddExtension(extension);
		else
			render::GetRenderer()->AddExtension(extension);

		extension->mAssociatedScene = scene;
		extension->Initialize(data);
	};

	GetRenderThread().PostCommand(renderThreadInitializer, "RendererExtension::Initialize");
}

void RendererExtension::QueueDeleteOnRenderThread(RendererExtension* extension)
{
	auto renderThreadDeleter = [extension]()
	{
		if(!B3DIsWeakUnassigned(extension->mAssociatedScene))
		{
			const SPtr<render::RendererScene>& scene = extension->mAssociatedScene.lock();
			if(scene != nullptr)
				scene->RemoveExtension(extension);
		}
		else
		{
			render::GetRenderer()->RemoveExtension(extension);
		}

		extension->Destroy();
		extension->~RendererExtension();

		B3DFree(extension);
	};

	// Queue deletion on the render thread
	GetRenderThread().PostCommand(renderThreadDeleter, "RendererExtension::Destroy");
}

bool RendererExtension::SortFunction::operator()(const RendererExtension* lhs, const RendererExtension* rhs) const
{
	// Sort by alpha setting first, then by cull mode, then by index
	if(lhs->GetLocation() == rhs->GetLocation())
	{
		if(lhs->GetPriority() == rhs->GetPriority())
			return lhs > rhs; // Use address, at this point it doesn't matter, but std::set requires us to differentiate
		else
			return lhs->GetPriority() > rhs->GetPriority();
	}
	else
		return (u32)lhs->GetLocation() < (u32)rhs->GetLocation();
}

