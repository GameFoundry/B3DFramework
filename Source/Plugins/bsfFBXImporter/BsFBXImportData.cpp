//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFBXImportData.h"

namespace bs
{
	FBXImportNode::~FBXImportNode()
	{
		for (auto& child : children)
			bs_delete(child);
	}

	FBXImportScene::~FBXImportScene()
	{
		if (rootNode != nullptr)
			bs_delete(rootNode);

		for (auto& mesh : meshes)
			bs_delete(mesh);
	}
}