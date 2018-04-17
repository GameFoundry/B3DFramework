//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElementBase.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** GUI element that may be inserted into layouts in order to make a space of a fixed size. */
	class BS_EXPORT GUIFixedSpace : public GUIElementBase
	{
	public:
		GUIFixedSpace(UINT32 size)
			:mSize(size)
		{ }

		~GUIFixedSpace();

		/**	Returns the size of the space in pixels. */
		UINT32 getSize() const { return mSize; }

		/**	Changes the size of the space to the specified value, in pixels. */
		void setSize(UINT32 size) { if (mSize != size) { mSize = size; _markLayoutAsDirty(); } }

		/**	Creates a new fixed space GUI element. */
		static GUIFixedSpace* create(UINT32 size);

		/**	Destroys the space and removes it from its parent. */
		static void destroy(GUIFixedSpace* space);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElementBase::_getType */
		Type _getType() const override { return GUIElementBase::Type::FixedSpace; }

		/** @copydoc GUIElementBase::_getOptimalSize */
		Vector2I _getOptimalSize() const override { return Vector2I(getSize(), getSize()); }

		/** @copydoc GUIElementBase::_calculateLayoutSizeRange */
		LayoutSizeRange _calculateLayoutSizeRange() const override;

		/** @copydoc GUIElementBase::_getPadding */
		const RectOffset& _getPadding() const override
		{
			static RectOffset padding;

			return padding;
		}

		/** @} */
	protected:
		UINT32 mSize;
	};

	/**
	 * GUI element that may be inserted into layouts to make a space of a flexible size. The space will expand only if 
	 * there is room and other elements are not squished because of it. If multiple flexible spaces are in a layout, their
	 * sizes will be shared equally.
	 *
	 * @note	
	 * For example if you had a horizontal layout with a button, and you wanted to align that button to the right of the
	 * layout, you would insert a flexible space before the button in the layout.
	 */
	class BS_EXPORT GUIFlexibleSpace : public GUIElementBase
	{
	public:
		GUIFlexibleSpace() {}
		~GUIFlexibleSpace();

		/**	Creates a new flexible space GUI element. */
		static GUIFlexibleSpace* create();

		/**	Destroys the space and removes it from its parent. */
		static void destroy(GUIFlexibleSpace* space);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElementBase::_getType */
		Type _getType() const override { return GUIElementBase::Type::FlexibleSpace; }

		/** @copydoc GUIElementBase::_getOptimalSize */
		Vector2I _getOptimalSize() const override { return Vector2I(0, 0); }

		/** @copydoc GUIElementBase::_calculateLayoutSizeRange */
		LayoutSizeRange _calculateLayoutSizeRange() const override;

		/** @copydoc GUIElementBase::_getPadding */
		const RectOffset& _getPadding() const override
		{
			static RectOffset padding;

			return padding;
		}

		/** @} */
	};

	/** @} */
}