//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsRendererExtension.h"
#include "BsVector2I.h"
#include "BsColor.h"
#include "BsMaterial.h"

namespace bs
{
	/** @addtogroup Scene-Editor-Internal
	 *  @{
	 */

	namespace ct { class SceneGridRenderer; }

	/** Determines how is the scene grid drawn. */
	enum class GridMode
	{
		Perspective, /**< Grid is drawn in XZ plane, at Y = 0. */
		OrthoX, /**< Grid is drawn in YZ plane, always visible along positive X. */
		OrthoY, /**< Grid is drawn in XZ plane, always visible along positive Y. */
		OrthoZ, /**< Grid is drawn in XY plane, always visible along positive Z. */
		OrthoNegX, /**< Grid is drawn in YZ plane, always visible along negative X. */
		OrthoNegY, /**< Grid is drawn in XZ plane, always visible along negative Y. */
		OrthoNegZ /**< Grid is drawn in XY plane, always visible along negative Z. */
	};

	/**	Handles rendering of the grid in the scene view. */
	class BS_ED_EXPORT SceneGrid
	{
	public:
		SceneGrid(const SPtr<Camera>& camera);
		~SceneGrid();

		/**	Sets the total width/height of the grid in XZ plane. */
		void setSize(UINT32 size);
		
		/**	Sets the spacing between grid lines. */
		void setSpacing(float spacing);

		/** Determines in what position and orientation is the grid drawn. */
		void setMode(GridMode mode);

		/** Changes the active editor settings. Grid properties will be updated internally when editor settings change. */
		void setSettings(const SPtr<EditorSettings>& settings);

		/** Called once per frame. */
		void _update();
	private:
		/** Updates internal grid parameters from the attached settings object. */
		void updateFromEditorSettings();

		/**	Rebuilds the scene grid mesh. Call this whenever grid parameters change. */
		void updateGridMesh();

		float mSpacing = 1.0f;
		UINT32 mSize = 256;
		GridMode mMode = GridMode::Perspective;
		bool mCoreDirty;

		SPtr<EditorSettings> mSettings;
		UINT32 mSettingsHash = 0;

		HMesh mGridMesh;
		SPtr<VertexDataDesc> mVertexDesc;
		SPtr<ct::SceneGridRenderer> mRenderer;
	};

	namespace ct
	{
	/** Handles scene grid rendering on the core thread. */
	class SceneGridRenderer : public RendererExtension
	{
	public:
		/** Structure used for initializing the renderer. */
		struct InitData
		{
			SPtr<CameraCore> camera;
			SPtr<MaterialCore> material;
		};

		SceneGridRenderer();

	private:
		friend class SceneGrid;

		/**	@copydoc RendererExtension::initialize */
		void initialize(const Any& data) override;

		/**	@copydoc RendererExtension::check */
		bool check(const CameraCore& camera) override;

		/**	@copydoc RendererExtension::render */
		void render(const CameraCore& camera) override;

		/**
		 * Updates the grid mesh to render.
		 * 			
		 * @param[in]	mesh			Grid mesh to render.
		 * @param[in]	spacing			Spacing between the grid lines.
		 * @param[in]	fadeGrid		Determines should the grid fade out at larger distances.
		 * @param[in]	gridPlaneNormal	Normal to the plane to render the grid on. Must be one of the basis vectors
		 *								(can't be arbitrary).						
		 */
		void updateData(const SPtr<MeshCore>& mesh, float spacing, bool fadeGrid, const Vector3& gridPlaneNormal);

		SPtr<CameraCore> mCamera;
		SPtr<MeshCore> mGridMesh;
		SPtr<MaterialCore> mGridMaterial;
		SPtr<GpuParamsSetCore> mMaterialParams;
		float mSpacing = 1.0f;
		bool mFadeGrid = true;
		Vector3 mGridPlaneNormal = Vector3::ZERO;

		MaterialParamMat4 mViewProjParam;
		MaterialParamVec4 mWorldCameraPosParam;
		MaterialParamColor mGridColorParam;
		MaterialParamFloat mGridSpacingParam;
		MaterialParamFloat mGridBorderWidthParam;
		MaterialParamFloat mGridFadeOutStartParam;
		MaterialParamFloat mGridFadeOutEndParam;
		MaterialParamVec3 mGridPlaneNormalParam;

		static const Color GRID_LINE_COLOR;
		static const float LINE_WIDTH;
		static const float LINE_BORDER_WIDTH;
		static const float FADE_OUT_START;
		static const float FADE_OUT_END;
	};
	}

	/** @} */
}