//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFBXImportData.h"

namespace bs
{
	FBXImportNode::~FBXImportNode()
	{
		for (auto& child : children)
			bs_delete(child);
	}

	FBXImportScene::FBXImportScene()
		:rootNode(nullptr), scaleFactor(1.0f), globalScale(Matrix4::IDENTITY)
	{ }

	FBXImportScene::~FBXImportScene()
	{
		if (rootNode != nullptr)
			bs_delete(rootNode);

		for (auto& mesh : meshes)
			bs_delete(mesh);
	}
}