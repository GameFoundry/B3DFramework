//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/BsVectorSprite.h"
#include "2D/BsSpriteManager.h"

using namespace bs;

VectorSprite::~VectorSprite()
{
	ClearMesh();
}

void VectorSprite::Update(const VectorSpriteInformation& information, u64 groupId)
{
	if(!information.VectorPath.IsLoaded())
	{
		ClearMesh();
		return;
	}

	// Actually generate a mesh
	if(mCachedRenderElements.size() < 1)
		mCachedRenderElements.resize(1);

	SpriteRenderElementData& renderElementData = mCachedRenderElements[0];
	{
		if(renderElementData.QuadCount != 1)
		{
			renderElementData.VertexPositions = mPositionBuffer.data();
			renderElementData.VertexUVs = mUVBuffer.data();
			renderElementData.Indices = mIndexBuffer.data();
			renderElementData.QuadCount = 1;
		}

		SpriteMaterialInfo& matInfo = renderElementData.MaterialInformation;
		matInfo.GroupId = groupId;
		//matInfo.Texture = tex; // TODO
		matInfo.Tint = information.Color;

		// TODO - Use premultiplied material
		//renderElementData.Material = SpriteManager::Instance().GetImageMaterial(
		//	information.Transparent ? SpriteMaterialTransparency::Alpha : SpriteMaterialTransparency::Opaque, animated);
	}

	renderElementData.Indices[0] = 0;
	renderElementData.Indices[1] = 1;
	renderElementData.Indices[2] = 2;
	renderElementData.Indices[3] = 1;
	renderElementData.Indices[4] = 3;
	renderElementData.Indices[5] = 2;

	renderElementData.VertexPositions[0] = Vector2(0.0f, 0.0f);
	renderElementData.VertexPositions[1] = Vector2((float)information.Width, 0.0f);
	renderElementData.VertexPositions[2] = Vector2(0.0f, (float)information.Height);
	renderElementData.VertexPositions[3] = Vector2((float)information.Width, (float)information.Height);

	// TODO
	//renderElementData.VertexUVs[0] = Vector2(uvOffset.X, uvOffset.Y);
	//renderElementData.VertexUVs[1] = Vector2(uvOffset.X + uvScale.X, uvOffset.Y);
	//renderElementData.VertexUVs[2] = Vector2(uvOffset.X, uvOffset.Y + uvScale.Y);
	//renderElementData.VertexUVs[3] = Vector2(uvOffset.X + uvScale.X, uvOffset.Y + uvScale.Y);

	UpdateBounds();
}

void VectorSprite::ClearMesh()
{
	mCachedRenderElements.clear();
	UpdateBounds();
}
