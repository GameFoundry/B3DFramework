//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/B3DSnapshotTestRunner.h"
#include "Components/B3DCamera.h"
#include "Debug/B3DDebug.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"
#include "Image/B3DPixelUtility.h"
#include "Scene/B3DSceneManager.h"
#include "Scene/B3DSceneInstance.h"
#include "Utility/B3DTime.h"
#include "Utility/B3DCommandLine.h"
#include "ThirdParty/json.hpp"

namespace
{
	using namespace b3d;

	const char* TestResultStatusToString(SnapshotTestStatus status)
	{
		switch(status)
		{
		case SnapshotTestStatus::Passed:
			return "passed";
		case SnapshotTestStatus::Failed:
			return "failed";
		case SnapshotTestStatus::PassedWithWarnings:
			return "passed_with_warnings";
		default:
			return "unknown";
		}
	}

	void WriteTestResultJson(const Path& outputPath, const SnapshotTestResult& result)
	{
		nlohmann::json root;
		root["type"] = "snapshot_test";
		root["testName"] = result.TestName.c_str();
		root["status"] = static_cast<int>(result.Status);
		root["statusText"] = TestResultStatusToString(result.Status);
		root["totalFrames"] = result.TotalFrames;
		root["executionTimeSeconds"] = result.ExecutionTimeSeconds;
		root["screenshotPath"] = result.ScreenshotPath.ToString().c_str();

		nlohmann::json errorsArray = nlohmann::json::array();
		for(const auto& error : result.Errors)
			errorsArray.push_back(error.c_str());
		root["errors"] = errorsArray;

		nlohmann::json warningsArray = nlohmann::json::array();
		for(const auto& warning : result.Warnings)
			warningsArray.push_back(warning.c_str());
		root["warnings"] = warningsArray;

		String jsonString(root.dump(2).c_str());
		SPtr<DataStream> fileStream = FileSystem::CreateAndOpenFile(outputPath);
		if(fileStream)
			fileStream->WriteString(jsonString);
	}
}

using namespace b3d;

SnapshotTestRunner::SnapshotTestRunner(const SnapshotTestConfiguration& configuration)
	: mConfiguration(configuration)
{
	mStartTimeUs = GetTime().GetTimePrecise();
	mStartFrame = GetTime().GetCurrentFrameIndex();
	mExitAfterNFrames = (u64)CommandLine::GetParameterValueAsInt("ExitAfterNFrames", 0);
	mResult.TestName = configuration.GetEffectiveTestName();

	// Set up log callback to capture logs
	auto fnLogCallback = [this](const String& msg, LogVerbosity verb, const char*) -> bool {
		return OnLogEntry(msg, verb);
	};

	GetDebug().SetLogCallback(fnLogCallback);
}

SnapshotTestRunner::~SnapshotTestRunner()
{
	GetDebug().SetLogCallback(nullptr);
}

void SnapshotTestRunner::Update()
{
	if(mFinalized || mCaptureRequested)
		return;

	const u64 currentFrame = GetTime().GetCurrentFrameIndex();

	// Determine capture frame: use config if specified, otherwise exitFrame - 2
	u64 captureFrame = mConfiguration.CaptureFrame;
	if(captureFrame == 0 && mExitAfterNFrames > 2)
		captureFrame = mExitAfterNFrames - 2;

	if(currentFrame >= captureFrame && !mCaptureRequested)
		RequestScreenCapture();
}

void SnapshotTestRunner::Finalize()
{
	if(mFinalized)
		return;

	mFinalized = true;

	// Clear log callback before finalization
	GetDebug().SetLogCallback(nullptr);

	// Calculate execution time and total frames
	const u64 executionTimeUs = GetTime().GetTimePrecise() - mStartTimeUs;
	mResult.ExecutionTimeSeconds = (float)((double)(executionTimeUs) * 1e-6);
	mResult.TotalFrames = GetTime().GetCurrentFrameIndex() - mStartFrame;

	// Wait for capture to complete if in progress
	if(mScreenCaptureOp != nullptr)
	{
		mScreenCaptureOp.BlockUntilComplete();
		SPtr<PixelData> pixelData = mScreenCaptureOp.GetReturnValue();
		if(pixelData)
		{
			if(!SaveScreenshot(pixelData))
				mResult.Status = SnapshotTestStatus::Failed;
		}
		else
		{
			mResult.Errors.push_back("Failed to capture screenshot: capture returned null");
			mResult.Status = SnapshotTestStatus::Failed;
		}
	}
	else if(!mCaptureRequested)
	{
		// No capture was requested (maybe exit frame is too short), try to capture now
		RequestScreenCapture();
		if(mScreenCaptureOp != nullptr)
		{
			mScreenCaptureOp.BlockUntilComplete();
			SPtr<PixelData> pixelData = mScreenCaptureOp.GetReturnValue();
			if(pixelData)
			{
				if(!SaveScreenshot(pixelData))
					mResult.Status = SnapshotTestStatus::Failed;
			}
		}
	}

	// Check for errors/warnings in logs
	if(!mResult.Errors.empty())
		mResult.Status = SnapshotTestStatus::Failed;
	else if(!mResult.Warnings.empty())
	{
		if(mResult.Status == SnapshotTestStatus::Passed)
			mResult.Status = SnapshotTestStatus::PassedWithWarnings;
	}

	// Ensure output directory exists
	if(!FileSystem::Exists(mConfiguration.OutputPath))
		FileSystem::CreateDir(mConfiguration.OutputPath);

	// Write result files
	WriteResultJson();
	WriteLogFile();
}

void SnapshotTestRunner::RequestScreenCapture()
{
	if(mCaptureRequested)
		return;

	mCaptureRequested = true;

	// Get main camera from main scene
	const SPtr<SceneInstance>& mainScene = GetSceneManager().GetMainScene();
	if(!mainScene)
	{
		mResult.Errors.push_back("No main scene available for screenshot capture");
		return;
	}

	HCamera mainCamera = mainScene->GetMainCamera();
	if(!mainCamera.IsValid())
	{
		mResult.Errors.push_back("No main camera available for screenshot capture");
		return;
	}

	mScreenCaptureOp = mainCamera->RequestCapture();
}

bool SnapshotTestRunner::SaveScreenshot(const SPtr<PixelData>& pixelData)
{
	if(!pixelData)
		return false;

	// Build screenshot path
	String filename = mResult.TestName + "_screenshot.png";
	Path screenshotPath = mConfiguration.OutputPath;
	screenshotPath.Append(filename);

	mResult.ScreenshotPath = screenshotPath;

	bool success = PixelUtility::SaveImage(pixelData, screenshotPath, ImageFormat::PNG);
	if(!success)
		mResult.Errors.push_back("Failed to save screenshot to: " + screenshotPath.ToString());

	return success;
}

bool SnapshotTestRunner::WriteResultJson()
{
	String filename = mResult.TestName + "_result.json";
	Path jsonPath = mConfiguration.OutputPath;
	jsonPath.Append(filename);

	WriteTestResultJson(jsonPath, mResult);
	return true;
}

bool SnapshotTestRunner::WriteLogFile()
{
	String filename = mResult.TestName + "_log.txt";
	Path logPath = mConfiguration.OutputPath;
	logPath.Append(filename);

	GetDebug().SaveTextLog(logPath);
	return true;
}

bool SnapshotTestRunner::OnLogEntry(const String& message, LogVerbosity verbosity)
{
	if(verbosity == LogVerbosity::Error || verbosity == LogVerbosity::Fatal)
		mResult.Errors.push_back(message);
	else if(verbosity == LogVerbosity::Warning)
		mResult.Warnings.push_back(message);

	// Return false to allow normal log processing to continue
	return false;
}
