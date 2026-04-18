//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "Utility/B3DModule.h"
#include "Utility/B3DDrawHelper.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "Renderer/B3DRendererExtension.h"
#include "Renderer/B3DRendererMaterial.h"

namespace b3d
{
	namespace render
	{
		class DebugDrawRenderer;
	}

	/** @addtogroup Debug-Engine
	 *  @{
	 */

	/**	Supported types of materials (shaders) by DebugDraw. */
	enum class DebugDrawMaterialType
	{
		Solid,
		Wire,
		Line
	};

	/** Provides an easy access to draw basic 2D and 3D shapes, primarily meant for debugging purposes. */
	class B3D_EXPORT DebugDraw : public Module<DebugDraw>
	{
	public:
		DebugDraw();
		~DebugDraw();

		/**	Changes the color of any further draw calls. */
		void SetColor(const Color& color);

		/**	Changes the transform that will be applied to meshes of any further draw calls.  */
		void SetTransform(const Matrix4& transform);

		/**
		 * Draws an axis aligned cuboid.
		 *
		 * @param	position	Center of the cuboid.
		 * @param	extents		Radius of the cuboid in each axis.
		 */
		void DrawCube(const Vector3& position, const Vector3& extents);

		/** Draws a sphere. */
		void DrawSphere(const Vector3& position, float radius);

		/**
		 * Draws a solid cone.
		 *
		 * @param	base		Position of the center of the base of the cone.
		 * @param	normal		Orientation of the cone, pointing from center base to the tip of the cone.
		 * @param	height		Height of the cone (along the normal).
		 * @param	radius		Radius of the base of the cone.
		 * @param	scale		Scale applied to cone's disc width & height. Allows you to create elliptical cones.
		 */
		void DrawCone(const Vector3& base, const Vector3& normal, float height, float radius, const Vector2& scale = Vector2::kOne);

		/**
		 * Draws a solid disc.
		 *
		 * @param	position	Center of the disc.
		 * @param	normal		Orientation of the disc, pointing in the direction the disc is visible in.
		 * @param	radius		Radius of the disc.
		 */
		void DrawDisc(const Vector3& position, const Vector3& normal, float radius);

		/**
		 * Draws a wireframe axis aligned cuboid.
		 *
		 * @param	position	Center of the cuboid.
		 * @param	extents		Radius of the cuboid in each axis.
		 */
		void DrawWireCube(const Vector3& position, const Vector3& extents);

		/** Draws a wireframe sphere represented by three discs. */
		void DrawWireSphere(const Vector3& position, float radius);

		/**
		 * Draws a wireframe cone.
		 *
		 * @param	base		Position of the center of the base of the cone.
		 * @param	normal		Orientation of the cone, pointing from center base to the tip of the cone.
		 * @param	height		Height of the cone (along the normal).
		 * @param	radius		Radius of the base of the cone.
		 * @param	scale		Scale applied to cone's disc width & height. Allows you to create elliptical cones.
		 */
		void DrawWireCone(const Vector3& base, const Vector3& normal, float height, float radius, const Vector2& scale = Vector2::kOne);

		/** Draws a line between two points. */
		void DrawLine(const Vector3& start, const Vector3& end);

		/**
		 * Draws a list of lines. Provided array must contain pairs of the line start point followed by an end point.
		 */
		void DrawLineList(const Vector<Vector3>& linePoints);

		/**
		 * Draws a wireframe disc.
		 *
		 * @param	position	Center of the disc.
		 * @param	normal		Orientation of the disc, pointing in the direction the disc is visible in.
		 * @param	radius		Radius of the disc.
		 */
		void DrawWireDisc(const Vector3& position, const Vector3& normal, float radius);

		/**
		 * Draws a wireframe arc.
		 *
		 * @param	position	Center of the arc.
		 * @param	normal		Orientation of the arc, pointing in the direction the arc is visible in.
		 * @param	radius		Radius of the arc.
		 * @param	startAngle	Angle at which to start the arc.
		 * @param	amountAngle	Length of the arc.
		 */
		void DrawWireArc(const Vector3& position, const Vector3& normal, float radius, Degree startAngle, Degree amountAngle);

		/**
		 * Draws a wireframe mesh.
		 *
		 * @param	meshData	Object containing mesh vertices and indices. Vertices must be Vertex3 and indices
		 *							32-bit.
		 */
		void DrawWireMesh(const SPtr<MeshData>& meshData);

		/**
		 * Draws a wireframe frustum.
		 *
		 * @param	position	Origin of the frustum, or the eye point.
		 * @param	aspect		Ratio of frustum width over frustum height.
		 * @param	FOV			Horizontal field of view in degrees.
		 * @param	near		Distance to the near frustum plane.
		 * @param	far			Distance to the far frustum plane.
		 */
		void DrawFrustum(const Vector3& position, float aspect, Degree FOV, float near, float far);

		/**
		 * Clears any objects that are currently drawing. All objects must be re-queued.
		 */
		void Clear();

		/** Performs per-frame operations. */
		void UpdateInternal();

	private:
		friend class render::DebugDrawRenderer;

		/** Data about a mesh rendered by the draw manager. */
		struct MeshRenderData
		{
			MeshRenderData(const SPtr<render::Mesh>& mesh, const SubMesh& subMesh, DebugDrawMaterialType type)
				: Mesh(mesh), SubMesh(subMesh), Type(type)
			{}

			SPtr<render::Mesh> Mesh;
			SubMesh SubMesh;
			DebugDrawMaterialType Type;
		};

		/** Converts mesh data from DrawHelper into mesh data usable by the debug draw renderer. */
		Vector<MeshRenderData> CreateMeshProxyData(const Vector<DrawHelper::ShapeMeshData>& meshData);

		DrawHelper* mDrawHelper = nullptr;
		Vector<DrawHelper::ShapeMeshData> mActiveMeshes;

		SPtr<render::DebugDrawRenderer> mRenderer;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup Debug-Internal
		 *  @{
		 */

		B3D_UNIFORM_BUFFER_BEGIN(DebugDrawUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatViewProj)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4, gViewDir)
		B3D_UNIFORM_BUFFER_END

		extern DebugDrawUniformDefinition gDebugDrawUniformDefinition;

		/** Handles rendering of debug shapes. */
		class DebugDrawMaterial : public RendererMaterial<DebugDrawMaterial>
		{
			RMAT_DEF("DebugDraw.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool solid, bool line, bool wire>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("SOLID", solid),
					  ShaderVariationParameter("LINE", line),
					  ShaderVariationParameter("WIRE", wire) });

				return variation;
			}

		public:
			DebugDrawMaterial() = default;

			/** Executes the material using the provided parameters. */
			void Execute(GpuCommandBuffer& commandBuffer, const GpuBufferSuballocation& uniformBuffer, const SPtr<Mesh>& mesh, const SubMesh& subMesh);

			/** Returns the material variation matching the provided parameters. */
			static DebugDrawMaterial* GetVariation(DebugDrawMaterialType drawMaterial);
		};

		/** Performs rendering of meshes provided by DebugDraw. */
		class DebugDrawRenderer : public RendererExtension
		{
			friend class b3d::DebugDraw;

		public:
			DebugDrawRenderer();

		private:
			void Initialize(const Any& data) override;
			RendererExtensionRequest Check(const render::Camera& camera) override;
			void Render(const Camera& camera, const RendererViewContext& viewContext) override;

			/**
			 * Updates the internal data that is used for rendering. Normally you would call this after updating the meshes
			 * on the main thread.
			 *
			 * @param	meshes			Meshes to render.
			 */
			void UpdateData(const Vector<DebugDraw::MeshRenderData>& meshes);

			Vector<DebugDraw::MeshRenderData> mMeshes;
		};

		/** @} */
	} // namespace render
} // namespace b3d
