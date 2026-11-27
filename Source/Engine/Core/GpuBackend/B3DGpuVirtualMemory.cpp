//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuVirtualMemory.h"

using namespace b3d;

void GpuVirtualTextureLayout::UpdatePageCounts(u32 arrayLayerCount)
{
	PageCountPerLayer = HasSharedTail ? 0 : TailPageCount;
	for(u32 mipLevel = 0; mipLevel < FirstTailMipLevel; mipLevel++)
	{
		const Vector3I tileCount = GetTileCount(mipLevel);
		PageCountPerLayer += (u32)(tileCount.X * tileCount.Y * tileCount.Z);
	}

	PageCount = (HasSharedTail ? TailPageCount : 0) + PageCountPerLayer * arrayLayerCount;
}

Vector3I GpuVirtualTextureLayout::GetTileCount(u32 mipLevel) const
{
	B3D_ASSERT(mipLevel < FirstTailMipLevel);

	const auto fnGetTileCount = [mipLevel](i32 size, i32 tileSize)
	{
		const i32 mipSize = std::max(size >> mipLevel, 1);
		return (mipSize + tileSize - 1) / tileSize;
	};

	return Vector3I(fnGetTileCount(Size.X, TileSize.X), fnGetTileCount(Size.Y, TileSize.Y), fnGetTileCount(Size.Z, TileSize.Z));
}

GpuPageRange GpuVirtualTextureLayout::GetMipLevelPages(u32 arrayLayer, u32 mipLevel) const
{
	B3D_ASSERT(mipLevel < MipLevelCount);

	const u32 sharedTailPageCount = HasSharedTail ? TailPageCount : 0;
	const u32 firstLayerPageIndex = sharedTailPageCount + arrayLayer * PageCountPerLayer;
	if(mipLevel >= FirstTailMipLevel)
		return GpuPageRange(HasSharedTail ? 0 : firstLayerPageIndex, TailPageCount);

	// Smaller mip levels come first
	u32 firstPageIndex = firstLayerPageIndex + (HasSharedTail ? 0 : TailPageCount);
	for(u32 smallerMipLevel = mipLevel + 1; smallerMipLevel < FirstTailMipLevel; smallerMipLevel++)
	{
		const Vector3I tileCount = GetTileCount(smallerMipLevel);
		firstPageIndex += (u32)(tileCount.X * tileCount.Y * tileCount.Z);
	}

	const Vector3I tileCount = GetTileCount(mipLevel);
	return GpuPageRange(firstPageIndex, (u32)(tileCount.X * tileCount.Y * tileCount.Z));
}

GpuPageRange GpuVirtualTextureLayout::GetMipChainPages(u32 arrayLayer, u32 firstMipLevel) const
{
	if(firstMipLevel >= FirstTailMipLevel)
		return GetMipLevelPages(arrayLayer, firstMipLevel);

	const u32 firstLayerPageIndex = (HasSharedTail ? TailPageCount : 0) + arrayLayer * PageCountPerLayer;
	const GpuPageRange mipLevelPages = GetMipLevelPages(arrayLayer, firstMipLevel);

	return GpuPageRange(firstLayerPageIndex, mipLevelPages.GetEndPageIndex() - firstLayerPageIndex);
}

u32 GpuVirtualTextureLayout::GetPageIndex(u32 arrayLayer, u32 mipLevel, const Vector3I& tile) const
{
	const Vector3I tileCount = GetTileCount(mipLevel);
	B3D_ASSERT(tile.X >= 0 && tile.Y >= 0 && tile.Z >= 0 && tile.X < tileCount.X && tile.Y < tileCount.Y && tile.Z < tileCount.Z);

	const u32 tileIndex = (u32)((tile.Z * tileCount.Y + tile.Y) * tileCount.X + tile.X);
	return GetMipLevelPages(arrayLayer, mipLevel).FirstPageIndex + tileIndex;
}
