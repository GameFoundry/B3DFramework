//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCoreRenderer.h"
#include "BsCoreThread.h"
#include "BsRenderAPI.h"
#include "BsMesh.h"
#include "BsMaterial.h"
#include "BsPass.h"
#include "BsBlendState.h"
#include "BsDepthStencilState.h"
#include "BsRasterizerState.h"
#include "BsGpuParams.h"

namespace bs
{
	CoreRenderer::CoreRenderer()
	{ }

	SPtr<RendererMeshData> CoreRenderer::_createMeshData(UINT32 numVertices, UINT32 numIndices, VertexLayout layout, IndexType indexType)
	{
		return bs_shared_ptr<RendererMeshData>(new (bs_alloc<RendererMeshData>()) 
			RendererMeshData(numVertices, numIndices, layout, indexType));
	}

	SPtr<RendererMeshData> CoreRenderer::_createMeshData(const SPtr<MeshData>& meshData)
	{
		return bs_shared_ptr<RendererMeshData>(new (bs_alloc<RendererMeshData>())
			RendererMeshData(meshData));
	}

	void CoreRenderer::registerRenderCallback(const CameraCore* camera, INT32 index, 
		const std::function<void()>& callback, bool isOverlay)
	{
		mRenderCallbacks[camera][index] = { isOverlay, callback };
	}

	void CoreRenderer::unregisterRenderCallback(const CameraCore* camera, INT32 index)
	{
		auto iterFind = mRenderCallbacks.find(camera);
		if (iterFind != mRenderCallbacks.end())
		{
			iterFind->second.erase(index);

			if (iterFind->second.empty())
				mRenderCallbacks.erase(iterFind);
		}
	}
}