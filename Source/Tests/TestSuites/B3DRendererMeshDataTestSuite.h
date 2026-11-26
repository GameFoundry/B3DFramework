//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/**
	 * Tests for reading and writing mesh vertex data by attribute. Uses a cube whose attributes are either interleaved in a single stream, or
	 * split into three streams: positions; normals, tangents and blend data; texture coordinates and colors.
	 */
	class RendererMeshDataTestSuite : public TestSuite
	{
	public:
		RendererMeshDataTestSuite();

	private:
		/**
		 * Checks which streams RendererMeshData::VertexLayoutVertexDesc() places attributes in: positions in stream 0, normals, tangents and
		 * blend data in stream 1, texture coordinates and colors in the last stream, with no empty streams in between. Also checks stream
		 * strides of a layout containing every attribute.
		 */
		void TestVertexLayout();

		/**
		 * Writes every attribute using RendererMeshData setters and reads it back using getters, for both interleaved and split layouts.
		 * Checks that values match (within packing precision for colors, normals and tangents), that tangent handedness is preserved, and
		 * that each attribute is found in the stream its layout declares.
		 */
		void TestAccessors();

		/**
		 * Converts split mesh data with full precision normals and tangents using RendererMeshData::Convert, then reads it back. Checks that
		 * values match (within packing precision for colors, normals and tangents).
		 */
		void TestConvert();

		/**
		 * Combines two split meshes using MeshData::Combine, where one of them has no colors. Checks that each attribute stays in its source
		 * stream and contains vertices of both meshes in order, and that colors of the second mesh are zero. Also checks that combining
		 * meshes with the same attribute in different streams reports an error.
		 */
		void TestCombine();

		/** Cooks a triangle and a convex physics mesh from the split cube, and checks that the cooked vertices are the cube's corners. */
		void TestPhysicsMesh();

		/**
		 * Spawns one particle per vertex of the split cube, in order, using a static mesh emitter shape. Checks that particles get the cube's
		 * positions and normals (within packing precision).
		 */
		void TestParticleEmitter();
	};
} // namespace b3d
