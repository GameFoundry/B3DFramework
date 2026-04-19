//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once


namespace b3d
{
	namespace render
	{
		struct GpuParticleConstants
		{
			static constexpr u32 kTexSize = 1024;
			static constexpr u32 kTileSize = 4;
			static constexpr u32 kParticlesPerTile = kTileSize * kTileSize;
			static constexpr u32 kTileCount1D = kTexSize / kTileSize;
			static constexpr u32 kTileCount = kTileCount1D * kTileCount1D;

			static_assert((kTexSize & (kTexSize - 1)) == 0, "Particle texture size not a power of two");
			static_assert((kTileSize & (kTileSize - 1)) == 0, "Particle tile size not a power of two");

			static constexpr u32 kTilesPerInstance = 8;
			static constexpr u32 kParticlesPerInstance = kTilesPerInstance * kParticlesPerTile;
		};

		/** @} */
	} // namespace render
} // namespace b3d
