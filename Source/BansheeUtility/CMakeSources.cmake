set(BS_BANSHEEUTILITY_INC_THREADING
	"Include/BsThreadDefines.h"
	"Include/BsAsyncOp.h"
	"Include/BsSpinLock.h"
	"Include/BsThreadPool.h"
	"Include/BsTaskScheduler.h"
)

set(BS_BANSHEEUTILITY_SRC_THIRDPARTY
	"Source/ThirdParty/md5.cpp"
)

set(BS_BANSHEEUTILITY_INC_PREREQUISITES
	"Include/BsTypes.h"
	"Include/BsStdHeaders.h"
	"Include/BsPrerequisitesUtil.h"
	"Include/BsPlatformDefines.h"
	"Include/BsFwdDeclUtil.h"
	"Include/BsRTTIPrerequisites.h"
)

set(BS_BANSHEEUTILITY_SRC_WIN32
	"Source/Win32/BsWin32FileSystem.cpp"
	"Source/Win32/BsWin32CrashHandler.cpp"
	"Source/Win32/BsWin32PlatformUtility.cpp"
	"Source/Win32/BsWin32Window.cpp"
)

set(BS_BANSHEEUTILITY_INC_IMAGE
	"Include/BsColor.h"
	"Include/BsTexAtlasGenerator.h"
)

set(BS_BANSHEEUTILITY_INC_STRING
	"Include/BsString.h"
	"Include/BsStringFormat.h"
	"Include/BsStringID.h"
)

set(BS_BANSHEEUTILITY_SRC_IMAGE
	"Source/BsColor.cpp"
	"Source/BsTexAtlasGenerator.cpp"
)

set(BS_BANSHEEUTILITY_SRC_GENERAL
	"Source/BsDynLib.cpp"
	"Source/BsDynLibManager.cpp"
	"Source/BsMessageHandler.cpp"
	"Source/BsTimer.cpp"
	"Source/BsTime.cpp"
	"Source/BsUtil.cpp"
)

set(BS_BANSHEEUTILITY_INC_DEBUG
	"Include/BsBitmapWriter.h"
	"Include/BsDebug.h"
	"Include/BsLog.h"
)

set(BS_BANSHEEUTILITY_INC_FILESYSTEM
	"Include/BsFileSystem.h"
	"Include/BsDataStream.h"
	"Include/BsPath.h"
)

set(BS_BANSHEEUTILITY_SRC_FILESYSTEM
	"Source/BsDataStream.cpp"
	"Source/BsPath.cpp"
)

set(BS_BANSHEEUTILITY_SRC_THREADING
	"Source/BsAsyncOp.cpp"
	"Source/BsTaskScheduler.cpp"
	"Source/BsThreadPool.cpp"
)

set(BS_BANSHEEUTILITY_INC_GENERAL
	"Include/BsAny.h"
	"Include/BsBitwise.h"
	"Include/BsDynLib.h"
	"Include/BsDynLibManager.h"
	"Include/BsEvent.h"
	"Include/BsMessageHandler.h"
	"Include/BsMessageHandlerFwd.h"
	"Include/BsModule.h"
	"Include/BsPlatformUtility.h"
	"Include/BsServiceLocator.h"
	"Include/BsTime.h"
	"Include/BsTimer.h"
	"Include/BsUtil.h"
	"Include/BsFlags.h"
)

set(BS_BANSHEEUTILITY_SRC_ALLOCATORS
	"Source/BsFrameAlloc.cpp"
	"Source/BsGlobalFrameAlloc.cpp"
	"Source/BsMemStack.cpp"
	"Source/BsMemoryAllocator.cpp"
)

set(BS_BANSHEEUTILITY_SRC_RTTI
	"Source/BsIReflectable.cpp"
	"Source/BsRTTIField.cpp"
	"Source/BsRTTIType.cpp"
)

set(BS_BANSHEEUTILITY_SRC_DEBUG
	"Source/BsBitmapWriter.cpp"
	"Source/BsLog.cpp"
	"Source/BsDebug.cpp"
)

set(BS_BANSHEEUTILITY_INC_RTTI
	"Include/BsSerializedObjectRTTI.h"
)

set(BS_BANSHEEUTILITY_INC_ALLOCATORS
	"Include/BsFrameAlloc.h"
	"Include/BsGlobalFrameAlloc.h"
	"Include/BsMemAllocProfiler.h"
	"Include/BsMemoryAllocator.h"
	"Include/BsMemStack.h"
	"Include/BsStaticAlloc.h"
)

set(BS_BANSHEEUTILITY_INC_THIRDPARTY
	"Include/ThirdParty/md5.h"
)

set(BS_BANSHEEUTILITY_INC_ERROR
	"Include/BsException.h"
	"Include/BsCrashHandler.h"
)

set(BS_BANSHEEUTILITY_SRC_MATH
	"Source/BsAABox.cpp"
	"Source/BsDegree.cpp"
	"Source/BsMath.cpp"
	"Source/BsMatrix3.cpp"
	"Source/BsMatrix4.cpp"
	"Source/BsPlane.cpp"
	"Source/BsQuaternion.cpp"
	"Source/BsRadian.cpp"
	"Source/BsRay.cpp"
	"Source/BsSphere.cpp"
	"Source/BsVector2.cpp"
	"Source/BsVector2I.cpp"
	"Source/BsVector3.cpp"
	"Source/BsVector4.cpp"
	"Source/BsBounds.cpp"
	"Source/BsConvexVolume.cpp"
	"Source/BsTorus.cpp"
	"Source/BsRect3.cpp"
	"Source/BsRect2.cpp"
	"Source/BsRect2I.cpp"
	"Source/BsLineSegment3.cpp"
	"Source/BsCapsule.cpp"
	"Source/BsLine2.cpp"
)

set(BS_BANSHEEUTILITY_INC_TESTING
	"Include/BsTestSuite.h"
	"Include/BsTestOutput.h"
)

set(BS_BANSHEEUTILITY_SRC_TESTING
	"Source/BsTestSuite.cpp"
	"Source/BsTestOutput.cpp"
)

set(BS_BANSHEEUTILITY_SRC_SERIALIZATION
	"Source/BsMemorySerializer.cpp"
	"Source/BsFileSerializer.cpp"
	"Source/BsBinarySerializer.cpp"
	"Source/BsBinaryDiff.cpp"
	"Source/BsSerializedObject.cpp"
	"Source/BsBinaryCloner.cpp"
)

set(BS_BANSHEEUTILITY_INC_MATH
	"Include/BsAABox.h"
	"Include/BsDegree.h"
	"Include/BsMath.h"
	"Include/BsMatrix3.h"
	"Include/BsMatrix4.h"
	"Include/BsPlane.h"
	"Include/BsQuaternion.h"
	"Include/BsRadian.h"
	"Include/BsRay.h"
	"Include/BsSphere.h"
	"Include/BsVector2.h"
	"Include/BsVector2I.h"
	"Include/BsVector3.h"
	"Include/BsVector4.h"
	"Include/BsBounds.h"
	"Include/BsConvexVolume.h"
	"Include/BsTorus.h"
	"Include/BsLineSegment3.h"
	"Include/BsRect3.h"
	"Include/BsRect2.h"
	"Include/BsRect2I.h"
	"Include/BsCapsule.h"
	"Include/BsMatrixNxM.h"
	"Include/BsVectorNI.h"
	"Include/BsLine2.h"
)

set(BS_BANSHEEUTILITY_SRC_ERROR
	"Source/BsException.cpp"
)

set(BS_BANSHEEUTILITY_INC_SERIALIZATION
	"Include/BsBinarySerializer.h"
	"Include/BsFileSerializer.h"
	"Include/BsMemorySerializer.h"
	"Include/BsBinaryDiff.h"
	"Include/BsSerializedObject.h"
	"Include/BsBinaryCloner.h"
)

set(BS_BANSHEEUTILITY_SRC_STRING
	"Source/BsStringID.cpp"
	"Source/BsString.cpp"
)

set(BS_BANSHEEUTILITY_INC_REFLECTION
	"Include/BsIReflectable.h"
	"Include/BsRTTIField.h"
	"Include/BsRTTIManagedDataBlockField.h"
	"Include/BsRTTIPlainField.h"
	"Include/BsRTTIReflectableField.h"
	"Include/BsRTTIReflectablePtrField.h"
	"Include/BsRTTIType.h"
)

set(BS_BANSHEEUTILITY_INC_WIN32
	"Include/Win32/BsWin32PlatformUtility.h"
	"Include/Win32/BsWin32Window.h"
)

source_group("Header Files\\Threading" FILES ${BS_BANSHEEUTILITY_INC_THREADING})
source_group("Source Files\\ThirdParty" FILES ${BS_BANSHEEUTILITY_SRC_THIRDPARTY})
source_group("Header Files\\Prerequisites" FILES ${BS_BANSHEEUTILITY_INC_PREREQUISITES})
source_group("Source Files\\Win32" FILES ${BS_BANSHEEUTILITY_SRC_WIN32})
source_group("Header Files\\Image" FILES ${BS_BANSHEEUTILITY_INC_IMAGE})
source_group("Header Files\\String" FILES ${BS_BANSHEEUTILITY_INC_STRING})
source_group("Source Files\\Image" FILES ${BS_BANSHEEUTILITY_SRC_IMAGE})
source_group("Source Files\\General" FILES ${BS_BANSHEEUTILITY_SRC_GENERAL})
source_group("Header Files\\Debug" FILES ${BS_BANSHEEUTILITY_INC_DEBUG})
source_group("Header Files\\Filesystem" FILES ${BS_BANSHEEUTILITY_INC_FILESYSTEM})
source_group("Source Files\\Filesystem" FILES ${BS_BANSHEEUTILITY_SRC_FILESYSTEM})
source_group("Source Files\\Threading" FILES ${BS_BANSHEEUTILITY_SRC_THREADING})
source_group("Header Files\\General" FILES ${BS_BANSHEEUTILITY_INC_GENERAL})
source_group("Source Files\\Allocators" FILES ${BS_BANSHEEUTILITY_SRC_ALLOCATORS})
source_group("Source Files\\RTTI" FILES ${BS_BANSHEEUTILITY_SRC_RTTI})
source_group("Source Files\\Debug" FILES ${BS_BANSHEEUTILITY_SRC_DEBUG})
source_group("Header Files\\RTTI" FILES ${BS_BANSHEEUTILITY_INC_RTTI})
source_group("Header Files\\Allocators" FILES ${BS_BANSHEEUTILITY_INC_ALLOCATORS})
source_group("Header Files\\ThirdParty" FILES ${BS_BANSHEEUTILITY_INC_THIRDPARTY})
source_group("Header Files\\Error" FILES ${BS_BANSHEEUTILITY_INC_ERROR})
source_group("Source Files\\Math" FILES ${BS_BANSHEEUTILITY_SRC_MATH})
source_group("Header Files\\Testing" FILES ${BS_BANSHEEUTILITY_INC_TESTING})
source_group("Source Files\\Testing" FILES ${BS_BANSHEEUTILITY_SRC_TESTING})
source_group("Source Files\\Serialization" FILES ${BS_BANSHEEUTILITY_SRC_SERIALIZATION})
source_group("Header Files\\Math" FILES ${BS_BANSHEEUTILITY_INC_MATH})
source_group("Source Files\\Error" FILES ${BS_BANSHEEUTILITY_SRC_ERROR})
source_group("Header Files\\Serialization" FILES ${BS_BANSHEEUTILITY_INC_SERIALIZATION})
source_group("Source Files\\String" FILES ${BS_BANSHEEUTILITY_SRC_STRING})
source_group("Header Files\\Reflection" FILES ${BS_BANSHEEUTILITY_INC_REFLECTION})
source_group("Header Files\\Win32" FILES ${BS_BANSHEEUTILITY_INC_WIN32})

set(BS_BANSHEEUTILITY_SRC
	${BS_BANSHEEUTILITY_INC_THREADING}
	${BS_BANSHEEUTILITY_SRC_THIRDPARTY}
	${BS_BANSHEEUTILITY_INC_PREREQUISITES}
	${BS_BANSHEEUTILITY_INC_IMAGE}
	${BS_BANSHEEUTILITY_INC_STRING}
	${BS_BANSHEEUTILITY_SRC_IMAGE}
	${BS_BANSHEEUTILITY_SRC_GENERAL}
	${BS_BANSHEEUTILITY_INC_DEBUG}
	${BS_BANSHEEUTILITY_INC_FILESYSTEM}
	${BS_BANSHEEUTILITY_SRC_FILESYSTEM}
	${BS_BANSHEEUTILITY_SRC_THREADING}
	${BS_BANSHEEUTILITY_INC_GENERAL}
	${BS_BANSHEEUTILITY_SRC_ALLOCATORS}
	${BS_BANSHEEUTILITY_SRC_RTTI}
	${BS_BANSHEEUTILITY_SRC_DEBUG}
	${BS_BANSHEEUTILITY_INC_RTTI}
	${BS_BANSHEEUTILITY_INC_ALLOCATORS}
	${BS_BANSHEEUTILITY_INC_THIRDPARTY}
	${BS_BANSHEEUTILITY_INC_ERROR}
	${BS_BANSHEEUTILITY_SRC_MATH}
	${BS_BANSHEEUTILITY_INC_TESTING}
	${BS_BANSHEEUTILITY_SRC_TESTING}
	${BS_BANSHEEUTILITY_SRC_SERIALIZATION}
	${BS_BANSHEEUTILITY_INC_MATH}
	${BS_BANSHEEUTILITY_SRC_ERROR}
	${BS_BANSHEEUTILITY_INC_SERIALIZATION}
	${BS_BANSHEEUTILITY_SRC_STRING}
	${BS_BANSHEEUTILITY_INC_REFLECTION}
)

if(WIN32)
	list(APPEND BS_BANSHEEUTILITY_SRC ${BS_BANSHEEUTILITY_SRC_WIN32})
	list(APPEND BS_BANSHEEUTILITY_SRC ${BS_BANSHEEUTILITY_INC_WIN32})
endif()
