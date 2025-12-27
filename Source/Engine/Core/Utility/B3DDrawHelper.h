//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector2.h"
#include "Image/B3DColor.h"
#include "Math/B3DRect3.h"
#include "RenderAPI/B3DSubMesh.h"
#include "Scene/B3DTransform.h"

namespace b3d
{
	/** @addtogroup Utility-Engine
	 *  @{
	 */

	/**	Helper class for immediate drawing of common geometric shapes. */
	class B3D_EXPORT DrawHelper
	{
	public:
		/** Controls in what order will elements be rendered, depending on some reference point. */
		enum class SortType
		{
			BackToFront,
			FrontToBack,
			None
		};

		/**	Type of meshes that are output by DrawHelper. */
		enum class MeshType
		{
			Solid,
			Wire,
			Line,
			Text
		};

		/**	Container for mesh of a specific type output by the DrawHelper. */
		struct ShapeMeshData
		{
			SPtr<Mesh> Mesh;
			SubMesh SubMesh;
			MeshType Type;
			HTexture Texture;
		};

		DrawHelper();

		/**	Sets a color that will be used for any shapes recorded after this call. */
		void SetColor(const Color& color);

		/**	Sets a transform matrix that will be used for any shapes recorded after this call. */
		void SetTransform(const Matrix4& transform);

		/** Sets the layer bitfield that can be used for filtering which objects are output into the final mesh. */
		void SetLayer(u64 layer);

		/**	Records a solid cuboid with the specified properties in the internal draw queue. */
		void Cube(const Vector3& position, const Vector3& extents);

		/**	Records a solid sphere with the specified properties in the internal draw queue. */
		void Sphere(const Vector3& position, float radius, u32 quality = 1);

		/**	Records a wireframe cube with the specified properties in the internal draw queue. */
		void WireCube(const Vector3& position, const Vector3& extents);

		/**	Records a wireframe sphere with the specified properties in the internal draw queue. */
		void WireSphere(const Vector3& position, float radius, u32 quality = 10);

		/**	Records a wireframe hemisphere with the specified properties in the internal draw queue. */
		void WireHemisphere(const Vector3& position, float radius, u32 quality = 10);

		/**	Records a line with the specified properties in the internal draw queue. */
		void Line(const Vector3& start, const Vector3& end);

		/**
		 * Records a list of lines in the internal draw queue. The list must contain lines as pair of vertices, starting
		 * point followed by an end point, and so on.
		 */
		void LineList(const Vector<Vector3>& lines);

		/**	Records a wireframe frustum with the specified properties in the internal draw queue. */
		void Frustum(const Vector3& position, float aspect, Degree FOV, float near, float far);

		/**	Records a solid cone with the specified properties in the internal draw queue. */
		void Cone(const Vector3& base, const Vector3& normal, float height, float radius, const Vector2& scale = Vector2::kOne, u32 quality = 10);

		/**	Records a wire cone with the specified properties in the internal draw queue. */
		void WireCone(const Vector3& base, const Vector3& normal, float height, float radius, const Vector2& scale = Vector2::kOne, u32 quality = 10);

		/**	Records a solid disc with the specified properties in the internal draw queue. */
		void Disc(const Vector3& position, const Vector3& normal, float radius, u32 quality = 10);

		/**	Records a wireframe disc with the specified properties in the internal draw queue. */
		void WireDisc(const Vector3& position, const Vector3& normal, float radius, u32 quality = 10);

		/**	Records a solid arc with the specified properties in the internal draw queue. */
		void Arc(const Vector3& position, const Vector3& normal, float radius, Degree startAngle, Degree amountAngle, u32 quality = 10);

		/**	Records a wireframe arc with the specified properties in the internal draw queue. */
		void WireArc(const Vector3& position, const Vector3& normal, float radius, Degree startAngle, Degree amountAngle, u32 quality = 10);

		/** Records a 3D mesh to be drawn as wireframe in the internal draw queue. */
		void WireMesh(const SPtr<MeshData>& meshData);

		/**	Records a solid rectangle with the specified properties in the internal draw queue. */
		void Rectangle(const Rect3& area);

		/**
		 * Records a mesh representing 2D text with the specified properties in the internal draw queue.
		 *
		 * @param[in]	position	Position to render the text at. Text will be centered around this point.
		 * @param[in]	text		Text to draw.
		 * @param[in]	font		Font to use for rendering the text's characters.
		 * @param[in]	size		Size of the characters, in points.
		 */
		void Text(const Vector3& position, const String& text, const HFont& font, float size = 10.0f);

		/**	Clears all recorded shapes. */
		void Clear();

		/**
		 * Generates a set of meshes from all the recorded solid and wireframe shapes. The meshes can be accessed via
		 * getMeshes() and released via clearMeshes().
		 *
		 * @param	sorting		(optional) Determines how (and if) should elements be sorted
		 *						based on their distance from the reference point.
		 * @param	camera		(optional) Camera through which the meshes will be rendered.
		 * @param	layers		(optional) Layers bitfield that can be used for controlling which shapes will be included
		 *						in the mesh. This bitfield will be ANDed with the layer specified when recording the shape.
		 * @return				Generated mesh data.
		 */
		Vector<ShapeMeshData> BuildMeshes(SortType sorting = SortType::None, const Camera* camera = nullptr, u64 layers = 0xFFFFFFFFFFFFFFFF);

	private:
		struct CommonData
		{
			Color Color;
			Matrix4 Transform;
			Vector3 Center;
			u64 Layer;
		};

		struct CubeData : CommonData
		{
			Vector3 Position;
			Vector3 Extents;
		};

		struct SphereData : CommonData
		{
			Vector3 Position;
			float Radius;
			u32 Quality;
		};

		struct LineData : CommonData
		{
			Vector3 Start;
			Vector3 End;
		};

		struct LineListData : CommonData
		{
			Vector<Vector3> Lines;
		};

		struct Rect3Data : CommonData
		{
			Rect3 Area;
		};

		struct FrustumData : CommonData
		{
			Vector3 Position;
			float Aspect;
			Degree FOV;
			float NearDist;
			float FarDist;
		};

		struct ConeData : CommonData
		{
			Vector3 Base;
			Vector3 Normal;
			float Height;
			float Radius;
			Vector2 Scale;
			u32 Quality;
		};

		struct DiscData : CommonData
		{
			Vector3 Position;
			Vector3 Normal;
			float Radius;
			u32 Quality;
		};

		struct ArcData : CommonData
		{
			Vector3 Position;
			Vector3 Normal;
			float Radius;
			Degree StartAngle;
			Degree AmountAngle;
			u32 Quality;
		};

		struct Text2DData : CommonData
		{
			Vector3 Position;
			String Text;
			HFont Font;
			float Size;
		};

		struct WireMeshData : CommonData
		{
			SPtr<MeshData> MeshData;
		};

		static const u32 kVertexBufferGrowth;
		static const u32 kIndexBufferGrowth;

		Color mColor;
		Matrix4 mTransform;
		u64 mLayer;

		Vector<CubeData> mSolidCubeData;
		Vector<CubeData> mWireCubeData;
		Vector<SphereData> mSolidSphereData;
		Vector<SphereData> mWireSphereData;
		Vector<SphereData> mWireHemisphereData;
		Vector<LineData> mLineData;
		Vector<LineListData> mLineListData;
		Vector<Rect3Data> mRect3Data;
		Vector<FrustumData> mFrustumData;
		Vector<ConeData> mConeData;
		Vector<ConeData> mWireConeData;
		Vector<DiscData> mDiscData;
		Vector<DiscData> mWireDiscData;
		Vector<ArcData> mArcData;
		Vector<ArcData> mWireArcData;
		Vector<Text2DData> mText2DData;
		Vector<WireMeshData> mWireMeshData;

		SPtr<VertexDescription> mSolidVertexDesc;
		SPtr<VertexDescription> mWireVertexDesc;
		SPtr<VertexDescription> mLineVertexDesc;
		SPtr<VertexDescription> mTextVertexDesc;
	};

	/** @} */
} // namespace b3d
