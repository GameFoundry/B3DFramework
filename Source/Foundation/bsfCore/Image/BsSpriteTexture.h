//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsResource.h"
#include "Math/BsVector2.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**
	 * Texture interface that encapsulates underlying texture which allows us to create a sprite texture atlas (for example 
	 * multiple SpriteTexture%s referencing different parts of a single Texture).
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) SpriteTexture : public Resource
	{
	public:
		/**	Determines the internal texture that the sprite texture references. */
		BS_SCRIPT_EXPORT(n:Texture,pr:setter)
		void setTexture(const HTexture& texture);

		/**	@copydoc setTexture() */
		BS_SCRIPT_EXPORT(n:Texture,pr:getter)
		const HTexture& getTexture() const;

		/**
		 * Determines the offset into the referenced texture where the sprite starts. The offset is in UV coordinates,
		 * in range [0, 1].
		 */
		BS_SCRIPT_EXPORT(n:Offset,pr:setter)
		void setOffset(const Vector2& offset) { mUVOffset = offset; }

		/** @copydoc setOffset() */
		BS_SCRIPT_EXPORT(n:Offset,pr:getter)
		Vector2 getOffset() const { return mUVOffset; }

		/** Determines the size of the sprite in the referenced texture. Size is in UV coordinates, range [0, 1]. */
		BS_SCRIPT_EXPORT(n:Scale,pr:setter)
		void setScale(const Vector2& scale) { mUVScale = scale; }

		/** @copydoc setScale() */
		BS_SCRIPT_EXPORT(n:Scale,pr:getter)
		Vector2 getScale() const { return mUVScale; }

		/** Transforms wanted UV coordinates into coordinates you can use for sampling the internal texture. */
		Vector2 transformUV(const Vector2& uv) const;

		/**	Returns a dummy sprite texture. */
		static const HSpriteTexture& dummy();

		/**	Returns width of the sprite texture in pixels. */
		BS_SCRIPT_EXPORT(n:Width,pr:getter)
		UINT32 getWidth() const;

		/**	Returns height of the sprite texture in pixels. */
		BS_SCRIPT_EXPORT(n:Height,pr:getter)
		UINT32 getHeight() const;

		/**	Creates a new sprite texture that references the entire area of the provided texture. */
		BS_SCRIPT_EXPORT(ec:SpriteTexture)
		static HSpriteTexture create(const HTexture& texture);

		/**	Creates a new sprite texture that references a sub-area of the provided	texture. */
		BS_SCRIPT_EXPORT(ec:SpriteTexture)
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
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */


	namespace ct
	{
		/** @addtogroup Resources
		 *  @{
		 */

		 /**
		  * Core thread version of a bs::SpriteTexture.
		  *
		  * @note	Core thread.
		  */
		class BS_CORE_EXPORT SpriteTexture : public CoreObject
		{
			// TODO - Not implemented
		};

		/** @} */
	}
}