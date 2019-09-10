//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElement.h"
#include "Math/BsMath.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Displays a Camera view in the form of a GUI element. */
	class BS_EXPORT GUIViewport : public GUIElement
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName();

		/**
		 * Creates a new GUI viewport element.
		 *
		 * @param[in]	camera			Camera whos view to display in the element. Element will update the camera as it
		 *								resizes.
		 * @param[in]	aspectRatio		Initial aspect of the camera. Determines how is FOV adjusted as the element resizes.
		 * @param[in]	fieldOfView		Initial FOV of the camera. Determines how is FOV adjusted as the element resizes.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 *
		 * @note Render target used by the GUIWidget and Camera must be the same.
		 */
		static GUIViewport* create(const HCamera& camera, float aspectRatio, Degree fieldOfView, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI viewport element.
		 *
		 * @param[in]	camera			Camera whos view to display in the element. Element will update the camera as it
		 *								resizes.
		 * @param[in]	aspectRatio		Initial aspect of the camera. Determines how is FOV adjusted as the element resizes.
		 * @param[in]	fieldOfView		Initial FOV of the camera. Determines how is FOV adjusted as the element resizes.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 *
		 * @note Render target used by the GUIWidget and Camera must be the same.
		 */
		static GUIViewport* create(const GUIOptions& options, const HCamera& camera,
			float aspectRatio, Degree fieldOfView, const String& styleName = StringUtil::BLANK);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElement::_getOptimalSize */
		Vector2I _getOptimalSize() const override;

		/** @} */

	protected:
		~GUIViewport() = default;

		/** @copydoc GUIElement::_fillBuffer */
		void _fillBuffer(
			UINT8* vertices,
			UINT32* indices,
			UINT32 vertexOffset,
			UINT32 indexOffset,
			const Vector2I& offset,
			UINT32 maxNumVerts,
			UINT32 maxNumIndices,
			UINT32 renderElementIdx) const override;

		/** @copydoc GUIElement::updateClippedBounds */
		void updateClippedBounds() override;

		/** @copydoc GUIElement::updateRenderElementsInternal */
		void updateRenderElementsInternal() override;

	private:
		GUIViewport(const String& styleName, const HCamera& camera, float aspectRatio, Degree fieldOfView, const GUIDimensions& dimensions);

		/** @copydoc GUIElement::_changeParentWidget */
		void _changeParentWidget(GUIWidget* widget) override;

		HCamera mCamera;
		float mAspectRatio;
		Degree mFieldOfView;
		Radian mVerticalFOV;
	};

	/** @} */
}
