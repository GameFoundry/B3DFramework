//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFBXImportData.h"

using namespace bs;

FBXImportNode::~FBXImportNode()
{
	for(auto& child : Children)
		bs_delete(child);
}

FBXImportScene::~FBXImportScene()
{
	if(RootNode != nullptr)
		bs_delete(RootNode);

	for(auto& mesh : Meshes)
		bs_delete(mesh);
}
