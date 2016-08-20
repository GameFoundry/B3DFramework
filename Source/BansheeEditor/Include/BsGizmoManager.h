//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsModule.h"
#include "BsColor.h"
#include "BsVector2I.h"
#include "BsMatrix4.h"
#include "BsGpuParam.h"
#include "BsDrawHelper.h"

namespace BansheeEngine
{
	/** @addtogroup Scene-Editor
	 *  @{
	 */

	class GizmoManagerCore;

	/**
	 * Handles the rendering and picking of gizmo elements. Gizmos are icons and 3D objects usually rendered in scene view
	 * for various visualization purposes (for example a Camera component will have a gizmo that draws a Camera icon since
	 * otherwise it has no visual representation). Aside from being rendered, gizmos can also be selected by the user as if
	 * they were normal scene elements.
	 */
	class BS_ED_EXPORT GizmoManager : public Module<GizmoManager>
	{
	public:
		GizmoManager();
		~GizmoManager();

		/**
		 * Starts gizmo creation. All further call will be referencing this gizmo. Must be followed by a matching 
		 * endGizmo().
		 *
		 * @param	gizmoParent	Scene object this gizmo is attached to. Selecting the gizmo will select this scene object.
		 */
		void startGizmo(const HSceneObject& gizmoParent);

		/**	Ends gizmo creation. Must be called after a matching startGizmo(). */
		void endGizmo();

		/**	Changes the color of any further gizmo draw calls. */
		void setColor(const Color& color);

		/**	Changes the transform that will be applied to meshes of any further gizmo draw calls.  */
		void setTransform(const Matrix4& transform);

		/**
		 * If pickable is set to true, gizmo can be selected by the user, otherwise it will be drawn but cannot be
		 * interacted with.
		 *
		 * @note	Must be called between startGizmo/endGizmo calls.
		 */
		void setPickable(bool pickable) { mPickable = pickable; }

		/**	Returns the currently set gizmo color. */
		Color getColor() const { return mColor; }

		/**	Returns the currently set gizmo transform. */
		Matrix4 getTransform() const { return mTransform; }

		/**
		 * Draws an axis aligned cuboid.
		 *
		 * @param[in]	position	Center of the cuboid.
		 * @param[in]	extents		Radius of the cuboid in each axis.
		 *
		 * @note	Must be called between startGizmo/endGizmo calls.
		 */
		void drawCube(const Vector3& position, const Vector3& extents);

		/**
		 * Draws a sphere.
		 *
		 * @note	Must be called between startGizmo/endGizmo calls.
		 */
		void drawSphere(const Vector3& position, float radius);

		/**
		 * Draws a solid cone.
		 *
		 * @param[in]	base		Position of the center of the base of the cone.
		 * @param[in]	normal		Orientation of the cone, pointing from center base to the tip of the cone.
		 * @param[in]	height		Height of the cone (along the normal).
		 * @param[in]	radius		Radius of the base of the cone.
		 * @param[in]	scale		Scale applied to cone's disc width & height. Allows you to create elliptical cones.
		 */
		void drawCone(const Vector3& base, const Vector3& normal, float height, float radius, 
			const Vector2& scale = Vector2::ONE);

		/**
		 * Draws a wireframe axis aligned cuboid.
		 *
		 * @param[in]	position	Center of the cuboid.
		 * @param[in]	extents		Radius of the cuboid in each axis.
		 *
		 * @note	Must be called between startGizmo/endGizmo calls.
		 */
		void drawWireCube(const Vector3& position, const Vector3& extents);

		/**
		 * Draws a wireframe sphere represented by three discs.
		 *
		 * @note	Must be called between startGizmo/endGizmo calls.
		 */
		void drawWireSphere(const Vector3& position, float radius);

		/**
		 * Draws a wireframe capsule.
		 *
		 * @param[in]	position	World coordinates of the center of the capsule.
		 * @param[in]	height		Distance between the centers of the capsule's hemispheres.
		 * @param[in]	radius		Distance of each point from the capsule's center-line.
		 */
		void drawWireCapsule(const Vector3& position, float height, float radius);

		/**
		 * Draws a wireframe cone.
		 *
		 * @param[in]	base		Position of the center of the base of the cone.
		 * @param[in]	normal		Orientation of the cone, pointing from center base to the tip of the cone.
		 * @param[in]	height		Height of the cone (along the normal).
		 * @param[in]	radius		Radius of the base of the cone.
		 * @param[in]	scale		Scale applied to cone's disc width & height. Allows you to create elliptical cones.
		 */
		void drawWireCone(const Vector3& base, const Vector3& normal, float height, float radius, 
			const Vector2& scale = Vector2::ONE);

		/**
		 * Draws a line between two points.
		 *
		 * @note	Must be called between startGizmo/endGizmo calls.
		 */
		void drawLine(const Vector3& start, const Vector3& end);

		/**
		 * Draws a list of lines. Provided array must contain pairs of the line start point followed by an end point.
		 *
		 * @note	Must be called between startGizmo/endGizmo calls.
		 */
		void drawLineList(const Vector<Vector3>& linePoints);

		/**
		 * Draws a wireframe disc.
		 *
		 * @param[in]	position	Center of the disc.
		 * @param[in]	normal		Orientation of the disc, pointing in the direction the disc is visible in.
		 * @param[in]	radius		Radius of the disc.
		 */
		void drawWireDisc(const Vector3& position, const Vector3& normal, float radius);

		/**
		 * Draws a wireframe arc.
		 *
		 * @param[in]	position	Center of the arc.
		 * @param[in]	normal		Orientation of the arc, pointing in the direction the arc is visible in.
		 * @param[in]	radius		Radius of the arc.
		 * @param[in]	startAngle	Angle at which to start the arc.
		 * @param[in]	amountAngle	Length of the arc.
		 */
		void drawWireArc(const Vector3& position, const Vector3& normal, float radius, Degree startAngle, Degree amountAngle);

		/** 
		 * Draws a wireframe mesh.
		 *
		 * @param[in]	meshData	Object containing mesh vertices and indices. Vertices must be Vertex3 and indices 
		 *							32-bit.
		 */
		void drawWireMesh(const SPtr<MeshData>& meshData);

		/**
		 * Draws a wireframe frustum.
		 *
		 * @param[in]	position	Origin of the frustum, or the eye point.
		 * @param[in]	aspect		Ratio of frustum width over frustum height.
		 * @param[in]	FOV			Horizontal field of view in degrees.
		 * @param[in]	near		Distance to the near frustum plane.
		 * @param[in]	far			Distance to the far frustum plane.
		 *
		 * @note	Must be called between startGizmo/endGizmo calls.
		 */
		void drawFrustum(const Vector3& position, float aspect, Degree FOV, float near, float far);

		/**
		 * Draws an icon that always faces the camera.
		 *
		 * @param[in]	position	Position of the center of the icon.
		 * @param[in]	image		Sprite image for the icon to draw.
		 * @param[in]	fixedScale	If true then the icon will remain the same size regardless of the distance from camera.
		 *
		 * @note	Must be called between startGizmo/endGizmo calls.
		 */
		void drawIcon(Vector3 position, HSpriteTexture image, bool fixedScale);

		/**
		 * Draws a mesh representing 2D text with the specified properties. 
		 *
		 * @param[in]	position	Position to render the text at. Text will be centered around this point.
		 * @param[in]	text		Text to draw.
		 * @param[in]	font		Font to use for rendering the text's characters.
		 * @param[in]	fontSize	Size of the characters, in points.
		 */
		void drawText(const Vector3& position, const WString& text, const HFont& font, UINT32 fontSize = 16);

		/**
		 * Clears all gizmo data, but doesn't update the meshes or the render data. (Calling update would create empty
		 * meshes, but before calling update gizmos will still be drawn).
		 */
		void clearGizmos();

		/**
		 * Clears gizmo render data like meshes, but doesn't clear the original gizmo data (Calling update would just
		 * recreate the render data).
		 */
		void clearRenderData();

		/**
		 * Returns a scene object that was attached to a specific gizmo.
		 *
		 * @param[in]	gizmoIdx	Index of the gizmo to look for.
		 */
		HSceneObject getSceneObject(UINT32 gizmoIdx);

		/** @name Internal
		 *  @{
		 */

		/**
		 * Updates all the gizmo meshes to reflect all draw calls submitted since clearGizmos().
		 *
		 * @note	Internal method.
		 */
		void update(const SPtr<Camera>& camera);

		/**
		 * Queues all gizmos to be rendered for picking. Each gizmo is draw with a separate color so we can identify them
		 * later.
		 *
		 * @param[in]	camera				Camera to draw the gizmos on.
		 * @param[in]	idxToColorCallback	Callback that assigns a unique color to each gizmo index.
		 *
		 * @note	Internal method.
		 */
		void renderForPicking(const SPtr<Camera>& camera, std::function<Color(UINT32)> idxToColorCallback);

		/** @} */

	private:
		friend class GizmoManagerCore;

		/**	Supported types of gizmo materials (shaders) */
		enum class GizmoMaterial
		{
			Solid, Wire, Line, Picking, PickingAlpha, Text
		};

		/**	Common data shared by all gizmo types. */
		struct CommonData
		{
			UINT32 idx;
			Color color;
			Matrix4 transform;
			HSceneObject sceneObject;
			bool pickable;
		};

		/**	Data required for rendering a cuboid gizmo. */
		struct CubeData : CommonData
		{
			Vector3 position;
			Vector3 extents;
		};

		/**	Data required for rendering a sphere gizmo. */
		struct SphereData : CommonData
		{
			Vector3 position;
			float radius;
		};

		/**	Data required for rendering a cone gizmo. */
		struct ConeData : CommonData
		{
			Vector3 base;
			Vector3 normal;
			float radius;
			float height;
			Vector2 scale;
		};

		/**	Data required for rendering a line gizmo. */
		struct LineData : CommonData
		{
			Vector3 start;
			Vector3 end;
		};

		/**	Data required for rendering a list of lines. */
		struct LineListData : CommonData
		{
			Vector<Vector3> linePoints;
		};

		/**	Data required for rendering a wireframe disc gizmo. */
		struct WireDiscData : CommonData
		{
			Vector3 position;
			Vector3 normal;
			float radius;
		};

		/**	Data required for rendering a wireframe arc gizmo. */
		struct WireArcData : CommonData
		{
			Vector3 position;
			Vector3 normal;
			float radius;
			Degree startAngle;
			Degree amountAngle;
		};

		/**	Data required for rendering a wireframe mesh gizmo. */
		struct WireMeshData : CommonData
		{
			SPtr<MeshData> meshData;
		};

		/**	Data required for rendering a frustum gizmo. */
		struct FrustumData : CommonData
		{
			Vector3 position;
			float aspect;
			Degree FOV;
			float near;
			float far;
		};

		/**	Data required for rendering an icon gizmo. */
		struct IconData : CommonData
		{
			Vector3 position;
			bool fixedScale;
			HSpriteTexture texture;
		};

		/**	Data required for rendering text. */
		struct TextData : CommonData
		{
			Vector3 position;
			WString text;
			HFont font;
			UINT32 fontSize;
		};

		/**	Stores how many icons use a specific texture. */
		struct IconRenderData
		{
			UINT32 count;
			SPtr<TextureCore> texture;
		};

		/**	Data used for initializing the core thread equivalent of the gizmo manager. */
		struct CoreInitData
		{
			SPtr<MaterialCore> solidMat;
			SPtr<MaterialCore> wireMat;
			SPtr<MaterialCore> lineMat;
			SPtr<MaterialCore> iconMat;
			SPtr<MaterialCore> textMat;
			SPtr<MaterialCore> pickingMat;
			SPtr<MaterialCore> alphaPickingMat;
		};

		typedef Vector<IconRenderData> IconRenderDataVec;
		typedef SPtr<IconRenderDataVec> IconRenderDataVecPtr;

		/**
		 * Builds a brand new mesh that can be used for rendering all icon gizmos.
		 *
		 * @param[in]	camera		Camera the mesh will be rendered to.
		 * @param[in]	iconData	A list of all icons and their properties.
		 * @param[in]	forPicking	Whether the icons will be rendered normally, or with a special material for picking.
		 * @param[in]	renderData	Output data that outlines the structure of the returned mesh. It tells us which portions
		 *							of the mesh use which icon texture.
		 *
		 * @return					A mesh containing all of the visible icons. Mesh is allocated using the icon mesh heap
		 *							and should be deallocated manually.
		 */
		SPtr<TransientMesh> buildIconMesh(const SPtr<Camera>& camera, const Vector<IconData>& iconData, bool forPicking, IconRenderDataVecPtr& renderData);

		/**	Resizes the icon width/height so it is always scaled to optimal size (with preserved aspect). */
		void limitIconSize(UINT32& width, UINT32& height);

		/**
		 * Calculates colors for an icon based on its position in the camera. For example icons too close to too far might
		 * be faded.
		 *
		 * @param[in]	tint		Primary tint for the icon.
		 * @param[in]	camera		Camera in which the icon will be rendered in.
		 * @param[in]	iconHeight	Height of the icon in pixels.
		 * @param[in]	fixedScale	Whether the icon size changes depending on distance from the camera.
		 * @param[in]	normalColor	Normal color of the icon.
		 * @param[in]	fadedColor	Faded color to be used when icon is occluded by geometry.
		 */
		void calculateIconColors(const Color& tint, const SPtr<Camera>& camera, UINT32 iconHeight, bool fixedScale,
			Color& normalColor, Color& fadedColor);

		/**	Initializes the core thread version of the gizmo manager. */
		void initializeCore(const CoreInitData& initData);

		/**	Destroys the core thread version of the gizmo manager. */
		void destroyCore(GizmoManagerCore* core);

		static const UINT32 VERTEX_BUFFER_GROWTH;
		static const UINT32 INDEX_BUFFER_GROWTH;
		static const UINT32 SPHERE_QUALITY;
		static const UINT32 WIRE_SPHERE_QUALITY;
		static const float MAX_ICON_RANGE;
		static const UINT32 OPTIMAL_ICON_SIZE;
		static const float ICON_TEXEL_WORLD_SIZE;

		typedef Set<IconData, std::function<bool(const IconData&, const IconData&)>> IconSet;

		Color mColor;
		Matrix4 mTransform;
		HSceneObject mActiveSO;
		bool mPickable;
		UINT32 mCurrentIdx;
		bool mTransformDirty;
		bool mColorDirty;

		DrawHelper* mDrawHelper;
		DrawHelper* mPickingDrawHelper;

		Vector<CubeData> mSolidCubeData;
		Vector<CubeData> mWireCubeData;
		Vector<SphereData> mSolidSphereData;
		Vector<SphereData> mWireSphereData;
		Vector<ConeData> mSolidConeData;
		Vector<ConeData> mWireConeData;
		Vector<LineData> mLineData;
		Vector<LineListData> mLineListData;
		Vector<WireDiscData> mWireDiscData;
		Vector<WireArcData> mWireArcData;
		Vector<WireMeshData> mWireMeshData;
		Vector<FrustumData> mFrustumData;
		Vector<IconData> mIconData;
		Vector<TextData> mTextData;
		Map<UINT32, HSceneObject> mIdxToSceneObjectMap;

		Vector<DrawHelper::ShapeMeshData> mActiveMeshes;

		SPtr<MeshHeap> mIconMeshHeap;
		SPtr<TransientMesh> mIconMesh;

		std::atomic<GizmoManagerCore*> mCore;

		// Immutable
		SPtr<VertexDataDesc> mIconVertexDesc;

		// Transient
		struct SortedIconData
		{
			float distance;
			Vector2I screenPosition;
			UINT32 iconIdx;
		};

		Vector<SortedIconData> mSortedIconData;
	};

	/** @} */
	/** @addtogroup Scene-Editor-Internal
	 *  @{
	 */

	/**
	 * Core thread version of the gizmo manager that handles most of the rendering of meshes provided by the gizmo manager.
	 */
	class GizmoManagerCore
	{
		friend class GizmoManager;

		/**	Solid gizmo material and parameter handles. */
		struct SolidMaterialData
		{
			SPtr<MaterialCore> mat;
			SPtr<GpuParamsSetCore> params;
			GpuParamMat4Core viewProj;
			GpuParamVec4Core viewDir;
		};

		/**	Wire gizmo material and parameter handles. */
		struct WireMaterialData
		{
			SPtr<MaterialCore> mat;
			SPtr<GpuParamsSetCore> params;
			GpuParamMat4Core viewProj;
		};

		/**	Icon gizmo material and parameter handles. */
		struct IconMaterialData
		{
			SPtr<MaterialCore> mat;
			SPtr<GpuParamsSetCore> params;
			GpuParamMat4Core viewProj[2];
			GpuParamTextureCore texture[2];
		};

		/**	Text gizmo material and parameter handles. */
		struct TextMaterialData
		{
			SPtr<MaterialCore> mat;
			SPtr<GpuParamsSetCore> params;
			GpuParamMat4Core viewProj;
			GpuParamTextureCore texture;
		};

		/**	Gizmo material and parameter handles used for picking. */
		struct PickingMaterialData
		{
			SPtr<MaterialCore> mat;
			SPtr<GpuParamsSetCore> params;
			GpuParamMat4Core viewProj;
		};

		/**
		 * Gizmo material and parameter handles used for picking, with blending support (generally used for icon picking).
		 */
		struct AlphaPickingMaterialData
		{
			SPtr<MaterialCore> mat;
			SPtr<GpuParamsSetCore> params;
			GpuParamMat4Core viewProj;
			GpuParamTextureCore texture;
		};

		/** Type of mesh that can be drawn. */
		enum class MeshType
		{
			Solid, Line, Wire, Text
		};

		/** Data about a mesh rendered by the draw manager. */
		struct MeshData
		{
			MeshData(const SPtr<MeshCoreBase>& mesh, SPtr<TextureCore> texture, MeshType type)
				:mesh(mesh), texture(texture), type(type)
			{ }

			SPtr<MeshCoreBase> mesh;
			SPtr<TextureCore> texture;
			MeshType type;
		};

		struct PrivatelyConstuct { };

	public:
		GizmoManagerCore(const PrivatelyConstuct& dummy);
		~GizmoManagerCore();

	private:
		/**	Initializes the core gizmo manager. Must be called right after creation. */
		void initialize(const GizmoManager::CoreInitData& initData);

		/**	Renders all gizmos in the parent camera. */
		void render();

		/**
		 * Renders a non-icon gizmo mesh using the provided parameters.
		 *
		 * @param[in]	viewMatrix	View matrix of the camera we are rendering with.
		 * @param[in]	projMatrix	Projection matrix of the camera we are rendering with.
		 * @param[in]	viewDir		View direction of the camera we are rendering with.
		 * @param[in]	mesh		Mesh to render. This is normally the solid or wireframe gizmo mesh.
		 * @param[in]	texture		Texture to apply to the material, if the material supports a texture.
		 * @param[in]	material	Material to use for rendering. This is normally the solid, wireframe or picking material.
		 */
		void renderGizmos(const Matrix4& viewMatrix, const Matrix4& projMatrix, const Vector3& viewDir, 
			const SPtr<MeshCoreBase>& mesh, const SPtr<TextureCore>& texture, GizmoManager::GizmoMaterial material);

		/**
		 * Renders the icon gizmo mesh using the provided parameters.
		 *
		 * @param[in]	screenArea			Area of the viewport to render the gizmos in, in pixels.
		 * @param[in]	mesh				Mesh containing the icons.
		 * @param[in]	renderData			Icon render data outlining which parts of the icon mesh use which textures.
		 * @param[in]	usePickingMaterial	Should the icons be rendered normally or for picking.
		 */
		void renderIconGizmos(Rect2I screenArea, SPtr<MeshCoreBase> mesh, GizmoManager::IconRenderDataVecPtr renderData, bool usePickingMaterial);

		/**
		 * Updates the internal data that is used for rendering. Normally you would call this after updating the camera or
		 * meshes on the sim thread.
		 *
		 * @param[in]	camera			Sets the camera all rendering will be performed to.
		 * @param[in]	meshes			Meshes to render.
		 * @param[in]	iconMesh		Mesh containing icon meshes.
		 * @param[in]	iconRenderData	Icon render data outlining which parts of the icon mesh use which textures.
		 */
		void updateData(const SPtr<CameraCore>& camera, const Vector<MeshData>& meshes, const SPtr<MeshCoreBase>& iconMesh, 
			const GizmoManager::IconRenderDataVecPtr& iconRenderData);

		static const float PICKING_ALPHA_CUTOFF;

		SPtr<CameraCore> mCamera;

		Vector<MeshData> mMeshes;
		SPtr<MeshCoreBase> mIconMesh;
		GizmoManager::IconRenderDataVecPtr mIconRenderData;

		// Immutable
		SolidMaterialData mSolidMaterial;
		SolidMaterialData mWireMaterial;
		WireMaterialData mLineMaterial;
		IconMaterialData mIconMaterial;
		TextMaterialData mTextMaterial;

		PickingMaterialData mPickingMaterial;
		AlphaPickingMaterialData mAlphaPickingMaterial;
	};

	/** @} */
}