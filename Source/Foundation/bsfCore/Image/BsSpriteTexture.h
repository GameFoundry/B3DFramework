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
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering), ExportAsStruct(true)) SpriteSheetGridAnimation
	{
		SpriteSheetGridAnimation() = default;

		SpriteSheetGridAnimation(u32 numRows, u32 numColumns, u32 count, u32 fps)
			: NumRows(numRows), NumColumns(numColumns), Count(count), Fps(fps)
		{}

		/**
		 * Number of rows to divide the parent's texture area. Determines height of the individual frame (depends on
		 * parent texture size).
		 */
		u32 NumRows = 1;

		/**
		 * Number of columns to divide the parent's texture area. Determines column of the individual frame (depends on
		 * parent texture size).
		 */
		u32 NumColumns = 1;

		/** Number of frames in the animation. Must be less or equal than @p numRows * @p numColumns. */
		u32 Count = 1;

		/** How many frames to evaluate each second. Determines the animation speed. */
		u32 Fps = 8;
	};

	/** Type of playback to use for an animation of a SpriteTexture. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) SpriteAnimationPlayback
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

	/** Base class used for both main and render thread counterparts of SpriteTexture. */
	class B3D_CORE_EXPORT SpriteTextureBase
	{
	public:
		SpriteTextureBase(const Vector2& uvOffset, const Vector2& uvScale)
			: mUVOffset(uvOffset), mUVScale(uvScale)
		{}

		virtual ~SpriteTextureBase() = default;

		/**
		 * Determines the offset into the referenced texture where the sprite starts. The offset is in UV coordinates,
		 * in range [0, 1].
		 */
		B3D_SCRIPT_EXPORT(ExportName(Offset), Property(Setter))
		void SetOffset(const Vector2& offset)
		{
			mUVOffset = offset;
			MarkCoreDirtyInternal();
		}

		/** @copydoc SetOffset() */
		B3D_SCRIPT_EXPORT(ExportName(Offset), Property(Getter))
		Vector2 GetOffset() const { return mUVOffset; }

		/** Determines the size of the sprite in the referenced texture. Size is in UV coordinates, range [0, 1]. */
		B3D_SCRIPT_EXPORT(ExportName(Scale), Property(Setter))
		void SetScale(const Vector2& scale)
		{
			mUVScale = scale;
			MarkCoreDirtyInternal();
		}

		/** @copydoc SetScale() */
		B3D_SCRIPT_EXPORT(ExportName(Scale), Property(Getter))
		Vector2 GetScale() const { return mUVScale; }

		/** Transforms wanted UV coordinates into coordinates you can use for sampling the internal texture. */
		Vector2 TransformUv(const Vector2& uv) const { return mUVOffset + uv * mUVScale; }

		/**
		 * Evaluates the UV coordinate offset and size to use at the specified time. If the sprite texture doesn't
		 * have animation playback enabled then just the default offset and size will be provided, otherwise the
		 * animation will be evaluated and appropriate UV returned.
		 */
		Rect2 Evaluate(float t) const;

		/** Returns the row and column of the current animation frame for time @p t. */
		void GetAnimationFrame(float t, u32& row, u32& column) const;

		/**
		 * Sets properties describing sprite animation. The animation splits the sprite area into a grid of sub-images
		 * which can be evaluated over time. In order to view the animation you must also enable playback through
		 * setAnimationPlayback().
		 */
		B3D_SCRIPT_EXPORT(ExportName(Animation), Property(Setter))
		void SetAnimation(const SpriteSheetGridAnimation& anim)
		{
			mAnimation = anim;
			MarkCoreDirtyInternal();
		}

		/** @copydoc SetAnimation */
		B3D_SCRIPT_EXPORT(ExportName(Animation), Property(Getter))
		const SpriteSheetGridAnimation& GetAnimation() const { return mAnimation; }

		/** Determines if and how should the sprite animation play. */
		B3D_SCRIPT_EXPORT(ExportName(AnimationPlayback), Property(Setter))
		void SetAnimationPlayback(SpriteAnimationPlayback playback)
		{
			mPlayback = playback;
			MarkCoreDirtyInternal();
		}

		/** @copydoc SetAnimationPlayback */
		B3D_SCRIPT_EXPORT(ExportName(AnimationPlayback), Property(Getter))
		SpriteAnimationPlayback GetAnimationPlayback() const { return mPlayback; };

	protected:
		/** Marks the contents of the main thread object as dirty, causing it to sync with its render thread counterpart. */
		virtual void MarkCoreDirtyInternal() {}

		Vector2 mUVOffset;
		Vector2 mUVScale;

		SpriteAnimationPlayback mPlayback = SpriteAnimationPlayback::None;
		SpriteSheetGridAnimation mAnimation;
	};

	/** Templated base class used for both main and render thread SpriteTexture counterparts. */
	template <bool IsRenderProxy>
	class B3D_CORE_EXPORT TSpriteTexture : public SpriteTextureBase
	{
	public:
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;

		TSpriteTexture(const Vector2& uvOffset, const Vector2& uvScale, TextureType atlasTexture)
			: SpriteTextureBase(uvOffset, uvScale), mAtlasTexture(std::move(atlasTexture))
		{}

		virtual ~TSpriteTexture() = default;

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
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) SpriteTexture : public Resource, public TSpriteTexture<false>
	{
	public:
		/**	Determines the internal texture that the sprite texture references. */
		B3D_SCRIPT_EXPORT(ExportName(Texture), Property(Setter))
		void SetTexture(const HTexture& texture);

		/**	@copydoc SetTexture() */
		B3D_SCRIPT_EXPORT(ExportName(Texture), Property(Getter))

		const HTexture& GetTexture() const { return mAtlasTexture; }

		/**	Returns width of the sprite texture in pixels. */
		B3D_SCRIPT_EXPORT(ExportName(Width), Property(Getter))
		u32 GetWidth() const;

		/**	Returns height of the sprite texture in pixels. */
		B3D_SCRIPT_EXPORT(ExportName(Height), Property(Getter))
		u32 GetHeight() const;

		/**
		 * Returns width of a single animation frame sprite texture in pixels. If the texture has no animation this
		 * is the same as getWidth().
		 */
		B3D_SCRIPT_EXPORT(ExportName(FrameWidth), Property(Getter))
		u32 GetFrameWidth() const;

		/**
		 * Returns height of a single animation frame sprite texture in pixels. If the texture has no animation this
		 * is the same as getHeight().
		 */
		B3D_SCRIPT_EXPORT(ExportName(FrameHeight), Property(Getter))
		u32 GetFrameHeight() const;

		/**	Retrieves the render proxy. */
		SPtr<ct::SpriteTexture> GetCore() const;

		/**	Creates a new sprite texture that references the entire area of the provided texture. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(SpriteTexture))
		static HSpriteTexture Create(const HTexture& texture);

		/**	Creates a new sprite texture that references a sub-area of the provided	texture. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(SpriteTexture))
		static HSpriteTexture Create(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture);

		/**	Checks if the sprite texture and its internal texture have been loaded. */
		static bool CheckIsLoaded(const HSpriteTexture& tex);

		/**	Returns a dummy sprite texture. */
		static const HSpriteTexture& Dummy();

		/** @name Internal
		 *  @{
		 */

		/** Creates a new SpriteTexture without a resource handle. Use create() for normal use. */
		static SPtr<SpriteTexture> CreatePtrInternal(const HTexture& texture);

		/** Creates a new SpriteTexture without a resource handle. Use create() for normal use. */
		static SPtr<SpriteTexture> CreatePtrInternal(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture);

		void MarkCoreDirtyInternal() override;

		/** @} */
	private:
		friend class SpriteTextureRTTI;
		friend class ct::SpriteTexture;
		struct SyncPacket;

		/** @copydoc Create(const Vector2&, const Vector2&, const HTexture&) */
		SpriteTexture(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture);

		void Initialize() override;
		SPtr<ct::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;
		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/**	Creates a new empty and uninitialized sprite texture. */
		static SPtr<SpriteTexture> CreateEmpty();

	public:
		friend class SpriteTextureRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup Resources-Internal
		 *  @{
		 */

		/**
		 * Render proxy counterpart of a bs::SpriteTexture.
		 *
		 * @note	Render thread.
		 */
		class B3D_CORE_EXPORT SpriteTexture : public RenderProxy, public TSpriteTexture<true>
		{
		public:
			/**	Determines the internal texture that the sprite texture references. */
			void SetTexture(const SPtr<ct::Texture>& texture) { mAtlasTexture = texture; }

			/**	@copydoc SetTexture() */
			const SPtr<ct::Texture>& GetTexture() const { return mAtlasTexture; }

		private:
			friend class bs::SpriteTexture;

			SpriteTexture(const Vector2& uvOffset, const Vector2& uvScale, SPtr<Texture> texture, const SpriteSheetGridAnimation& anim, SpriteAnimationPlayback playback);

			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;
		};

		/** @} */
	} // namespace ct
} // namespace bs
