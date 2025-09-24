//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DRendererScene.h"
#include "B3DRenderer.h"

namespace b3d
{
	SPtr<RendererScene> RendererScene::Create()
	{
		RendererScene* rendererScene = new (B3DAllocate<RendererScene>()) RendererScene();
		SPtr<RendererScene> rendererSceneShared = B3DMakeSharedFromExisting(rendererScene);
		rendererSceneShared->SetShared(rendererSceneShared);
		rendererSceneShared->Initialize();

		return rendererSceneShared;
	}

	SPtr<render::RenderProxy> RendererScene::CreateRenderProxy() const
	{
		return render::GetRenderer()->CreateScene();
	}

	namespace render
	{
		void RendererScene::UpdateCombinedRendererExtensionsIfNeeded(const Set<RendererExtension*, RendererExtension::SortFunction>& globalRendererExtensions, bool forceUpdate)
		{
			if(!forceUpdate && !mCombinedRendererExtensionsDirty)
				return;

			mCombinedRendererExtensions.clear();

			for(const auto& entry : globalRendererExtensions)
				mCombinedRendererExtensions.insert(entry);

			for(const auto& entry : mRendererExtensions)
				mCombinedRendererExtensions.insert(entry);

			mCombinedRendererExtensionsDirty = false;
		}
	}
}
