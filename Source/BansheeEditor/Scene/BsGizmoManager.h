//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "Utility/BsModule.h"
#include "Image/BsColor.h"
#include "Math/BsVector2I.h"
#include "Math/BsMatrix4.h"
#include "RenderAPI/BsGpuParam.h"
#include "Utility/BsDrawHelper.h"
#include "Renderer/BsParamBlocks.h"
#include "Renderer/BsRendererExtension.h"

namespace bs
{
	/** @addtogroup Scene-Editor
	 *  @{
	 */

	namespace ct { class GizmoRenderer; }

	/** Type of mesh that can be drawn by the gizmo renderer. */
	enum class GizmoMeshType
	{
		Solid, Line, Wire, Text, Count
	};

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
		 * Draws a solid disc.
		 *
		 * @param[in]	position	Center of the disc.
		 * @param[in]	normal		Orientation of the disc, pointing in the direction the disc is visible in.
		 * @param[in]	radius		Radius of the disc.
		 */
		void drawDisc(const Vector3& position, const Vector3& normal, float radius);

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
		friend class ct::GizmoRenderer;

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

		/**	Data required for rendering a disc gizmo. */
		struct DiscData : CommonData
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
			SPtr<ct::Texture> texture;
			UINT32 paramsIdx;
		};

		/** Data about a mesh rendered by the draw manager. */
		struct MeshRenderData
		{
			MeshRenderData(const SPtr<ct::Mesh>& mesh, const SubMesh& subMesh, SPtr<ct::Texture> texture, GizmoMeshType type)
				:mesh(mesh), subMesh(subMesh), texture(texture), type(type), paramsIdx(0)
			{ }

			SPtr<ct::Mesh> mesh;
			SubMesh subMesh;
			SPtr<ct::Texture> texture;
			GizmoMeshType type;

			UINT32 paramsIdx;
		};

		/**	Data used for initializing the core thread equivalent of the gizmo manager. */
		struct CoreInitData
		{
			SPtr<ct::Material> solidMat;
			SPtr<ct::Material> wireMat;
			SPtr<ct::Material> lineMat;
			SPtr<ct::Material> iconMat;
			SPtr<ct::Material> textMat;
			SPtr<ct::Material> pickingMat;
			SPtr<ct::Material> alphaPickingMat;
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
		 * @return					A mesh containing all of the visible icons.	
		 */
		SPtr<Mesh> buildIconMesh(const SPtr<Camera>& camera, const Vector<IconData>& iconData, bool forPicking, 
			IconRenderDataVecPtr& renderData);

		/**	Resizes the icon width/height so it is always scaled to optimal size (with preserved aspect). */
		void limitIconSize(UINT32& width, UINT32& height);

		/** Converts mesh data from DrawHelper into mesh data usable by the gizmo renderer. */
		Vector<MeshRenderData> createMeshProxyData(const Vector<DrawHelper::ShapeMeshData>& meshData);

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
		Vector<DiscData> mSolidDiscData;
		Vector<DiscData> mWireDiscData;
		Vector<WireArcData> mWireArcData;
		Vector<WireMeshData> mWireMeshData;
		Vector<FrustumData> mFrustumData;
		Vector<IconData> mIconData;
		Vector<TextData> mTextData;
		Map<UINT32, HSceneObject> mIdxToSceneObjectMap;

		Vector<DrawHelper::ShapeMeshData> mActiveMeshes;

		SPtr<Mesh> mIconMesh;

		SPtr<ct::GizmoRenderer> mGizmoRenderer;

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

	namespace ct
	{
	/** @addtogroup Scene-Editor-Internal
	 *  @{
	 */

	BS_PARAM_BLOCK_BEGIN(GizmoParamBlockDef)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatViewProj)
		BS_PARAM_BLOCK_ENTRY(Vector4, gViewDir)
	BS_PARAM_BLOCK_END

	extern GizmoParamBlockDef gGizmoParamBlockDef;

	BS_PARAM_BLOCK_BEGIN(GizmoPickingParamBlockDef)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatViewProj)
		BS_PARAM_BLOCK_ENTRY(float, gAlphaCutoff)
	BS_PARAM_BLOCK_END

	extern GizmoPickingParamBlockDef gGizmoPickingParamBlockDef;

	/** Performs rendering of gizmos on the core thread, as managed by the GizmoManager. */
	class GizmoRenderer : public RendererExtension
	{
		friend class bs::GizmoManager;

	public:
		GizmoRenderer();

	private:
		/**	@copydoc RendererExtension::initialize */
		void initialize(const Any& data) override;

		/**	@copydoc RendererExtension::check */
		bool check(const Camera& camera) override;

		/**	@copydoc RendererExtension::render */
		void render(const Camera& camera) override;

		/**	
		 * Renders all provided meshes using the provided camera. 
		 *
		 * @param[in]	camera				Sets the camera all rendering will be performed to.
		 * @param[in]	meshes				Meshes to render.
		 * @param[in]	iconMesh			Mesh containing icon meshes.
		 * @param[in]	iconRenderData		Icon render data outlining which parts of the icon mesh use which textures.
		 * @param[in]	usePickingMaterial	If true, meshes will be rendered using a special picking materials, otherwise
		 *									they'll be rendered using normal drawing materials.
		 */
		void renderData(const SPtr<Camera>& camera, Vector<GizmoManager::MeshRenderData>& meshes, 
			const SPtr<MeshBase>& iconMesh, const GizmoManager::IconRenderDataVecPtr& iconRenderData, 
			bool usePickingMaterial);

		/**
		 * Renders the icon gizmo mesh using the provided parameters.
		 *
		 * @param[in]	screenArea			Area of the viewport to render the gizmos in, in pixels.
		 * @param[in]	mesh				Mesh containing the icons.
		 * @param[in]	renderData			Icon render data outlining which parts of the icon mesh use which textures.
		 * @param[in]	usePickingMaterial	Should the icons be rendered normally or for picking.
		 */
		void renderIconGizmos(Rect2I screenArea, SPtr<MeshBase> mesh, GizmoManager::IconRenderDataVecPtr renderData, 
			bool usePickingMaterial);

		/**
		 * Updates the internal data that is used for rendering. Normally you would call this after updating the camera or
		 * meshes on the sim thread.
		 *
		 * @param[in]	camera			Sets the camera all rendering will be performed to.
		 * @param[in]	meshes			Meshes to render.
		 * @param[in]	iconMesh		Mesh containing icon meshes.
		 * @param[in]	iconRenderData	Icon render data outlining which parts of the icon mesh use which textures.
		 */
		void updateData(const SPtr<Camera>& camera, const Vector<GizmoManager::MeshRenderData>& meshes, 
			const SPtr<MeshBase>& iconMesh,  const GizmoManager::IconRenderDataVecPtr& iconRenderData);

		static const float PICKING_ALPHA_CUTOFF;

		SPtr<Camera> mCamera;

		Vector<GizmoManager::MeshRenderData> mMeshes;
		SPtr<MeshBase> mIconMesh;
		GizmoManager::IconRenderDataVecPtr mIconRenderData;

		Vector<SPtr<GpuParamsSet>> mMeshParamSets[(UINT32)GizmoMeshType::Count];
		Vector<SPtr<GpuParamsSet>> mIconParamSets;
		Vector<SPtr<GpuParamsSet>> mPickingParamSets[2];

		SPtr<GpuParamBlockBuffer> mMeshGizmoBuffer;
		SPtr<GpuParamBlockBuffer> mIconGizmoBuffer;
		SPtr<GpuParamBlockBuffer> mMeshPickingParamBuffer;
		SPtr<GpuParamBlockBuffer> mIconPickingParamBuffer;

		// Immutable
		SPtr<Material> mMeshMaterials[(UINT32)GizmoMeshType::Count];
		SPtr<Material> mIconMaterial;
		SPtr<Material> mPickingMaterials[2];
	};

	/** @} */
	}
}