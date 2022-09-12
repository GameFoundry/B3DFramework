//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsDrawHelper.h"
#include "Mesh/BsMesh.h"
#include "Math/BsAABox.h"
#include "Math/BsSphere.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Utility/BsShapeMeshes3D.h"
#include "Text/BsTextData.h"
#include "Math/BsVector2.h"
#include "Math/BsQuaternion.h"
#include "String/BsUnicode.h"
#include "Renderer/BsCamera.h"

namespace bs
{
	const UINT32 DrawHelper::VERTEX_BUFFER_GROWTH = 4096;
	const UINT32 DrawHelper::INDEX_BUFFER_GROWTH = 4096 * 2;

	DrawHelper::DrawHelper()
		:mLayer(1)
	{
		mTransform = Matrix4::IDENTITY;

		mSolidVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		mSolidVertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);
		mSolidVertexDesc->AddVertElem(VET_FLOAT3, VES_NORMAL);
		mSolidVertexDesc->AddVertElem(VET_COLOR, VES_COLOR);

		mWireVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		mWireVertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);
		mWireVertexDesc->AddVertElem(VET_COLOR, VES_COLOR);

		mLineVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		mLineVertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);
		mLineVertexDesc->AddVertElem(VET_COLOR, VES_COLOR);

		mTextVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		mTextVertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);
		mTextVertexDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD);
		mTextVertexDesc->AddVertElem(VET_COLOR, VES_COLOR);
	}

	void DrawHelper::SetColor(const Color& color)
	{
		mColor = color;
	}

	void DrawHelper::SetTransform(const Matrix4& transform)
	{
		mTransform = transform;
	}

	void DrawHelper::SetLayer(UINT64 layer)
	{
		mLayer = layer;
	}

	void DrawHelper::Cube(const Vector3& position, const Vector3& extents)
	{
		mSolidCubeData.push_back(CubeData());
		CubeData& cubeData = mSolidCubeData.Back();

		cubeData.position = position;
		cubeData.extents = extents;
		cubeData.color = mColor;
		cubeData.transform = mTransform;
		cubeData.layer = mLayer;
		cubeData.center = mTransform.MultiplyAffine(position);
	}

	void DrawHelper::Sphere(const Vector3& position, float radius, UINT32 quality)
	{
		mSolidSphereData.push_back(SphereData());
		SphereData& sphereData = mSolidSphereData.Back();

		sphereData.position = position;
		sphereData.radius = radius;
		sphereData.quality = quality;
		sphereData.color = mColor;
		sphereData.transform = mTransform;
		sphereData.layer = mLayer;
		sphereData.center = mTransform.MultiplyAffine(position);
	}

	void DrawHelper::WireCube(const Vector3& position, const Vector3& extents)
	{
		mWireCubeData.push_back(CubeData());
		CubeData& cubeData = mWireCubeData.Back();

		cubeData.position = position;
		cubeData.extents = extents;
		cubeData.color = mColor;
		cubeData.transform = mTransform;
		cubeData.layer = mLayer;
		cubeData.center = mTransform.MultiplyAffine(position);
	}

	void DrawHelper::WireSphere(const Vector3& position, float radius, UINT32 quality)
	{
		mWireSphereData.push_back(SphereData());
		SphereData& sphereData = mWireSphereData.Back();

		sphereData.position = position;
		sphereData.radius = radius;
		sphereData.quality = quality;
		sphereData.color = mColor;
		sphereData.transform = mTransform;
		sphereData.layer = mLayer;
		sphereData.center = mTransform.MultiplyAffine(position);
	}

	void DrawHelper::WireHemisphere(const Vector3& position, float radius, UINT32 quality)
	{
		mWireHemisphereData.push_back(SphereData());
		SphereData& sphereData = mWireHemisphereData.Back();

		sphereData.position = position;
		sphereData.radius = radius;
		sphereData.quality = quality;
		sphereData.color = mColor;
		sphereData.transform = mTransform;
		sphereData.layer = mLayer;
		sphereData.center = mTransform.MultiplyAffine(position);
	}

	void DrawHelper::Line(const Vector3& start, const Vector3& end)
	{
		mLineData.push_back(LineData());
		LineData& lineData = mLineData.Back();

		lineData.start = start;
		lineData.end = end;
		lineData.color = mColor;
		lineData.transform = mTransform;
		lineData.layer = mLayer;
		lineData.center = mTransform.MultiplyAffine((start + end) * 0.5f);
	}

	void DrawHelper::LineList(const Vector<Vector3>& lines)
	{
		if (lines.Size() < 2)
			return;

		mLineListData.push_back(LineListData());
		LineListData& lineListData = mLineListData.Back();

		Vector3 Center(BsZero);
		for (auto& point : lines)
			center += point;

		lineListData.lines = lines;
		lineListData.color = mColor;
		lineListData.transform = mTransform;
		lineListData.layer = mLayer;
		lineListData.center = center / (float)lines.Size();;
	}

	void DrawHelper::Frustum(const Vector3& position, float aspect, Degree FOV, float near, float far)
	{
		mFrustumData.push_back(FrustumData());
		FrustumData& frustumData = mFrustumData.Back();

		frustumData.position = position;
		frustumData.aspect = aspect;
		frustumData.FOV = FOV;
		frustumData.nearDist = near;
		frustumData.farDist = far;
		frustumData.color = mColor;
		frustumData.transform = mTransform;
		frustumData.layer = mLayer;
		frustumData.center = mTransform.MultiplyAffine(position);
	}

	void DrawHelper::cone(const Vector3& base, const Vector3& normal, float height, float radius, const Vector2& scale,
		UINT32 quality)
	{
		mConeData.push_back(ConeData());
		ConeData& coneData = mConeData.Back();

		coneData.base = base;
		coneData.normal = normal;
		coneData.height = height;
		coneData.radius = radius;
		coneData.scale = scale;
		coneData.quality = quality;
		coneData.color = mColor;
		coneData.transform = mTransform;
		coneData.layer = mLayer;
		coneData.center = mTransform.MultiplyAffine(base + normal * height * 0.5f);
	}

	void DrawHelper::wireCone(const Vector3& base, const Vector3& normal, float height, float radius, const Vector2& scale,
		UINT32 quality)
	{
		mWireConeData.push_back(ConeData());
		ConeData& coneData = mWireConeData.Back();

		coneData.base = base;
		coneData.normal = normal;
		coneData.height = height;
		coneData.radius = radius;
		coneData.scale = scale;
		coneData.quality = quality;
		coneData.color = mColor;
		coneData.transform = mTransform;
		coneData.layer = mLayer;
		coneData.center = mTransform.MultiplyAffine(base + normal * height * 0.5f);
	}

	void DrawHelper::Disc(const Vector3& position, const Vector3& normal, float radius, UINT32 quality)
	{
		mDiscData.push_back(DiscData());
		DiscData& discData = mDiscData.Back();

		discData.position = position;
		discData.normal = normal;
		discData.radius = radius;
		discData.quality = quality;
		discData.color = mColor;
		discData.transform = mTransform;
		discData.layer = mLayer;
		discData.center = mTransform.MultiplyAffine(position);
	}

	void DrawHelper::WireDisc(const Vector3& position, const Vector3& normal, float radius, UINT32 quality)
	{
		mWireDiscData.push_back(DiscData());
		DiscData& discData = mWireDiscData.Back();

		discData.position = position;
		discData.normal = normal;
		discData.radius = radius;
		discData.quality = quality;
		discData.color = mColor;
		discData.transform = mTransform;
		discData.layer = mLayer;
		discData.center = mTransform.MultiplyAffine(position);
	}

	void DrawHelper::arc(const Vector3& position, const Vector3& normal, float radius,
		Degree startAngle, Degree amountAngle, UINT32 quality)
	{
		mArcData.push_back(ArcData());
		ArcData& arcData = mArcData.Back();

		arcData.position = position;
		arcData.normal = normal;
		arcData.radius = radius;
		arcData.startAngle = startAngle;
		arcData.amountAngle = amountAngle;
		arcData.quality = quality;
		arcData.color = mColor;
		arcData.transform = mTransform;
		arcData.layer = mLayer;
		arcData.center = mTransform.MultiplyAffine(position);
	}

	void DrawHelper::wireArc(const Vector3& position, const Vector3& normal, float radius,
		Degree startAngle, Degree amountAngle, UINT32 quality)
	{
		mWireArcData.push_back(ArcData());
		ArcData& arcData = mWireArcData.Back();

		arcData.position = position;
		arcData.normal = normal;
		arcData.radius = radius;
		arcData.startAngle = startAngle;
		arcData.amountAngle = amountAngle;
		arcData.quality = quality;
		arcData.color = mColor;
		arcData.transform = mTransform;
		arcData.layer = mLayer;
		arcData.center = mTransform.MultiplyAffine(position);
	}

	void DrawHelper::Rectangle(const Rect3& area)
	{
		mRect3Data.push_back(Rect3Data());
		Rect3Data& rectData = mRect3Data.Back();

		rectData.area = area;
		rectData.color = mColor;
		rectData.transform = mTransform;
		rectData.layer = mLayer;
		rectData.center = mTransform.MultiplyAffine(area.getCenter());
	}

	void DrawHelper::Text(const Vector3& position, const String& text, const HFont& font, UINT32 size)
	{
		if (!font.IsLoaded() || text.empty())
			return;

		mText2DData.push_back(Text2DData());
		Text2DData& textData = mText2DData.Back();

		textData.position = position;
		textData.color = mColor;
		textData.transform = mTransform;
		textData.layer = mLayer;
		textData.center = mTransform.MultiplyAffine(position);
		textData.text = text;
		textData.font = font;
		textData.size = size;
	}

	void DrawHelper::WireMesh(const SPtr<MeshData>& meshData)
	{
		if (meshData == nullptr)
			return;

		mWireMeshData.push_back(WireMeshData());
		WireMeshData& wireMeshData = mWireMeshData.Back();

		wireMeshData.meshData = meshData;
		wireMeshData.color = mColor;
		wireMeshData.transform = mTransform;
		wireMeshData.layer = mLayer;
		wireMeshData.center = mTransform.MultiplyAffine(Vector3::ZERO);
	}

	void DrawHelper::Clear()
	{
		mSolidCubeData.Clear();
		mWireCubeData.Clear();
		mSolidSphereData.Clear();
		mWireSphereData.Clear();
		mWireHemisphereData.Clear();
		mLineData.Clear();
		mLineListData.Clear();
		mRect3Data.Clear();
		mFrustumData.Clear();
		mDiscData.Clear();
		mWireDiscData.Clear();
		mArcData.Clear();
		mWireArcData.Clear();
		mConeData.Clear();
		mWireConeData.Clear();
		mText2DData.Clear();
		mWireMeshData.Clear();
	}

	Vector<DrawHelper::ShapeMeshData> DrawHelper::BuildMeshes(SortType sorting, const Camera* camera, UINT64 layers)
	{
		Vector<ShapeMeshData> meshInfos;

		enum class ShapeType
		{
			Cube, Sphere, WireCube, WireSphere, WireCone, Line, LineList, Frustum,
			Cone, Disc, WireDisc, Arc, WireArc, Rectangle, Text, WireMesh, WireHemisphere
		};

		struct RawData
		{
			ShapeType shapeType;
			MeshType meshType;
			UINT32 idx;
			UINT32 textIdx;
			float distance;
			UINT32 numVertices;
			UINT32 numIndices;
		};

		/************************************************************************/
		/* 			Sort everything according to specified sorting rule         */
		/************************************************************************/

		UINT32 idx = 0;
		Vector<RawData> allShapes;
		Vector3 reference = Vector3::ZERO;
		
		if(camera)
			reference = camera->GetTransform().GetPosition();

		UINT32 localIdx = 0;
		for (auto& shapeData : mSolidCubeData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Solid;
			rawData.shapeType = ShapeType::Cube;
			rawData.distance = shapeData.center.Distance(reference);

			ShapeMeshes3D::getNumElementsAABox(rawData.numVertices, rawData.numIndices);
		}

		localIdx = 0;
		for (auto& shapeData : mSolidSphereData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Solid;
			rawData.shapeType = ShapeType::Sphere;
			rawData.distance = shapeData.center.Distance(reference);

			ShapeMeshes3D::getNumElementsSphere(shapeData.quality,
				rawData.numVertices, rawData.numIndices);
		}

		localIdx = 0;
		for (auto& shapeData : mConeData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Solid;
			rawData.shapeType = ShapeType::Cone;
			rawData.distance = shapeData.center.Distance(reference);

			ShapeMeshes3D::getNumElementsCone(shapeData.quality,
				rawData.numVertices, rawData.numIndices);
		}

		localIdx = 0;
		for (auto& shapeData : mDiscData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Solid;
			rawData.shapeType = ShapeType::Disc;
			rawData.distance = shapeData.center.Distance(reference);

			ShapeMeshes3D::getNumElementsDisc(shapeData.quality,
				rawData.numVertices, rawData.numIndices);
		}

		localIdx = 0;
		for (auto& shapeData : mArcData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Solid;
			rawData.shapeType = ShapeType::Arc;
			rawData.distance = shapeData.center.Distance(reference);

			ShapeMeshes3D::getNumElementsArc(shapeData.quality,
				rawData.numVertices, rawData.numIndices);
		}

		localIdx = 0;
		for (auto& shapeData : mRect3Data)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Solid;
			rawData.shapeType = ShapeType::Rectangle;
			rawData.distance = shapeData.center.Distance(reference);

			ShapeMeshes3D::getNumElementsQuad(rawData.numVertices, rawData.numIndices);
		}

		localIdx = 0;
		for (auto& shapeData : mWireCubeData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Line;
			rawData.shapeType = ShapeType::WireCube;
			rawData.distance = shapeData.center.Distance(reference);

			ShapeMeshes3D::getNumElementsWireAABox(rawData.numVertices, rawData.numIndices);
		}

		localIdx = 0;
		for (auto& shapeData : mWireSphereData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Line;
			rawData.shapeType = ShapeType::WireSphere;
			rawData.distance = shapeData.center.Distance(reference);

			ShapeMeshes3D::getNumElementsWireSphere(shapeData.quality,
				rawData.numVertices, rawData.numIndices);
		}

		localIdx = 0;
		for (auto& shapeData : mWireHemisphereData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Line;
			rawData.shapeType = ShapeType::WireHemisphere;
			rawData.distance = shapeData.center.Distance(reference);

			ShapeMeshes3D::getNumElementsWireHemisphere(shapeData.quality,
				rawData.numVertices, rawData.numIndices);
		}

		localIdx = 0;
		for (auto& shapeData : mWireConeData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Line;
			rawData.shapeType = ShapeType::WireCone;
			rawData.distance = shapeData.center.Distance(reference);

			ShapeMeshes3D::getNumElementsWireCone(shapeData.quality,
				rawData.numVertices, rawData.numIndices);
		}

		localIdx = 0;
		for (auto& shapeData : mLineData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Line;
			rawData.shapeType = ShapeType::Line;
			rawData.distance = shapeData.center.Distance(reference);
			rawData.numVertices = 2;
			rawData.numIndices = 2;
		}

		localIdx = 0;
		for (auto& shapeData : mLineListData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			UINT32 numLines = (UINT32)shapeData.lines.Size() / 2;
			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Line;
			rawData.shapeType = ShapeType::LineList;
			rawData.distance = shapeData.center.Distance(reference);
			rawData.numVertices = numLines * 2;
			rawData.numIndices = numLines * 2;
		}

		localIdx = 0;
		for (auto& shapeData : mFrustumData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Line;
			rawData.shapeType = ShapeType::Frustum;
			rawData.distance = shapeData.center.Distance(reference);

			ShapeMeshes3D::getNumElementsFrustum(rawData.numVertices, rawData.numIndices);
		}

		localIdx = 0;
		for (auto& shapeData : mWireDiscData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Line;
			rawData.shapeType = ShapeType::WireDisc;
			rawData.distance = shapeData.center.Distance(reference);

			ShapeMeshes3D::getNumElementsWireDisc(shapeData.quality,
				rawData.numVertices, rawData.numIndices);
		}

		localIdx = 0;
		for (auto& shapeData : mWireArcData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Line;
			rawData.shapeType = ShapeType::WireArc;
			rawData.distance = shapeData.center.Distance(reference);

			ShapeMeshes3D::getNumElementsWireArc(shapeData.quality,
				rawData.numVertices, rawData.numIndices);
		}

		localIdx = 0;
		for (auto& shapeData : mWireMeshData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.Back();

			rawData.idx = localIdx++;
			rawData.textIdx = 0;
			rawData.meshType = MeshType::Wire;
			rawData.shapeType = ShapeType::WireMesh;
			rawData.distance = shapeData.center.Distance(reference);
			rawData.numVertices = shapeData.meshData->GetNumVertices();
			rawData.numIndices = shapeData.meshData->GetNumIndices();
		}

		struct TextRenderData
		{
			UINT32 page;
			SPtr<TextData<>> textData;
		};

		UnorderedMap<UINT32, TextRenderData> textRenderData;
		UINT32 textIdx = 0;

		localIdx = 0;
		for (auto& shapeData : mText2DData)
		{
			if ((shapeData.layer & layers) == 0)
			{
				localIdx++;
				continue;
			}

			U32String utf32text = UTF8::toUTF32(shapeData.text);
			SPtr<TextData<>> textData = bs_shared_ptr_new<TextData<>>(utf32text, shapeData.font, shapeData.size);

			UINT32 numPages = textData->GetNumPages();
			for (UINT32 j = 0; j < numPages; j++)
			{
				UINT32 numQuads = textData->GetNumQuadsForPage(j);

				allShapes.push_back(RawData());
				RawData& rawData = allShapes.Back();

				rawData.idx = localIdx;
				rawData.textIdx = textIdx;
				rawData.meshType = MeshType::Text;
				rawData.shapeType = ShapeType::Text;
				rawData.distance = shapeData.center.Distance(reference);
				rawData.numVertices = numQuads * 4;
				rawData.numIndices = numQuads * 6;

				TextRenderData& renderData = textRenderData[textIdx];
				renderData.page = j;
				renderData.textData = textData;

				textIdx++;
				idx++;
			}

			localIdx++;
		}

		if (sorting == SortType::FrontToBack)
		{
			std::sort(begin(allShapes), end(allShapes),
				[&](const RawData& x, const RawData& y)
			{
				return x.distance < y.distance;
			});
		}
		else if (sorting == SortType::BackToFront)
		{
			std::sort(begin(allShapes), end(allShapes),
				[&](const RawData& x, const RawData& y)
			{
				return y.distance < x.distance;
			});
		}

		/************************************************************************/
		/* 							Create batches                      		*/
		/************************************************************************/
		struct Batch
		{
			MeshType type;
			HTexture texture;
			UINT32 startIdx;
			UINT32 endIdx;
			UINT32 numVertices;
			UINT32 numIndices;
		};

		UINT32 numShapes = (UINT32)allShapes.Size();

		Vector<Batch> batches;
		if (numShapes > 0)
		{
			batches.push_back(Batch());

			{
				Batch& currentBatch = batches.Back();
				currentBatch.startIdx = 0;
				currentBatch.type = allShapes[0].meshType;
				currentBatch.numVertices = allShapes[0].numVertices;
				currentBatch.numIndices = allShapes[0].numIndices;

				if (allShapes[0].meshType == MeshType::Text)
				{
					TextRenderData& renderData = textRenderData[allShapes[0].textIdx];
					currentBatch.texture = renderData.textData->GetTextureForPage(renderData.page);
				}
			}

			for (UINT32 i = 1; i < numShapes; i++)
			{
				Batch& currentBatch = batches.Back();

				HTexture texture;
				if (allShapes[i].meshType == MeshType::Text)
				{
					TextRenderData& renderData = textRenderData[allShapes[i].textIdx];
					texture = renderData.textData->GetTextureForPage(renderData.page);
				}

				bool startNewBatch = allShapes[i].meshType != currentBatch.type || texture != currentBatch.texture;
				if (startNewBatch)
				{
					currentBatch.endIdx = i - 1;

					batches.push_back(Batch());

					Batch& newBatch = batches.Back();
					newBatch.startIdx = i;
					newBatch.type = allShapes[i].meshType;
					newBatch.numVertices = allShapes[i].numVertices;
					newBatch.numIndices = allShapes[i].numIndices;
					newBatch.texture = texture;
				}
				else
				{
					currentBatch.endIdx = i;
					currentBatch.numVertices += allShapes[i].numVertices;
					currentBatch.numIndices += allShapes[i].numIndices;
				}
			}

			{
				Batch& currentBatch = batches.Back();
				currentBatch.endIdx = numShapes - 1;
			}
		}

		// Allocate space for all the batch vertices/indices, per type
		UINT32 vertexCount[4] = { 0, 0, 0, 0 };
		UINT32 indexCount[4] = { 0, 0, 0, 0 };
		for (auto& batch : batches)
		{
			UINT32 typeIdx = (UINT32)batch.type;

			vertexCount[typeIdx] += batch.numVertices;
			indexCount[typeIdx] += batch.numIndices;
		}

		SPtr<VertexDataDesc> vertexDesc[4] = { mSolidVertexDesc, mWireVertexDesc, mLineVertexDesc, mTextVertexDesc };
		SPtr<MeshData> meshData[4];
		for(UINT32 i = 0; i < 4; i++)
		{
			if(vertexCount[i] > 0 && indexCount[i] > 0)
				meshData[i] = MeshData::create(vertexCount[i], indexCount[i], vertexDesc[i]);
		}

		/************************************************************************/
		/* 					Generate geometry for each batch                    */
		/************************************************************************/
		UINT32 vertexOffset[4] = { 0, 0, 0, 0 };
		UINT32 indexOffset[4] = { 0, 0, 0, 0 };

		VertexElemIter<Vector3> positionIter[4];
		VertexElemIter<UINT32> colorIter[4];

		for(UINT32 i = 0; i < 4; i++)
		{
			if(!meshData[i])
				continue;

			positionIter[i] = meshData[i]->GetVec3DataIter(VES_POSITION);
			colorIter[i] = meshData[i]->GetDWORDDataIter(VES_COLOR);
		}

		VertexElemIter<Vector3> solidNormalIter;
		if(meshData[0])
			solidNormalIter = meshData[0]->GetVec3DataIter(VES_NORMAL);

		VertexElemIter<Vector2> textUVIter;
		
		if(meshData[3])
			textUVIter = meshData[3]->GetVec2DataIter(VES_TEXCOORD);

		for (auto& batch : batches)
		{
			UINT32 typeIdx = (UINT32)batch.type;

			if (batch.type == MeshType::Solid)
			{
				meshInfos.push_back(ShapeMeshData());
				ShapeMeshData& newMesh = meshInfos.Back();
				newMesh.subMesh.indexOffset = indexOffset[typeIdx];
				newMesh.subMesh.indexCount = batch.numIndices;
				newMesh.subMesh.drawOp = DOT_TRIANGLE_LIST;
				newMesh.type = MeshType::Solid;

				for (UINT32 i = batch.startIdx; i <= batch.endIdx; i++)
				{
					RawData& shapeData = allShapes[i];

					Matrix4* transform = nullptr;
					RGBA color = 0;

					switch (shapeData.shapeType)
					{
					case ShapeType::Cube:
					{
						CubeData& cubeData = mSolidCubeData[shapeData.idx];
						AABox Box(cubeData.position - cubeData.extents, cubeData.position + cubeData.extents);
						ShapeMeshes3D::solidAABox(box, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx]);

						transform = &cubeData.transform;
						color = cubeData.color.GetAsRGBA();
					}
						break;
					case ShapeType::Sphere:
					{
						SphereData& sphereData = mSolidSphereData[shapeData.idx];
						Sphere Sphere(sphereData.position, sphereData.radius);
						ShapeMeshes3D::solidSphere(sphere, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx],
							sphereData.quality);

						transform = &sphereData.transform;
						color = sphereData.color.GetAsRGBA();
					}
						break;
					case ShapeType::Cone:
					{
						ConeData& coneData = mConeData[shapeData.idx];
						ShapeMeshes3D::solidCone(coneData.base, coneData.normal, coneData.height, coneData.radius,
							coneData.scale, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx], coneData.quality);

						transform = &coneData.transform;
						color = coneData.color.GetAsRGBA();
					}
						break;
					case ShapeType::Disc:
					{
						DiscData& discData = mDiscData[shapeData.idx];
						ShapeMeshes3D::solidDisc(discData.position, discData.radius, discData.normal,
							meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx], discData.quality);

						transform = &discData.transform;
						color = discData.color.GetAsRGBA();
					}
						break;
					case ShapeType::Arc:
					{
						ArcData& arcData = mArcData[shapeData.idx];
						ShapeMeshes3D::solidArc(arcData.position, arcData.radius, arcData.normal,
							arcData.startAngle, arcData.amountAngle, meshData[typeIdx], vertexOffset[typeIdx],
							indexOffset[typeIdx], arcData.quality);

						transform = &arcData.transform;
						color = arcData.color.GetAsRGBA();
					}
						break;
					case ShapeType::Rectangle:
					{
						Rect3Data& rectData = mRect3Data[shapeData.idx];
						ShapeMeshes3D::solidQuad(rectData.area, meshData[typeIdx], vertexOffset[typeIdx],
							indexOffset[typeIdx]);

						transform = &rectData.transform;
						color = rectData.color.GetAsRGBA();
					}
						break;
					default:
						break;
					}

					Matrix4 transformIT = transform->InverseAffine().Transpose();
					for (UINT32 i = 0; i < shapeData.numVertices; i++)
					{
						Vector3 worldPos = transform->MultiplyAffine(positionIter[typeIdx].GetValue());
						Vector3 worldNormal = transformIT.MultiplyAffine(solidNormalIter.getValue());

						positionIter[typeIdx].AddValue(worldPos);
						solidNormalIter.AddValue(worldNormal);
						colorIter[typeIdx].AddValue(color);
					}

					vertexOffset[typeIdx] += shapeData.numVertices;
					indexOffset[typeIdx] += shapeData.numIndices;
				}
			}
			else if (batch.type == MeshType::Wire)
			{
				meshInfos.push_back(ShapeMeshData());
				ShapeMeshData& newMesh = meshInfos.Back();
				newMesh.subMesh.indexOffset = indexOffset[typeIdx];
				newMesh.subMesh.indexCount = batch.numIndices;
				newMesh.subMesh.drawOp = DOT_TRIANGLE_LIST;
				newMesh.type = MeshType::Wire;

				for (UINT32 i = batch.startIdx; i <= batch.endIdx; i++)
				{
					RawData& shapeData = allShapes[i];

					Matrix4* transform = nullptr;
					RGBA color = 0;

					switch (shapeData.shapeType)
					{
					case ShapeType::WireMesh:
					{
						WireMeshData& wireMeshData = mWireMeshData[shapeData.idx];

						transform = &wireMeshData.transform;
						color = wireMeshData.color.GetAsRGBA();

						auto vertIterRead = wireMeshData.meshData->GetVec3DataIter(VES_POSITION);
						for (UINT32 j = 0; j < vertIterRead.GetNumElements(); j++)
						{
							Vector3 worldPos = transform->MultiplyAffine(vertIterRead.GetValue());

							positionIter[typeIdx].AddValue(worldPos);
							colorIter[typeIdx].AddValue(color);

							vertIterRead.MoveNext();
						}

						UINT32* srcIndexData = wireMeshData.meshData->GetIndices32();
						UINT32* destIndexData = meshData[typeIdx]->GetIndices32() + indexOffset[typeIdx];

						for(UINT32 j = 0; j < shapeData.numIndices; j++)
							destIndexData[j] = srcIndexData[j] + vertexOffset[typeIdx];

						vertexOffset[typeIdx] += shapeData.numVertices;
						indexOffset[typeIdx] += shapeData.numIndices;
					}
					break;
					default:
						break;
					}
				}
			}
			else If(batch.type == MeshType::Line)
			{
				meshInfos.push_back(ShapeMeshData());
				ShapeMeshData& newMesh = meshInfos.Back();
				newMesh.subMesh.indexOffset = indexOffset[typeIdx];
				newMesh.subMesh.indexCount = batch.numIndices;
				newMesh.subMesh.drawOp = DOT_LINE_LIST;
				newMesh.type = MeshType::Line;

				for (UINT32 i = batch.startIdx; i <= batch.endIdx; i++)
				{
					RawData& shapeData = allShapes[i];

					Matrix4* transform = nullptr;
					RGBA color = 0;

					switch (shapeData.shapeType)
					{
					case ShapeType::WireCube:
					{
						CubeData& cubeData = mWireCubeData[shapeData.idx];

						AABox Box(cubeData.position - cubeData.extents, cubeData.position + cubeData.extents);
						ShapeMeshes3D::wireAABox(box, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx]);

						transform = &cubeData.transform;
						color = cubeData.color.GetAsRGBA();
					}
						break;
					case ShapeType::WireSphere:
					{
						SphereData& sphereData = mWireSphereData[shapeData.idx];

						Sphere Sphere(sphereData.position, sphereData.radius);
						ShapeMeshes3D::wireSphere(sphere, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx],
							sphereData.quality);

						transform = &sphereData.transform;
						color = sphereData.color.GetAsRGBA();
					}
						break;
					case ShapeType::WireHemisphere:
					{
						SphereData& sphereData = mWireHemisphereData[shapeData.idx];

						Sphere Sphere(sphereData.position, sphereData.radius);
						ShapeMeshes3D::wireHemisphere(sphere, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx],
							sphereData.quality);

						transform = &sphereData.transform;
						color = sphereData.color.GetAsRGBA();
					}
						break;
					case ShapeType::WireCone:
					{
						ConeData& coneData = mWireConeData[shapeData.idx];
						ShapeMeshes3D::wireCone(coneData.base, coneData.normal, coneData.height, coneData.radius,
							coneData.scale, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx],
							coneData.quality);

						transform = &coneData.transform;
						color = coneData.color.GetAsRGBA();
					}
					break;
					case ShapeType::Line:
					{
						LineData& lineData = mLineData[shapeData.idx];

						ShapeMeshes3D::pixelLine(lineData.start, lineData.end, meshData[typeIdx], vertexOffset[typeIdx],
							indexOffset[typeIdx]);

						transform = &lineData.transform;
						color = lineData.color.GetAsRGBA();
					}
						break;
					case ShapeType::LineList:
					{
						LineListData& lineListData = mLineListData[shapeData.idx];

						ShapeMeshes3D::pixelLineList(lineListData.lines, meshData[typeIdx], vertexOffset[typeIdx],
							indexOffset[typeIdx]);

						transform = &lineListData.transform;
						color = lineListData.color.GetAsRGBA();
					}
					break;
					case ShapeType::Frustum:
					{
						FrustumData& frustumData = mFrustumData[shapeData.idx];

						ShapeMeshes3D::wireFrustum(frustumData.position, frustumData.aspect, frustumData.FOV, frustumData.nearDist,
							frustumData.farDist, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx]);

						transform = &frustumData.transform;
						color = frustumData.color.GetAsRGBA();
					}
						break;
					case ShapeType::WireDisc:
					{
						DiscData& discData = mWireDiscData[shapeData.idx];

						ShapeMeshes3D::wireDisc(discData.position, discData.radius, discData.normal,
							meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx], discData.quality);

						transform = &discData.transform;
						color = discData.color.GetAsRGBA();
					}
						break;
					case ShapeType::WireArc:
					{
						ArcData& arcData = mWireArcData[shapeData.idx];

						ShapeMeshes3D::wireArc(arcData.position, arcData.radius, arcData.normal,
							arcData.startAngle, arcData.amountAngle, meshData[typeIdx], vertexOffset[typeIdx],
							indexOffset[typeIdx], arcData.quality);

						transform = &arcData.transform;
						color = arcData.color.GetAsRGBA();
					}
						break;
					default:
						break;
					}

					for (UINT32 i = 0; i < shapeData.numVertices; i++)
					{
						Vector3 worldPos = transform->MultiplyAffine(positionIter[typeIdx].GetValue());

						positionIter[typeIdx].AddValue(worldPos);
						colorIter[typeIdx].AddValue(color);
					}

					vertexOffset[typeIdx] += shapeData.numVertices;
					indexOffset[typeIdx] += shapeData.numIndices;
				}
			}
			else // Text
			{
				// Text drawing requires a camera in order to determine screen space coordinates
				if(!camera)
					continue;

				meshInfos.push_back(ShapeMeshData());
				ShapeMeshData& newMesh = meshInfos.Back();
				newMesh.subMesh.indexOffset = indexOffset[typeIdx];
				newMesh.subMesh.indexCount = batch.numIndices;
				newMesh.subMesh.drawOp = DOT_TRIANGLE_LIST;
				newMesh.type = MeshType::Text;
				newMesh.texture = batch.texture;

				for (UINT32 i = batch.startIdx; i <= batch.endIdx; i++)
				{
					RawData& shapeData = allShapes[i];
					Text2DData& text2DData = mText2DData[shapeData.idx];

					TextRenderData& renderData = textRenderData[shapeData.textIdx];
					UINT32 numQuads = renderData.textData->GetNumQuadsForPage(renderData.page);

					UINT32* indices = meshData[typeIdx]->GetIndices32() + indexOffset[typeIdx];

					// Note: Need temporary buffers because TextLine doesn't support arbitrary vertex stride. Eventually
					// that should be supported (should be almost trivial to implement)
					Vector2* tempVertices = bs_stack_alloc<Vector2>(shapeData.numVertices);
					Vector2* tempUVs = bs_stack_alloc<Vector2>(shapeData.numVertices);

					UINT32 numLines = renderData.textData->GetNumLines();
					UINT32 quadOffset = 0;
					for (UINT32 j = 0; j < numLines; j++)
					{
						const TextDataBase::TextLine& line = renderData.textData->GetLine(j);
						UINT32 writtenQuads = line.FillBuffer(renderData.page, tempVertices, tempUVs, indices, quadOffset, numQuads);

						quadOffset += writtenQuads;
					}

					for(UINT32 j = 0; j < shapeData.numIndices; j++)
						indices[j] += vertexOffset[typeIdx];

					Vector3 worldSpacePos = text2DData.transform.MultiplyAffine(text2DData.position);
					Vector2I screenPos = camera->WorldToScreenPoint(worldSpacePos);
					screenPos.x -= renderData.textData->GetWidth() / 2;
					screenPos.y -= renderData.textData->GetHeight() / 2;

					float z = camera->ProjectPoint(camera->worldToViewPoint(worldSpacePos)).z;

					for (UINT32 j = 0; j < shapeData.numVertices; j++)
					{
						Vector3 VertexPos(screenPos.x + tempVertices[j].x, screenPos.y + tempVertices[j].y, z);

						positionIter[typeIdx].AddValue(vertexPos);
						textUVIter.AddValue(tempUVs[j]);
						colorIter[typeIdx].AddValue(text2DData.color.getAsRGBA());
					}

					bs_stack_free(tempUVs);
					bs_stack_free(tempVertices);

					vertexOffset[typeIdx] += shapeData.numVertices;
					indexOffset[typeIdx] += shapeData.numIndices;
				}
			}
		}

		SPtr<Mesh> meshes[4];
		for(UINT32 i = 0; i < 4; i++)
		{
			if(meshData[i])
				meshes[i] = Mesh::_createPtr(meshData[i]);
		}

		for(auto& entry : meshInfos)
		{
			switch(entry.type)
			{
			case MeshType::Solid:
				entry.mesh = meshes[0];
				break;
			case MeshType::Wire:
				entry.mesh = meshes[1];
				break;
			case MeshType::Line:
				entry.mesh = meshes[2];
				break;
			case MeshType::Text:
				entry.mesh = meshes[3];
				break;
			default: ;
			}
		}

		return meshInfos;
	}
}
