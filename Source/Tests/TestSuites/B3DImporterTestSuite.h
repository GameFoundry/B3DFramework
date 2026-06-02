//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"
#include "FileSystem/B3DPath.h"

namespace b3d
{
	class ImporterTestSuite : public TestSuite
	{
	public:
		ImporterTestSuite();

	protected:
		void StartUp() override;

	private:
		void TestPngImport_Default();
		void TestPngImport_WithMips();
		void TestPngImport_BC3();
		void TestGpuCompress_Psnr();
		void TestGpuCompress_BC6H_Psnr();
		void TestGpuMipmaps_BoxFilter();

		void TestOggImport_Default();
		void TestOggImport_KeepCompressed();
		void TestFlacImport_Default();

		Path mImagePath;
		Path mOggPath;
		Path mFlacPath;
	};
} // namespace b3d
