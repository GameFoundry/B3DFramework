//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUIConstructionMethods.h"
#include "B3DPrerequisites.h"
#include "GUI/B3DGUILayout.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** Structure describing contents of a GUIPanel element. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(GUI)) GUIPanelContent
	{
		GUIPanelContent(i16 depth = 0, u16 depthRangeMinimum = 65535, u16 depthRangeMaximum = 65535)
			: Depth(depth), DepthRangeMinimum(depthRangeMinimum), DepthRangeMaximum(depthRangeMaximum)
		{ }

		/**
		 * Determines rendering order of the GUI panel. Panels with lower depth will be rendered in front of panels
		 * with higher depth. Provided depth is relative to depth of the parent GUI panel (if any).
		 */
		i16 Depth;

		/**
		 * Minimum range of depths that children of this GUI panel can have. If any panel has depth outside of the
		 * range [Depth - DepthRangeMinimum, Depth + DepthRangeMaximum] it will be clamped to nearest extreme. Value
		 * of 65535 means infinite range.
		 */
		u16 DepthRangeMinimum;

		/**
		 * Maximum range of depths that children of this GUI panel can have. If any panel has depth outside of the
		 * range [Depth - DepthRangeMinimum, Depth + DepthRangeMaximum] it will be clamped to nearest extreme. Value
		 * of 65535 means infinite range.
		 */
		u16 DepthRangeMaximum;
	};

	/**	Represents a GUI panel that you can use for free placement of GUI elements within its bounds. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIPanel final : public GUILayout, public TGUIConstructionMethods<GUIPanel, GUIPanelContent>
	{
	public:
		/** @name Internal
		 *  @{
		 */

		struct PrivatelyConstruct { };

		/** @} */

		GUIPanel() = default;
		GUIPanel(PrivatelyConstruct, const GUIPanelContent& content, const String& styleClass, const GUISizeConstraints& sizeConstraints);
		~GUIPanel() = default;

		/**
		 * Changes values that control at which depth is GUI panel and its children rendered.
		 *
		 * @param	depth					Determines rendering order of the GUI panel. Panels with lower depth will be
		 *									rendered in front of panels with higher depth. Provided depth is relative to depth
		 *									of the parent GUI panel (if any).
		 * @param	depthRangeMinimum		Minimum range of depths that children of this GUI panel can have. If any panel has
		 *									depth outside of the range [depth - depthRangeMinimum, depth + depthRangeMaximum] it will
		 *									be clamped to nearest extreme. Value of -1 means infinite range.
		 * @param	depthRangeMaximum		Maximum range of depths that children of this GUI panel can have. If any panel has
		 *									depth outside of the range [depth - depthRangeMinimum, depth + depthRangeMaximum] it will
		 *									be clamped to nearest extreme. Value of -1 means infinite range.
		 */
		void SetDepthRange(i16 depth = 0, u16 depthRangeMinimum = -1, u16 depthRangeMaximum = -1);

		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& GetGuiTypeName();
	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		void UpdateOptimalLayoutSizes() override;

		/**
		 * Calculates positions & sizes of all elements in the layout. This method expects a pre-allocated array to store
		 * the data in.
		 *
		 * @param	layoutSize			Size of the parent layout area to position the child elements in.
		 * @param	outElementPositions	Array to hold output positions. Must be the same size as the number of child elements.
		 * @param	outElementSizes		Array to hold output areas. Must be the same size as the number of child elements.
		 * @param	elementCount		Size of the element positions/sizes arrays.
		 * @param	sizeRanges			Ranges of possible sizes used for the child elements. Array must be same size as elements array.
		 */
		void GetChildRelativeLayoutAreas(const GUILogicalSize& layoutSize, GUILogicalPoint* outElementPositions, GUILogicalSize* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSizeRange>& sizeRanges) const;

		/** Calculates the size of the provided child within this layout with the provided dimensions. */
		GUILogicalArea CalculateRelativeElementArea(const GUILogicalSize& layoutSize, const GUIElement* element, const GUIConstrainedSizeRange& sizeRange) const;

		/**
		 * Calculates an element size range for the provided child of the GUI panel. Will return cached bounds so make sure
		 * to update optimal size ranges before calling.
		 */
		GUIConstrainedSizeRange GetChildConstrainedSizeRange(const GUIElement* element) const;

		void UpdateLayoutForChildren() override;

		/**
		 * Updates the provided depth range by taking into consideration the depth range of the panel. This depth range
		 * should be passed on to child elements of the panel.
		 */
		void UpdateDepthRangeInternal(GUILayoutData& data);

		/** @} */

	protected:
		i16 mDepthOffset;
		u16 mDepthRangeMin;
		u16 mDepthRangeMax;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GUIPanelRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
