//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFBXImportData.h"

using namespace b3d;

FBXImportNode::~FBXImportNode()
{
	for(auto& child : Children)
		B3DDelete(child);
}

FBXImportScene::~FBXImportScene()
{
	if(RootNode != nullptr)
		B3DDelete(RootNode);

	for(auto& mesh : Meshes)
		B3DDelete(mesh);
}
