set(BS_BANSHEEUTILITY_INC_THREADING
	"Threading/BsThreadDefines.h"
	"Threading/BsAsyncOp.h"
	"Threading/BsSpinLock.h"
	"Threading/BsThreadPool.h"
	"Threading/BsTaskScheduler.h"
)

set(BS_BANSHEEUTILITY_SRC_THIRDPARTY
	"ThirdParty/md5.cpp"
	"ThirdParty/TetGen/tetgen.cxx"
	"ThirdParty/TetGen/predicates.cxx"
)

set(BS_BANSHEEUTILITY_INC_PREREQUISITES
	"Prerequisites/BsTypes.h"
	"Prerequisites/BsStdHeaders.h"
	"Prerequisites/BsPrerequisitesUtil.h"
	"Prerequisites/BsPlatformDefines.h"
	"Prerequisites/BsFwdDeclUtil.h"
	"Prerequisites/BsRTTIPrerequisites.h"
)

set(BS_BANSHEEUTILITY_INC_IMAGE
	"Image/BsColor.h"
	"Image/BsTextureAtlasLayout.h"
)

set(BS_BANSHEEUTILITY_INC_STRING
	"String/BsString.h"
	"String/BsStringFormat.h"
	"String/BsStringID.h"
	"String/BsUnicode.h"
)

set(BS_BANSHEEUTILITY_SRC_IMAGE
	"Image/BsColor.cpp"
	"Image/BsTextureAtlasLayout.cpp"
)

set(BS_BANSHEEUTILITY_SRC_UTILITY
	"Utility/BsDynLib.cpp"
	"Utility/BsDynLibManager.cpp"
	"Utility/BsMessageHandler.cpp"
	"Utility/BsTimer.cpp"
	"Utility/BsTime.cpp"
	"Utility/BsUtil.cpp"
	"Utility/BsCompression.cpp"
	"Utility/BsTriangulation.cpp"
	"Utility/BsUUID.cpp"
)

set(BS_BANSHEEUTILITY_INC_DEBUG
	"Debug/BsBitmapWriter.h"
	"Debug/BsDebug.h"
	"Debug/BsLog.h"
)

set(BS_BANSHEEUTILITY_INC_FILESYSTEM
	"FileSystem/BsFileSystem.h"
	"FileSystem/BsDataStream.h"
	"FileSystem/BsPath.h"
)

set(BS_BANSHEEUTILITY_SRC_FILESYSTEM
	"FileSystem/BsDataStream.cpp"
	"FileSystem/BsFileSystem.cpp"
	"FileSystem/BsPath.cpp"
)

set(BS_BANSHEEUTILITY_SRC_THREADING
	"Threading/BsAsyncOp.cpp"
	"Threading/BsTaskScheduler.cpp"
	"Threading/BsThreadPool.cpp"
)

set(BS_BANSHEEUTILITY_INC_UTILITY
	"Utility/BsAny.h"
	"Utility/BsBitwise.h"
	"Utility/BsDynLib.h"
	"Utility/BsDynLibManager.h"
	"Utility/BsEvent.h"
	"Utility/BsMessageHandler.h"
	"Utility/BsMessageHandlerFwd.h"
	"Utility/BsModule.h"
	"Utility/BsPlatformUtility.h"
	"Utility/BsServiceLocator.h"
	"Utility/BsTime.h"
	"Utility/BsTimer.h"
	"Utility/BsUtil.h"
	"Utility/BsFlags.h"
	"Utility/BsCompression.h"
	"Utility/BsTriangulation.h"
	"Utility/BsNonCopyable.h"
	"Utility/BsUUID.h"
)

set(BS_BANSHEEUTILITY_SRC_ALLOCATORS
	"Allocators/BsFrameAlloc.cpp"
	"Allocators/BsGlobalFrameAlloc.cpp"
	"Allocators/BsMemStack.cpp"
	"Allocators/BsMemoryAllocator.cpp"
)

set(BS_BANSHEEUTILITY_SRC_REFLECTION
	"Reflection/BsIReflectable.cpp"
	"Reflection/BsRTTIField.cpp"
	"Reflection/BsRTTIType.cpp"
)

set(BS_BANSHEEUTILITY_SRC_DEBUG
	"Debug/BsBitmapWriter.cpp"
	"Debug/BsLog.cpp"
	"Debug/BsDebug.cpp"
)

set(BS_BANSHEEUTILITY_INC_RTTI
	"RTTI/BsSerializedObjectRTTI.h"
	"RTTI/BsIReflectableRTTI.h"
)

set(BS_BANSHEEUTILITY_INC_ALLOCATORS
	"Allocators/BsFrameAlloc.h"
	"Allocators/BsGlobalFrameAlloc.h"
	"Allocators/BsMemAllocProfiler.h"
	"Allocators/BsMemoryAllocator.h"
	"Allocators/BsMemStack.h"
	"Allocators/BsStaticAlloc.h"
	"Allocators/BsGroupAlloc.h"
)

set(BS_BANSHEEUTILITY_INC_THIRDPARTY
	"ThirdParty/md5.h"
	"ThirdParty/json.hpp"
	"ThirdParty/TetGen/tetgen.h"
)

set(BS_BANSHEEUTILITY_INC_ERROR
	"Error/BsException.h"
	"Error/BsCrashHandler.h"
)

set(BS_BANSHEEUTILITY_SRC_MATH
	"Math/BsAABox.cpp"
	"Math/BsDegree.cpp"
	"Math/BsMath.cpp"
	"Math/BsMatrix3.cpp"
	"Math/BsMatrix4.cpp"
	"Math/BsPlane.cpp"
	"Math/BsQuaternion.cpp"
	"Math/BsRadian.cpp"
	"Math/BsRay.cpp"
	"Math/BsSphere.cpp"
	"Math/BsVector2.cpp"
	"Math/BsVector2I.cpp"
	"Math/BsVector3.cpp"
	"Math/BsVector4.cpp"
	"Math/BsBounds.cpp"
	"Math/BsConvexVolume.cpp"
	"Math/BsTorus.cpp"
	"Math/BsRect3.cpp"
	"Math/BsRect2.cpp"
	"Math/BsRect2I.cpp"
	"Math/BsLineSegment3.cpp"
	"Math/BsCapsule.cpp"
	"Math/BsLine2.cpp"
)

set(BS_BANSHEEUTILITY_INC_TESTING
	"Testing/BsFileSystemTestSuite.h"
	"Testing/BsTestSuite.h"
	"Testing/BsTestOutput.h"
	"Testing/BsConsoleTestOutput.h"
)

set(BS_BANSHEEUTILITY_SRC_TESTING
	"Testing/BsFileSystemTestSuite.cpp"
	"Testing/BsTestSuite.cpp"
	"Testing/BsTestOutput.cpp"
	"Testing/BsConsoleTestOutput.cpp"
)

set(BS_BANSHEEUTILITY_SRC_SERIALIZATION
	"Serialization/BsMemorySerializer.cpp"
	"Serialization/BsFileSerializer.cpp"
	"Serialization/BsBinarySerializer.cpp"
	"Serialization/BsBinaryDiff.cpp"
	"Serialization/BsSerializedObject.cpp"
	"Serialization/BsBinaryCloner.cpp"
)

set(BS_BANSHEEUTILITY_INC_MATH
	"Math/BsAABox.h"
	"Math/BsDegree.h"
	"Math/BsMath.h"
	"Math/BsMatrix3.h"
	"Math/BsMatrix4.h"
	"Math/BsPlane.h"
	"Math/BsQuaternion.h"
	"Math/BsRadian.h"
	"Math/BsRay.h"
	"Math/BsSphere.h"
	"Math/BsVector2.h"
	"Math/BsVector2I.h"
	"Math/BsVector3.h"
	"Math/BsVector3I.h"
	"Math/BsVector4.h"
	"Math/BsVector4I.h"
	"Math/BsBounds.h"
	"Math/BsConvexVolume.h"
	"Math/BsTorus.h"
	"Math/BsLineSegment3.h"
	"Math/BsRect3.h"
	"Math/BsRect2.h"
	"Math/BsRect2I.h"
	"Math/BsCapsule.h"
	"Math/BsMatrixNxM.h"
	"Math/BsLine2.h"
)

set(BS_BANSHEEUTILITY_SRC_ERROR
	"Error/BsException.cpp"
	"Error/BsCrashHandler.cpp"
)

set(BS_BANSHEEUTILITY_INC_SERIALIZATION
	"Serialization/BsBinarySerializer.h"
	"Serialization/BsFileSerializer.h"
	"Serialization/BsMemorySerializer.h"
	"Serialization/BsBinaryDiff.h"
	"Serialization/BsSerializedObject.h"
	"Serialization/BsBinaryCloner.h"
)

set(BS_BANSHEEUTILITY_SRC_STRING
	"String/BsStringID.cpp"
	"String/BsString.cpp"
	"String/BsUnicode.cpp"
)

set(BS_BANSHEEUTILITY_INC_REFLECTION
	"Reflection/BsIReflectable.h"
	"Reflection/BsRTTIField.h"
	"Reflection/BsRTTIManagedDataBlockField.h"
	"Reflection/BsRTTIPlainField.h"
	"Reflection/BsRTTIReflectableField.h"
	"Reflection/BsRTTIReflectablePtrField.h"
	"Reflection/BsRTTIType.h"
)

set(BS_BANSHEEUTILITY_INC_WIN32
	"Win32/BsWin32PlatformUtility.h"
	"Win32/BsWin32Window.h"
)

set(BS_BANSHEEUTILITY_SRC_WIN32
	"Win32/BsWin32FileSystem.cpp"
	"Win32/BsWin32CrashHandler.cpp"
	"Win32/BsWin32PlatformUtility.cpp"
	"Win32/BsWin32Window.cpp"
)

set(BS_BANSHEEUTILITY_SRC_UNIX
	"Unix/BsUnixFileSystem.cpp"
)

set(BS_BANSHEEUTILITY_SRC_LINUX
	"Linux/BsLinuxCrashHandler.cpp"
	"Linux/BsLinuxPlatformUtility.cpp"
)

set(BS_BANSHEEUTILITY_SRC_MACOS
	"MacOS/BsMacOSCrashHandler.cpp"
	"MacOS/BsMacOSPlatformUtility.cpp"
)

source_group("Header Files\\Threading" FILES ${BS_BANSHEEUTILITY_INC_THREADING})
source_group("Source Files\\ThirdParty" FILES ${BS_BANSHEEUTILITY_SRC_THIRDPARTY})
source_group("Header Files\\Prerequisites" FILES ${BS_BANSHEEUTILITY_INC_PREREQUISITES})
source_group("Source Files\\Win32" FILES ${BS_BANSHEEUTILITY_SRC_WIN32})
source_group("Header Files\\Image" FILES ${BS_BANSHEEUTILITY_INC_IMAGE})
source_group("Header Files\\String" FILES ${BS_BANSHEEUTILITY_INC_STRING})
source_group("Source Files\\Image" FILES ${BS_BANSHEEUTILITY_SRC_IMAGE})
source_group("Source Files\\Utility" FILES ${BS_BANSHEEUTILITY_SRC_UTILITY})
source_group("Header Files\\Debug" FILES ${BS_BANSHEEUTILITY_INC_DEBUG})
source_group("Header Files\\FileSystem" FILES ${BS_BANSHEEUTILITY_INC_FILESYSTEM})
source_group("Source Files\\FileSystem" FILES ${BS_BANSHEEUTILITY_SRC_FILESYSTEM})
source_group("Source Files\\Threading" FILES ${BS_BANSHEEUTILITY_SRC_THREADING})
source_group("Header Files\\Utility" FILES ${BS_BANSHEEUTILITY_INC_UTILITY})
source_group("Source Files\\Allocators" FILES ${BS_BANSHEEUTILITY_SRC_ALLOCATORS})
source_group("Source Files\\Reflection" FILES ${BS_BANSHEEUTILITY_SRC_REFLECTION})
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
	${BS_BANSHEEUTILITY_SRC_UTILITY}
	${BS_BANSHEEUTILITY_INC_DEBUG}
	${BS_BANSHEEUTILITY_INC_FILESYSTEM}
	${BS_BANSHEEUTILITY_SRC_FILESYSTEM}
	${BS_BANSHEEUTILITY_SRC_THREADING}
	${BS_BANSHEEUTILITY_INC_UTILITY}
	${BS_BANSHEEUTILITY_SRC_ALLOCATORS}
	${BS_BANSHEEUTILITY_SRC_REFLECTION}
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

if(UNIX)
	list(APPEND BS_BANSHEEUTILITY_SRC ${BS_BANSHEEUTILITY_SRC_UNIX})

	if(LINUX)
		list(APPEND BS_BANSHEEUTILITY_SRC ${BS_BANSHEEUTILITY_SRC_LINUX})
	elseif(APPLE)
		list(APPEND BS_BANSHEEUTILITY_SRC ${BS_BANSHEEUTILITY_SRC_MACOS})
	endif()
endif()
