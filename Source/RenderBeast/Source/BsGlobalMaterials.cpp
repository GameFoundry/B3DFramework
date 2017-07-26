//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGlobalMaterials.h"

namespace bs { namespace ct
{
	GlobalMaterials::GlobalMaterials()
	{
		mTiledDeferredIBLMats[0] = bs_new<TTiledDeferredImageBasedLightingMat<1>>();
		mTiledDeferredIBLMats[1] = bs_new<TTiledDeferredImageBasedLightingMat<2>>();
		mTiledDeferredIBLMats[2] = bs_new<TTiledDeferredImageBasedLightingMat<4>>();
		mTiledDeferredIBLMats[3] = bs_new<TTiledDeferredImageBasedLightingMat<8>>();

		mTiledDeferredLightingMats[0] = bs_new<TTiledDeferredLightingMat<1>>();
		mTiledDeferredLightingMats[1] = bs_new<TTiledDeferredLightingMat<2>>();
		mTiledDeferredLightingMats[2] = bs_new<TTiledDeferredLightingMat<4>>();
		mTiledDeferredLightingMats[3] = bs_new<TTiledDeferredLightingMat<8>>();

		mSkyboxTextureMat = bs_new<SkyboxMat<false>>();
		mSkyboxColorMat = bs_new<SkyboxMat<true>>();
		mFlatFramebufferToTextureMat = bs_new<FlatFramebufferToTextureMat>();
	}

	GlobalMaterials::~GlobalMaterials()
	{
		for (UINT32 i = 0; i < 4; i++)
			bs_delete(mTiledDeferredIBLMats[i]);

		for (UINT32 i = 0; i < 4; i++)
			bs_delete(mTiledDeferredLightingMats[i]);

		bs_delete(mSkyboxTextureMat);
		bs_delete(mSkyboxColorMat);
		bs_delete(mFlatFramebufferToTextureMat);
	}

	ITiledDeferredImageBasedLightingMat* GlobalMaterials::getTileDeferredIBL(UINT32 msaa)
	{
		if (msaa == 1)
			return mTiledDeferredIBLMats[0];
		else if (msaa == 2)
			return mTiledDeferredIBLMats[1];
		else if (msaa == 4)
			return mTiledDeferredIBLMats[2];
		else
			return mTiledDeferredIBLMats[3];
	}

	ITiledDeferredLightingMat* GlobalMaterials::getTileDeferredLighting(bs::UINT32 msaa)
	{
		if (msaa == 1)
			return mTiledDeferredLightingMats[0];
		else if (msaa == 2)
			return mTiledDeferredLightingMats[1];
		else if (msaa == 4)
			return mTiledDeferredLightingMats[2];
		else
			return mTiledDeferredLightingMats[3];
	}
}}