//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Utility/BsModule.h"
#include "Utility/BsDrawHelper.h"
#include "Renderer/BsParamBlocks.h"
#include "Renderer/BsRendererExtension.h"
#include "Renderer/BsRendererMaterial.h"

namespace bs
{
	namespace ct { class DebugDrawRenderer; }

	/** @addtogroup Utility-Engine
	 *  @{
	 */

	/**	Supported types of materials (shaders) by DebugDraw. */
	enum class DebugDrawMaterial
	{
		Solid, Wire, Line
	};

	/** Provides an easy access to draw basic 2D and 3D shapes, primarily meant for debugging purposes. */
	class BS_EXPORT DebugDraw : public Module<DebugDraw>
	{
	public:
		DebugDraw();
		~DebugDraw();

		/**	Changes the color of any further draw calls. */
		void setColor(const Color& color);

		/**	Changes the transform that will be applied to meshes of any further draw calls.  */
		void setTransform(const Matrix4& transform);

		/**
		 * Draws an axis aligned cuboid.
		 *
		 * @param[in]	position	Center of the cuboid.
		 * @param[in]	extents		Radius of the cuboid in each axis.
		 */
		void drawCube(const Vector3& position, const Vector3& extents);

		/** Draws a sphere. */
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
		 */
		void drawWireCube(const Vector3& position, const Vector3& extents);

		/** Draws a wireframe sphere represented by three discs. */
		void drawWireSphere(const Vector3& position, float radius);

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

		/** Draws a line between two points. */
		void drawLine(const Vector3& start, const Vector3& end);

		/**
		 * Draws a list of lines. Provided array must contain pairs of the line start point followed by an end point.
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
		 */
		void drawFrustum(const Vector3& position, float aspect, Degree FOV, float near, float far);

		/**
		 * Clears any objects that are currently drawing. All objects must be re-queued.
		 */
		void clear();

		/** Performs per-frame operations. */
		void _update();

	private:
		friend class ct::DebugDrawRenderer;

		/** Data about a mesh rendered by the draw manager. */
		struct MeshRenderData
		{
			MeshRenderData(const SPtr<ct::Mesh>& mesh, const SubMesh& subMesh, DebugDrawMaterial type)
				:mesh(mesh), subMesh(subMesh), type(type)
			{ }

			SPtr<ct::Mesh> mesh;
			SubMesh subMesh;
			DebugDrawMaterial type;
		};

		/** Converts mesh data from DrawHelper into mesh data usable by the debug draw renderer. */
		Vector<MeshRenderData> createMeshProxyData(const Vector<DrawHelper::ShapeMeshData>& meshData);

		DrawHelper* mDrawHelper = nullptr;
		Vector<DrawHelper::ShapeMeshData> mActiveMeshes;

		SPtr<ct::DebugDrawRenderer> mRenderer;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup Utility-Engine-Internal
	 *  @{
	 */

	BS_PARAM_BLOCK_BEGIN(DebugDrawParamsDef)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatViewProj)
		BS_PARAM_BLOCK_ENTRY(Vector4, gViewDir)
	BS_PARAM_BLOCK_END

	extern DebugDrawParamsDef gDebugDrawParamsDef;

	/** Handles rendering of debug shapes. */
	class DebugDrawMat : public RendererMaterial<DebugDrawMat>
	{
		RMAT_DEF("DebugDraw.bsl");

		/** Helper method used for initializing variations of this material. */
		template<bool solid, bool line, bool wire>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("SOLID", solid),
				ShaderVariation::Param("LINE", line),
				ShaderVariation::Param("WIRE", wire)
			});

			return variation;
		}
	public:
		DebugDrawMat();

		/** Executes the material using the provided parameters. */
		void execute(const SPtr<GpuParamBlockBuffer>& params, const SPtr<Mesh>& mesh, const SubMesh& subMesh);

		/** Returns the material variation matching the provided parameters. */
		static DebugDrawMat* getVariation(DebugDrawMaterial drawMat);
	};

	/** Performs rendering of meshes provided by DebugDraw. */
	class DebugDrawRenderer : public RendererExtension
	{
		friend class bs::DebugDraw;

	public:
		DebugDrawRenderer();

	private:
		/**	@copydoc RendererExtension::initialize */
		void initialize(const Any& data) override;

		/**	@copydoc RendererExtension::check */
		RendererExtensionRequest check(const ct::Camera& camera) override;

		/**	@copydoc RendererExtension::render */
		void render(const Camera& camera, const RendererViewContext& viewContext) override;

		/**
		 * Updates the internal data that is used for rendering. Normally you would call this after updating the meshes
		 * on the sim thread.
		 *
		 * @param[in]	meshes			Meshes to render.
		 */
		void updateData(const Vector<DebugDraw::MeshRenderData>& meshes);

		Vector<DebugDraw::MeshRenderData> mMeshes;
		SPtr<GpuParamBlockBuffer> mParamBuffer;
	};

	/** @} */
	}
}
