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
	 * Descriptor that describes a simple sprite sheet animation. The parent texture is split into a grid of 
	 * @p numRows x @p numColumns, each representing one frame of the animation. Every frame is of equal size. Frames are
	 * sequentially evaluated starting from the top-most row, iterating over all columns in a row and then moving to next
	 * row, up to @p count frames. Frames in rows/colums past @p count. @p fps frames are evaluated every second, allowing
	 * you to control animation speed.
	 */
	struct BS_SCRIPT_EXPORT(m:Rendering,pl:true) SpriteSheetGridAnimation
	{
		SpriteSheetGridAnimation() = default;
		SpriteSheetGridAnimation(UINT32 numRows, UINT32 numColumns, UINT32 count, UINT32 fps)
			: numRows(numRows), numColumns(numColumns), count(count), fps(fps)
		{ }

		/** 
		 * Number of rows to divide the parent's texture area. Determines height of the individual frame (depends on 
		 * parent texture size). 
		 */
		UINT32 numRows = 1;

		/** 
		 * Number of columns to divide the parent's texture area. Determines column of the individual frame (depends on 
		 * parent texture size). 
		 */
		UINT32 numColumns = 1;

		/** Number of frames in the animation. Must be less or equal than @p numRows * @p numColumns. */
		UINT32 count = 1;

		/** How many frames to evaluate each second. Determines the animation speed. */
		UINT32 fps = 8;
	};

	/** Type of playback to use for an animation of a SpriteTexture. */
	enum class BS_SCRIPT_EXPORT(m:Rendering) SpriteAnimationPlayback
	{
		/** Do not animate. */
		None,
		/** Animate once until the end of the animation is reached. */
		Normal,
		/** Animate to the end of the animation then loop around. */
		Loop,
		/** Loop the animation but reverse playback when the end is reached. */
		PingPong
	};

	/** @} */

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Base class used for both sim and core thread SpriteTexture implementations. */
	class BS_CORE_EXPORT SpriteTextureBase
	{
	public:
		SpriteTextureBase(const Vector2& uvOffset, const Vector2& uvScale)
			:mUVOffset(uvOffset), mUVScale(uvScale)
		{ }
		virtual ~SpriteTextureBase() = default;

		/**
		 * Determines the offset into the referenced texture where the sprite starts. The offset is in UV coordinates,
		 * in range [0, 1].
		 */
		BS_SCRIPT_EXPORT(n:Offset,pr:setter)
		void setOffset(const Vector2& offset) { mUVOffset = offset; _markCoreDirty(); }

		/** @copydoc setOffset() */
		BS_SCRIPT_EXPORT(n:Offset,pr:getter)
		Vector2 getOffset() const { return mUVOffset; }

		/** Determines the size of the sprite in the referenced texture. Size is in UV coordinates, range [0, 1]. */
		BS_SCRIPT_EXPORT(n:Scale,pr:setter)
		void setScale(const Vector2& scale) { mUVScale = scale; _markCoreDirty(); }

		/** @copydoc setScale() */
		BS_SCRIPT_EXPORT(n:Scale,pr:getter)
		Vector2 getScale() const { return mUVScale; }

		/** Transforms wanted UV coordinates into coordinates you can use for sampling the internal texture. */
		Vector2 transformUV(const Vector2& uv) const { return mUVOffset + uv * mUVScale; }

		/** 
		 * Evaluates the UV coordinate offset and size to use at the specified time. If the sprite texture doesn't
		 * have animation playback enabled then just the default offset and size will be provided, otherwise the
		 * animation will be evaluated and appropriate UV returned.
		 */
		Rect2 evaluate(float t) const;

		/** 
		 * Sets properties describing sprite animation. The animation splits the sprite area into a grid of sub-images
		 * which can be evaluated over time. In order to view the animation you must also enable playback through
		 * setAnimationPlayback().
		 */
		BS_SCRIPT_EXPORT(n:Animation,pr:setter)
		void setAnimation(const SpriteSheetGridAnimation& anim) { mAnimation = anim; _markCoreDirty(); }

		/** @copydoc setAnimation */
		BS_SCRIPT_EXPORT(n:Animation,pr:getter)
		const SpriteSheetGridAnimation& getAnimation() const { return mAnimation; }

		/** Determines if and how should the sprite animation play. */
		BS_SCRIPT_EXPORT(n:AnimationPlayback,pr:setter)
		void setAnimationPlayback(SpriteAnimationPlayback playback) { mPlayback = playback; _markCoreDirty(); }

		/** @copydoc setAnimationPlayback */
		BS_SCRIPT_EXPORT(n:AnimationPlayback,pr:getter)
		SpriteAnimationPlayback getAnimationPlayback() const { return mPlayback; };

	protected:
		/** Marks the contents of the sim thread object as dirty, causing it to sync with its core thread counterpart. */
		virtual void _markCoreDirty() { }

		Vector2 mUVOffset;
		Vector2 mUVScale;

		SpriteAnimationPlayback mPlayback = SpriteAnimationPlayback::None;
		SpriteSheetGridAnimation mAnimation;
	};

	/** Templated base class used for both sim and core thread SpriteTexture implementations. */
	template<bool Core>
	class BS_CORE_EXPORT TSpriteTexture : public SpriteTextureBase
	{
	public:
		using TextureType = CoreVariantHandleType<Texture, Core>;

		TSpriteTexture(const Vector2& uvOffset, const Vector2& uvScale, TextureType atlasTexture)
			:SpriteTextureBase(uvOffset, uvScale), mAtlasTexture(std::move(atlasTexture))
		{ }

		virtual ~TSpriteTexture() = default;

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P p);

	protected:
		TextureType mAtlasTexture;
	};

	/** @} */
	/** @addtogroup Resources
	 *  @{
	 */


	/**
	 * Texture that references a part of a larger texture by specifying an UV subset. When the sprite texture is rendererd
	 * only the portion of the texture specified by the UV subset will be rendered. This allows you to use the same texture
	 * for multiple sprites (texture atlasing). Sprite textures also allow you to specify sprite sheet animation by varying
	 * which portion of the UV is selected over time.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) SpriteTexture : public Resource, public TSpriteTexture<false>
	{
	public:
		/**	Determines the internal texture that the sprite texture references. */
		BS_SCRIPT_EXPORT(n:Texture,pr:setter)
		void setTexture(const HTexture& texture) { mAtlasTexture = texture; markDependenciesDirty(); }

		/**	@copydoc setTexture() */
		BS_SCRIPT_EXPORT(n:Texture,pr:getter)
		const HTexture& getTexture() const { return mAtlasTexture; }

		/**	Returns width of the sprite texture in pixels. */
		BS_SCRIPT_EXPORT(n:Width,pr:getter)
		UINT32 getWidth() const;

		/**	Returns height of the sprite texture in pixels. */
		BS_SCRIPT_EXPORT(n:Height,pr:getter)
		UINT32 getHeight() const;

		/**	Retrieves a core implementation of a sprite texture usable only from the core thread. */
		SPtr<ct::SpriteTexture> getCore() const;

		/**	Creates a new sprite texture that references the entire area of the provided texture. */
		BS_SCRIPT_EXPORT(ec:SpriteTexture)
		static HSpriteTexture create(const HTexture& texture);

		/**	Creates a new sprite texture that references a sub-area of the provided	texture. */
		BS_SCRIPT_EXPORT(ec:SpriteTexture)
		static HSpriteTexture create(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture);

		/**	Checks if the sprite texture and its internal texture have been loaded. */
		static bool checkIsLoaded(const HSpriteTexture& tex);

		/**	Returns a dummy sprite texture. */
		static const HSpriteTexture& dummy();

		/** @name Internal
		 *  @{
		 */

		/** Creates a new SpriteTexture without a resource handle. Use create() for normal use. */
		static SPtr<SpriteTexture> _createPtr(const HTexture& texture);

		/** Creates a new SpriteTexture without a resource handle. Use create() for normal use. */
		static SPtr<SpriteTexture> _createPtr(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture);

		/** @copydoc SpriteTextureBase::_markCoreDirty */
		void _markCoreDirty() override;

		/** @} */
	private:
		friend class SpriteTextureRTTI;

		/** @copydoc create(const Vector2&, const Vector2&, const HTexture&) */
		SpriteTexture(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/** @copydoc Resource::getResourceDependencies */
		void getResourceDependencies(FrameVector<HResource>& dependencies) const override;

		/** @copydoc CoreObject::getCoreDependencies */
		void getCoreDependencies(Vector<CoreObject*>& dependencies) override;

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
		/** @addtogroup Resources-Internal
		 *  @{
		 */

		/**
		 * Core thread version of a bs::SpriteTexture.
		 *
		 * @note	Core thread.
		 */
		class BS_CORE_EXPORT SpriteTexture : public CoreObject, public TSpriteTexture<true>
		{
		public:
			/**	Determines the internal texture that the sprite texture references. */
			void setTexture(const SPtr<ct::Texture>& texture) { mAtlasTexture = texture; }

			/**	@copydoc setTexture() */
			const SPtr<ct::Texture>& getTexture() const { return mAtlasTexture; }

		private:
			friend class bs::SpriteTexture;

			SpriteTexture(const Vector2& uvOffset, const Vector2& uvScale, SPtr<Texture> texture, 
				const SpriteSheetGridAnimation& anim, SpriteAnimationPlayback playback);

			/** @copydoc CoreObject::syncToCore */
			void syncToCore(const CoreSyncData& data) override;
		};

		/** @} */
	}
}