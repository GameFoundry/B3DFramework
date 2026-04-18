//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup GpuBackend
		 *  @{
		 */

		/** Data describing a texture view. */
		struct B3D_EXPORT TextureViewInformation
		{
			/** Determines which part of the texture is being viewed through the texture view. */
			TextureSurface Surface;

			/** Type of texture view. */
			GpuViewUsage Usage;
		};

		/**
		 * Texture views allow you to reference only a party of a texture. They may reference one or multiple mip-levels on one
		 * or multiple texture array slices. Selected mip level will apply to all slices.
		 *
		 * They also allow you to re-purpose a texture (for example make a render target a bindable texture).
		 *
		 * @note	Render thread.
		 */
		class B3D_EXPORT TextureView
		{
		public:

			/** @name Internal
			 *  @{
			 */

			class HashFunction
			{
			public:
				size_t operator()(const TextureViewInformation& key) const;
			};

			class EqualFunction
			{
			public:
				bool operator()(const TextureViewInformation& a, const TextureViewInformation& b) const;
			};

			/** @} */

			virtual ~TextureView() = default;

			/** Returns information describing the object. */
			const TextureViewInformation& GetInformation() const { return mInformation; }

		protected:
			TextureView(const TextureViewInformation &_desc);

		protected:
			friend class Texture;

			TextureViewInformation mInformation;
		};

		/** @} */
	} // namespace render
} // namespace b3d
