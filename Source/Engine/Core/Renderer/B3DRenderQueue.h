//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Math/B3DVector3.h"
#include "RenderAPI/B3DSubMesh.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Renderer-Internal
		 *  @{
		 */

		/**
		 * Controls if and how a render queue groups draw commands by material in order to reduce number of state changes.
		 */
		enum class StateReduction
		{
			None, /**< No grouping based on material will be done. */
			Material, /**< Commands will be grouped by material first, by distance second. */
			Distance /**< Commands will be grouped by distance first, material second. */
		};

		/** Contains data needed for performing a single rendering pass. */
		struct RenderQueueEntry
		{
			const DrawCommand* DrawCommand = nullptr;
			u32 PassIndex = 0;
			u32 VariationIndex = 0;
			bool ApplyPass = true;
		};

		/**
		 * Render queue determines rendering order of draw commands contained within it. Rendering order is determined by draw command
		 * material, and can influence rendering of transparent or opaque objects, or be used to improve performance by grouping
		 * similar commands together.
		 */
		class B3D_EXPORT RenderQueue
		{
			/**	Data used for renderable element sorting. Represents a single material pass for a single mesh. */
			struct SortableElement
			{
				u32 SequentialIndex;
				i32 Priority;
				float DistFromCamera;
				u32 ShaderId;
				u32 VariationIndex;
				u32 PassIndex;
			};

		public:
			RenderQueue(StateReduction grouping = StateReduction::Distance);
			virtual ~RenderQueue() = default;

			/**
			 * Adds a new draw command to the render queue.
			 *
			 * @param	drawCommand		Draw command to add to the queue.
			 * @param	distFromCamera	Distance of this object from the camera. Used for distance sorting.
			 * @param	variationIndex	Index of the technique within @p element's material that's to be used to render the element with.
			 */
			void Add(const DrawCommand* drawCommand, float distFromCamera, u32 variationIndex);

			/**	Clears all render operations from the queue. */
			void Clear();

			/**	Sorts all the render operations using user-defined rules. */
			virtual void Sort();

			/** Returns a list of sorted render elements. Caller must ensure Sort() is called before this method. */
			const Vector<RenderQueueEntry>& GetSortedEntries() const;

			/** Controls if and how a render queue groups draw commands by material in order to reduce number of state changes. */
			void SetStateReduction(StateReduction mode) { mStateReductionMode = mode; }

		protected:
			/**	Callback used for sorting commands with no material grouping. */
			static bool CommandSortNoGroup(u32 aIdx, u32 bIdx, const Vector<SortableElement>& lookup);

			/**	Callback used for sorting commands with preferred material grouping. */
			static bool CommandSortPreferGroup(u32 aIdx, u32 bIdx, const Vector<SortableElement>& lookup);

			/**	Callback used for sorting commands with material grouping after sorting. */
			static bool CommandSortPreferDistance(u32 aIdx, u32 bIdx, const Vector<SortableElement>& lookup);

			Vector<SortableElement> mSortableElements;
			Vector<u32> mSortableElementIndex;
			Vector<const DrawCommand*> mCommands;

			Vector<RenderQueueEntry> mSortedRenderElements;
			StateReduction mStateReductionMode;
		};

		/** @} */
	} // namespace render
} // namespace b3d
