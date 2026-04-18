//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DDrawHelper.h"
#include "Mesh/B3DMesh.h"
#include "Math/B3DAABox.h"
#include "Math/B3DSphere.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Utility/B3DShapeMeshes3D.h"
#include "Text/B3DTextGeometry.h"
#include "Math/B3DVector2.h"
#include "Math/B3DQuaternion.h"
#include "String/B3DUnicode.h"
#include "Components/B3DCamera.h"
#include "Scene/B3DSceneObject.h"

using namespace b3d;

const u32 DrawHelper::kVertexBufferGrowth = 4096;
const u32 DrawHelper::kIndexBufferGrowth = 4096 * 2;

DrawHelper::DrawHelper()
	: mLayer(1)
{
	mTransform = Matrix4::kIdentity;

	TInlineArray<VertexElement, 8> solidVertexElements;
	solidVertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));
	solidVertexElements.Add(VertexElement(VET_FLOAT3, VES_NORMAL));
	solidVertexElements.Add(VertexElement(VET_COLOR, VES_COLOR));

	mSolidVertexDesc = B3DMakeShared<VertexDescription>(solidVertexElements);

	TInlineArray<VertexElement, 8> wireVertexElements;
	wireVertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));
	wireVertexElements.Add(VertexElement(VET_COLOR, VES_COLOR));

	mWireVertexDesc = B3DMakeShared<VertexDescription>(wireVertexElements);

	TInlineArray<VertexElement, 8> lineVertexElements;
	lineVertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));
	lineVertexElements.Add(VertexElement(VET_COLOR, VES_COLOR));

	mLineVertexDesc = B3DMakeShared<VertexDescription>(lineVertexElements);

	TInlineArray<VertexElement, 8> textVertexElements;
	textVertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));
	textVertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD));
	textVertexElements.Add(VertexElement(VET_COLOR, VES_COLOR));

	mTextVertexDesc = B3DMakeShared<VertexDescription>(textVertexElements);
}

void DrawHelper::SetColor(const Color& color)
{
	mColor = color;
}

void DrawHelper::SetTransform(const Matrix4& transform)
{
	mTransform = transform;
}

void DrawHelper::SetLayer(u64 layer)
{
	mLayer = layer;
}

void DrawHelper::Cube(const Vector3& position, const Vector3& extents)
{
	mSolidCubeData.push_back(CubeData());
	CubeData& cubeData = mSolidCubeData.back();

	cubeData.Position = position;
	cubeData.Extents = extents;
	cubeData.Color = mColor;
	cubeData.Transform = mTransform;
	cubeData.Layer = mLayer;
	cubeData.Center = mTransform.MultiplyAffine(position);
}

void DrawHelper::Sphere(const Vector3& position, float radius, u32 quality)
{
	mSolidSphereData.push_back(SphereData());
	SphereData& sphereData = mSolidSphereData.back();

	sphereData.Position = position;
	sphereData.Radius = radius;
	sphereData.Quality = quality;
	sphereData.Color = mColor;
	sphereData.Transform = mTransform;
	sphereData.Layer = mLayer;
	sphereData.Center = mTransform.MultiplyAffine(position);
}

void DrawHelper::WireCube(const Vector3& position, const Vector3& extents)
{
	mWireCubeData.push_back(CubeData());
	CubeData& cubeData = mWireCubeData.back();

	cubeData.Position = position;
	cubeData.Extents = extents;
	cubeData.Color = mColor;
	cubeData.Transform = mTransform;
	cubeData.Layer = mLayer;
	cubeData.Center = mTransform.MultiplyAffine(position);
}

void DrawHelper::WireSphere(const Vector3& position, float radius, u32 quality)
{
	mWireSphereData.push_back(SphereData());
	SphereData& sphereData = mWireSphereData.back();

	sphereData.Position = position;
	sphereData.Radius = radius;
	sphereData.Quality = quality;
	sphereData.Color = mColor;
	sphereData.Transform = mTransform;
	sphereData.Layer = mLayer;
	sphereData.Center = mTransform.MultiplyAffine(position);
}

void DrawHelper::WireHemisphere(const Vector3& position, float radius, u32 quality)
{
	mWireHemisphereData.push_back(SphereData());
	SphereData& sphereData = mWireHemisphereData.back();

	sphereData.Position = position;
	sphereData.Radius = radius;
	sphereData.Quality = quality;
	sphereData.Color = mColor;
	sphereData.Transform = mTransform;
	sphereData.Layer = mLayer;
	sphereData.Center = mTransform.MultiplyAffine(position);
}

void DrawHelper::Line(const Vector3& start, const Vector3& end)
{
	mLineData.push_back(LineData());
	LineData& lineData = mLineData.back();

	lineData.Start = start;
	lineData.End = end;
	lineData.Color = mColor;
	lineData.Transform = mTransform;
	lineData.Layer = mLayer;
	lineData.Center = mTransform.MultiplyAffine((start + end) * 0.5f);
}

void DrawHelper::LineList(const Vector<Vector3>& lines)
{
	if(lines.size() < 2)
		return;

	mLineListData.push_back(LineListData());
	LineListData& lineListData = mLineListData.back();

	Vector3 center(kZeroTag);
	for(auto& point : lines)
		center += point;

	lineListData.Lines = lines;
	lineListData.Color = mColor;
	lineListData.Transform = mTransform;
	lineListData.Layer = mLayer;
	lineListData.Center = center / (float)lines.size();
	;
}

void DrawHelper::Frustum(const Vector3& position, float aspect, Degree FOV, float near, float far)
{
	mFrustumData.push_back(FrustumData());
	FrustumData& frustumData = mFrustumData.back();

	frustumData.Position = position;
	frustumData.Aspect = aspect;
	frustumData.FOV = FOV;
	frustumData.NearDist = near;
	frustumData.FarDist = far;
	frustumData.Color = mColor;
	frustumData.Transform = mTransform;
	frustumData.Layer = mLayer;
	frustumData.Center = mTransform.MultiplyAffine(position);
}

void DrawHelper::Cone(const Vector3& base, const Vector3& normal, float height, float radius, const Vector2& scale, u32 quality)
{
	mConeData.push_back(ConeData());
	ConeData& coneData = mConeData.back();

	coneData.Base = base;
	coneData.Normal = normal;
	coneData.Height = height;
	coneData.Radius = radius;
	coneData.Scale = scale;
	coneData.Quality = quality;
	coneData.Color = mColor;
	coneData.Transform = mTransform;
	coneData.Layer = mLayer;
	coneData.Center = mTransform.MultiplyAffine(base + normal * height * 0.5f);
}

void DrawHelper::WireCone(const Vector3& base, const Vector3& normal, float height, float radius, const Vector2& scale, u32 quality)
{
	mWireConeData.push_back(ConeData());
	ConeData& coneData = mWireConeData.back();

	coneData.Base = base;
	coneData.Normal = normal;
	coneData.Height = height;
	coneData.Radius = radius;
	coneData.Scale = scale;
	coneData.Quality = quality;
	coneData.Color = mColor;
	coneData.Transform = mTransform;
	coneData.Layer = mLayer;
	coneData.Center = mTransform.MultiplyAffine(base + normal * height * 0.5f);
}

void DrawHelper::Disc(const Vector3& position, const Vector3& normal, float radius, u32 quality)
{
	mDiscData.push_back(DiscData());
	DiscData& discData = mDiscData.back();

	discData.Position = position;
	discData.Normal = normal;
	discData.Radius = radius;
	discData.Quality = quality;
	discData.Color = mColor;
	discData.Transform = mTransform;
	discData.Layer = mLayer;
	discData.Center = mTransform.MultiplyAffine(position);
}

void DrawHelper::WireDisc(const Vector3& position, const Vector3& normal, float radius, u32 quality)
{
	mWireDiscData.push_back(DiscData());
	DiscData& discData = mWireDiscData.back();

	discData.Position = position;
	discData.Normal = normal;
	discData.Radius = radius;
	discData.Quality = quality;
	discData.Color = mColor;
	discData.Transform = mTransform;
	discData.Layer = mLayer;
	discData.Center = mTransform.MultiplyAffine(position);
}

void DrawHelper::Arc(const Vector3& position, const Vector3& normal, float radius, Degree startAngle, Degree amountAngle, u32 quality)
{
	mArcData.push_back(ArcData());
	ArcData& arcData = mArcData.back();

	arcData.Position = position;
	arcData.Normal = normal;
	arcData.Radius = radius;
	arcData.StartAngle = startAngle;
	arcData.AmountAngle = amountAngle;
	arcData.Quality = quality;
	arcData.Color = mColor;
	arcData.Transform = mTransform;
	arcData.Layer = mLayer;
	arcData.Center = mTransform.MultiplyAffine(position);
}

void DrawHelper::WireArc(const Vector3& position, const Vector3& normal, float radius, Degree startAngle, Degree amountAngle, u32 quality)
{
	mWireArcData.push_back(ArcData());
	ArcData& arcData = mWireArcData.back();

	arcData.Position = position;
	arcData.Normal = normal;
	arcData.Radius = radius;
	arcData.StartAngle = startAngle;
	arcData.AmountAngle = amountAngle;
	arcData.Quality = quality;
	arcData.Color = mColor;
	arcData.Transform = mTransform;
	arcData.Layer = mLayer;
	arcData.Center = mTransform.MultiplyAffine(position);
}

void DrawHelper::Rectangle(const Rect3& area)
{
	mRect3Data.push_back(Rect3Data());
	Rect3Data& rectData = mRect3Data.back();

	rectData.Area = area;
	rectData.Color = mColor;
	rectData.Transform = mTransform;
	rectData.Layer = mLayer;
	rectData.Center = mTransform.MultiplyAffine(area.Center);
}

void DrawHelper::Text(const Vector3& position, const String& text, const HFont& font, float size)
{
	if(!font.IsLoaded() || text.empty())
		return;

	mText2DData.push_back(Text2DData());
	Text2DData& textData = mText2DData.back();

	textData.Position = position;
	textData.Color = mColor;
	textData.Transform = mTransform;
	textData.Layer = mLayer;
	textData.Center = mTransform.MultiplyAffine(position);
	textData.Text = text;
	textData.Font = font;
	textData.Size = size;
}

void DrawHelper::WireMesh(const SPtr<MeshData>& meshData)
{
	if(meshData == nullptr)
		return;

	mWireMeshData.push_back(WireMeshData());
	WireMeshData& wireMeshData = mWireMeshData.back();

	wireMeshData.MeshData = meshData;
	wireMeshData.Color = mColor;
	wireMeshData.Transform = mTransform;
	wireMeshData.Layer = mLayer;
	wireMeshData.Center = mTransform.MultiplyAffine(Vector3::kZero);
}

void DrawHelper::Clear()
{
	mSolidCubeData.clear();
	mWireCubeData.clear();
	mSolidSphereData.clear();
	mWireSphereData.clear();
	mWireHemisphereData.clear();
	mLineData.clear();
	mLineListData.clear();
	mRect3Data.clear();
	mFrustumData.clear();
	mDiscData.clear();
	mWireDiscData.clear();
	mArcData.clear();
	mWireArcData.clear();
	mConeData.clear();
	mWireConeData.clear();
	mText2DData.clear();
	mWireMeshData.clear();
}

Vector<DrawHelper::ShapeMeshData> DrawHelper::BuildMeshes(SortType sorting, const Camera* camera, u64 layers)
{
	Vector<ShapeMeshData> meshInfos;

	enum class ShapeType
	{
		Cube,
		Sphere,
		WireCube,
		WireSphere,
		WireCone,
		Line,
		LineList,
		Frustum,
		Cone,
		Disc,
		WireDisc,
		Arc,
		WireArc,
		Rectangle,
		Text,
		WireMesh,
		WireHemisphere
	};

	struct RawData
	{
		ShapeType ShapeType;
		MeshType MeshType;
		u32 Idx;
		u32 TextIdx;
		float Distance;
		u32 NumVertices;
		u32 NumIndices;
	};

	/************************************************************************/
	/* 			Sort everything according to specified sorting rule         */
	/************************************************************************/

	u32 idx = 0;
	Vector<RawData> allShapes;
	Vector3 reference = Vector3::kZero;

	if(camera)
		reference = camera->SceneObject()->GetTransform().GetPosition();

	u32 localIdx = 0;
	for(auto& shapeData : mSolidCubeData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Solid;
		rawData.ShapeType = ShapeType::Cube;
		rawData.Distance = shapeData.Center.Distance(reference);

		ShapeMeshes3D::GetNumElementsAaBox(rawData.NumVertices, rawData.NumIndices);
	}

	localIdx = 0;
	for(auto& shapeData : mSolidSphereData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Solid;
		rawData.ShapeType = ShapeType::Sphere;
		rawData.Distance = shapeData.Center.Distance(reference);

		ShapeMeshes3D::GetNumElementsSphere(shapeData.Quality, rawData.NumVertices, rawData.NumIndices);
	}

	localIdx = 0;
	for(auto& shapeData : mConeData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Solid;
		rawData.ShapeType = ShapeType::Cone;
		rawData.Distance = shapeData.Center.Distance(reference);

		ShapeMeshes3D::GetNumElementsCone(shapeData.Quality, rawData.NumVertices, rawData.NumIndices);
	}

	localIdx = 0;
	for(auto& shapeData : mDiscData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Solid;
		rawData.ShapeType = ShapeType::Disc;
		rawData.Distance = shapeData.Center.Distance(reference);

		ShapeMeshes3D::GetNumElementsDisc(shapeData.Quality, rawData.NumVertices, rawData.NumIndices);
	}

	localIdx = 0;
	for(auto& shapeData : mArcData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Solid;
		rawData.ShapeType = ShapeType::Arc;
		rawData.Distance = shapeData.Center.Distance(reference);

		ShapeMeshes3D::GetNumElementsArc(shapeData.Quality, rawData.NumVertices, rawData.NumIndices);
	}

	localIdx = 0;
	for(auto& shapeData : mRect3Data)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Solid;
		rawData.ShapeType = ShapeType::Rectangle;
		rawData.Distance = shapeData.Center.Distance(reference);

		ShapeMeshes3D::GetNumElementsQuad(rawData.NumVertices, rawData.NumIndices);
	}

	localIdx = 0;
	for(auto& shapeData : mWireCubeData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Line;
		rawData.ShapeType = ShapeType::WireCube;
		rawData.Distance = shapeData.Center.Distance(reference);

		ShapeMeshes3D::GetNumElementsWireAaBox(rawData.NumVertices, rawData.NumIndices);
	}

	localIdx = 0;
	for(auto& shapeData : mWireSphereData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Line;
		rawData.ShapeType = ShapeType::WireSphere;
		rawData.Distance = shapeData.Center.Distance(reference);

		ShapeMeshes3D::GetNumElementsWireSphere(shapeData.Quality, rawData.NumVertices, rawData.NumIndices);
	}

	localIdx = 0;
	for(auto& shapeData : mWireHemisphereData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Line;
		rawData.ShapeType = ShapeType::WireHemisphere;
		rawData.Distance = shapeData.Center.Distance(reference);

		ShapeMeshes3D::GetNumElementsWireHemisphere(shapeData.Quality, rawData.NumVertices, rawData.NumIndices);
	}

	localIdx = 0;
	for(auto& shapeData : mWireConeData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Line;
		rawData.ShapeType = ShapeType::WireCone;
		rawData.Distance = shapeData.Center.Distance(reference);

		ShapeMeshes3D::GetNumElementsWireCone(shapeData.Quality, rawData.NumVertices, rawData.NumIndices);
	}

	localIdx = 0;
	for(auto& shapeData : mLineData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Line;
		rawData.ShapeType = ShapeType::Line;
		rawData.Distance = shapeData.Center.Distance(reference);
		rawData.NumVertices = 2;
		rawData.NumIndices = 2;
	}

	localIdx = 0;
	for(auto& shapeData : mLineListData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		u32 numLines = (u32)shapeData.Lines.size() / 2;
		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Line;
		rawData.ShapeType = ShapeType::LineList;
		rawData.Distance = shapeData.Center.Distance(reference);
		rawData.NumVertices = numLines * 2;
		rawData.NumIndices = numLines * 2;
	}

	localIdx = 0;
	for(auto& shapeData : mFrustumData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Line;
		rawData.ShapeType = ShapeType::Frustum;
		rawData.Distance = shapeData.Center.Distance(reference);

		ShapeMeshes3D::GetNumElementsFrustum(rawData.NumVertices, rawData.NumIndices);
	}

	localIdx = 0;
	for(auto& shapeData : mWireDiscData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Line;
		rawData.ShapeType = ShapeType::WireDisc;
		rawData.Distance = shapeData.Center.Distance(reference);

		ShapeMeshes3D::GetNumElementsWireDisc(shapeData.Quality, rawData.NumVertices, rawData.NumIndices);
	}

	localIdx = 0;
	for(auto& shapeData : mWireArcData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Line;
		rawData.ShapeType = ShapeType::WireArc;
		rawData.Distance = shapeData.Center.Distance(reference);

		ShapeMeshes3D::GetNumElementsWireArc(shapeData.Quality, rawData.NumVertices, rawData.NumIndices);
	}

	localIdx = 0;
	for(auto& shapeData : mWireMeshData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		allShapes.push_back(RawData());
		RawData& rawData = allShapes.back();

		rawData.Idx = localIdx++;
		rawData.TextIdx = 0;
		rawData.MeshType = MeshType::Wire;
		rawData.ShapeType = ShapeType::WireMesh;
		rawData.Distance = shapeData.Center.Distance(reference);
		rawData.NumVertices = shapeData.MeshData->GetVertexCount();
		rawData.NumIndices = shapeData.MeshData->GetIndexCount();
	}

	struct TextRenderData
	{
		u32 Page;
		SPtr<TTextGeometry<>> TextGeometry;
	};

	UnorderedMap<u32, TextRenderData> textRenderData;
	u32 textIdx = 0;

	localIdx = 0;
	for(auto& shapeData : mText2DData)
	{
		if((shapeData.Layer & layers) == 0)
		{
			localIdx++;
			continue;
		}

		U32String utf32text = UTF8::ToUtF32(shapeData.Text);
		SPtr<TTextGeometry<>> textData = B3DMakeShared<TTextGeometry<>>(utf32text, shapeData.Font, shapeData.Size);

		u32 numPages = textData->GetPageCount();
		for(u32 j = 0; j < numPages; j++)
		{
			u32 numQuads = textData->GetQuadCount(j);

			allShapes.push_back(RawData());
			RawData& rawData = allShapes.back();

			rawData.Idx = localIdx;
			rawData.TextIdx = textIdx;
			rawData.MeshType = MeshType::Text;
			rawData.ShapeType = ShapeType::Text;
			rawData.Distance = shapeData.Center.Distance(reference);
			rawData.NumVertices = numQuads * 4;
			rawData.NumIndices = numQuads * 6;

			TextRenderData& renderData = textRenderData[textIdx];
			renderData.Page = j;
			renderData.TextGeometry = textData;

			textIdx++;
			idx++;
		}

		localIdx++;
	}

	if(sorting == SortType::FrontToBack)
	{
		std::sort(begin(allShapes), end(allShapes), [&](const RawData& x, const RawData& y)
				  { return x.Distance < y.Distance; });
	}
	else if(sorting == SortType::BackToFront)
	{
		std::sort(begin(allShapes), end(allShapes), [&](const RawData& x, const RawData& y)
				  { return y.Distance < x.Distance; });
	}

	/************************************************************************/
	/* 							Create batches                      		*/
	/************************************************************************/
	struct Batch
	{
		MeshType Type;
		HTexture Texture;
		u32 StartIdx;
		u32 EndIdx;
		u32 NumVertices;
		u32 NumIndices;
	};

	u32 numShapes = (u32)allShapes.size();

	Vector<Batch> batches;
	if(numShapes > 0)
	{
		batches.push_back(Batch());

		{
			Batch& currentBatch = batches.back();
			currentBatch.StartIdx = 0;
			currentBatch.Type = allShapes[0].MeshType;
			currentBatch.NumVertices = allShapes[0].NumVertices;
			currentBatch.NumIndices = allShapes[0].NumIndices;

			if(allShapes[0].MeshType == MeshType::Text)
			{
				TextRenderData& renderData = textRenderData[allShapes[0].TextIdx];
				currentBatch.Texture = renderData.TextGeometry->GetTextureForPage(renderData.Page);
			}
		}

		for(u32 i = 1; i < numShapes; i++)
		{
			Batch& currentBatch = batches.back();

			HTexture texture;
			if(allShapes[i].MeshType == MeshType::Text)
			{
				TextRenderData& renderData = textRenderData[allShapes[i].TextIdx];
				texture = renderData.TextGeometry->GetTextureForPage(renderData.Page);
			}

			bool startNewBatch = allShapes[i].MeshType != currentBatch.Type || texture != currentBatch.Texture;
			if(startNewBatch)
			{
				currentBatch.EndIdx = i - 1;

				batches.push_back(Batch());

				Batch& newBatch = batches.back();
				newBatch.StartIdx = i;
				newBatch.Type = allShapes[i].MeshType;
				newBatch.NumVertices = allShapes[i].NumVertices;
				newBatch.NumIndices = allShapes[i].NumIndices;
				newBatch.Texture = texture;
			}
			else
			{
				currentBatch.EndIdx = i;
				currentBatch.NumVertices += allShapes[i].NumVertices;
				currentBatch.NumIndices += allShapes[i].NumIndices;
			}
		}

		{
			Batch& currentBatch = batches.back();
			currentBatch.EndIdx = numShapes - 1;
		}
	}

	// Allocate space for all the batch vertices/indices, per type
	u32 vertexCount[4] = { 0, 0, 0, 0 };
	u32 indexCount[4] = { 0, 0, 0, 0 };
	for(auto& batch : batches)
	{
		u32 typeIdx = (u32)batch.Type;

		vertexCount[typeIdx] += batch.NumVertices;
		indexCount[typeIdx] += batch.NumIndices;
	}

	SPtr<VertexDescription> vertexDesc[4] = { mSolidVertexDesc, mWireVertexDesc, mLineVertexDesc, mTextVertexDesc };
	SPtr<MeshData> meshData[4];
	for(u32 i = 0; i < 4; i++)
	{
		if(vertexCount[i] > 0 && indexCount[i] > 0)
			meshData[i] = MeshData::Create(vertexCount[i], indexCount[i], vertexDesc[i]);
	}

	/************************************************************************/
	/* 					Generate geometry for each batch                    */
	/************************************************************************/
	u32 vertexOffset[4] = { 0, 0, 0, 0 };
	u32 indexOffset[4] = { 0, 0, 0, 0 };

	VertexElemIterator<Vector3> positionIter[4];
	VertexElemIterator<u32> colorIter[4];

	for(u32 i = 0; i < 4; i++)
	{
		if(!meshData[i])
			continue;

		positionIter[i] = meshData[i]->GetVec3DataIter(VES_POSITION);
		colorIter[i] = meshData[i]->GetDwordDataIter(VES_COLOR);
	}

	VertexElemIterator<Vector3> solidNormalIter;
	if(meshData[0])
		solidNormalIter = meshData[0]->GetVec3DataIter(VES_NORMAL);

	VertexElemIterator<Vector2> textUVIter;

	if(meshData[3])
		textUVIter = meshData[3]->GetVec2DataIter(VES_TEXCOORD);

	for(auto& batch : batches)
	{
		u32 typeIdx = (u32)batch.Type;

		if(batch.Type == MeshType::Solid)
		{
			meshInfos.push_back(ShapeMeshData());
			ShapeMeshData& newMesh = meshInfos.back();
			newMesh.SubMesh.IndexOffset = indexOffset[typeIdx];
			newMesh.SubMesh.IndexCount = batch.NumIndices;
			newMesh.SubMesh.DrawOp = DOT_TRIANGLE_LIST;
			newMesh.Type = MeshType::Solid;

			for(u32 i = batch.StartIdx; i <= batch.EndIdx; i++)
			{
				RawData& shapeData = allShapes[i];

				Matrix4* transform = nullptr;
				RGBA color = 0;

				switch(shapeData.ShapeType)
				{
				case ShapeType::Cube:
					{
						CubeData& cubeData = mSolidCubeData[shapeData.Idx];
						AABox box(cubeData.Position - cubeData.Extents, cubeData.Position + cubeData.Extents);
						ShapeMeshes3D::SolidAaBox(box, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx]);

						transform = &cubeData.Transform;
						color = cubeData.Color.GetAsRgba();
					}
					break;
				case ShapeType::Sphere:
					{
						SphereData& sphereData = mSolidSphereData[shapeData.Idx];
						TSphere<float> sphere(sphereData.Position, sphereData.Radius);
						ShapeMeshes3D::SolidSphere(sphere, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx], sphereData.Quality);

						transform = &sphereData.Transform;
						color = sphereData.Color.GetAsRgba();
					}
					break;
				case ShapeType::Cone:
					{
						ConeData& coneData = mConeData[shapeData.Idx];
						ShapeMeshes3D::SolidCone(coneData.Base, coneData.Normal, coneData.Height, coneData.Radius, coneData.Scale, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx], coneData.Quality);

						transform = &coneData.Transform;
						color = coneData.Color.GetAsRgba();
					}
					break;
				case ShapeType::Disc:
					{
						DiscData& discData = mDiscData[shapeData.Idx];
						ShapeMeshes3D::SolidDisc(discData.Position, discData.Radius, discData.Normal, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx], discData.Quality);

						transform = &discData.Transform;
						color = discData.Color.GetAsRgba();
					}
					break;
				case ShapeType::Arc:
					{
						ArcData& arcData = mArcData[shapeData.Idx];
						ShapeMeshes3D::SolidArc(arcData.Position, arcData.Radius, arcData.Normal, arcData.StartAngle, arcData.AmountAngle, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx], arcData.Quality);

						transform = &arcData.Transform;
						color = arcData.Color.GetAsRgba();
					}
					break;
				case ShapeType::Rectangle:
					{
						Rect3Data& rectData = mRect3Data[shapeData.Idx];
						ShapeMeshes3D::SolidQuad(rectData.Area, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx]);

						transform = &rectData.Transform;
						color = rectData.Color.GetAsRgba();
					}
					break;
				default:
					break;
				}

				Matrix4 transformIT = transform->InverseAffine().Transpose();
				for(u32 i = 0; i < shapeData.NumVertices; i++)
				{
					Vector3 worldPos = transform->MultiplyAffine(positionIter[typeIdx].GetValue());
					Vector3 worldNormal = transformIT.MultiplyAffine(solidNormalIter.GetValue());

					positionIter[typeIdx].AddValue(worldPos);
					solidNormalIter.AddValue(worldNormal);
					colorIter[typeIdx].AddValue(color);
				}

				vertexOffset[typeIdx] += shapeData.NumVertices;
				indexOffset[typeIdx] += shapeData.NumIndices;
			}
		}
		else if(batch.Type == MeshType::Wire)
		{
			meshInfos.push_back(ShapeMeshData());
			ShapeMeshData& newMesh = meshInfos.back();
			newMesh.SubMesh.IndexOffset = indexOffset[typeIdx];
			newMesh.SubMesh.IndexCount = batch.NumIndices;
			newMesh.SubMesh.DrawOp = DOT_TRIANGLE_LIST;
			newMesh.Type = MeshType::Wire;

			for(u32 i = batch.StartIdx; i <= batch.EndIdx; i++)
			{
				RawData& shapeData = allShapes[i];

				Matrix4* transform = nullptr;
				RGBA color = 0;

				switch(shapeData.ShapeType)
				{
				case ShapeType::WireMesh:
					{
						WireMeshData& wireMeshData = mWireMeshData[shapeData.Idx];

						transform = &wireMeshData.Transform;
						color = wireMeshData.Color.GetAsRgba();

						auto vertIterRead = wireMeshData.MeshData->GetVec3DataIter(VES_POSITION);
						for(u32 j = 0; j < vertIterRead.GetNumElements(); j++)
						{
							Vector3 worldPos = transform->MultiplyAffine(vertIterRead.GetValue());

							positionIter[typeIdx].AddValue(worldPos);
							colorIter[typeIdx].AddValue(color);

							vertIterRead.MoveNext();
						}

						u32* srcIndexData = wireMeshData.MeshData->GetIndices32();
						u32* destIndexData = meshData[typeIdx]->GetIndices32() + indexOffset[typeIdx];

						for(u32 j = 0; j < shapeData.NumIndices; j++)
							destIndexData[j] = srcIndexData[j] + vertexOffset[typeIdx];

						vertexOffset[typeIdx] += shapeData.NumVertices;
						indexOffset[typeIdx] += shapeData.NumIndices;
					}
					break;
				default:
					break;
				}
			}
		}
		else if(batch.Type == MeshType::Line)
		{
			meshInfos.push_back(ShapeMeshData());
			ShapeMeshData& newMesh = meshInfos.back();
			newMesh.SubMesh.IndexOffset = indexOffset[typeIdx];
			newMesh.SubMesh.IndexCount = batch.NumIndices;
			newMesh.SubMesh.DrawOp = DOT_LINE_LIST;
			newMesh.Type = MeshType::Line;

			for(u32 i = batch.StartIdx; i <= batch.EndIdx; i++)
			{
				RawData& shapeData = allShapes[i];

				Matrix4* transform = nullptr;
				RGBA color = 0;

				switch(shapeData.ShapeType)
				{
				case ShapeType::WireCube:
					{
						CubeData& cubeData = mWireCubeData[shapeData.Idx];

						AABox box(cubeData.Position - cubeData.Extents, cubeData.Position + cubeData.Extents);
						ShapeMeshes3D::WireAaBox(box, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx]);

						transform = &cubeData.Transform;
						color = cubeData.Color.GetAsRgba();
					}
					break;
				case ShapeType::WireSphere:
					{
						SphereData& sphereData = mWireSphereData[shapeData.Idx];

						TSphere<float> sphere(sphereData.Position, sphereData.Radius);
						ShapeMeshes3D::WireSphere(sphere, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx], sphereData.Quality);

						transform = &sphereData.Transform;
						color = sphereData.Color.GetAsRgba();
					}
					break;
				case ShapeType::WireHemisphere:
					{
						SphereData& sphereData = mWireHemisphereData[shapeData.Idx];

						TSphere<float> sphere(sphereData.Position, sphereData.Radius);
						ShapeMeshes3D::WireHemisphere(sphere, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx], sphereData.Quality);

						transform = &sphereData.Transform;
						color = sphereData.Color.GetAsRgba();
					}
					break;
				case ShapeType::WireCone:
					{
						ConeData& coneData = mWireConeData[shapeData.Idx];
						ShapeMeshes3D::WireCone(coneData.Base, coneData.Normal, coneData.Height, coneData.Radius, coneData.Scale, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx], coneData.Quality);

						transform = &coneData.Transform;
						color = coneData.Color.GetAsRgba();
					}
					break;
				case ShapeType::Line:
					{
						LineData& lineData = mLineData[shapeData.Idx];

						ShapeMeshes3D::PixelLine(lineData.Start, lineData.End, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx]);

						transform = &lineData.Transform;
						color = lineData.Color.GetAsRgba();
					}
					break;
				case ShapeType::LineList:
					{
						LineListData& lineListData = mLineListData[shapeData.Idx];

						ShapeMeshes3D::PixelLineList(lineListData.Lines, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx]);

						transform = &lineListData.Transform;
						color = lineListData.Color.GetAsRgba();
					}
					break;
				case ShapeType::Frustum:
					{
						FrustumData& frustumData = mFrustumData[shapeData.Idx];

						ShapeMeshes3D::WireFrustum(frustumData.Position, frustumData.Aspect, frustumData.FOV, frustumData.NearDist, frustumData.FarDist, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx]);

						transform = &frustumData.Transform;
						color = frustumData.Color.GetAsRgba();
					}
					break;
				case ShapeType::WireDisc:
					{
						DiscData& discData = mWireDiscData[shapeData.Idx];

						ShapeMeshes3D::WireDisc(discData.Position, discData.Radius, discData.Normal, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx], discData.Quality);

						transform = &discData.Transform;
						color = discData.Color.GetAsRgba();
					}
					break;
				case ShapeType::WireArc:
					{
						ArcData& arcData = mWireArcData[shapeData.Idx];

						ShapeMeshes3D::WireArc(arcData.Position, arcData.Radius, arcData.Normal, arcData.StartAngle, arcData.AmountAngle, meshData[typeIdx], vertexOffset[typeIdx], indexOffset[typeIdx], arcData.Quality);

						transform = &arcData.Transform;
						color = arcData.Color.GetAsRgba();
					}
					break;
				default:
					break;
				}

				for(u32 i = 0; i < shapeData.NumVertices; i++)
				{
					Vector3 worldPos = transform->MultiplyAffine(positionIter[typeIdx].GetValue());

					positionIter[typeIdx].AddValue(worldPos);
					colorIter[typeIdx].AddValue(color);
				}

				vertexOffset[typeIdx] += shapeData.NumVertices;
				indexOffset[typeIdx] += shapeData.NumIndices;
			}
		}
		else // Text
		{
			// Text drawing requires a camera in order to determine screen space coordinates
			if(!camera)
				continue;

			meshInfos.push_back(ShapeMeshData());
			ShapeMeshData& newMesh = meshInfos.back();
			newMesh.SubMesh.IndexOffset = indexOffset[typeIdx];
			newMesh.SubMesh.IndexCount = batch.NumIndices;
			newMesh.SubMesh.DrawOp = DOT_TRIANGLE_LIST;
			newMesh.Type = MeshType::Text;
			newMesh.Texture = batch.Texture;

			for(u32 i = batch.StartIdx; i <= batch.EndIdx; i++)
			{
				RawData& shapeData = allShapes[i];
				Text2DData& text2DData = mText2DData[shapeData.Idx];

				TextRenderData& renderData = textRenderData[shapeData.TextIdx];
				u32 numQuads = renderData.TextGeometry->GetQuadCount(renderData.Page);

				u32* indices = meshData[typeIdx]->GetIndices32() + indexOffset[typeIdx];

				// Note: Need temporary buffers because TextLine doesn't support arbitrary vertex stride. Eventually
				// that should be supported (should be almost trivial to implement)
				Vector2* tempVertices = B3DStackAllocate<Vector2>(shapeData.NumVertices);
				Vector2* tempUVs = B3DStackAllocate<Vector2>(shapeData.NumVertices);

				u32 numLines = renderData.TextGeometry->GetLineCount();
				u32 quadOffset = 0;
				for(u32 j = 0; j < numLines; j++)
				{
					const TextGeometry::Line& line = renderData.TextGeometry->GetLine(j);
					u32 writtenQuads = line.FillBuffer(renderData.Page, tempVertices, tempUVs, indices, quadOffset, numQuads);

					quadOffset += writtenQuads;
				}

				for(u32 j = 0; j < shapeData.NumIndices; j++)
					indices[j] += vertexOffset[typeIdx];

				Vector3 worldSpacePos = text2DData.Transform.MultiplyAffine(text2DData.Position);
				Vector2I screenPos = camera->WorldToScreenPoint(worldSpacePos);
				screenPos.X -= Math::RoundToI32(renderData.TextGeometry->GetWidth() / 2.0f);
				screenPos.Y -= Math::RoundToI32(renderData.TextGeometry->GetHeight() / 2.0f);

				float z = camera->ProjectPoint(camera->WorldToViewPoint(worldSpacePos)).Z;

				for(u32 j = 0; j < shapeData.NumVertices; j++)
				{
					Vector3 vertexPos(screenPos.X + tempVertices[j].X, screenPos.Y + tempVertices[j].Y, z);

					positionIter[typeIdx].AddValue(vertexPos);
					textUVIter.AddValue(tempUVs[j]);
					colorIter[typeIdx].AddValue(text2DData.Color.GetAsRgba());
				}

				B3DStackFree(tempUVs);
				B3DStackFree(tempVertices);

				vertexOffset[typeIdx] += shapeData.NumVertices;
				indexOffset[typeIdx] += shapeData.NumIndices;
			}
		}
	}

	SPtr<Mesh> meshes[4];
	for(u32 i = 0; i < 4; i++)
	{
		if(meshData[i])
			meshes[i] = Mesh::CreateShared(meshData[i]);
	}

	for(auto& entry : meshInfos)
	{
		switch(entry.Type)
		{
		case MeshType::Solid:
			entry.Mesh = meshes[0];
			break;
		case MeshType::Wire:
			entry.Mesh = meshes[1];
			break;
		case MeshType::Line:
			entry.Mesh = meshes[2];
			break;
		case MeshType::Text:
			entry.Mesh = meshes[3];
			break;
		default:;
		}
	}

	return meshInfos;
}
