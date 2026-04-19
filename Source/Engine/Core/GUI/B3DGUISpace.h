//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIElement.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** GUI element that may be inserted into layouts in order to make a space of a fixed size. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIFixedSpace : public GUIElement
	{
	public:
		GUIFixedSpace(GUILogicalUnit size);
		~GUIFixedSpace() override = default;

		/**	Returns the size of the space in pixels. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Size))
		GUILogicalUnit GetSize() const { return mSize; }

		/**	Changes the size of the space to the specified value, in pixels. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Size))
		void SetSize(GUILogicalUnit size)
		{
			if(mSize != size)
			{
				mSize = size;
				MarkLayoutAsDirty();
			}
		}

		/**	Creates a new fixed space GUI element. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(GUIFixedSpace))
		static GUIFixedSpace* Create(GUILogicalUnit size);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		GUILogicalSize CalculateUnconstrainedOptimalSize() const override { return GUILogicalSize(GetSize(), GetSize()); }
		GUIConstrainedSizeRange CalculateConstrainedSizeRange() const override;

		/** @} */
	protected:
		GUILogicalUnit mSize;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GUIFixedSpaceRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
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
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIFlexibleSpace : public GUIElement
	{
	public:
		GUIFlexibleSpace();
		~GUIFlexibleSpace() override = default;

		/**	Creates a new flexible space GUI element. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(GUIFlexibleSpace))
		static GUIFlexibleSpace* Create();

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		GUILogicalSize CalculateUnconstrainedOptimalSize() const override { return GUILogicalSize(0, 0); }
		GUIConstrainedSizeRange CalculateConstrainedSizeRange() const override;

		/** @} */

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GUIFlexibleSpaceRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
