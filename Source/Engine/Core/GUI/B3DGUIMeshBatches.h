//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Math/B3DArea2.h"
#include "2D/B3DSpriteMaterial.h"
#include "RenderAPI/B3DSubMesh.h"

namespace b3d
{
	class GUIRenderable;

	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	Data required for rendering a single batch of GUI elements.  */
	struct GUIMeshRenderData
	{
		SPtr<render::Mesh> Mesh;
		SubMesh SubMesh;
		SpriteMaterial* Material;
		render::SpriteMaterialInfo MaterialInformation;
		Area2I Bounds;

		u32 GpuParametersIndex;
	};

	/** Information about a GUI element that is displaying a render target. */
	struct GUIRenderTargetRenderData
	{
		GUIRenderTargetRenderData() = default;

		GUIRenderTargetRenderData(SPtr<render::RenderTarget> target, const Area2I& area)
			: Target(std::move(target)), Area(area)
		{}

		SPtr<render::RenderTarget> Target;
		u64 LastUpdateCount = (u64)-1;
		Area2I Area;
	};

	/** Data required for rendering a single batch of GUI elements. */
	struct GUIBatchRenderData
	{
		u32 Id = 0;
		Area2I Bounds;

		Vector<GUIMeshRenderData> Elements;
		Vector<GUIRenderTargetRenderData> RenderTargetElements;
	};

	/**
	 * Contains data about which draw group needs to be redrawn, as well as a set of new draw groups if
	 * draw groups were updated.
	 */
	struct GUIDrawGroupRenderDataUpdate
	{
		Vector<GUIBatchRenderData> Batches;
		Vector<Area2I> DirtyRegions;
	};

	/**
	 * Maintains a set of meshes used for drawing GUI elements. When possible GUI render elements will be merged into the same mesh (i.e. a batch)
	 * in order to reduce render time. Additionally, each batch maintains a list of dirty regions that need to be updated by the GUI renderer.
	 **/
	class B3D_EXPORT GUIMeshBatches
	{
		/** Flags signaling which part of a GUIElement is dirty. */
		enum DirtyFlags
		{
			DirtyMesh = 1 << 0,
			DirtyContent = 1 << 1
		};

	public:
		GUIMeshBatches(GUIWidget* parentWidget);

		/** Iterates over all the render elements in the GUI elements and adds them to suitable batches. */
		void Add(GUIRenderable* guiElement);

		/** Removes all render elements in the provided GUI element from their current set of batches. */
		void Remove(GUIRenderable* guiElement);

		/** Rebuilds any dirty internal data and returns the data structure required for updating the GUI renderer. */
		GUIDrawGroupRenderDataUpdate RebuildDirty(bool forceRebuildMeshes);

		/** Notifies the system that element's contents were marked as dirty. */
		void MarkContentDirty(GUIRenderable* guiElement);

		/** Notifies the system that element's mesh was marked as dirty. */
		void MarkMeshDirty(GUIRenderable* guiElement);

	private:
		/** Information about a material used by a batch. */
		struct BatchedMaterial
		{
			BatchedMaterial() = default;

			bool CanBeMergedWith(const BatchedMaterial& other) const
			{
				return IsBatchingAllowed && other.IsBatchingAllowed && MaterialHash == other.MaterialHash && MeshType == other.MeshType;
			}

			void Merge(const BatchedMaterial& other)
			{
				B3D_ASSERT(SpriteMaterial != nullptr);
				SpriteMaterial->Merge(SpriteMaterialInformation, other.SpriteMaterialInformation);
			}

			bool IsBatchingAllowed = true;
			u64 MaterialHash = 0;
			GUIMeshType MeshType = GUIMeshType::Triangle;
			SpriteMaterial* SpriteMaterial = nullptr;
			SpriteMaterialInfo SpriteMaterialInformation;
		};

		/** Represents a batched GUI render element. */
		struct BatchedGUIRenderElement
		{
			BatchedGUIRenderElement() = default;

			BatchedGUIRenderElement(GUIRenderable* element, u32 renderElementIndex, u32 depth)
				: ParentGUIElement(element), RenderElementIndex(renderElementIndex), Depth(depth)
			{}

			GUIRenderable* ParentGUIElement = nullptr;
			u32 RenderElementIndex = 0;
			u32 Depth = 0;
		};

		/** Represents a batched GUI element. */
		struct BatchedGUIElement
		{
			GUIRenderable* GUIElement = nullptr;
			TInlineArray<u32, 4> BatchPerRenderElement;
			Area2I Bounds;
		};

		/**
		 * A set of GUI render elements that can be drawn together using a single mesh and material. 
		 * Additionally each batch maintains a list of dirty regions, so the renderer doesn't need to redraw the entire batch when a portion of it changes.
		 */
		struct Batch
		{
			u32 Id = ~0u;
			u32 DepthRangeId = ~0u;

			BatchedMaterial Material;
			Vector<BatchedGUIRenderElement> RenderElements;
			Vector<Area2I> DirtyRegions;

			// Bounds
			bool IsBoundsDirty = true;
			Area2I Bounds;

			// Mesh
			u32 IndexCount = 0;
			u32 VertexCount = 0;
			SPtr<Mesh> Mesh;
			bool IsMeshDirty = true;
		};

		/** Contains a set of batches for a specific depth range. */
		struct BatchesInDepthRange
		{
			u32 Id = ~0u;
			Vector<u32> BatchIds;

			// Depth
			u32 DepthRange = 0;
			u32 MinDepth = 0;
		};

		/** Returns a unique batch id. */
		u32 AllocateBatchId();

		/** Frees a batch id allocated with AllocateBatchId(). */
		void FreeBatchId(u32 id);

		/** Splits the provided depth range at the specified depth. Returns the index of second half of the depth range. */
		u32 SplitDepthRange(u32 depthRangeIndex, u32 depth);

		/** Attempts to collapse the provided depth range and the previous depth range into a single depth range. Returns true if the merge was performed. */
		bool CollapseDepthRange(u32 depthRangeIndex);

		/** Builds a mesh used for rendering the provided batch. */
		void RebuildMesh(Batch& batch);

		/** Rebuilds the GUI element meshes. */
		void RebuildMeshes();

		/**
		 * Adds a specific render element of a GUI element to a batch in the depth range at the provided index. Caller is responsible for
		 * ensuring the element falls within the correct depth range.
		 */
		void Add(BatchedGUIElement& batchedGuiElement, u32 renderElementIndex, u32 depthRangeIndex);

		/** Adds a specific render element of a GUI element to a batch in a suitable depth range. */
		void Add(BatchedGUIElement& batchedGuiElement, u32 renderElementIndex);

		/**
		 * Adds the specified render element of a GUI element to the specified depth range. Based on the provided material information a new batch will be created
		 * in the depth range, or the render element will be appended to an existing batch. Batch it was added to will be returned.
		 */
		Batch* Add(BatchedGUIElement& batchedGuiElement, const BatchedGUIRenderElement& batchedGuiRenderElement, const BatchedMaterial& batchedMaterial, u32 depthRangeIndex);

		/**
		 * Removes a specific render element in the provided GUI element from their batch in the provided depth range.
		 * Caller is responsible for ensuring the provided draw group is contained in the provided depth range.
		 */
		void Remove(BatchedGUIElement& batchedGuiElement, u32 renderElementIndex, u32 depthRangeIndex);

		/** Removes a specific render element in the provided GUI element from their current batch. */
		void Remove(BatchedGUIElement& batchedGuiElement, u32 renderElementIndex);

		/**
		 * Marks region covered by @p element of all the batches associated with the element as dirty, so they
		 * will be redrawn on the next frame. If element is being resized or moved, this should be called on the old
		 * position/size, as well as on the new position/size.
		 */
		void MarkBoundsDirty(const BatchedGUIElement& element);

		/**
		 * Marks region covered by @p element of a particular batch associated with the element as dirty, so it
		 * will be redrawn on the next frame. If element is being resized or moved, this should be called on the old
		 * position/size, as well as on the new position/size.
		 */
		void MarkBoundsDirty(const BatchedGUIElement& element, u32 batchId);

		/** Builds a structure with information required for rendering the provided batch. */
		static GUIBatchRenderData GetRenderData(const Batch& batch);

		/** Calculates the bounds of all elements in all the batches in the provided batch. */
		static Area2I CalculateBounds(Batch& batch);

		/** Creates information about a material for the provided render element. */
		static BatchedMaterial CreateBatchedMaterial(const BatchedGUIRenderElement& batchedGuiRenderElement);

		/** Creates information about a material for the provided render element. */
		static BatchedMaterial CreateBatchedMaterial(const GUIRenderable& guiElement, u32 renderElementIndex);

		Vector<BatchesInDepthRange> mDepthRanges;
		UnorderedMap<u32, Batch> mBatches;

		UnorderedMap<GUIRenderable*, BatchedGUIElement> mElements;
		UnorderedMap<GUIRenderable*, u32> mDirtyElements;
		Vector<Area2I> mDirtyRegionsForRemovedBatches;
		bool mBatchesOutOfDateInRenderer = true;
		GUIWidget* mWidget;

		u32 mNextDepthRangeId = 0;
		u32 mNextBatchId = 0;
		Vector<u32> mFreeBatchIds;
	};

	/** @} */
} // namespace b3d
