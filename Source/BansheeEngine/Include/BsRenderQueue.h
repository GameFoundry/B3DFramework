//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsVector3.h"
#include "BsSubMesh.h"

namespace bs 
{
	/** @addtogroup Renderer-Engine-Internal
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
	struct BS_EXPORT RenderQueueElement
	{
		RenderQueueElement()
			:renderElem(nullptr), passIdx(0), applyPass(true)
		{ }

		RenderableElement* renderElem;
		UINT32 passIdx;
		bool applyPass;
	};

	/**
	 * Render objects determines rendering order of objects contained within it. Rendering order is determined by object
	 * material, and can influence rendering of transparent or opaque objects, or be used to improve performance by grouping
	 * similar objects together.
	 */
	class BS_EXPORT RenderQueue
	{
		/**	Data used for renderable element sorting. Represents a single pass for a single mesh. */
		struct SortableElement
		{
			UINT32 seqIdx;
			INT32 priority;
			float distFromCamera;
			UINT32 shaderId;
			UINT32 passIdx;
		};

	public:
		RenderQueue(StateReduction grouping = StateReduction::Distance);
		virtual ~RenderQueue() { }

		/**
		 * Adds a new entry to the render queue.
		 *
		 * @param[in]	element			Renderable element to add to the queue.
		 * @param[in]	distFromCamera	Distance of this object from the camera. Used for distance sorting.
		 */
		void add(RenderableElement* element, float distFromCamera);

		/**	Clears all render operations from the queue. */
		void clear();
		
		/**	Sorts all the render operations using user-defined rules. */
		virtual void sort();

		/** Returns a list of sorted render elements. Caller must ensure sort() is called before this method. */
		const Vector<RenderQueueElement>& getSortedElements() const;

		/**
		 * Controls if and how a render queue groups renderable objects by material in order to reduce number of state 
		 * changes.
		 */
		void setStateReduction(StateReduction mode) { mStateReductionMode = mode; }

	protected:
		/**	Callback used for sorting elements with no material grouping. */
		static bool elementSorterNoGroup(UINT32 aIdx, UINT32 bIdx, const Vector<SortableElement>& lookup);

		/**	Callback used for sorting elements with preferred material grouping. */
		static bool elementSorterPreferGroup(UINT32 aIdx, UINT32 bIdx, const Vector<SortableElement>& lookup);

		/**	Callback used for sorting elements with material grouping after sorting. */
		static bool elementSorterPreferSort(UINT32 aIdx, UINT32 bIdx, const Vector<SortableElement>& lookup);

		Vector<SortableElement> mSortableElements;
		Vector<UINT32> mSortableElementIdx;
		Vector<RenderableElement*> mElements;

		Vector<RenderQueueElement> mSortedRenderElements;
		StateReduction mStateReductionMode;
	};

	/** @} */
}