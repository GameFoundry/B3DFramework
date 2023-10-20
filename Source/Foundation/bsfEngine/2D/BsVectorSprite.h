//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "2D/BsSprite.h"
#include "Image/BsColor.h"
#include "Image/BsTextureAtlasLayout.h"
#include "VectorGraphics/BsVectorGraphics.h"

namespace bs
{
	class GUIVectorSpriteAtlasAllocation;

	namespace ct
	{
		class VectorPathRenderable;
	}

	struct VectorGraphicsSettings;
	/** @addtogroup 2D
	 *  @{
	 */

	/**	Information used for initializing or updating a vector sprite. */
	struct VectorSpriteInformation : SpriteInformation
	{
		VectorSpriteInformation() = default;

		HVectorPath VectorPath; /**< Vector path to render on the sprite. */
	};

	/**	A sprite consisting of a vector path represented by a sprite texture. */
	class B3D_EXPORT VectorSprite : public Sprite
	{
	public:
		VectorSprite() = default;
		~VectorSprite();

		/**
		 * Recreates internal sprite data according the specified description structure.
		 *
		 * @param	information		Describes the geometry and material of the sprite.
		 * @param	groupId			Group identifier that forces different materials to be used for different groups (for
		 *							example you don't want the sprites to share the same material if they use different world
		 *							transform matrices).
		 */
		void Update(const VectorSpriteInformation& information, u64 groupId);

	private:
		/**	Clears internal geometry buffers. */
		void ClearMesh();

		Array<Vector2, 4> mPositionBuffer;
		Array<Vector2, 4> mUVBuffer;
		Array<u32, 6> mIndexBuffer;

		SPtr<GUIVectorSpriteAtlasAllocation> mSpriteAtlasAllocation;
	};

	class GUIVectorSpriteAtlas;

	/** Represents a single allocation in a GUIVectorSpriteAtlas. */
	class GUIVectorSpriteAtlasAllocation : public std::enable_shared_from_this<GUIVectorSpriteAtlasAllocation>
	{
	public:
		GUIVectorSpriteAtlasAllocation(GUIVectorSpriteAtlas* owner, u64 vectorPathId, const HSpriteTexture& image, const Optional<TreeTextureAtlasLayout::Allocation>& layoutAllocation, u32 textureId, const SPtr<ct::VectorPathRenderable>& renderable)
			: Image(image), mVectorPathId(vectorPathId), mOwner(owner), mLayoutAllocation(layoutAllocation), mTextureId(textureId), mRenderable(renderable)
		{ }

		const HSpriteTexture Image;

	private:
		friend GUIVectorSpriteAtlas;

		/** Key that unique identifies a single allocation. */
		struct Key
		{
			Key(const VectorPath& vectorPath, const VectorGraphicsSettings& settings)
				: VectorPathId((u64)&vectorPath), Settings(settings)
			{ }

			Key(u64 vectorPathId, const VectorGraphicsSettings& settings)
				: VectorPathId(vectorPathId), Settings(settings)
			{ }

			bool operator==(const Key& other) const { return VectorPathId == other.VectorPathId && Settings == other.Settings; }
			struct Hash { size_t operator()(const Key& value) const; };

			u64 VectorPathId;
			VectorGraphicsSettings Settings;
		};

		/** Returns the atlas that owns this allocation. */
		GUIVectorSpriteAtlas* GetOwner() const { return mOwner; }

		/** Returns a key that uniquely identifies the allocation. */
		Key GetKey() const;

		GUIVectorSpriteAtlas* const mOwner = nullptr;
		u64 mVectorPathId = 0;
		const Optional<TreeTextureAtlasLayout::Allocation> mLayoutAllocation; /**< Allocation in the texture atlas layout, if allocated in the atlas. If null, sprite is allocated as a unique texture. */
		const u32 mTextureId = ~0u;
		const SPtr<ct::VectorPathRenderable> mRenderable;
	};

	/** Settings used for initializing GUIVectorSpriteAtlas. */
	struct GUIVectorSpriteAtlasSettings
	{
		u32 AtlasPageSize = 2048; /**< Size of a single page in the texture atlas. */
		u32 UniqueAllocationSize = 512; /**< Any allocations equal or above this size will be created in a unique texture, rather than the atlas. */
		u32 KeepUnusedTexturesFor = 30; /**< Number of frames to keep unused textures for, in case they are re-used. */
	};

	/** Manages a cache of all VectorPath objects used by the GUI and maintains an atlas containing their rasterized representation for use by GUI. */
	class GUIVectorSpriteAtlas
	{
	public:
		GUIVectorSpriteAtlas(const GUIVectorSpriteAtlasSettings& settings);
		~GUIVectorSpriteAtlas();

		/**
		 * Allocates a new sprite entry in the atlas. If the entry with the same path & settings combination already exists, an existing entry
		 * will be returned instead. Note that before using the texture that is part of the allocation you must call RenderDirtySprites() on the
		 * core thread.
		 */
		SPtr<GUIVectorSpriteAtlasAllocation> Allocate(const VectorPath& vectorPath, const VectorGraphicsSettings& settings);

		/** To be called once per frame on the main thread. */
		void Update();

		/** Render any sprites that are newly allocated. Core thread only. */
		void RenderDirtySprites();
		
	private:
		friend class GUIVectorSpriteAtlasAllocation;

		/** Triggered by the GUIVectorSpriteAtlasAllocation deleter. */
		void NotifyAllocationReleased(GUIVectorSpriteAtlasAllocation* allocation);

		/** Attempts to find an existing unused texture matching the requested size, or creates a new texture. */
		HTexture CreateOrFindTexture(Size2UI size) const;

		/** Notifies the system that a texture is no longer being used. */
		void ReleaseTexture(const HTexture& texture);

		/** Returns the next available texture ID. */
		u32 GetNextUniqueTextureId() const;

		/** Notifies the system that the specified texture ID is no longer being used. */
		void ReleaseTextureId(u32 id);

		/** Cleans up any allocations that were released, but haven't yet been destroyed. */
		void DestroyPendingReleasedAllocations();

		/** Information about a texture that is no longer used, but we're keeping around in case it gets re-used. */
		struct FreeTextureInformation
		{
			struct Key
			{
				Key(Size2UI size)
					: Size(size)
				{ }

				bool operator==(const Key& other) const { return Size == other.Size; }
				struct Hash { size_t operator()(const Key& value) const; };

				Size2UI Size;
			};

			FreeTextureInformation(const HTexture& texture, u64 lastUsedFrame)
				: Texture(texture), LastUsedFrame(lastUsedFrame)
			{ }

			HTexture Texture;
			u64 LastUsedFrame = 0;
		};

		/** Information about a vector path that should be re-rendered. */
		struct DirtySpriteInformation
		{
			SPtr<ct::VectorPathRenderable> Renderable;
			SPtr<ct::Texture> Texture;
			Rect2 UVRegion = Rect2::kEmpty;
			Size2UI Size = Size2UI::kZero;
		};

		const GUIVectorSpriteAtlasSettings mSettings;
		TreeTextureAtlasLayout mAtlasLayout;
		UnorderedMap<GUIVectorSpriteAtlasAllocation::Key, GUIVectorSpriteAtlasAllocation*, GUIVectorSpriteAtlasAllocation::Key::Hash> mAllocations;

		UnorderedMap<u32, HTexture> mAtlasLayoutTextures;
		UnorderedMap<u32, HTexture> mUniqueTextures;

		Mutex mAllocationsMutex;
		Vector<GUIVectorSpriteAtlasAllocation*> mFreeAllocations; // Allocations recorded here in a thread safe manner
		Vector<GUIVectorSpriteAtlasAllocation*> mFreeAllocationsTemp; // Temporary buffer when iterating over the array on the main thread

		Mutex mDirtySpriteMutex;
		Vector<DirtySpriteInformation> mDirtySprites;// Dirty sprites recorded here in a thread safe manner
		Vector<DirtySpriteInformation> mDirtySpritesTemp; // Dirty sprites iterated here on the core thread when rendering

		mutable UnorderedMap<FreeTextureInformation::Key, FreeTextureInformation, FreeTextureInformation::Key::Hash> mFreeTextureCache;

		mutable Vector<u32> mFreeUniqueTextureIds;
		mutable u32 mNextUniqueTextureId = 1;
	};


	/** @} */
} // namespace bs
