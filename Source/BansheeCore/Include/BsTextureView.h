//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Data describing a texture view. */
	struct BS_CORE_EXPORT TEXTURE_VIEW_DESC
	{
		/**
		 * First mip level of the parent texture the view binds (0 - base level). This applied to all array slices 
		 * specified below.
		 */
		UINT32 mostDetailMip;

		/** Number of mip levels to bind to the view. This applied to all array slices specified below. */
		UINT32 numMips;

		/**
		 * First array slice the view binds to. This will be array index for 1D and 2D array textures, texture slice index 
		 * for 3D textures, and face index for cube textures(cube index * 6).
 		 */
		UINT32 firstArraySlice;

		/**
		 * Number of array slices to bind tot he view. This will be number of array elements for 1D and 2D array textures, 
		 * number of slices for 3D textures, and number of cubes for cube textures.
		 */
		UINT32 numArraySlices;

		/** Type of texture view. */
		GpuViewUsage usage;
	};

	/**
	 * Texture views allow you to reference only a party of a texture. They may reference one or multiple mip-levels on one
	 * or multiple texture array slices. Selected mip level will apply to all slices.
	 *
	 * They also allow you to re-purpose a texture (for example make a render target a bindable texture).
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT TextureView
	{
	public:
		class HashFunction
		{
		public:
			size_t operator()(const TEXTURE_VIEW_DESC &key) const;
		};

		class EqualFunction
		{
		public:
			bool operator()(const TEXTURE_VIEW_DESC &a, const TEXTURE_VIEW_DESC &b) const;
		};

		virtual ~TextureView();

		/**	Returns the most detailed mip level visible by the view. */
		UINT32 getMostDetailedMip() const { return mDesc.mostDetailMip; }

		/**	Returns the number of mip levels in a single slice visible by the view. */
		UINT32 getNumMips() const { return mDesc.numMips; }

		/**	Returns the first array slice index visible by this view. */
		UINT32 getFirstArraySlice() const { return mDesc.firstArraySlice; }

		/**	Returns the number of array slices visible by this view. */
		UINT32 getNumArraySlices() const { return mDesc.numArraySlices; }

		/**	Returns texture view usage. This determines where on the pipeline can be bind the view. */
		GpuViewUsage getUsage() const { return mDesc.usage; }

		/**	Returns the descriptor structure used for initializing the view. */
		const TEXTURE_VIEW_DESC& getDesc() const { return mDesc; }

		/**	Gets the owner texture the view is referencing. */
		SPtr<TextureCore> getTexture() const { return mOwnerTexture; }

	protected:
		TextureView(const SPtr<TextureCore>& texture, const TEXTURE_VIEW_DESC& _desc);

	protected:
		friend class TextureCore;

		TEXTURE_VIEW_DESC mDesc;
		SPtr<TextureCore> mOwnerTexture;
	};

	/** @} */
}