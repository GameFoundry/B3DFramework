//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DShapeMeshes3D.h"
#include "Math/B3DArea2.h"
#include "Mesh/B3DMesh.h"
#include "Math/B3DVector2.h"
#include "Math/B3DQuaternion.h"
#include "Math/B3DSphere.h"
#include "Material/B3DPass.h"
#include "Components/B3DCamera.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Mesh/B3DMeshUtility.h"

using namespace b3d;

const u32 ShapeMeshes3D::kNumVerticesAaLine = 8;
const u32 ShapeMeshes3D::kNumIndicesAaLine = 30;

inline u8* WriteVector3(u8* buffer, u32 stride, const Vector3& value)
{
	*(Vector3*)buffer = value;
	return buffer + stride;
}

inline u8* WriteVector2(u8* buffer, u32 stride, const Vector2& value)
{
	*(Vector2*)buffer = value;
	return buffer + stride;
}

void ShapeMeshes3D::WireAaBox(const AABox& box, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset)
{
	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);

	B3D_ASSERT((vertexOffset + 8) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + 24) <= meshData->GetIndexCount());

	WireAaBox(box, positionData, vertexOffset, meshData->GetVertexDescription()->GetVertexStride(), indexData, indexOffset);
}

void ShapeMeshes3D::SolidAaBox(const AABox& box, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset)
{
	const SPtr<VertexDescription>& desc = meshData->GetVertexDescription();

	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);
	u8* normalData = meshData->GetElementData(VES_NORMAL);

	u32 numVertices = meshData->GetVertexCount();
	u32 numIndices = meshData->GetIndexCount();
	u32 vertexStride = desc->GetVertexStride();

	B3D_ASSERT((vertexOffset + 24) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + 36) <= meshData->GetIndexCount());

	u8* uvData = nullptr;
	if(desc->HasElement(VES_TEXCOORD))
		uvData = meshData->GetElementData(VES_TEXCOORD);

	SolidAaBox(box, positionData, normalData, uvData, vertexOffset, vertexStride, indexData, indexOffset);

	if(uvData != nullptr && desc->HasElement(VES_TANGENT))
	{
		u8* tangentData = meshData->GetElementData(VES_TANGENT);
		GenerateTangents(positionData, normalData, uvData, indexData, numVertices, numIndices, vertexOffset, indexOffset, vertexStride, tangentData);
	}
}

void ShapeMeshes3D::WireSphere(const Sphere& sphere, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality)
{
	u32 requiredNumVertices, requiredNumIndices;
	GetNumElementsWireSphere(quality, requiredNumVertices, requiredNumIndices);

	B3D_ASSERT((vertexOffset + requiredNumVertices) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + requiredNumIndices) <= meshData->GetIndexCount());

	u32 verticesPerArc = (quality + 1) * 5;
	u32 indicesPerArc = (verticesPerArc - 1) * 2;

	WireDisc(sphere.Center, sphere.Radius, Vector3::kUnitX, meshData, vertexOffset, indexOffset, quality);

	WireDisc(sphere.Center, sphere.Radius, Vector3::kUnitY, meshData, vertexOffset + verticesPerArc, indexOffset + indicesPerArc, quality);

	WireDisc(sphere.Center, sphere.Radius, Vector3::kUnitZ, meshData, vertexOffset + verticesPerArc * 2, indexOffset + indicesPerArc * 2, quality);
}

void ShapeMeshes3D::WireHemisphere(const Sphere& sphere, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality)
{
	u32 requiredNumVertices, requiredNumIndices;
	GetNumElementsWireHemisphere(quality, requiredNumVertices, requiredNumIndices);

	B3D_ASSERT((vertexOffset + requiredNumVertices) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + requiredNumIndices) <= meshData->GetIndexCount());

	u32 verticesPerArc = (quality + 1) * 5;
	u32 indicesPerArc = (verticesPerArc - 1) * 2;

	WireArc(sphere.Center, sphere.Radius, Vector3::kUnitX, Degree(0.0f), Degree(180.0f), meshData, vertexOffset, indexOffset, quality);

	WireArc(sphere.Center, sphere.Radius, Vector3::kUnitY, Degree(0.0f), Degree(180.0f), meshData, vertexOffset + verticesPerArc, indexOffset + indicesPerArc, quality);

	WireDisc(sphere.Center, sphere.Radius, Vector3::kUnitZ, meshData, vertexOffset + verticesPerArc * 2, indexOffset + indicesPerArc * 2, quality);
}

void ShapeMeshes3D::SolidSphere(const Sphere& sphere, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality)
{
	const SPtr<VertexDescription>& desc = meshData->GetVertexDescription();

	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);
	u8* normalData = meshData->GetElementData(VES_NORMAL);

	u32 numVertices = meshData->GetVertexCount();
	u32 numIndices = meshData->GetIndexCount();
	u32 vertexStride = desc->GetVertexStride();

	u32 requiredNumVertices, requiredNumIndices;
	GetNumElementsSphere(quality, requiredNumVertices, requiredNumIndices);

	B3D_ASSERT((vertexOffset + requiredNumVertices) <= numVertices);
	B3D_ASSERT((indexOffset + requiredNumIndices) <= numIndices);

	u8* uvData = nullptr;
	if(desc->HasElement(VES_TEXCOORD))
		uvData = meshData->GetElementData(VES_TEXCOORD);

	SolidSphere(sphere, positionData, normalData, uvData, vertexOffset, vertexStride, indexData, indexOffset, quality);

	if(uvData != nullptr && desc->HasElement(VES_TANGENT))
	{
		u8* tangentData = meshData->GetElementData(VES_TANGENT);
		GenerateTangents(positionData, normalData, uvData, indexData, numVertices, numIndices, vertexOffset, indexOffset, vertexStride, tangentData);
	}
}

void ShapeMeshes3D::WireDisc(const Vector3& center, float radius, const Vector3& normal, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality)
{
	WireArc(center, radius, normal, Degree(0), Degree(360), meshData, vertexOffset, indexOffset, quality);
}

void ShapeMeshes3D::SolidDisc(const Vector3& center, float radius, const Vector3& normal, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality)
{
	SolidArc(center, radius, normal, Degree(0), Degree(360), meshData, vertexOffset, indexOffset, quality);
}

void ShapeMeshes3D::WireArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality)
{
	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);

	u32 requiredNumVertices, requiredNumIndices;
	GetNumElementsWireArc(quality, requiredNumVertices, requiredNumIndices);

	B3D_ASSERT((vertexOffset + requiredNumVertices) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + requiredNumIndices) <= meshData->GetIndexCount());

	WireArc(center, radius, normal, startAngle, amountAngle, positionData, vertexOffset, meshData->GetVertexDescription()->GetVertexStride(), indexData, indexOffset, quality);
}

void ShapeMeshes3D::SolidArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality)
{
	const SPtr<VertexDescription>& desc = meshData->GetVertexDescription();

	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);
	u8* normalData = meshData->GetElementData(VES_NORMAL);

	u32 numVertices = meshData->GetVertexCount();
	u32 numIndices = meshData->GetIndexCount();
	u32 vertexStride = desc->GetVertexStride();

	u32 requiredNumVertices, requiredNumIndices;
	GetNumElementsArc(quality, requiredNumVertices, requiredNumIndices);

	B3D_ASSERT((vertexOffset + requiredNumVertices) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + requiredNumIndices) <= meshData->GetIndexCount());

	u8* uvData = nullptr;
	if(desc->HasElement(VES_TEXCOORD))
		uvData = meshData->GetElementData(VES_TEXCOORD);

	SolidArc(center, radius, normal, startAngle, amountAngle, positionData, normalData, uvData, vertexOffset, vertexStride, indexData, indexOffset, quality);

	if(uvData != nullptr && desc->HasElement(VES_TANGENT))
	{
		u8* tangentData = meshData->GetElementData(VES_TANGENT);
		GenerateTangents(positionData, normalData, uvData, indexData, numVertices, numIndices, vertexOffset, indexOffset, vertexStride, tangentData);
	}
}

void ShapeMeshes3D::WireFrustum(const Vector3& position, float aspect, Degree FOV, float near, float far, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset)
{
	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);

	B3D_ASSERT((vertexOffset + 8) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + 24) <= meshData->GetIndexCount());

	WireFrustum(position, aspect, FOV, near, far, positionData, vertexOffset, meshData->GetVertexDescription()->GetVertexStride(), indexData, indexOffset);
}

void ShapeMeshes3D::SolidCone(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality)
{
	const SPtr<VertexDescription>& desc = meshData->GetVertexDescription();

	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);
	u8* normalData = meshData->GetElementData(VES_NORMAL);

	u32 numVertices = meshData->GetVertexCount();
	u32 numIndices = meshData->GetIndexCount();
	u32 vertexStride = desc->GetVertexStride();

	u32 requiredNumVertices, requiredNumIndices;
	GetNumElementsCone(quality, requiredNumVertices, requiredNumIndices);

	B3D_ASSERT((vertexOffset + requiredNumVertices) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + requiredNumIndices) <= meshData->GetIndexCount());

	u8* uvData = nullptr;
	if(desc->HasElement(VES_TEXCOORD))
		uvData = meshData->GetElementData(VES_TEXCOORD);

	SolidCone(base, normal, height, radius, scale, positionData, normalData, uvData, vertexOffset, vertexStride, indexData, indexOffset, quality);

	if(uvData != nullptr && desc->HasElement(VES_TANGENT))
	{
		u8* tangentData = meshData->GetElementData(VES_TANGENT);
		GenerateTangents(positionData, normalData, uvData, indexData, numVertices, numIndices, vertexOffset, indexOffset, vertexStride, tangentData);
	}
}

void ShapeMeshes3D::WireCone(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality)
{
	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);

	u32 requiredNumVertices, requiredNumIndices;
	GetNumElementsWireCone(quality, requiredNumVertices, requiredNumIndices);

	B3D_ASSERT((vertexOffset + requiredNumVertices) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + requiredNumIndices) <= meshData->GetIndexCount());

	WireCone(base, normal, height, radius, scale, positionData, vertexOffset, meshData->GetVertexDescription()->GetVertexStride(), indexData, indexOffset, quality);
}

void ShapeMeshes3D::SolidCylinder(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality)
{
	const SPtr<VertexDescription>& desc = meshData->GetVertexDescription();

	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);
	u8* normalData = meshData->GetElementData(VES_NORMAL);

	u32 numVertices = meshData->GetVertexCount();
	u32 numIndices = meshData->GetIndexCount();
	u32 vertexStride = desc->GetVertexStride();

	u32 requiredNumVertices, requiredNumIndices;
	GetNumElementsCylinder(quality, requiredNumVertices, requiredNumIndices);

	B3D_ASSERT((vertexOffset + requiredNumVertices) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + requiredNumIndices) <= meshData->GetIndexCount());

	u8* uvData = nullptr;
	if(desc->HasElement(VES_TEXCOORD))
		uvData = meshData->GetElementData(VES_TEXCOORD);

	SolidCylinder(base, normal, height, radius, scale, positionData, normalData, uvData, vertexOffset, vertexStride, indexData, indexOffset, quality);

	if(uvData != nullptr && desc->HasElement(VES_TANGENT))
	{
		u8* tangentData = meshData->GetElementData(VES_TANGENT);
		GenerateTangents(positionData, normalData, uvData, indexData, numVertices, numIndices, vertexOffset, indexOffset, vertexStride, tangentData);
	}
}

void ShapeMeshes3D::WireCylinder(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality)
{
	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);

	u32 requiredNumVertices, requiredNumIndices;
	GetNumElementsWireCylinder(quality, requiredNumVertices, requiredNumIndices);

	B3D_ASSERT((vertexOffset + requiredNumVertices) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + requiredNumIndices) <= meshData->GetIndexCount());

	WireCylinder(base, normal, height, radius, scale, positionData, vertexOffset, meshData->GetVertexDescription()->GetVertexStride(), indexData, indexOffset, quality);
}

void ShapeMeshes3D::SolidQuad(const Rect3& area, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset)
{
	const SPtr<VertexDescription>& desc = meshData->GetVertexDescription();

	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);
	u8* normalData = meshData->GetElementData(VES_NORMAL);

	u32 numVertices = meshData->GetVertexCount();
	u32 numIndices = meshData->GetIndexCount();
	u32 vertexStride = desc->GetVertexStride();

	B3D_ASSERT((vertexOffset + 8) <= numVertices);
	B3D_ASSERT((indexOffset + 12) <= numIndices);

	u8* uvData = nullptr;
	if(desc->HasElement(VES_TEXCOORD))
		uvData = meshData->GetElementData(VES_TEXCOORD);

	SolidQuad(area, positionData, normalData, uvData, vertexOffset, meshData->GetVertexDescription()->GetVertexStride(), indexData, indexOffset);

	if(uvData != nullptr && desc->HasElement(VES_TANGENT))
	{
		u8* tangentData = meshData->GetElementData(VES_TANGENT);
		GenerateTangents(positionData, normalData, uvData, indexData, numVertices, numIndices, vertexOffset, indexOffset, vertexStride, tangentData);
	}
}

void ShapeMeshes3D::PixelLine(const Vector3& a, const Vector3& b, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset)
{
	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);

	B3D_ASSERT((vertexOffset + 2) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + 2) <= meshData->GetIndexCount());

	PixelLine(a, b, positionData, vertexOffset, meshData->GetVertexDescription()->GetVertexStride(), indexData, indexOffset);
}

void ShapeMeshes3D::PixelLineList(const Vector<Vector3>& linePoints, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset)
{
	B3D_ASSERT(linePoints.size() % 2 == 0);

	B3D_ASSERT((vertexOffset + linePoints.size()) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + linePoints.size()) <= meshData->GetIndexCount());

	u32 curVertOffset = vertexOffset;
	u32 curIdxOffset = indexOffset;

	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);

	u32 numPoints = (u32)linePoints.size();
	for(u32 i = 0; i < numPoints; i += 2)
	{
		PixelLine(linePoints[i], linePoints[i + 1], positionData, curVertOffset, meshData->GetVertexDescription()->GetVertexStride(), indexData, curIdxOffset);

		curVertOffset += 2;
		curIdxOffset += 2;
	}
}

void ShapeMeshes3D::AntialiasedLine(const Vector3& a, const Vector3& b, const Vector3& up, float width, float borderWidth, const Color& color, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset)
{
	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);
	u8* colorData = meshData->GetElementData(VES_COLOR);

	B3D_ASSERT((vertexOffset + kNumVerticesAaLine) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + kNumIndicesAaLine) <= meshData->GetIndexCount());

	AntialiasedLine(a, b, up, width, borderWidth, color, positionData, colorData, vertexOffset, meshData->GetVertexDescription()->GetVertexStride(), indexData, indexOffset);
}

void ShapeMeshes3D::AntialiasedLineList(const Vector<Vector3>& linePoints, const Vector3& up, float width, float borderWidth, const Color& color, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset)
{
	B3D_ASSERT(linePoints.size() % 2 == 0);

	B3D_ASSERT((vertexOffset + linePoints.size() * 4) <= meshData->GetVertexCount());
	B3D_ASSERT((indexOffset + linePoints.size() * 15) <= meshData->GetIndexCount());

	u32 curVertOffset = vertexOffset;
	u32 curIdxOffset = indexOffset;

	u32* indexData = meshData->GetIndices32();
	u8* positionData = meshData->GetElementData(VES_POSITION);
	u8* colorData = meshData->GetElementData(VES_COLOR);

	u32 numPoints = (u32)linePoints.size();
	for(u32 i = 0; i < numPoints; i += 2)
	{
		AntialiasedLine(linePoints[i], linePoints[i + 1], up, width, borderWidth, color, positionData, colorData, curVertOffset, meshData->GetVertexDescription()->GetVertexStride(), indexData, curIdxOffset);

		curVertOffset += kNumVerticesAaLine;
		curIdxOffset += kNumIndicesAaLine;
	}
}

/************************************************************************/
/* 								ELEMENT COUNT                      		*/
/************************************************************************/

void ShapeMeshes3D::GetNumElementsAaBox(u32& numVertices, u32& numIndices)
{
	numVertices = 24;
	numIndices = 36;
}

void ShapeMeshes3D::GetNumElementsWireAaBox(u32& numVertices, u32& numIndices)
{
	numVertices = 8;
	numIndices = 24;
}

void ShapeMeshes3D::GetNumElementsSphere(u32 quality, u32& numVertices, u32& numIndices)
{
	numVertices = 20 * (3 * ((u32)std::pow(4, quality)));
	numIndices = numVertices;

	// Extra for the seam fix. 4 initial triangles each split 4 times per quality level, one vertex each
	numVertices += (3 * (u32)pow(4, quality));
}

void ShapeMeshes3D::GetNumElementsWireSphere(u32 quality, u32& numVertices, u32& numIndices)
{
	GetNumElementsWireArc(quality, numVertices, numIndices);
	numVertices *= 3;
	numIndices *= 3;
}

void ShapeMeshes3D::GetNumElementsWireHemisphere(u32 quality, u32& numVertices, u32& numIndices)
{
	GetNumElementsWireArc(quality, numVertices, numIndices);
	numVertices *= 3;
	numIndices *= 3;
}

void ShapeMeshes3D::GetNumElementsArc(u32 quality, u32& numVertices, u32& numIndices)
{
	numVertices = ((quality + 1) * 5 + 1) * 2;
	numIndices = ((quality + 1) * 5) * 6;
}

void ShapeMeshes3D::GetNumElementsWireArc(u32 quality, u32& numVertices, u32& numIndices)
{
	numVertices = (quality + 1) * 5;
	numIndices = ((quality + 1) * 5 - 1) * 2;
}

void ShapeMeshes3D::GetNumElementsDisc(u32 quality, u32& numVertices, u32& numIndices)
{
	GetNumElementsArc(quality, numVertices, numIndices);
}

void ShapeMeshes3D::GetNumElementsWireDisc(u32 quality, u32& numVertices, u32& numIndices)
{
	GetNumElementsWireArc(quality, numVertices, numIndices);
}

void ShapeMeshes3D::GetNumElementsCone(u32 quality, u32& numVertices, u32& numIndices)
{
	numVertices = ((quality + 1) * 4) * 3 + 1;
	numIndices = ((quality + 1) * 4) * 6;
}

void ShapeMeshes3D::GetNumElementsWireCone(u32 quality, u32& numVertices, u32& numIndices)
{
	numVertices = (quality + 1) * 4 + 5;
	numIndices = ((quality + 1) * 4 + 4) * 2;
}

void ShapeMeshes3D::GetNumElementsCylinder(u32 quality, u32& numVertices, u32& numIndices)
{
	numVertices = ((quality + 1) * 4 + 1) * 4;
	numIndices = ((quality + 1) * 4) * 12;
}

void ShapeMeshes3D::GetNumElementsWireCylinder(u32 quality, u32& numVertices, u32& numIndices)
{
	numVertices = ((quality + 1) * 4) * 2;
	numIndices = ((quality + 1) * 4) * 6;
}

void ShapeMeshes3D::GetNumElementsQuad(u32& numVertices, u32& numIndices)
{
	numVertices = 8;
	numIndices = 12;
}

void ShapeMeshes3D::GetNumElementsFrustum(u32& numVertices, u32& numIndices)
{
	numVertices = 8;
	numIndices = 36;
}

void ShapeMeshes3D::WireAaBox(const AABox& box, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset)
{
	outVertices += vertexOffset * vertexStride;

	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_LEFT_BOTTOM));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_RIGHT_BOTTOM));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_RIGHT_TOP));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_LEFT_TOP));

	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_RIGHT_BOTTOM));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_LEFT_BOTTOM));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_LEFT_TOP));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_RIGHT_TOP));

	outIndices += indexOffset;

	// Front
	outIndices[0] = vertexOffset + 0;
	outIndices[1] = vertexOffset + 1;

	outIndices[2] = vertexOffset + 1;
	outIndices[3] = vertexOffset + 2;

	outIndices[4] = vertexOffset + 2;
	outIndices[5] = vertexOffset + 3;

	outIndices[6] = vertexOffset + 3;
	outIndices[7] = vertexOffset + 0;

	// Center
	outIndices[8] = vertexOffset + 0;
	outIndices[9] = vertexOffset + 5;

	outIndices[10] = vertexOffset + 1;
	outIndices[11] = vertexOffset + 4;

	outIndices[12] = vertexOffset + 2;
	outIndices[13] = vertexOffset + 7;

	outIndices[14] = vertexOffset + 3;
	outIndices[15] = vertexOffset + 6;

	// Back
	outIndices[16] = vertexOffset + 4;
	outIndices[17] = vertexOffset + 5;

	outIndices[18] = vertexOffset + 5;
	outIndices[19] = vertexOffset + 6;

	outIndices[20] = vertexOffset + 6;
	outIndices[21] = vertexOffset + 7;

	outIndices[22] = vertexOffset + 7;
	outIndices[23] = vertexOffset + 4;
}

void ShapeMeshes3D::SolidAaBox(const AABox& box, u8* outVertices, u8* outNormals, u8* outUVs, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset)
{
	outVertices += (vertexOffset * vertexStride);

	// Front face
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_LEFT_BOTTOM));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_RIGHT_BOTTOM));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_RIGHT_TOP));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_LEFT_TOP));

	// Back face
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_RIGHT_BOTTOM));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_LEFT_BOTTOM));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_LEFT_TOP));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_RIGHT_TOP));

	// Left face
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_LEFT_BOTTOM));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_LEFT_BOTTOM));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_LEFT_TOP));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_LEFT_TOP));

	// Right face
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_RIGHT_BOTTOM));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_RIGHT_BOTTOM));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_RIGHT_TOP));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_RIGHT_TOP));

	// Top face
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_LEFT_TOP));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_LEFT_TOP));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_RIGHT_TOP));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_RIGHT_TOP));

	// Bottom face
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_LEFT_BOTTOM));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::FAR_RIGHT_BOTTOM));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_RIGHT_BOTTOM));
	outVertices = WriteVector3(outVertices, vertexStride, box.GetCorner(AABox::NEAR_LEFT_BOTTOM));

	// Normals
	static const Vector3 kFaceNormals[6] = {
		Vector3(0, 0, 1),
		Vector3(0, 0, -1),
		Vector3(-1, 0, 0),
		Vector3(1, 0, 0),
		Vector3(0, 1, 0),
		Vector3(0, -1, 0)
	};

	if(outNormals != nullptr)
	{
		outNormals += (vertexOffset * vertexStride);
		for(u32 face = 0; face < 6; face++)
		{
			outNormals = WriteVector3(outNormals, vertexStride, kFaceNormals[face]);
			outNormals = WriteVector3(outNormals, vertexStride, kFaceNormals[face]);
			outNormals = WriteVector3(outNormals, vertexStride, kFaceNormals[face]);
			outNormals = WriteVector3(outNormals, vertexStride, kFaceNormals[face]);
		}
	}

	// UV
	if(outUVs != nullptr)
	{
		outUVs += (vertexOffset * vertexStride);
		for(u32 face = 0; face < 6; face++)
		{
			outUVs = WriteVector2(outUVs, vertexStride, Vector2(0.0f, 1.0f));
			outUVs = WriteVector2(outUVs, vertexStride, Vector2(1.0f, 1.0f));
			outUVs = WriteVector2(outUVs, vertexStride, Vector2(1.0f, 0.0f));
			outUVs = WriteVector2(outUVs, vertexStride, Vector2(0.0f, 0.0f));
		}
	}

	// Indices
	u32* indices = outIndices + indexOffset;
	for(u32 face = 0; face < 6; face++)
	{
		u32 faceVertOffset = vertexOffset + face * 4;

		indices[face * 6 + 0] = faceVertOffset + 2;
		indices[face * 6 + 1] = faceVertOffset + 1;
		indices[face * 6 + 2] = faceVertOffset + 0;
		indices[face * 6 + 3] = faceVertOffset + 0;
		indices[face * 6 + 4] = faceVertOffset + 3;
		indices[face * 6 + 5] = faceVertOffset + 2;
	}
}

void ShapeMeshes3D::SolidSphere(const Sphere& sphere, u8* outVertices, u8* outNormals, u8* outUV, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset, u32 quality)
{
	// Create icosahedron
	static const float kX = 0.525731112119133606f;
	static const float kZ = 0.850650808352039932f;

	static const Vector3 kVertices[12] = {
		Vector3(-kX, 0.0f, kZ),
		Vector3(kX, 0.0f, kZ),
		Vector3(-kX, 0.0f, -kZ),
		Vector3(kX, 0.0f, -kZ),
		Vector3(0.0f, kZ, kX),
		Vector3(0.0f, kZ, -kX),
		Vector3(0.0f, -kZ, kX),
		Vector3(0.0f, -kZ, -kX),
		Vector3(kZ, kX, 0.0f),
		Vector3(-kZ, kX, 0.0f),
		Vector3(kZ, -kX, 0.0f),
		Vector3(-kZ, -kX, 0.0f)
	};

	static const u32 kTriangles[20][3] = {
		{ 0, 4, 1 }, { 0, 9, 4 }, { 9, 5, 4 }, { 4, 5, 8 }, { 4, 8, 1 }, { 8, 10, 1 }, { 8, 3, 10 }, { 5, 3, 8 }, { 5, 2, 3 }, { 2, 7, 3 }, { 7, 10, 3 }, { 7, 6, 10 }, { 7, 11, 6 }, { 11, 0, 6 }, { 0, 1, 6 }, { 6, 1, 10 }, { 9, 0, 11 }, { 9, 11, 2 }, { 9, 2, 5 }, { 7, 2, 11 }
	};

	// Tessellate it
	u32 curVertOffset = vertexOffset;
	for(int i = 0; i < 20; ++i)
	{
		curVertOffset += SubdivideTriangleOnSphere(sphere.Center, sphere.Radius, quality, kVertices[kTriangles[i][2]], kVertices[kTriangles[i][1]], kVertices[kTriangles[i][0]], outVertices, outNormals, curVertOffset, vertexStride);
	}

	// Create UV if required
	if(outUV != nullptr)
	{
		u32 numVertices = curVertOffset - vertexOffset;

		outUV += (vertexOffset * vertexStride);
		u8* curUV = outUV;
		for(u32 i = 0; i < numVertices; i++)
		{
			Vector3 position = *(Vector3*)&outVertices[(vertexOffset + i) * vertexStride];
			Vector3 normal = Vector3::Normalize(position);

			Vector2 uv;
			uv.X = 0.5f - atan2(normal.X, normal.Z) / Math::kTwoPi;
			uv.Y = 0.5f - asin(normal.Y) / Math::kPi;

			curUV = WriteVector2(curUV, vertexStride, uv);
		}
	}

	// Create indices
	outIndices += indexOffset;

	u32 numIndices = 20 * (3 * (u32)std::pow(4, quality));
	for(u32 i = 0; i < numIndices; i += 3)
	{
		outIndices[i] = vertexOffset + i + 2;
		outIndices[i + 1] = vertexOffset + i + 1;
		outIndices[i + 2] = vertexOffset + i + 0;
	}

	// Fix UV seams
	u8* extraPositions = outVertices + curVertOffset * vertexStride;

	u8* extraNormals = nullptr;
	if(outNormals)
		extraNormals = outNormals + curVertOffset * vertexStride;

	u8* extraUV = nullptr;
	if(outUV)
		extraUV = outUV + curVertOffset * vertexStride;

	const u32 maxExtraVerts = 3 * (u32)pow(4, quality);
	u32 extraVertIdx = 0;
	if(outUV != nullptr)
	{
		// Note: This only fixes seams for tileable textures. To properly fix seams for all textures the triangles
		// would actually need to be split along the UV seam. This is ignored as non-tileable textures won't look
		// good on a sphere regardless of the seam.
		for(u32 i = 0; i < numIndices; i += 3)
		{
			const Vector2& uv0 = *(Vector2*)&outUV[(i + 0) * vertexStride];
			const Vector2& uv1 = *(Vector2*)&outUV[(i + 1) * vertexStride];
			const Vector2& uv2 = *(Vector2*)&outUV[(i + 2) * vertexStride];

			u32 indexToSplit = (u32)-1;
			float offset = 1.0f;
			if(fabs(uv2.X - uv0.X) > 0.5f)
			{
				if(uv0.X < 0.5f)
				{
					// 2 is the odd-one out, > 0.5
					if(uv1.X < 0.5f)
					{
						indexToSplit = 2;
						offset = -1.0f;
					}
					// 0 is the odd-one out, < 0.5
					else
					{
						indexToSplit = 0;
						offset = 1.0f;
					}
				}
				else
				{
					// 2 is the odd-one out, < 0.5
					if(uv1.X > 0.5f)
					{
						indexToSplit = 2;
						offset = 1.0f;
					}
					// 0 is the odd-one out, > 0.5
					else
					{
						indexToSplit = 0;
						offset = -1.0f;
					}
				}
			}
			else if(fabs(uv1.X - uv0.X) > 0.5f)
			{
				if(uv0.X < 0.5f)
				{
					// 1 is the odd-one out, > 0.5
					if(uv2.X < 0.5f)
					{
						indexToSplit = 1;
						offset = -1.0f;
					}
					// 0 is the odd-one out, < 0.5
					else
					{
						indexToSplit = 0;
						offset = 1.0f;
					}
				}
				else
				{
					// 1 is the odd-one out, < 0.5
					if(uv2.X > 0.5f)
					{
						indexToSplit = 1;
						offset = 1.0f;
					}
					// 0 is the odd-one out, > 0.5
					else
					{
						indexToSplit = 0;
						offset = -1.0f;
					}
				}
			}
			else if(fabs(uv1.X - uv2.X) > 0.5f)
			{
				if(uv2.X < 0.5f)
				{
					// 1 is the odd-one out, > 0.5
					if(uv0.X < 0.5f)
					{
						indexToSplit = 1;
						offset = -1.0f;
					}
					// 2 is the odd-one out, < 0.5
					else
					{
						indexToSplit = 2;
						offset = 1.0f;
					}
				}
				else
				{
					// 1 is the odd-one out, < 0.5
					if(uv0.X > 0.5f)
					{
						indexToSplit = 1;
						offset = 1.0f;
					}
					// 2 is the odd-one out, > 0.5
					else
					{
						indexToSplit = 2;
						offset = -1.0f;
					}
				}
			}

			if(indexToSplit != (u32)-1)
			{
				Vector3 position = *(Vector3*)&outVertices[(vertexOffset + i + indexToSplit) * vertexStride];
				extraPositions = WriteVector3(extraPositions, vertexStride, position);

				if(extraNormals)
				{
					Vector3 normal = *(Vector3*)&outNormals[(vertexOffset + i + indexToSplit) * vertexStride];
					extraNormals = WriteVector3(extraNormals, vertexStride, normal);
				}

				Vector2 uv = *(Vector2*)&outUV[(i + indexToSplit) * vertexStride];
				uv.X += offset;

				extraUV = WriteVector2(extraUV, vertexStride, uv);

				// Index 0 maps to vertex 2, index 1 to vertex 1, index 2 to vertex 0
				if(indexToSplit == 0)
					indexToSplit = 2;
				else if(indexToSplit == 2)
					indexToSplit = 0;

				outIndices[i + indexToSplit] = vertexOffset + numIndices + extraVertIdx;

				B3D_ASSERT(extraVertIdx < maxExtraVerts);
				extraVertIdx++;
			}
		}
	}

	// Fill out the remaining extra vertices, just so they aren't uninitialized
	for(; extraVertIdx < maxExtraVerts; extraVertIdx++)
	{
		extraPositions = WriteVector3(extraPositions, vertexStride, sphere.Center);

		if(extraNormals)
			extraNormals = WriteVector3(extraNormals, vertexStride, Vector3::kUnitZ);

		if(extraUV)
			extraUV = WriteVector2(extraUV, vertexStride, Vector2::kZero);
	}
}

void ShapeMeshes3D::WireArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset, u32 quality)
{
	u32 numVertices = (quality + 1) * 5;

	GenerateArcVertices(center, normal, radius, startAngle, amountAngle, Vector2::kOne, numVertices, outVertices, vertexOffset, vertexStride);

	outIndices += indexOffset;
	u32 numLines = numVertices - 1;
	for(u32 i = 0; i < numLines; i++)
	{
		outIndices[i * 2 + 0] = vertexOffset + i;
		outIndices[i * 2 + 1] = vertexOffset + i + 1;
	}
}

void ShapeMeshes3D::SolidArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle, u8* outVertices, u8* outNormals, u8* outUV, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset, u32 quality)
{
	outVertices += vertexOffset * vertexStride;
	outNormals += vertexOffset * vertexStride;
	outIndices += indexOffset;

	bool reverseOrder = amountAngle.GetValueInDegrees() < 0.0f;
	Vector3 visibleNormal = normal;

	outVertices = WriteVector3(outVertices, vertexStride, center);
	outNormals = WriteVector3(outNormals, vertexStride, visibleNormal);

	u32 numArcVertices = (quality + 1) * 5;
	GenerateArcVertices(center, normal, radius, startAngle, amountAngle, Vector2::kOne, numArcVertices, outVertices, vertexOffset, vertexStride);

	u8* otherSideVertices = outVertices + (numArcVertices * vertexStride);
	u8* otherSideNormals = outNormals + (numArcVertices * vertexStride);

	otherSideVertices = WriteVector3(otherSideVertices, vertexStride, center);
	otherSideNormals = WriteVector3(otherSideNormals, vertexStride, -visibleNormal);

	for(u32 i = 0; i < numArcVertices; i++)
	{
		otherSideVertices = WriteVector3(otherSideVertices, vertexStride, *(Vector3*)&outVertices[i * vertexStride]);

		outNormals = WriteVector3(outNormals, vertexStride, visibleNormal);
		otherSideNormals = WriteVector3(otherSideNormals, vertexStride, -visibleNormal);
	}

	// UV
	if(outUV != nullptr)
	{
		outUV += vertexOffset * vertexStride;

		// Center
		outUV = WriteVector2(outUV, vertexStride, Vector2(0.5f, 0.5f));

		Vector3 arcNormal = normal;
		Vector3 right, top;
		arcNormal.OrthogonalComplement(right, top);

		for(u32 i = 0; i < numArcVertices; i++)
		{
			Vector3 vec = *(Vector3*)&outVertices[i * vertexStride];
			Vector3 diff = Vector3::Normalize(vec - center);

			Vector2 uv;
			uv.X = Vector3::Dot(diff, right) * 0.5f + 0.5f;
			uv.Y = Vector3::Dot(diff, top) * 0.5f + 0.5f;

			outUV = WriteVector2(outUV, vertexStride, uv);
		}

		// Reverse
		outUV = WriteVector2(outUV, vertexStride, Vector2(0.5f, 0.5f));

		for(u32 i = 0; i < numArcVertices; i++)
		{
			Vector3 vec = *(Vector3*)&outVertices[(numArcVertices + i + 1) * vertexStride];
			Vector3 diff = Vector3::Normalize(vec - center);

			Vector2 uv;
			uv.X = Vector3::Dot(diff, -right) * 0.5f + 0.5f;
			uv.Y = Vector3::Dot(diff, -top) * 0.5f + 0.5f;

			outUV = WriteVector2(outUV, vertexStride, uv);
		}
	}

	u32 numTriangles = numArcVertices;

	// If angle is negative the order of vertices is reversed so we need to reverse the indexes too
	u32 frontSideOffset = vertexOffset + (reverseOrder ? (numArcVertices + 1) : 0);
	u32 backSideOffset = vertexOffset + (!reverseOrder ? (numArcVertices + 1) : 0);

	for(u32 i = 0; i < numTriangles; i++)
	{
		outIndices[i * 6 + 0] = frontSideOffset + 0;
		outIndices[i * 6 + 1] = frontSideOffset + i;
		outIndices[i * 6 + 2] = frontSideOffset + i + 1;

		outIndices[i * 6 + 3] = backSideOffset + 0;
		outIndices[i * 6 + 4] = backSideOffset + i + 1;
		outIndices[i * 6 + 5] = backSideOffset + i;
	}
}

void ShapeMeshes3D::WireFrustum(const Vector3& position, float aspect, Degree FOV, float near, float far, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset)
{
	float fovTan = Math::Tan(FOV * 0.5f);

	Vector3 nearPoint(0, 0, near);
	Vector3 nearWidth(near * fovTan * aspect, 0, 0);
	Vector3 nearHeight(0, (near * fovTan) / aspect, 0);

	Vector3 farPoint(0, 0, far);
	Vector3 farWidth(far * fovTan * aspect, 0, 0);
	Vector3 farHeight(0, (far * fovTan) / aspect, 0);

	Vector3 points[8] = {
		nearPoint + nearWidth + nearHeight,
		nearPoint - nearWidth + nearHeight,
		nearPoint - nearWidth - nearHeight,
		nearPoint + nearWidth - nearHeight,
		farPoint + farWidth + farHeight,
		farPoint - farWidth + farHeight,
		farPoint - farWidth - farHeight,
		farPoint + farWidth - farHeight
	};

	outVertices += vertexOffset * vertexStride;

	for(u32 i = 0; i < 8; i++)
		outVertices = WriteVector3(outVertices, vertexStride, position + points[i]);

	outIndices += indexOffset;

	// Front
	outIndices[0] = vertexOffset + 0;
	outIndices[1] = vertexOffset + 1;
	outIndices[2] = vertexOffset + 1;
	outIndices[3] = vertexOffset + 2;
	outIndices[4] = vertexOffset + 2;
	outIndices[5] = vertexOffset + 3;
	outIndices[6] = vertexOffset + 3;
	outIndices[7] = vertexOffset + 0;

	// Center
	outIndices[8] = vertexOffset + 0;
	outIndices[9] = vertexOffset + 4;
	outIndices[10] = vertexOffset + 1;
	outIndices[11] = vertexOffset + 5;
	outIndices[12] = vertexOffset + 2;
	outIndices[13] = vertexOffset + 6;
	outIndices[14] = vertexOffset + 3;
	outIndices[15] = vertexOffset + 7;

	// Back
	outIndices[16] = vertexOffset + 4;
	outIndices[17] = vertexOffset + 5;
	outIndices[18] = vertexOffset + 5;
	outIndices[19] = vertexOffset + 6;
	outIndices[20] = vertexOffset + 6;
	outIndices[21] = vertexOffset + 7;
	outIndices[22] = vertexOffset + 7;
	outIndices[23] = vertexOffset + 4;
}

void ShapeMeshes3D::SolidCone(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, u8* outVertices, u8* outNormals, u8* outUV, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset, u32 quality)
{
	outVertices += vertexOffset * vertexStride;
	outIndices += indexOffset;

	if(outUV != nullptr)
		outUV += vertexOffset * vertexStride;

	if(outNormals != nullptr)
		outNormals += vertexOffset * vertexStride;

	// Generate base disc
	u32 numArcVertices = (quality + 1) * 4;

	GenerateArcVertices(base, normal, radius, Degree(0), Degree(360), scale, numArcVertices + 1, outVertices, 0, vertexStride);

	if(outUV != nullptr)
	{
		Vector3 arcNormal = normal;
		Vector3 right, top;
		arcNormal.OrthogonalComplement(right, top);

		for(u32 i = 0; i < numArcVertices; i++)
		{
			Vector3 vec = *(Vector3*)&outVertices[i * vertexStride];
			Vector3 diff = Vector3::Normalize(vec - base);

			Vector2 uv;
			uv.X = Vector3::Dot(diff, right) * 0.5f + 0.5f;
			uv.Y = Vector3::Dot(diff, top) * 0.5f + 0.5f;

			outUV = WriteVector2(outUV, vertexStride, uv);
		}

		// Center base
		outUV = WriteVector2(outUV, vertexStride, Vector2(0.5f, 0.5f));
	}

	outVertices += numArcVertices * vertexStride;
	outVertices = WriteVector3(outVertices, vertexStride, base); // Write base vertex

	u32 baseIdx = numArcVertices;

	if(outNormals != nullptr)
	{
		u32 totalNumBaseVertices = numArcVertices + 1;
		for(u32 i = 0; i < totalNumBaseVertices; i++)
			outNormals = WriteVector3(outNormals, vertexStride, -normal);
	}

	u32 numTriangles = numArcVertices;
	for(u32 i = 0; i < numTriangles - 1; i++)
	{
		outIndices[i * 3 + 0] = vertexOffset + baseIdx;
		outIndices[i * 3 + 1] = vertexOffset + i + 1;
		outIndices[i * 3 + 2] = vertexOffset + i;
	}

	{
		u32 i = numTriangles - 1;
		outIndices[i * 3 + 0] = vertexOffset + baseIdx;
		outIndices[i * 3 + 1] = vertexOffset + 0;
		outIndices[i * 3 + 2] = vertexOffset + i;
	}

	//// Generate cone
	// Base vertices
	GenerateArcVertices(base, normal, radius, Degree(0), Degree(360), scale, numArcVertices + 1, outVertices, 0, vertexStride);

	Vector3 topVertex = base + normal * height;

	// Normals
	if(outNormals != nullptr)
	{
		u8* outNormalsBase = outNormals;
		u8* outNormalsTop = outNormals + numArcVertices * vertexStride;
		for(i32 i = 0; i < (i32)numArcVertices; i++)
		{
			int offsetA = i == 0 ? numArcVertices - 1 : i - 1;
			int offsetB = i;
			int offsetC = (i + 1) % numArcVertices;

			Vector3* a = (Vector3*)(outVertices + (offsetA * vertexStride));
			Vector3* b = (Vector3*)(outVertices + (offsetB * vertexStride));
			Vector3* c = (Vector3*)(outVertices + (offsetC * vertexStride));

			Vector3 toTop = topVertex - *b;

			Vector3 normalLeft = Vector3::Cross(*a - *b, toTop);
			normalLeft.Normalize();

			Vector3 normalRight = Vector3::Cross(toTop, *c - *b);
			normalRight.Normalize();

			Vector3 triNormal = Vector3::Normalize(normalLeft + normalRight);

			outNormalsBase = WriteVector3(outNormalsBase, vertexStride, triNormal);
			outNormalsTop = WriteVector3(outNormalsTop, vertexStride, triNormal);
		}
	}

	// UV
	if(outUV != nullptr)
	{
		float angle = 0.0f;
		float angleIncrement = Math::kTwoPi / numArcVertices;

		// Bottom
		for(u32 i = 0; i < numArcVertices; i++)
		{
			Vector2 uv;
			uv.X = angle / Math::kTwoPi;
			uv.Y = 1.0f;

			outUV = WriteVector2(outUV, vertexStride, uv);
			angle += angleIncrement;
		}

		// Top
		angle = 0.0f;
		for(u32 i = 0; i < numArcVertices; i++)
		{
			Vector2 uv;
			uv.X = angle / Math::kTwoPi;
			uv.Y = 0.0f;

			outUV = WriteVector2(outUV, vertexStride, uv);
			angle += angleIncrement;
		}
	}

	// Top vertices (All same position, but need them separate because of different normals & uv)
	outVertices += numArcVertices * vertexStride;

	for(u32 i = 0; i < numArcVertices; i++)
		outVertices = WriteVector3(outVertices, vertexStride, topVertex);

	outIndices += numTriangles * 3;
	u32 curVertBaseOffset = vertexOffset + numArcVertices + 1;
	u32 curVertTopOffset = curVertBaseOffset + numArcVertices;
	for(u32 i = 0; i < numTriangles - 1; i++)
	{
		outIndices[i * 3 + 0] = curVertTopOffset + i;
		outIndices[i * 3 + 1] = curVertBaseOffset + i;
		outIndices[i * 3 + 2] = curVertBaseOffset + i + 1;
	}

	{
		u32 i = numTriangles - 1;
		outIndices[i * 3 + 0] = curVertTopOffset + i;
		outIndices[i * 3 + 1] = curVertBaseOffset + i;
		outIndices[i * 3 + 2] = curVertBaseOffset + 0;
	}
}

void ShapeMeshes3D::WireCone(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset, u32 quality)
{
	outVertices += vertexOffset * vertexStride;
	outIndices += indexOffset;

	// Generate arc vertices
	u32 numArcVertices = (quality + 1) * 4;

	GenerateArcVertices(base, normal, radius, Degree(0), Degree(360), scale, numArcVertices + 1, outVertices, 0, vertexStride);

	outVertices += numArcVertices * vertexStride;

	u32 numLines = numArcVertices;
	for(u32 i = 0; i < numLines; i++)
	{
		outIndices[i * 2 + 0] = vertexOffset + i;
		outIndices[i * 2 + 1] = vertexOffset + i + 1;
	}

	outIndices += numLines * 2;

	// Generate cone vertices
	GenerateArcVertices(base, normal, radius, Degree(0), Degree(360), scale, 5, outVertices, 0, vertexStride);

	// Cone point
	outVertices += 4 * vertexStride;
	outVertices = WriteVector3(outVertices, vertexStride, base + normal * height);

	vertexOffset += numArcVertices;

	for(u32 i = 0; i < 4; i++)
	{
		outIndices[i * 2 + 0] = vertexOffset + 4;
		outIndices[i * 2 + 1] = vertexOffset + i;
	}
}

void ShapeMeshes3D::SolidCylinder(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, u8* outVertices, u8* outNormals, u8* outUV, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset, u32 quality)
{
	outVertices += vertexOffset * vertexStride;
	outIndices += indexOffset;

	if(outUV != nullptr)
		outUV += vertexOffset * vertexStride;

	if(outNormals != nullptr)
		outNormals += vertexOffset * vertexStride;

	// Generate base disc
	u32 numArcVertices = (quality + 1) * 4;

	GenerateArcVertices(base, normal, radius, Degree(0), Degree(360), scale, numArcVertices + 1, outVertices, 0, vertexStride);

	if(outUV != nullptr)
	{
		Vector3 arcNormal = normal;
		Vector3 right, top;
		arcNormal.OrthogonalComplement(right, top);

		for(u32 i = 0; i < numArcVertices; i++)
		{
			Vector3 vec = *(Vector3*)&outVertices[i * vertexStride];
			Vector3 diff = Vector3::Normalize(vec - base);

			Vector2 uv;
			uv.X = Vector3::Dot(diff, right) * 0.5f + 0.5f;
			uv.Y = Vector3::Dot(diff, top) * 0.5f + 0.5f;

			outUV = WriteVector2(outUV, vertexStride, uv);
		}

		outUV = WriteVector2(outUV, vertexStride, Vector2(0.5f, 0.5f));
	}

	outVertices += numArcVertices * vertexStride;
	outVertices = WriteVector3(outVertices, vertexStride, base); // Write base vertex

	u32 baseIdx = numArcVertices;

	if(outNormals != nullptr)
	{
		u32 totalNumBaseVertices = numArcVertices + 1;
		for(u32 i = 0; i < totalNumBaseVertices; i++)
			outNormals = WriteVector3(outNormals, vertexStride, -normal);
	}

	u32 numTriangles = numArcVertices;
	for(u32 i = 0; i < numTriangles - 1; i++)
	{
		outIndices[i * 3 + 0] = vertexOffset + baseIdx;
		outIndices[i * 3 + 1] = vertexOffset + i + 1;
		outIndices[i * 3 + 2] = vertexOffset + i;
	}

	{
		u32 i = numTriangles - 1;
		outIndices[i * 3 + 0] = vertexOffset + baseIdx;
		outIndices[i * 3 + 1] = vertexOffset + 0;
		outIndices[i * 3 + 2] = vertexOffset + i;
	}

	outIndices += numTriangles * 3;
	u32 vertexOffsetBase = vertexOffset + numArcVertices + 1;

	// Generate cap disc
	Vector3 topVertex = base + normal * height;

	GenerateArcVertices(topVertex, normal, radius, Degree(0), Degree(360), scale, numArcVertices + 1, outVertices, 0, vertexStride);

	if(outUV != nullptr)
	{
		Vector3 arcNormal = normal;
		Vector3 right, top;
		arcNormal.OrthogonalComplement(right, top);

		for(u32 i = 0; i < numArcVertices; i++)
		{
			Vector3 vec = *(Vector3*)&outVertices[i * vertexStride];
			Vector3 diff = Vector3::Normalize(vec - topVertex);

			Vector2 uv;
			uv.X = Vector3::Dot(diff, right) * 0.5f + 0.5f;
			uv.Y = Vector3::Dot(diff, top) * 0.5f + 0.5f;

			outUV = WriteVector2(outUV, vertexStride, uv);
		}

		outUV = WriteVector2(outUV, vertexStride, Vector2(0.5f, 0.5f));
	}

	outVertices += numArcVertices * vertexStride;
	outVertices = WriteVector3(outVertices, vertexStride, topVertex); // Write top vertex

	if(outNormals != nullptr)
	{
		u32 totalNumBaseVertices = numArcVertices + 1;
		for(u32 i = 0; i < totalNumBaseVertices; i++)
			outNormals = WriteVector3(outNormals, vertexStride, normal);
	}

	for(u32 i = 0; i < numTriangles - 1; i++)
	{
		outIndices[i * 3 + 0] = vertexOffsetBase + baseIdx;
		outIndices[i * 3 + 1] = vertexOffsetBase + i;
		outIndices[i * 3 + 2] = vertexOffsetBase + i + 1;
	}

	{
		u32 i = numTriangles - 1;
		outIndices[i * 3 + 0] = vertexOffsetBase + baseIdx;
		outIndices[i * 3 + 1] = vertexOffsetBase + i;
		outIndices[i * 3 + 2] = vertexOffsetBase + 0;
	}

	outIndices += numTriangles * 3;
	u32 vertexOffsetCap = vertexOffsetBase + numArcVertices + 1;

	// Generate cylinder
	GenerateArcVertices(base, normal, radius, Degree(0), Degree(360), scale, numArcVertices + 1, outVertices, 0, vertexStride);

	GenerateArcVertices(topVertex, normal, radius, Degree(0), Degree(360), scale, numArcVertices + 1, outVertices, numArcVertices + 1, vertexStride);

	// Normals
	if(outNormals != nullptr)
	{
		u8* outNormalsBase = outNormals;
		u8* outNormalsCap = outNormals + (numArcVertices + 1) * vertexStride;
		for(u32 i = 0; i < numArcVertices + 1; i++)
		{
			u32 offsetA = i == 0 ? numArcVertices - 1 : i - 1;
			u32 offsetB = i;
			u32 offsetC = i == numArcVertices ? 1 : i + 1;

			Vector3* a = (Vector3*)(outVertices + (offsetA * vertexStride));
			Vector3* b = (Vector3*)(outVertices + (offsetB * vertexStride));
			Vector3* c = (Vector3*)(outVertices + (offsetC * vertexStride));

			Vector3 toTop = normal;

			Vector3 normalLeft = Vector3::Cross(*a - *b, toTop);
			normalLeft.Normalize();

			Vector3 normalRight = Vector3::Cross(toTop, *c - *b);
			normalRight.Normalize();

			Vector3 triNormal = Vector3::Normalize(normalLeft + normalRight);

			outNormalsBase = WriteVector3(outNormalsBase, vertexStride, triNormal);
			outNormalsCap = WriteVector3(outNormalsCap, vertexStride, triNormal);
		}
	}

	// UV
	if(outUV != nullptr)
	{
		float angle = 0.0f;
		float angleIncrement = Math::kTwoPi / numArcVertices;

		for(u32 i = 0; i < numArcVertices + 1; i++)
		{
			Vector2 uv;
			uv.X = angle / Math::kTwoPi;
			uv.Y = 1.0f;

			outUV = WriteVector2(outUV, vertexStride, uv);
			angle += angleIncrement;
		}

		angle = 0.0f;
		for(u32 i = 0; i < numArcVertices + 1; i++)
		{
			Vector2 uv;
			uv.X = angle / Math::kTwoPi;
			uv.Y = 0.0f;

			outUV = WriteVector2(outUV, vertexStride, uv);
			angle += angleIncrement;
		}
	}

	vertexOffsetBase = vertexOffsetCap;
	vertexOffsetCap = vertexOffsetCap + numArcVertices + 1;

	for(u32 i = 0; i < numTriangles; i++)
	{
		outIndices[i * 6 + 0] = vertexOffsetCap + i;
		outIndices[i * 6 + 1] = vertexOffsetBase + i;
		outIndices[i * 6 + 2] = vertexOffsetBase + i + 1;

		outIndices[i * 6 + 3] = vertexOffsetCap + i;
		outIndices[i * 6 + 4] = vertexOffsetBase + i + 1;
		outIndices[i * 6 + 5] = vertexOffsetCap + i + 1;
	}
}

void ShapeMeshes3D::WireCylinder(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset, u32 quality)
{
	outVertices += vertexOffset * vertexStride;
	outIndices += indexOffset;

	// Generate base and cap discs
	u32 numArcVertices = (quality + 1) * 4;

	Degree angleAmount = Degree(360) - Degree(360) / (float)(numArcVertices);

	GenerateArcVertices(base, normal, radius, Degree(0), angleAmount, scale, numArcVertices, outVertices, 0, vertexStride);

	Vector3 topVertex = base + normal * height;

	GenerateArcVertices(topVertex, normal, radius, Degree(0), angleAmount, scale, numArcVertices, outVertices, numArcVertices, vertexStride);

	u32 vertexOffsetBase = vertexOffset;
	u32 vertexOffsetCap = vertexOffset + numArcVertices;

	u32 numLines = numArcVertices;
	for(u32 i = 0; i < numLines - 1; i++)
	{
		outIndices[i * 4 + 0] = vertexOffsetBase + i;
		outIndices[i * 4 + 1] = vertexOffsetBase + i + 1;
		outIndices[i * 4 + 2] = vertexOffsetCap + i;
		outIndices[i * 4 + 3] = vertexOffsetCap + i + 1;
	}
	{
		u32 i = numLines - 1;
		outIndices[i * 4 + 0] = vertexOffsetBase + i;
		outIndices[i * 4 + 1] = vertexOffsetBase + 0;
		outIndices[i * 4 + 2] = vertexOffsetCap + i;
		outIndices[i * 4 + 3] = vertexOffsetCap + 0;
	}

	// Generate cylinder
	outIndices += numLines * 4;
	for(u32 i = 0; i < numLines; i++)
	{
		outIndices[i * 2 + 0] = vertexOffsetBase + i;
		outIndices[i * 2 + 1] = vertexOffsetCap + i;
	}
}

void ShapeMeshes3D::SolidQuad(const Rect3& area, u8* outVertices, u8* outNormals, u8* outUV, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset)
{
	outVertices += (vertexOffset * vertexStride);

	Vector3 topLeft = area.Center - area.HorizontalAxis * area.HorizontalExtent + area.VerticalAxis * area.VerticalExtent;
	Vector3 topRight = area.Center + area.HorizontalAxis * area.HorizontalExtent + area.VerticalAxis * area.VerticalExtent;
	Vector3 botRight = area.Center + area.HorizontalAxis * area.HorizontalExtent - area.VerticalAxis * area.VerticalExtent;
	Vector3 botLeft = area.Center - area.HorizontalAxis * area.HorizontalExtent - area.VerticalAxis * area.VerticalExtent;

	outVertices = WriteVector3(outVertices, vertexStride, topLeft);
	outVertices = WriteVector3(outVertices, vertexStride, topRight);
	outVertices = WriteVector3(outVertices, vertexStride, botRight);
	outVertices = WriteVector3(outVertices, vertexStride, botLeft);

	outVertices = WriteVector3(outVertices, vertexStride, topLeft);
	outVertices = WriteVector3(outVertices, vertexStride, topRight);
	outVertices = WriteVector3(outVertices, vertexStride, botRight);
	outVertices = WriteVector3(outVertices, vertexStride, botLeft);

	Vector3 normal = area.HorizontalAxis.Cross(area.VerticalAxis);
	Vector3 reverseNormal = -normal;

	outNormals += (vertexOffset * vertexStride);
	outNormals = WriteVector3(outNormals, vertexStride, normal);
	outNormals = WriteVector3(outNormals, vertexStride, normal);
	outNormals = WriteVector3(outNormals, vertexStride, normal);
	outNormals = WriteVector3(outNormals, vertexStride, normal);

	outNormals = WriteVector3(outNormals, vertexStride, reverseNormal);
	outNormals = WriteVector3(outNormals, vertexStride, reverseNormal);
	outNormals = WriteVector3(outNormals, vertexStride, reverseNormal);
	outNormals = WriteVector3(outNormals, vertexStride, reverseNormal);

	if(outUV != nullptr)
	{
		outUV += (vertexOffset * vertexStride);
		outUV = WriteVector2(outUV, vertexStride, Vector2(0.0f, 0.0f));
		outUV = WriteVector2(outUV, vertexStride, Vector2(1.0f, 0.0f));
		outUV = WriteVector2(outUV, vertexStride, Vector2(1.0f, 1.0f));
		outUV = WriteVector2(outUV, vertexStride, Vector2(0.0f, 1.0f));

		outUV = WriteVector2(outUV, vertexStride, Vector2(0.0f, 0.0f));
		outUV = WriteVector2(outUV, vertexStride, Vector2(1.0f, 0.0f));
		outUV = WriteVector2(outUV, vertexStride, Vector2(1.0f, 1.0f));
		outUV = WriteVector2(outUV, vertexStride, Vector2(0.0f, 1.0f));
	}

	outIndices += indexOffset;
	outIndices[0] = vertexOffset;
	outIndices[1] = vertexOffset + 1;
	outIndices[2] = vertexOffset + 2;

	outIndices[3] = vertexOffset;
	outIndices[4] = vertexOffset + 2;
	outIndices[5] = vertexOffset + 3;

	outIndices[6] = vertexOffset + 4;
	outIndices[7] = vertexOffset + 6;
	outIndices[8] = vertexOffset + 5;

	outIndices[9] = vertexOffset + 4;
	outIndices[10] = vertexOffset + 7;
	outIndices[11] = vertexOffset + 6;
}

Vector3 ShapeMeshes3D::CalcCenter(u8* vertices, u32 numVertices, u32 vertexStride)
{
	Vector3 center = Vector3::kZero;
	for(u32 i = 0; i < numVertices; i++)
	{
		Vector3* curVert = (Vector3*)vertices;
		center += *curVert;

		vertices += vertexStride;
	}

	center /= (float)numVertices;
	return center;
}

void ShapeMeshes3D::PixelLine(const Vector3& a, const Vector3& b, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset)
{
	outVertices += (vertexOffset * vertexStride);

	Vector3* vertices = (Vector3*)outVertices;
	(*vertices) = a;

	vertices = (Vector3*)(outVertices + vertexStride);
	(*vertices) = b;

	outIndices += indexOffset;
	outIndices[0] = vertexOffset + 0;
	outIndices[1] = vertexOffset + 1;
}

void ShapeMeshes3D::AntialiasedLine(const Vector3& a, const Vector3& b, const Vector3& up, float width, float borderWidth, const Color& color, u8* outVertices, u8* outColors, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset)
{
	Vector3 dir = b - a;
	dir.Normalize();

	Vector3 right = dir.Cross(up);
	right.Normalize();

	Vector<Vector3> points(4);

	float r = width * 0.5f;
	dir = dir * r;
	right = right * r;

	Vector3 v0 = a - dir - right;
	Vector3 v1 = a - dir + right;
	Vector3 v2 = b + dir + right;
	Vector3 v3 = b + dir - right;

	points[0] = v0;
	points[1] = v1;
	points[2] = v2;
	points[3] = v3;

	AntialiasedPolygon(points, up, borderWidth, color, outVertices, outColors, vertexOffset, vertexStride, outIndices, indexOffset);
}

void ShapeMeshes3D::PixelSolidPolygon(const Vector<Vector3>& points, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset)
{
	outVertices += (vertexOffset * vertexStride);

	for(auto& point : points)
	{
		Vector3* vertices = (Vector3*)outVertices;
		(*vertices) = point;

		outVertices += vertexStride;
	}

	outIndices += indexOffset;
	i32 numPoints = (i32)points.size();
	u32 idxCnt = 0;
	for(int i = 2; i < numPoints; i++)
	{
		outIndices[idxCnt++] = vertexOffset;
		outIndices[idxCnt++] = vertexOffset + i - 1;
		outIndices[idxCnt++] = vertexOffset + i;
	}
}

void ShapeMeshes3D::PixelWirePolygon(const Vector<Vector3>& points, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset)
{
	i32 numPoints = (i32)points.size();
	u32 curVertOffset = vertexOffset;
	u32 curIdxOffset = indexOffset;
	for(i32 i = 0, j = numPoints - 1; i < numPoints; j = i++)
	{
		PixelLine(points[j], points[i], outVertices, curVertOffset, vertexStride, outIndices, curIdxOffset);
		curVertOffset += 2;
		curIdxOffset += 2;
	}
}

void ShapeMeshes3D::AntialiasedPolygon(const Vector<Vector3>& points, const Vector3& up, float borderWidth, const Color& color, u8* outVertices, u8* outColors, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset)
{
	u32 numCoords = (u32)points.size();

	outVertices += vertexOffset * vertexStride;
	outColors += vertexOffset * vertexStride;
	Vector<Vector3> tempNormals(numCoords);

	for(u32 i = 0, j = numCoords - 1; i < numCoords; j = i++)
	{
		const Vector3& v0 = points[j];
		const Vector3& v1 = points[i];

		Vector3 dir = v1 - v0;
		Vector3 right = dir.Cross(up);
		right.Normalize();

		tempNormals[j] = right;

		// Also start populating the vertex array
		Vector3* vertices = (Vector3*)outVertices;
		*vertices = v1;

		u32* colors = (u32*)outColors;
		*colors = color.GetAsRgba();

		outVertices += vertexStride;
		outColors += vertexStride;
	}

	Color transparentColor = color;
	transparentColor.A = 0.0f;

	for(u32 i = 0, j = numCoords - 1; i < numCoords; j = i++)
	{
		const Vector3& n0 = tempNormals[j];
		const Vector3& n1 = tempNormals[i];

		Vector3 avgNrm = (n0 + n1) * 0.5f;
		float magSqrd = avgNrm.SquaredLength();

		if(magSqrd > 0.000001f)
		{
			float scale = 1.0f / magSqrd;
			if(scale > 10.0f)
				scale = 10.0f;

			avgNrm = avgNrm * scale;
		}

		Vector3 tempCoord = points[i] + avgNrm * borderWidth;

		// Move it to the vertex array
		Vector3* vertices = (Vector3*)outVertices;
		*vertices = tempCoord;

		u32* colors = (u32*)outColors;
		*colors = transparentColor.GetAsRgba();

		outVertices += vertexStride;
		outColors += vertexStride;
	}

	// Populate index buffer
	outIndices += indexOffset;

	u32 idxCnt = 0;
	for(u32 i = 0, j = numCoords - 1; i < numCoords; j = i++)
	{
		outIndices[idxCnt++] = vertexOffset + i;
		outIndices[idxCnt++] = vertexOffset + j;
		outIndices[idxCnt++] = vertexOffset + numCoords + j;

		outIndices[idxCnt++] = vertexOffset + numCoords + j;
		outIndices[idxCnt++] = vertexOffset + numCoords + i;
		outIndices[idxCnt++] = vertexOffset + i;
	}

	for(u32 i = 2; i < numCoords; ++i)
	{
		outIndices[idxCnt++] = vertexOffset + 0;
		outIndices[idxCnt++] = vertexOffset + i - 1;
		outIndices[idxCnt++] = vertexOffset + i;
	}
}

u32 ShapeMeshes3D::SubdivideTriangleOnSphere(const Vector3& center, float radius, u32 numLevels, const Vector3& a, const Vector3& b, const Vector3& c, u8* outVertices, u8* outNormals, u32 vertexOffset, u32 vertexStride)
{
	outVertices += (vertexOffset * vertexStride);

	if(outNormals != nullptr)
		outNormals += (vertexOffset * vertexStride);

	u32 numVertices = 0;

	if(numLevels > 0)
	{
		Vector3 sub1 = Vector3::Normalize((a + b) * 0.5f);
		Vector3 sub2 = Vector3::Normalize((b + c) * 0.5f);
		Vector3 sub3 = Vector3::Normalize((c + a) * 0.5f);

		numLevels--;

		numVertices += SubdivideTriangleOnSphere(center, radius, numLevels, a, sub1, sub3, outVertices, outNormals, numVertices, vertexStride);
		numVertices += SubdivideTriangleOnSphere(center, radius, numLevels, sub1, b, sub2, outVertices, outNormals, numVertices, vertexStride);
		numVertices += SubdivideTriangleOnSphere(center, radius, numLevels, sub1, sub2, sub3, outVertices, outNormals, numVertices, vertexStride);
		numVertices += SubdivideTriangleOnSphere(center, radius, numLevels, sub3, sub2, c, outVertices, outNormals, numVertices, vertexStride);
	}
	else
	{
		*((Vector3*)outVertices) = center + a * radius;
		outVertices += vertexStride;

		*((Vector3*)outVertices) = center + b * radius;
		outVertices += vertexStride;

		*((Vector3*)outVertices) = center + c * radius;
		outVertices += vertexStride;

		if(outNormals != nullptr)
		{
			*((Vector3*)outNormals) = a;
			outNormals += vertexStride;

			*((Vector3*)outNormals) = b;
			outNormals += vertexStride;

			*((Vector3*)outNormals) = c;
			outNormals += vertexStride;
		}

		numVertices += 3;
	}

	return numVertices;
}

void ShapeMeshes3D::GenerateArcVertices(const Vector3& center, const Vector3& up, float radius, Degree startAngle, Degree angleAmount, Vector2 scale, u32 numVertices, u8* outVertices, u32 vertexOffset, u32 vertexStride)
{
	B3D_ASSERT(numVertices >= 2);

	Quaternion alignWithStart = Quaternion(-Vector3::kUnitY, startAngle);
	Quaternion alignWithUp = Quaternion::GetRotationFromTo(Vector3::kUnitY, up);

	Vector3 right = alignWithUp.Rotate(alignWithStart.Rotate(Vector3::kUnitX));
	right.Normalize();

	Quaternion increment(-up, angleAmount / (float)(numVertices - 1));

	outVertices += vertexOffset * vertexStride;
	Vector3 curDirection = right * radius;
	for(u32 i = 0; i < numVertices; i++)
	{
		// Note: Ignoring scale
		outVertices = WriteVector3(outVertices, vertexStride, (center + curDirection));
		curDirection = increment.Rotate(curDirection);
	}
}

void ShapeMeshes3D::GenerateTangents(u8* positions, u8* normals, u8* uv, u32* indices, u32 numVertices, u32 numIndices, u32 vertexOffset, u32 indexOffset, u32 vertexStride, u8* tangents)
{
	Vector3* tempTangents = B3DStackAllocate<Vector3>(numVertices);
	Vector3* tempBitangents = B3DStackAllocate<Vector3>(numVertices);

	MeshUtility::CalculateTangents(
		(Vector3*)(positions + vertexOffset * vertexStride),
		(Vector3*)(normals + vertexOffset * vertexStride),
		(Vector2*)(uv + vertexOffset * vertexStride),
		(u8*)(indices + indexOffset),
		numVertices, numIndices, tempTangents, tempBitangents, 4, vertexStride);

	for(u32 i = 0; i < (u32)numVertices; i++)
	{
		Vector3 normal = *(Vector3*)&normals[(vertexOffset + i) * vertexStride];
		Vector3 tangent = tempTangents[i];
		Vector3 bitangent = tempBitangents[i];

		Vector3 engineBitangent = Vector3::Cross(normal, tangent);
		float sign = Vector3::Dot(engineBitangent, bitangent);

		Vector4 packedTangent(tangent.X, tangent.Y, tangent.Z, sign > 0 ? 1.0f : -1.0f);
		memcpy(tangents + (vertexOffset + i) * vertexStride, &packedTangent, sizeof(Vector4));
	}

	B3DStackFree(tempBitangents);
	B3DStackFree(tempTangents);
}
