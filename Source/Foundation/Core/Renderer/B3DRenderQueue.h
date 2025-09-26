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
		 * Controls if and how a render queue groups renderable objects by material in order to reduce number of state changes.
		 */
		enum class StateReduction
		{
			None, /**< No grouping based on material will be done. */
			Material, /**< Elements will be grouped by material first, by distance second. */
			Distance /**< Elements will be grouped by distance first, material second. */
		};

		/** Contains data needed for performing a single rendering pass. */
		struct RenderQueueElement
		{
			const RenderElement* RenderElem = nullptr;
			u32 PassIdx = 0;
			u32 TechniqueIdx = 0;
			bool ApplyPass = true;
		};

		/**
		 * Render objects determines rendering order of objects contained within it. Rendering order is determined by object
		 * material, and can influence rendering of transparent or opaque objects, or be used to improve performance by grouping
		 * similar objects together.
		 */
		class B3D_EXPORT RenderQueue
		{
			/**	Data used for renderable element sorting. Represents a single pass for a single mesh. */
			struct SortableElement
			{
				u32 SeqIdx;
				i32 Priority;
				float DistFromCamera;
				u32 ShaderId;
				u32 TechniqueIdx;
				u32 PassIdx;
			};

		public:
			RenderQueue(StateReduction grouping = StateReduction::Distance);
			virtual ~RenderQueue() = default;

			/**
			 * Adds a new entry to the render queue.
			 *
			 * @param[in]	element			Renderable element to add to the queue.
			 * @param[in]	distFromCamera	Distance of this object from the camera. Used for distance sorting.
			 * @param[in]	techniqueIdx	Index of the technique within @p element's material that's to be used to render the
			 *								element with.
			 */
			void Add(const RenderElement* element, float distFromCamera, u32 techniqueIdx);

			/**	Clears all render operations from the queue. */
			void Clear();

			/**	Sorts all the render operations using user-defined rules. */
			virtual void Sort();

			/** Returns a list of sorted render elements. Caller must ensure sort() is called before this method. */
			const Vector<RenderQueueElement>& GetSortedElements() const;

			/**
			 * Controls if and how a render queue groups renderable objects by material in order to reduce number of state
			 * changes.
			 */
			void SetStateReduction(StateReduction mode) { mStateReductionMode = mode; }

		protected:
			/**	Callback used for sorting elements with no material grouping. */
			static bool ElementSorterNoGroup(u32 aIdx, u32 bIdx, const Vector<SortableElement>& lookup);

			/**	Callback used for sorting elements with preferred material grouping. */
			static bool ElementSorterPreferGroup(u32 aIdx, u32 bIdx, const Vector<SortableElement>& lookup);

			/**	Callback used for sorting elements with material grouping after sorting. */
			static bool ElementSorterPreferDistance(u32 aIdx, u32 bIdx, const Vector<SortableElement>& lookup);

			Vector<SortableElement> mSortableElements;
			Vector<u32> mSortableElementIdx;
			Vector<const RenderElement*> mElements;

			Vector<RenderQueueElement> mSortedRenderElements;
			StateReduction mStateReductionMode;
		};

		/** @} */
	} // namespace render
} // namespace b3d
