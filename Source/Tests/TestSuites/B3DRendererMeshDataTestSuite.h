//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/**
	 * Tests for the code that reads and writes mesh vertex data by attribute, on a cube whose attributes are either interleaved in one
	 * vertex stream or split over three: positions; normals, tangents and blend data; texture coordinates and colors.
	 */
	class RendererMeshDataTestSuite : public TestSuite
	{
	public:
		RendererMeshDataTestSuite();

	private:
		/**
		 * Checks the streams RendererMeshData::VertexLayoutVertexDesc() places attributes in: positions in stream 0, normals, tangents and
		 * blend data in stream 1, texture coordinates and colors in the next one, with no stream left empty. Checks the strides of the
		 * layout with every attribute.
		 */
		void TestVertexLayout();

		/**
		 * Writes every attribute through the RendererMeshData setters and reads it back through the getters, on the interleaved and on the
		 * split layout. Positions, texture coordinates, bone weights and indices are equal; colors within one 8-bit step; normals and
		 * tangents within 1/127, and the tangents keep their handedness. Each attribute is found in the stream its layout declares it in.
		 */
		void TestAccessors();

		/**
		 * Converts split mesh data with full precision normals and tangents with RendererMeshData::Convert, and reads the result back:
		 * positions, texture coordinates and bone weights are equal, colors within one 8-bit step, normals and tangents within 1/127.
		 */
		void TestConvert();

		/**
		 * Combines two split meshes, one of which has no colors, with MeshData::Combine. Each attribute of the result is in its source
		 * stream, holds the vertices of both meshes in order, and the colors of the second mesh are zero. Combining meshes that declare
		 * the same attribute in different streams reports an error.
		 */
		void TestCombine();

		/** Cooks a triangle and a convex physics mesh from the split cube. The cooked vertices are the cube's corners. */
		void TestPhysicsMesh();

		/**
		 * Spawns one particle per vertex of the split cube, in order, from a static mesh emitter shape. The particles have the cube's
		 * positions, and its normals within 1/127.
		 */
		void TestParticleEmitter();
	};
} // namespace b3d
