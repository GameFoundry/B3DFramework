//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "Renderer/BsRendererExtension.h"
#include "RenderAPI/BsGpuParams.h"
#include "Utility/BsDrawHelper.h"
#include "Renderer/BsParamBlocks.h"

namespace bs
{
	/** @addtogroup Handles
	 *  @{
	 */

	namespace ct { class HandleRenderer; }

	/**
	 * Allows you to easily draw various kinds of simple shapes, primarily used for drawing handles in the scene view.
	 *
	 * Drawn elements only persist for a single draw() call and need to be re-queued after.
	 */
	class BS_ED_EXPORT HandleDrawManager
	{
	public:
		HandleDrawManager();
		~HandleDrawManager();

		/**	Sets the color of all the following draw* calls. */
		void setColor(const Color& color);

		/** Sets the transform matrix that will be applied to all following draw* calls. */
		void setTransform(const Matrix4& transform);

		/**
		 * Sets the layer bitfield that controls whether a handle is considered visible in a specific camera. Handle layer 
		 * must match camera layer in order for the camera to render it
		 */
		void setLayer(UINT64 layer);

		/**
		 * Draws a solid cuboid.
		 *
		 * @param[in]	position	Center of the cuboid.
		 * @param[in]	extents		Radius of the cuboid in all directions.
		 * @param[in]	size		Uniform scale of the object.
		 */
		void drawCube(const Vector3& position, const Vector3& extents, float size = 1.0f);

		/**
		 * Draws a solid sphere.
		 *
		 * @param[in]	position	Center of the sphere.
		 * @param[in]	radius		Radius of the sphere.
		 * @param[in]	size		Uniform scale of the object.
		 */
		void drawSphere(const Vector3& position, float radius, float size = 1.0f);

		/**
		 * Draws a wireframe cuboid.
		 *
		 * @param[in]	position	Center of the cuboid.
		 * @param[in]	extents		Radius of the cuboid in all directions.
		 * @param[in]	size		Uniform scale of the object.
		 */
		void drawWireCube(const Vector3& position, const Vector3& extents, float size = 1.0f);

		/**
		 * Draws a wireframe sphere.
		 *
		 * @param[in]	position	Center of the sphere.
		 * @param[in]	radius		Radius of the sphere.
		 * @param[in]	size		Uniform scale of the object.
		 */
		void drawWireSphere(const Vector3& position, float radius, float size = 1.0f);

		/**
		 * Draws a solid cone.
		 *
		 * @param[in]	base		Position of the center of the base of the cone.
		 * @param[in]	normal		Orientation of the cone, pointing from center base to the tip of the cone.
		 * @param[in]	height		Height of the cone (along the normal).
		 * @param[in]	radius		Radius of the base of the cone.
		 * @param[in]	size		Uniform scale of the object.
		 */
		void drawCone(const Vector3& base, const Vector3& normal, float height, float radius, float size = 1.0f);

		/**
		 * Draws a line.
		 *
		 * @param[in]	start		Starting point for the line.
		 * @param[in]	end			Ending point for the line.
		 * @param[in]	size		Uniform scale of the object.
		 */
		void drawLine(const Vector3& start, const Vector3& end, float size = 1.0f);

		/**
		 * Draws a double-sided solid disc.
		 *
		 * @param[in]	position	Center of the disc.
		 * @param[in]	normal		Orientation of the disc, pointing in the direction the disc is visible in.
		 * @param[in]	radius		Radius of the disc.
		 * @param[in]	size		Uniform scale of the object.
		 */
		void drawDisc(const Vector3& position, const Vector3& normal, float radius, float size = 1.0f);

		/**
		 * Draws a wireframe disc.
		 *
		 * @param[in]	position	Center of the disc.
		 * @param[in]	normal		Orientation of the disc, pointing in the direction the disc is visible in.
		 * @param[in]	radius		Radius of the disc.
		 * @param[in]	size		Uniform scale of the object.
		 */
		void drawWireDisc(const Vector3& position, const Vector3& normal, float radius, float size = 1.0f);

		/**
		 * Draws a double-sided solid arc.
		 *
		 * @param[in]	position	Center of the arc.
		 * @param[in]	normal		Orientation of the arc, pointing in the direction the arc is visible in.
		 * @param[in]	radius		Radius of the arc.
		 * @param[in]	startAngle	Angle at which to start the arc.
		 * @param[in]	amountAngle	Length of the arc.
		 * @param[in]	size		Uniform scale of the object.
		 */
		void drawArc(const Vector3& position, const Vector3& normal, float radius, Degree startAngle, Degree amountAngle, float size = 1.0f);

		/**
		 * Draws a wireframe arc.
		 *
		 * @param[in]	position	Center of the arc.
		 * @param[in]	normal		Orientation of the arc, pointing in the direction the arc is visible in.
		 * @param[in]	radius		Radius of the arc.
		 * @param[in]	startAngle	Angle at which to start the arc.
		 * @param[in]	amountAngle	Length of the arc.
		 * @param[in]	size		Uniform scale of the object.
		 */
		void drawWireArc(const Vector3& position, const Vector3& normal, float radius, Degree startAngle, Degree amountAngle, float size = 1.0f);

		/**
		 * Draws a double-sided solid rectangle.
		 *
		 * @param[in]	area		Position and size of the rectangle.
		 * @param[in]	size		Uniform scale of the object.
		 */
		void drawRect(const Rect3& area, float size = 1.0f);

		/**
		 * Draws a mesh representing 2D text with the specified properties. 
		 *
		 * @param[in]	position	Position to render the text at. Text will be centered around this point.
		 * @param[in]	text		Text to draw.
		 * @param[in]	font		Font to use for rendering the text's characters.
		 * @param[in]	fontSize	Size of the characters, in points.
		 * @param[in]	size		Uniform scale of the rendered mesh.
		 */
		void drawText(const Vector3& position, const WString& text, const HFont& font, UINT32 fontSize = 16, float size = 1.0f);

		/**	Queues all the handle draw commands queued since the last call to clear() for rendering. */
		void draw(const SPtr<Camera>& camera);

		/** Clears all handle draw commands. */
		void clear();

	private:
		friend class ct::HandleRenderer;

		/** Destroys all meshes allocated since the last call to this method. */
		void clearMeshes();

		static const UINT32 SPHERE_QUALITY;
		static const UINT32 WIRE_SPHERE_QUALITY;
		static const UINT32 ARC_QUALITY;

		Vector<Vector<DrawHelper::ShapeMeshData>> mActiveMeshes;
		UINT64 mLastFrameIdx;

		Matrix4 mTransform;
		SPtr<ct::HandleRenderer> mRenderer;
		DrawHelper* mDrawHelper;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup Handles-Internal
	 *  @{
	 */

	BS_PARAM_BLOCK_BEGIN(HandleParamBlockDef)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatViewProj)
		BS_PARAM_BLOCK_ENTRY(Vector4, gViewDir)
	BS_PARAM_BLOCK_END

	extern HandleParamBlockDef gHandleParamBlockDef;

	/** Performs rendering of handles on the core thread, managed by the parent HandleDrawManager. */
	class BS_ED_EXPORT HandleRenderer : public RendererExtension
	{
		/** Type of mesh that can be drawn. */
		enum class MeshType
		{
			Solid, Line, Text, Count
		};

		/** Data about a mesh rendered by the draw manager. */
		struct MeshData
		{
			MeshData(const SPtr<Mesh>& mesh, const SubMesh& subMesh, SPtr<Texture> texture, MeshType type)
				:mesh(mesh), subMesh(subMesh), texture(texture), type(type)
			{ }

			SPtr<Mesh> mesh;
			SubMesh subMesh;
			SPtr<Texture> texture;
			UINT32 paramIdx;
			MeshType type;
		};

		/** Data about a camera and the meshes that are queued for rendering on it */
		struct QueuedData
		{
			SPtr<Camera> camera;
			Vector<MeshData> meshes;
		};

		/** Data used for initializing the renderer. */
		struct InitData
		{
			SPtr<Material> lineMat;
			SPtr<Material> solidMat;
			SPtr<Material> textMat;
			SPtr<Material> clearMat;
		};

	public:
		HandleRenderer();

	private:
		friend class bs::HandleDrawManager;

		/** @copydoc RendererExtension::initialize */
		void initialize(const Any& data) override;

		/** @copydoc RendererExtension::destroy */
		void destroy() override;

		/** @copydoc RendererExtension::check */
		bool check(const Camera& camera) override;

		/** @copydoc RendererExtension::render */
		void render(const Camera& camera) override;

		/**
		 * Queues new data for rendering.
		 *
		 * @param[in]	camera	Camera to render to.
		 * @param[in]	meshes	Meshes to render.
		 */
		void queueForDraw(const SPtr<Camera>& camera, Vector<MeshData>& meshes);

		/** Deletes any meshes queued for rendering. */
		void clearQueued();

		Vector<QueuedData> mQueuedData;

		SPtr<GpuParamBlockBuffer> mParamBuffer;
		Vector<SPtr<GpuParamsSet>> mParamSets[(UINT32)MeshType::Count];
		UINT32 mTypeCounters[(UINT32)MeshType::Count];

		// Immutable
		SPtr<Material> mMaterials[(UINT32)MeshType::Count];
		SPtr<Material> mClearMaterial;
	};

	/** @} */
	}
}