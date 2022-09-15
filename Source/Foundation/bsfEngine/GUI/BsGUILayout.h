//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElementBase.h"
#include "Math/BsVector2I.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**
	 * Base class for layout GUI element. Layout element positions and sizes any child elements according to element styles
	 * and layout options.
	 */
	class BS_EXPORT GUILayout : public GUIElementBase
	{
	public:
		GUILayout(const GUIDimensions& dimensions);
		GUILayout();
		virtual ~GUILayout();

		/**	Creates a new element and adds it to the layout after all existing elements. */
		template<class Type, class... Args>
		Type* AddNewElement(Args &&...args)
		{
			Type* elem = Type::Create(std::forward<Args>(args)...);
			AddElement(elem);
			return elem;
		}

		/**	Creates a new element and inserts it before the element at the specified index. */
		template<class Type, class... Args>
		Type* InsertNewElement(UINT32 idx, Args &&...args)
		{
			Type* elem = Type::Create(std::forward<Args>(args)...);
			InsertElement(idx, elem);
			return elem;
		}

		/**	Adds a new element to the layout after all existing elements. */
		void AddElement(GUIElementBase* element);

		/**	Removes the specified element from the layout. */
		void RemoveElement(GUIElementBase* element);

		/**	Removes a child element at the specified index. */
		void RemoveElementAt(UINT32 idx);

		/**	Inserts a GUI element before the element at the specified index. */
		void InsertElement(UINT32 idx, GUIElementBase* element);

		/** Removes all child elements and destroys them. */
		void Clear();

		/**	Returns number of child elements in the layout. */
		UINT32 GetNumChildren() const { return (UINT32)mChildren.size(); }

		/**	Destroy the layout. Removes it from parent and widget, and deletes it. */
		static void Destroy(GUILayout* layout);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElementBase::_getLayoutSizeRange */
		LayoutSizeRange GetLayoutSizeRangeInternal() const override { return GetCachedSizeRangeInternal(); }

		/** Returns a size range that was cached during the last GUIElementBase::_updateOptimalLayoutSizes call. */
		LayoutSizeRange GetCachedSizeRangeInternal() const { return mSizeRange; }

		/**
		 * Returns a size ranges for all children that was cached during the last GUIElementBase::_updateOptimalLayoutSizes
		 * call.
		 */
		const Vector<LayoutSizeRange>& GetCachedChildSizeRangesInternal() const { return mChildSizeRanges; }

		/** @copydoc GUIElementBase::GetOptimalSizeInternal */
		Vector2I GetOptimalSizeInternal() const override { return mSizeRange.optimal; }

		/** @copydoc GUIElementBase::GetPaddingInternal */
		const RectOffset& GetPaddingInternal() const override;

		/** @copydoc GUIElementBase::_getType */
		Type GetTypeInternal() const override { return GUIElementBase::Type::Layout; }

		/** @} */

	protected:
		Vector<LayoutSizeRange> mChildSizeRanges;
		LayoutSizeRange mSizeRange;
	};

	/** @} */
}
