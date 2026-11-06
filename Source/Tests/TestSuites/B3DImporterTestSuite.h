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
		void TestGpuCompress_Perf();
		void TestGpuMipmaps_BoxFilter();

		void TestOggImport_Default();
		void TestOggImport_KeepCompressed();
		void TestFlacImport_Default();
		void TestAudioClipSerialization();
		/** Verifies saving over a package a loaded clip streams from replaces the package, and that failures reach the caller. */
		void TestAudioClipOverwriteWhileStreaming();
		/** Checks that a clip loaded from a package can be saved again in any read mode, by reading its data from the package file, and that it fails gracefully once the file no longer contains the data. */
		void TestAudioClipResaveFromPackage();

		/** Checks that a clip deserialized from one package can be saved into a new package and leaves the source package file free. */
		void TestAudioClipCopyToNewPackage();

		/** Checks that a loaded clip stays usable, and keeps track of its data, as its package file gets written to, moved, replaced and removed. */
		void TestAudioClipFollowsPackageFile();
		/** Verifies a failed package save leaves the package able to read from its existing file. */
		void TestPackageFailedSaveKeepsState();
		void TestPackageSaveReportsCopiedResources();
		void TestPackageMoveReservesDestination();
		void TestPackageWriteLockWithResourceDestroy();
		/** Verifies resources that are not compressible stay uncompressed in a compressed package, and that others are compressed. */
		void TestPackagePerResourceCompression();
		/** Verifies updating only the package meta-data leaves a package that loads, with its resources stored as they were. */
		void TestPackageMetaDataOnlySave();
		/** Verifies a package returns its resources when looked up by path. */
		void TestPackageGetResourceByPath();
		/** Verifies changing a resource in a package doesn't repeat its dependencies. */
		void TestPackageSetResourceDependencies();
		/** Verifies resources can be removed from a package by their id, including folders with their contents. */
		void TestPackageRemoveResourceById();
		/** Verifies threads locking the same missing package don't wait forever for each other. */
		void TestPackageConcurrentLockOfMissingPackage();

		/** Checks if a file can be opened for writing, which requires nothing else to have it open. */
		bool CanOpenForWriting(const Path& path);

		Path mImagePath;
		Path mOggPath;
		Path mFlacPath;
	};
} // namespace b3d
