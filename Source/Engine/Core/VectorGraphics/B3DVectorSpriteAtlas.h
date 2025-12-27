//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DTextureAtlasLayout.h"
#include "VectorGraphics/B3DVectorGraphics.h"

namespace b3d
{
	class GUIVectorSpriteAtlasAllocation;

	namespace render
	{
		class VectorPathRenderable;
	}

	struct VectorGraphicsSettings;
	/** @addtogroup 2D
	 *  @{
	 */

	class GUIVectorSpriteAtlas;

	/** Manages lifetime of an allocation in a sprite atlas. When this object goes out of scope the atlas will be notified so it may free the allocation. */
	struct B3D_EXPORT GUIVectorSpriteAtlasAllocationHandle : public std::enable_shared_from_this<GUIVectorSpriteAtlasAllocationHandle>
	{
		GUIVectorSpriteAtlasAllocationHandle(GUIVectorSpriteAtlas* owner, const UUID& vectorPathId, const TOptional<TreeTextureAtlasLayout::Allocation>& layoutAllocation, u32 textureId, const SPtr<render::VectorPathRenderable>& renderable)
			: mVectorPathId(vectorPathId), mOwner(owner), mLayoutAllocation(layoutAllocation), mTextureId(textureId), mRenderable(renderable)
		{ }

	private:
		friend GUIVectorSpriteAtlas;

		/** Key that unique identifies a single allocation. */
		struct Key
		{
			Key(const VectorPath& vectorPath, const VectorGraphicsSettings& settings)
				: VectorPathId(vectorPath.GetId()), Settings(settings)
			{ }

			Key(const UUID& vectorPathId, const VectorGraphicsSettings& settings)
				: VectorPathId(vectorPathId), Settings(settings)
			{ }

			bool operator==(const Key& other) const { return VectorPathId == other.VectorPathId && Settings == other.Settings; }
			struct Hash { size_t operator()(const Key& value) const; };

			UUID VectorPathId;
			VectorGraphicsSettings Settings;
		};

		/** Returns the atlas that owns this allocation. */
		GUIVectorSpriteAtlas* GetOwner() const { return mOwner; }

		/** Returns a key that uniquely identifies the allocation. */
		Key GetKey() const;

		GUIVectorSpriteAtlas* const mOwner = nullptr;
		UUID mVectorPathId = UUID::kEmpty;
		const TOptional<TreeTextureAtlasLayout::Allocation> mLayoutAllocation; /**< Allocation in the texture atlas layout, if allocated in the atlas. If null, sprite is allocated as a unique texture. */
		const u32 mTextureId = ~0u;
		const SPtr<render::VectorPathRenderable> mRenderable;
	};

	/** Represents a single allocation in a GUIVectorSpriteAtlas. */
	class B3D_EXPORT GUIVectorSpriteAtlasAllocation : public std::enable_shared_from_this<GUIVectorSpriteAtlasAllocation>
	{
	public:
		GUIVectorSpriteAtlasAllocation() = default;
		GUIVectorSpriteAtlasAllocation(const HTexture& atlasTexture, const Area2& uvRange, const SPtr<GUIVectorSpriteAtlasAllocationHandle>& allocationHandle)
			: AtlasTexture(atlasTexture), UVRange(uvRange), AllocationHandle(allocationHandle)
		{ }

		HTexture AtlasTexture;
		Area2 UVRange;
		SPtr<GUIVectorSpriteAtlasAllocationHandle> AllocationHandle;
	};

	/** Settings used for initializing GUIVectorSpriteAtlas. */
	struct GUIVectorSpriteAtlasSettings
	{
		u32 AtlasPageSize = 2048; /**< Size of a single page in the texture atlas. */
		u32 UniqueAllocationSize = 512; /**< Any allocations equal or above this size will be created in a unique texture, rather than the atlas. */
		u32 KeepUnusedTexturesFor = 30; /**< Number of frames to keep unused textures for, in case they are re-used. */
	};

	/** Manages a cache of all VectorPath objects used by the GUI and maintains an atlas containing their rasterized representation for use by GUI. */
	class B3D_EXPORT GUIVectorSpriteAtlas
	{
	public:
		GUIVectorSpriteAtlas(const GUIVectorSpriteAtlasSettings& settings);
		~GUIVectorSpriteAtlas();

		/**
		 * Allocates a new sprite entry in the atlas. If the entry with the same path & settings combination already exists, an existing entry
		 * will be returned instead. Note that before using the texture that is part of the allocation you must call RenderDirtySprites() on the
		 * render thread.
		 */
		GUIVectorSpriteAtlasAllocation Allocate(const VectorPath& vectorPath, const VectorGraphicsSettings& settings);

		/** To be called once per frame on the main thread. */
		void Update();

		/** Render any sprites that are newly allocated. Render thread only. */
		void RenderDirtySprites(u32 bufferIndex);
		
	private:
		friend struct GUIVectorSpriteAtlasAllocationHandle;

		/** Triggered by the GUIVectorSpriteAtlasAllocation deleter. */
		void NotifyAllocationReleased(GUIVectorSpriteAtlasAllocationHandle* allocationHandle);

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
			SPtr<render::VectorPathRenderable> Renderable;
			SPtr<render::Texture> Texture;
			Area2 UVRegion = Area2::kEmpty;
			Size2UI Size = Size2UI::kZero;
		};

		/** Represents a single allocation in the atlas. */
		class AllocationInformation
		{
		public:
			AllocationInformation() = default;
			AllocationInformation(const HTexture& atlasTexture, const Area2& uvRange, const WeakSPtr<GUIVectorSpriteAtlasAllocationHandle>& allocationHandle)
				: AtlasTexture(atlasTexture), UVRange(uvRange), AllocationHandle(allocationHandle)
			{ }

			HTexture AtlasTexture;
			Area2 UVRange;
			WeakSPtr<GUIVectorSpriteAtlasAllocationHandle> AllocationHandle;
		};

		/** Information about allocation to free. */
		struct FreeAllocationInformation
		{
			FreeAllocationInformation() = default;
			FreeAllocationInformation(const HTexture& atlasTexture, GUIVectorSpriteAtlasAllocationHandle* allocationHandle)
				: AtlasTexture(atlasTexture), AllocationHandle(allocationHandle)
			{}

			HTexture AtlasTexture;
			GUIVectorSpriteAtlasAllocationHandle* AllocationHandle = nullptr;
		};


		const GUIVectorSpriteAtlasSettings mSettings;
		TreeTextureAtlasLayout mAtlasLayout;
		UnorderedMap<GUIVectorSpriteAtlasAllocationHandle::Key, AllocationInformation, GUIVectorSpriteAtlasAllocationHandle::Key::Hash> mAllocations;

		UnorderedMap<u32, HTexture> mAtlasLayoutTextures;
		UnorderedMap<u32, HTexture> mUniqueTextures;

		Mutex mAllocationsMutex;
		Vector<FreeAllocationInformation> mFreeAllocations; // Allocations recorded here in a thread safe manner
		Vector<FreeAllocationInformation> mFreeAllocationsTemp; // Temporary buffer when iterating over the array on the main thread

		u32 mDirtySpriteWriteBufferIndex = 0;
		Vector<DirtySpriteInformation> mDirtySpriteBuffers[RenderThread::kSyncBufferCount + 1];// Dirty sprites recorded here in a thread safe manner

		mutable UnorderedMap<FreeTextureInformation::Key, FreeTextureInformation, FreeTextureInformation::Key::Hash> mFreeTextureCache;

		mutable Vector<u32> mFreeUniqueTextureIds;
		mutable u32 mNextUniqueTextureId = 1;
	};


	/** @} */
} // namespace b3d
