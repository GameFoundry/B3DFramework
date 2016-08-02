//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsResource.h"
#include "BsVector2.h"

namespace BansheeEngine
{
	/** @addtogroup 2D
	 *  @{
	 */

	/**
	 * Texture interface that encapsulates underlying texture which allows us to create a sprite texture atlas (for example 
	 * multiple SpriteTexture%s referencing different parts of a single Texture).
	 */
	class BS_EXPORT SpriteTexture : public Resource
	{
	public:
		/**	Gets the internal texture that the sprite texture references. */
		const HTexture& getTexture() const;

		/**	Sets the internal texture that the sprite texture references. */
		void setTexture(const HTexture& texture);

		/**
		 * Gets the offset into the referenced texture where the sprite starts.
		 *
		 * @return	Offset in UV coordinates, range [0, 1].
		 */
		Vector2 getOffset() const { return mUVOffset; }

		/**
		 * Sets the offset into the referenced texture where the sprite starts.
		 *
		 * @param	offset	Offset in UV coordinates, range [0, 1].
		 */
		void setOffset(const Vector2& offset) { mUVOffset = offset; }

		/**
		 * Gets the size of the sprite in the referenced texture.
		 *
		 * @return	Size in UV coordinates, range [0, 1].
		 */
		Vector2 getScale() const { return mUVScale; }

		/**
		 * Sets the size of the sprite in the referenced texture.
		 *
		 * @param	scale	Size in UV coordinates, range [0, 1].
		 */
		void setScale(const Vector2& scale) { mUVScale = scale; }

		/** Transforms wanted UV coordinates into coordinates you can use for sampling the internal texture. */
		Vector2 transformUV(const Vector2& uv) const;

		/**	Returns a dummy sprite texture. */
		static const HSpriteTexture& dummy();

		/**	Returns width of the sprite texture in pixels. */
		UINT32 getWidth() const;

		/**	Returns height of the sprite texture in pixels. */
		UINT32 getHeight() const;

		/**	Creates a new sprite texture that references the entire area of the provided texture. */
		static HSpriteTexture create(const HTexture& texture);

		/**	Creates a new sprite texture that references a sub-area of the provided	texture. */
		static HSpriteTexture create(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture);

		/**	Checks if the sprite texture and its internal texture have been loaded. */
		static bool checkIsLoaded(const HSpriteTexture& tex);

		/** @name Internal
		 *  @{
		 */

		/** Creates a new SpriteTexture without a resource handle. Use create() for normal use. */
		static SPtr<SpriteTexture> _createPtr(const HTexture& texture);

		/** Creates a new SpriteTexture without a resource handle. Use create() for normal use. */
		static SPtr<SpriteTexture> _createPtr(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture);

		/** @} */
	private:
		friend class SpriteTextureRTTI;

		/** @copydoc create(const Vector2&, const Vector2&, const HTexture&) */
		SpriteTexture(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture);

		/** @copydoc Resource::getResourceDependencies */
		void getResourceDependencies(FrameVector<HResource>& dependencies) const override;

		/** @copydoc CoreObject::getCoreDependencies */
		void getCoreDependencies(Vector<CoreObject*>& dependencies) override;

		HTexture mAtlasTexture;
		Vector2 mUVOffset;
		Vector2 mUVScale;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/**	Creates a new empty and uninitialized sprite texture. To be used by factory methods. */
		static SPtr<SpriteTexture> createEmpty();
	public:
		friend class SpriteTextureRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}