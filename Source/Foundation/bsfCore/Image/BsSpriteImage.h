//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Math/BsArea2.h"
#include "Resources/BsResource.h"
#include "Math/BsVector2.h"

namespace bs
{
	/** @addtogroup Image
	 *  @{
	 */

	namespace ct
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
		/** Range in the atlas texture that the image maps to. */
		Area2 UVRange = Area2(0.0f, 0.0f, 1.0f, 1.0f);

		/** Determines if animation is enabled and how should it play. */
		SpriteAnimationPlayback AnimationPlayback = SpriteAnimationPlayback::None;

		/** Describes the sprite sheet grid used for animation, if animation is used. */
		SpriteSheetGridAnimation Animation;
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
	class B3D_CORE_EXPORT SpriteImageBase
	{
	public:
		SpriteImageBase(const SpriteImageCreateInformation& createInformation)
			: mInformation(createInformation)
		{ }
		virtual ~SpriteImageBase() = default;

		/** Determines the UV range that the image is referencing. */
		B3D_SCRIPT_EXPORT(ExportName(UVRange), Property(Setter))
		void SetUVRange(const Area2& uvRange)
		{
			mInformation.UVRange = uvRange;
			MarkRenderProxyDataDirtyInternal();
		}

		/** Determines the UV range that the image is referencing. */
		B3D_SCRIPT_EXPORT(ExportName(UVRange), Property(Getter))
		const Area2& GetUVRange() const { return mInformation.UVRange; }

		/** Transforms local UV coordinates into atlas UV coordinates. */
		Vector2 TransformUV(const Vector2& uv) const { return Vector2(mInformation.UVRange.X + uv.X * mInformation.UVRange.Width, mInformation.UVRange.Y + uv.Y * mInformation.UVRange.Height); }

		/**
		 * Evaluates the UV coordinate offset and size to use at the specified animation time. If the sprite texture doesn't
		 * have animation playback enabled then just the default offset and size will be provided, otherwise the
		 * animation will be evaluated and appropriate UV returned.
		 */
		Area2 EvaluateAnimation(float t) const;

		/** Returns the row and column of the current animation frame for time @p t. */
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
	class B3D_CORE_EXPORT TSpriteImage : public SpriteImageBase
	{
	public:
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;

		TSpriteImage(const SpriteImageCreateInformation& createInformation)
			:SpriteImageBase(createInformation)
		{ }
		~TSpriteImage() override = default;

		/**	Returns the pixel size of the UV subrange covered in the texture atlas. If the image includes animation, this will return the size of the entire animation grid. */
		B3D_SCRIPT_EXPORT(ExportName(Size), Property(Getter))
		Size2UI GetSize() const;

		/** Returns the size of a single animation frame in pixels. If the texture has no animation this is the same as GetWidth(). */
		B3D_SCRIPT_EXPORT(ExportName(AnimationFrameSize), Property(Getter))
		Size2UI GetAnimationFrameSize() const;

		/** Retrieves the atlas texture where the image is stored. */
		B3D_SCRIPT_EXPORT(ExportName(Texture), Property(Getter))
		const TextureType& GetAtlasTexture() const { return mAtlasTexture; }

	protected:
		TextureType mAtlasTexture;
	};

	/**
	 * Image that references a part of a texture by specifying an UV range. When the sprite image is rendered
	 * only the portion of the texture specified by the UV range will be rendered.
	 *
	 * Sprite images also allow you to specify sprite sheet animation by varying which portion of the UV is selected over time. 
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) SpriteImage : public Resource, public TSpriteImage<false>
	{
	public:
		struct SyncPacket;

		/**	Checks if the sprite image and its atlas texture have been loaded. */
		static bool CheckIsLoaded(const HSpriteImage& image); // TODO - Can we get rid of this?

		/** @name Internal
		 *  @{
		 */

		void MarkRenderProxyDataDirtyInternal() override;

		/** @} */
	protected:
		friend class ct::SpriteImage;

		SpriteImage(const SpriteImageCreateInformation& createInformation)
			: TSpriteImage(createInformation)
		{ }

		SPtr<ct::RenderProxy> CreateRenderProxy() const override;
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

	namespace ct
	{
		/** @addtogroup RenderThread
		 *  @{
		 */

		/**
		 * Render proxy counterpart of a bs::SpriteImage.
		 *
		 * @note	Render thread.
		 */
		class B3D_CORE_EXPORT SpriteImage : public RenderProxy, public TSpriteImage<true>
		{
		protected:
			friend class bs::SpriteImage;

			SpriteImage(const SpriteImageCreateInformation& createInformation)
				: TSpriteImage(createInformation)
			{ }

			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;
		};

		/** @} */
	} // namespace ct
} // namespace bs
