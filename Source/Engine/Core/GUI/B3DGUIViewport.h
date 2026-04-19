//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIInteractable.h"
#include "Math/B3DMath.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Displays a Camera view in the form of a GUI element. */
	class B3D_EXPORT GUIViewport : public GUIInteractable
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/**
		 * Creates a new GUI viewport element.
		 *
		 * @param	camera			Camera whos view to display in the element. Element will update the camera as it
		 *							resizes.
		 * @param	aspectRatio		Initial aspect of the camera. Determines how is FOV adjusted as the element resizes.
		 * @param	fieldOfView		Initial FOV of the camera. Determines how is FOV adjusted as the element resizes.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *							GUIWidget the element is used on. If not specified default style is used.
		 *
		 * @note Render target used by the GUIWidget and Camera must be the same.
		 */
		static GUIViewport* Create(const HCamera& camera, float aspectRatio, Degree fieldOfView, const String& styleName = StringUtility::kBlank);

		/**
		 * Creates a new GUI viewport element.
		 *
		 * @param	camera			Camera whos view to display in the element. Element will update the camera as it
		 *							resizes.
		 * @param	aspectRatio		Initial aspect of the camera. Determines how is FOV adjusted as the element resizes.
		 * @param	fieldOfView		Initial FOV of the camera. Determines how is FOV adjusted as the element resizes.
		 * @param	options			Options that allow you to control how is the element positioned and sized. This will
		 *							override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *							GUIWidget the element is used on. If not specified default style is used.
		 *
		 * @note Render target used by the GUIWidget and Camera must be the same.
		 */
		static GUIViewport* Create(const GUIOptions& options, const HCamera& camera, float aspectRatio, Degree fieldOfView, const String& styleName = StringUtility::kBlank);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		GUILogicalSize CalculateUnconstrainedOptimalSize() const override;

		/** @} */

	protected:
		~GUIViewport() = default;

		void FillBuffer(u8* vertices, u32* indices, u32 vertexOffset, u32 indexOffset, const Vector2I& offset, u32 maxVertexCount, u32 maxIndexCount, u32 renderElementIdx) const override;
		void UpdateRenderElements() override;

	private:
		GUIViewport(const String& styleName, const HCamera& camera, float aspectRatio, Degree fieldOfView, const GUISizeConstraints& dimensions);

		void ChangeParentWidget(GUIWidget* widget) override;

		HCamera mCamera;
		float mAspectRatio;
		Degree mFieldOfView;
		Radian mVerticalFOV;
	};

	/** @} */
} // namespace b3d
