//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * Contains a set of elements that can be navigated between using keyboard or gamepad buttons (i.e. the 'Tab' button)
	 */
	class BS_EXPORT GUINavGroup
	{
	public:
		/** Creates a new navigation group. */
		static SPtr<GUINavGroup> Create();

		/** Sets the focus to the first element in the group. */
		void FocusFirst();

		/**
		 * Sets the focus to the next element in the navigation order.
		 *
		 * @param[in]	anchor		Element relative to which to determine the navigation. This is usually the currently
		 *							focused element.
		 */
		void FocusNext(GUIElement* anchor);

	private:
		friend class GUIElement;
		friend class GUIManager;

		/**
		 * Registers a new element with the tab group. Caller must ensure the element has been removed from the previous
		 * tab group, if any.
		 *
		 * @param[in]	element		Element to register.
		 * @param[in]	tabIdx		Index of the element in the tab group. Set setIndex() for more information on how
		 *							is the index interpreted.
		 */
		void RegisterElement(GUIElement* element, i32 tabIdx = 0);

		/**
		 * Sets the index of a previously registered element in the tab group. The index determines in what order will the
		 * element be visited compared to other elements.
		 *
		 * Index 0 is reserved and means that tab index will be calculated automatically based on element position. The
		 * rest of indices are visited in order from lowest to highest. Negative indices are visited before auto-positioned
		 * 0-index element, and positive indices are visited after.
		 */
		void SetIndex(GUIElement* element, i32 tabIdx);

		/** Unregisters an element from the tab group. */
		void UnregisterElement(GUIElement* element);

		/** Sets the focus to the top-left element. Only iterates over elements with no explicit tab index. */
		void FocusTopLeft();

		UnorderedMap<GUIElement*, i32> mElements;
		MultiMap<i32, GUIElement*> mOrderedElements;
	};

	/** @} */
} // namespace bs
