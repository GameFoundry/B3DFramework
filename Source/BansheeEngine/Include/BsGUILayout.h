//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUIElementBase.h"
#include "BsVector2I.h"

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
		Type* addNewElement(Args &&...args)
		{
			Type* elem = Type::create(std::forward<Args>(args)...);
			addElement(elem);
			return elem;
		}

		/**	Creates a new element and inserts it before the element at the specified index. */
		template<class Type, class... Args>
		Type* insertNewElement(UINT32 idx, Args &&...args)
		{
			Type* elem = Type::create(std::forward<Args>(args)...);
			insertElement(idx, elem);
			return elem;
		}

		/**	Adds a new element to the layout after all existing elements. */
		void addElement(GUIElementBase* element);

		/**	Removes the specified element from the layout. */
		void removeElement(GUIElementBase* element);

		/**	Removes a child element at the specified index. */
		void removeElementAt(UINT32 idx);

		/**	Inserts a GUI element before the element at the specified index. */
		void insertElement(UINT32 idx, GUIElementBase* element);

		/**	Returns number of child elements in the layout. */
		UINT32 getNumChildren() const { return (UINT32)mChildren.size(); }

		/**	Destroy the layout. Removes it from parent and widget, and deletes it. */
		static void destroy(GUILayout* layout);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElementBase::_getLayoutSizeRange */
		LayoutSizeRange _getLayoutSizeRange() const override { return _getCachedSizeRange(); }

		/** Returns a size range that was cached during the last GUIElementBase::_updateOptimalLayoutSizes call. */
		LayoutSizeRange _getCachedSizeRange() const { return mSizeRange; }

		/**
		 * Returns a size ranges for all children that was cached during the last GUIElementBase::_updateOptimalLayoutSizes
		 * call.
		 */
		const Vector<LayoutSizeRange>& _getCachedChildSizeRanges() const { return mChildSizeRanges; }

		/** @copydoc GUIElementBase::_getOptimalSize */
		Vector2I _getOptimalSize() const override { return mSizeRange.optimal; }

		/** @copydoc GUIElementBase::_getPadding */
		const RectOffset& _getPadding() const override;

		/** @copydoc GUIElementBase::_getType */
		virtual Type _getType() const override { return GUIElementBase::Type::Layout; }

		/** @} */

	protected:
		Vector<LayoutSizeRange> mChildSizeRanges;
		LayoutSizeRange mSizeRange;
	};

	/** @} */
}