//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Math/B3DArea2.h"
#include "Resources/B3DResource.h"
#include "Math/B3DVector2.h"

namespace b3d
{
	class SpriteImageBase;
	/** @addtogroup 2D
	 *  @{
	 */

	namespace render
	{
		class SpriteImage;
	}

	/**
	 * Descriptor that describes a simple sprite sheet animation. The parent area is split into a grid of
	 * @p RowCount x @p ColumnCount, each representing one frame of the animation. Every frame is of equal size. Frames are
	 * sequentially evaluated starting from the top-most row, iterating over all columns in a row and then moving to next
	 * row, up to @p FrameCount frames. @p FramesPerSecond frames are evaluated every second, allowing you to control animation speed.
	 */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering), ExportAsStruct(true)) SpriteSheetGridAnimation
	{
		SpriteSheetGridAnimation() = default;

		SpriteSheetGridAnimation(u32 rowCount, u32 columnCount, u32 frameCount, u32 framesPerSecond)
			: RowCount(rowCount), ColumnCount(columnCount), FrameCount(frameCount), FramesPerSecond(framesPerSecond)
		{}

		/**
		 * Number of rows to divide the parent area in. Determines height of the individual frame (depends on
		 * parent area size).
		 */
		u32 RowCount = 1;

		/**
		 * Number of columns to divide the parent area in. Determines column of the individual frame (depends on
		 * parent area size).
		 */
		u32 ColumnCount = 1;

		/** Number of frames in the animation. Must be less or equal than @p RowCount * @p ColumnCount. */
		u32 FrameCount = 1;

		/** How many frames to evaluate each second. Determines the animation speed. */
		u32 FramesPerSecond = 8;
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

	/** Information about a SpriteImage. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Rendering)) SpriteImageInformation
	{
		/** Determines if animation is enabled and how should it play. */
		SpriteAnimationPlayback AnimationPlayback = SpriteAnimationPlayback::None;

		/** Describes the sprite sheet grid used for animation, if animation is used. */
		SpriteSheetGridAnimation Animation;
	};

	/**
	 * Provides information about a sprite image rendered into a texture. Also tracks lifetime of such allocations.
	 * One sprite image may have one or multiple such allocations, resulting from different scale/size requirements.
	 */
	template<bool IsRenderProxy>
	class B3D_EXPORT TSpriteImageAllocation
	{
	public:
		using SpriteImageType = CoreVariantType<SpriteImage, IsRenderProxy>;
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;

		virtual ~TSpriteImageAllocation() = default;

		/** Retrieves the texture where the image is stored. */
		const TextureType& GetTexture() const { return mTexture; }

		/**	Returns the pixel size of the UV subrange covered in the texture atlas. If the image includes animation, this will return the size of the entire animation grid. */
		Size2I GetSize() const;

		/** Determines the UV range that the image is referencing. */
		Area2 GetUVRange() const { return mUVRange; }

		/** Transforms local UV coordinates into atlas UV coordinates. */
		Vector2 TransformUV(const Vector2& uv) const { return Vector2(mUVRange.X + uv.X * mUVRange.Width, mUVRange.Y + uv.Y * mUVRange.Height); }

	protected:
		TSpriteImageAllocation() = default;
		TSpriteImageAllocation(const WeakSPtr<SpriteImageType>& owner, const TextureType& atlasTexture, const Area2& uvRange)
			:mOwner(owner), mTexture(atlasTexture), mUVRange(uvRange)
		{ }

		/** Owner sprite image that this allocation is a part of. */
		WeakSPtr<SpriteImageType> mOwner;

		/** Texture within which the image is allocated. */
		TextureType mTexture;

		/** Range in the atlas texture that the image is to be read from, in [0, 1] range. */
		Area2 mUVRange = Area2(0.0f, 0.0f, 1.0f, 1.0f);
	};

	/** @copydoc TSpriteImageAllocation. */
	class SpriteImageAllocation : public CoreObject, public TSpriteImageAllocation<false>
	{
	public:
		struct SyncPacket;

		virtual ~SpriteImageAllocation();

		/** Creates a new sprite image allocation. */
		static SPtr<SpriteImageAllocation> Create(const WeakSPtr<SpriteImage>& owner, const HTexture& atlasTexture, const Area2& uvRange);

	protected:
		friend class render::SpriteImageAllocation;

		using TSpriteImageAllocation::TSpriteImageAllocation;

		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;
	};

	/** Descriptor structure used for initialization of a SpriteImage. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Rendering)) SpriteImageCreateInformation : SpriteImageInformation 
	{
		SpriteImageCreateInformation() = default;
		SpriteImageCreateInformation(const SpriteImageInformation& other)
			:SpriteImageInformation(other)
		{ }
	};

	/** Base class for both render and main thread counterparts of SpriteImage. */
	class B3D_EXPORT SpriteImageBase
	{
	public:
		SpriteImageBase(const SpriteImageCreateInformation& createInformation)
			: mInformation(createInformation)
		{ }
		virtual ~SpriteImageBase() = default;

		/**
		 * Returns the row and column of the current animation frame for time @p t.
		 *
		 * @param	t			Time to evaluate the animation at.
		 * @param	outRow		Row containing the animation frame at time @p t.
		 * @param	outColumn	Column containing the animation frame at time @p t.
		 */
		void GetAnimationFrame(float t, u32& outRow, u32& outColumn) const;

		/**
		 * Sets properties describing sprite animation. The animation splits the sprite area into a grid of sub-images
		 * which can be evaluated over time. In order to view the animation you must also enable playback through
		 * setAnimationPlayback().
		 */
		B3D_SCRIPT_EXPORT(ExportName(Animation), Property(Setter))
		void SetAnimation(const SpriteSheetGridAnimation& animation)
		{
			mInformation.Animation = animation;
			MarkRenderProxyDataDirtyInternal();
		}

		/** @copydoc SetAnimation */
		B3D_SCRIPT_EXPORT(ExportName(Animation), Property(Getter))
		const SpriteSheetGridAnimation& GetAnimation() const { return mInformation.Animation; }

		/** Determines if and how should the sprite animation play. */
		B3D_SCRIPT_EXPORT(ExportName(AnimationPlayback), Property(Setter))
		void SetAnimationPlayback(SpriteAnimationPlayback playback)
		{
			mInformation.AnimationPlayback = playback;
			MarkRenderProxyDataDirtyInternal();
		}

		/** @copydoc SetAnimationPlayback */
		B3D_SCRIPT_EXPORT(ExportName(AnimationPlayback), Property(Getter))
		SpriteAnimationPlayback GetAnimationPlayback() const { return mInformation.AnimationPlayback; }

	protected:
		virtual void MarkRenderProxyDataDirtyInternal() { }
		
		SpriteImageInformation mInformation;
	};

	/** Templated base class for both render and main thread counterparts of SpriteImage. */
	template<bool IsRenderProxy>
	class B3D_EXPORT TSpriteImage : public SpriteImageBase
	{
	public:
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;
		using SpriteImageAllocationType = CoreVariantType<SpriteImageAllocation, IsRenderProxy>;

		TSpriteImage(const SpriteImageCreateInformation& createInformation, const SPtr<SpriteImageAllocationType>& defaultAllocatedImage = nullptr)
			:SpriteImageBase(createInformation), mDefaultAllocatedImage(defaultAllocatedImage)
		{ }
		~TSpriteImage() override = default;

		/**
		 * Evaluates the UV coordinate offset and size to use at the specified animation time. If the sprite texture doesn't
		 * have animation playback enabled then just the default offset and size will be provided, otherwise the
		 * animation will be evaluated and appropriate UV returned.
		 *
		 * @param	allocation	Image allocation to evaluate animation for. Must be owned by this sprite image.
		 * @param	t			Time to evaluate the animation at.
		 * @return 				UV range of the animation frame at the specified time.
		 */
		Area2 EvaluateAnimation(const SpriteImageAllocationType& allocation, float t) const;

		/** Returns the default (unscaled) image allocation, using the image size as provided on the sprite image construction. */
		const SpriteImageAllocationType& GetDefaultAllocatedImage() const { return *mDefaultAllocatedImage; }

		/** Returns the default (unscaled) image allocation, using the image size as provided on the sprite image construction. */
		SPtr<SpriteImageAllocationType> GetDefaultAllocatedImageAsShared() const { return mDefaultAllocatedImage; }

		/** Returns the size of a single animation frame in logical pixel units. If the texture has no animation this is the same as GetLogicalSize(). */
		B3D_SCRIPT_EXPORT(ExportName(AnimationFrameSize), Property(Getter))
		Size2UI GetAnimationFrameSize() const;

	protected:
		SPtr<SpriteImageAllocationType> mDefaultAllocatedImage;
		TInlineArray<SpriteImageAllocationType*, 2> mScaledAllocatedImages;
	};

	/**
	 * Image that references a part of a texture by specifying an UV range. When the sprite image is rendered
	 * only the portion of the texture specified by the UV range will be rendered.
	 *
	 * Sprite images also allow you to specify sprite sheet animation by varying which portion of the UV is selected over time. 
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) SpriteImage : public Resource, public TSpriteImage<false>
	{
	public:
		struct SyncPacket;

		/**
		 * Attempts to allocate a new image that is of appropriate quality to fit into the provided area. This is mostly used by vector shapes, which will
		 * usually want to re-render themselves to fit the requested size and achieve the best possible quality.
		 *
		 * The image will be scaled uniformly until one or both dimensions of the image match the provided size. One dimension may be smaller than the
		 * provided size.
		 *
		 * Implementations may choose to avoid re-rendering and just return the default allocated image instead, in which case its up to the caller
		 * to scale the image appropriately (e.g. using bilinear filtering or scale9grid).
		 *
		 * If an existing allocation for the provided size already exists, it will be returned and a new allocation will not be made alive. Allocated
		 * portion of the texture will remain alive as long as there is at least a single reference to the returned SpriteImageAllocation.
		 *
		 * @param	size	Requested size of the allocation, in physical pixel units.
		 * @return 			Allocation structure that provides information about the allocation and used for tracking allocation lifetime.
		 */
		virtual SPtr<SpriteImageAllocation> FindOrAllocateImageToFitArea(const Size2I& size) = 0;

		/**
		 * Attempts to allocate a new image that is of appropriate quality for a scaled version of the default image. This is mostly used by vector shapes,
		 * which will usually want to re-render themselves to fit the requested scale and achieve the best possible quality (e.g. scale may change when
		 * display DPI changes, or when user zooms in/out).
		 *
		 * Implementations may choose to avoid re-rendering and just return the default allocated image instead, in which case its up to the caller
		 * to scale the image appropriately (e.g. using bilinear filtering or scale9grid).
		 *
		 * If an existing allocation for the provided size already exists, it will be returned and a new allocation will not be made alive. Allocated
		 * portion of the texture will remain alive as long as there is at least a single reference to the returned SpriteImageAllocation.
		 *
		 * @param	scale	Scale to apply.
		 * @return 			Allocation structure that provides information about the allocation and used for tracking allocation lifetime.
		 */
		virtual SPtr<SpriteImageAllocation> FindOrAllocateScaledImage(float scale) = 0;

		/** Frees any data associated with the provided allocation. */
		virtual void DeallocateImage(SpriteImageAllocation* allocation);

		/** @name Internal
		 *  @{
		 */

		void MarkRenderProxyDataDirtyInternal() override;

		/** @} */
	protected:
		friend class render::SpriteImage;

		SpriteImage(const SpriteImageCreateInformation& createInformation)
			: TSpriteImage(createInformation)
		{ }

		void Destroy() override;
		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SpriteImageRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup Renderer
		 *  @{
		 */

		/** @copydoc TSpriteImageAllocation. */
		class SpriteImageAllocation : public RenderProxy, public TSpriteImageAllocation<true>
		{
		protected:
			friend class b3d::SpriteImageAllocation;

			SpriteImageAllocation(const WeakSPtr<SpriteImageType>& owner, const TextureType& atlasTexture, const Area2& uvRange);
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;
		};

		/**
		 * Render proxy counterpart of a b3d::SpriteImage.
		 *
		 * @note	Render thread.
		 */
		class B3D_EXPORT SpriteImage : public RenderProxy, public TSpriteImage<true>
		{
		protected:
			friend class b3d::SpriteImage;

			SpriteImage(const SpriteImageCreateInformation& createInformation, const SPtr<SpriteImageAllocation>& defaultAllocatedImage)
				: TSpriteImage(createInformation, defaultAllocatedImage)
			{ }

			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;
		};

		/** @} */
	} // namespace render
} // namespace b3d
