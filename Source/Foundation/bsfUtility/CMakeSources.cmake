set(BS_UTILITY_INC_THREADING
	"bsfUtility/Threading/BsThreading.h"
	"bsfUtility/Threading/BsAsyncOp.h"
	"bsfUtility/Threading/BsSpinLock.h"
	"bsfUtility/Threading/BsThreadPool.h"
	"bsfUtility/Threading/BsTaskScheduler.h"
)

set(BS_UTILITY_SRC_THIRDPARTY
	"bsfUtility/ThirdParty/md5.cpp"
	"bsfUtility/ThirdParty/TetGen/tetgen.cxx"
	"bsfUtility/ThirdParty/TetGen/predicates.cxx"
)

set(BS_UTILITY_INC_PREREQUISITES
	"bsfUtility/Prerequisites/BsTypes.h"
	"bsfUtility/Prerequisites/BsStdHeaders.h"
	"bsfUtility/Prerequisites/BsPrerequisitesUtil.h"
	"bsfUtility/Prerequisites/BsPlatformDefines.h"
	"bsfUtility/Prerequisites/BsFwdDeclUtil.h"
)

set(BS_UTILITY_INC_IMAGE
	"bsfUtility/Image/BsColor.h"
	"bsfUtility/Image/BsTextureAtlasLayout.h"
	"bsfUtility/Image/BsColorGradient.h"
)

set(BS_UTILITY_INC_STRING
	"bsfUtility/String/BsString.h"
	"bsfUtility/String/BsStringFormat.h"
	"bsfUtility/String/BsStringID.h"
	"bsfUtility/String/BsUnicode.h"
)

set(BS_UTILITY_SRC_IMAGE
	"bsfUtility/Image/BsColor.cpp"
	"bsfUtility/Image/BsTextureAtlasLayout.cpp"
	"bsfUtility/Image/BsColorGradient.cpp"
)

set(BS_UTILITY_SRC_UTILITY
	"bsfUtility/Utility/BsDynLib.cpp"
	"bsfUtility/Utility/BsDynLibManager.cpp"
	"bsfUtility/Utility/BsMessageHandler.cpp"
	"bsfUtility/Utility/BsTimer.cpp"
	"bsfUtility/Utility/BsTime.cpp"
	"bsfUtility/Utility/BsUtil.cpp"
	"bsfUtility/Utility/BsCompression.cpp"
	"bsfUtility/Utility/BsTriangulation.cpp"
	"bsfUtility/Utility/BsUUID.cpp"
	"bsfUtility/Utility/BsLookupTable.cpp"
	"bsfUtility/Utility/BsBitstream.cpp"
)

set(BS_UTILITY_INC_DEBUG
	"bsfUtility/Debug/BsBitmapWriter.h"
	"bsfUtility/Debug/BsDebug.h"
	"bsfUtility/Debug/BsLog.h"
)

set(BS_UTILITY_INC_FILESYSTEM
	"bsfUtility/FileSystem/BsFileSystem.h"
	"bsfUtility/FileSystem/BsDataStream.h"
	"bsfUtility/FileSystem/BsPath.h"
)

set(BS_UTILITY_SRC_FILESYSTEM
	"bsfUtility/FileSystem/BsDataStream.cpp"
	"bsfUtility/FileSystem/BsFileSystem.cpp"
	"bsfUtility/FileSystem/BsPath.cpp"
)

set(BS_UTILITY_SRC_THREADING
	"bsfUtility/Threading/BsAsyncOp.cpp"
	"bsfUtility/Threading/BsTaskScheduler.cpp"
	"bsfUtility/Threading/BsThreadPool.cpp"
)

set(BS_UTILITY_INC_UTILITY
	"bsfUtility/Utility/BsAny.h"
	"bsfUtility/Utility/BsBitfield.h"
	"bsfUtility/Utility/BsBitwise.h"
	"bsfUtility/Utility/BsBitstream.h"
	"bsfUtility/Utility/BsBitLength.h"
	"bsfUtility/Utility/BsBufferedBitstream.h"
	"bsfUtility/Utility/BsDynLib.h"
	"bsfUtility/Utility/BsDynLibManager.h"
	"bsfUtility/Utility/BsEvent.h"
	"bsfUtility/Utility/BsMessageHandler.h"
	"bsfUtility/Utility/BsMessageHandlerFwd.h"
	"bsfUtility/Utility/BsModule.h"
	"bsfUtility/Utility/BsPlatformUtility.h"
	"bsfUtility/Utility/BsServiceLocator.h"
	"bsfUtility/Utility/BsTime.h"
	"bsfUtility/Utility/BsTimer.h"
	"bsfUtility/Utility/BsUtil.h"
	"bsfUtility/Utility/BsFlags.h"
	"bsfUtility/Utility/BsCompression.h"
	"bsfUtility/Utility/BsTriangulation.h"
	"bsfUtility/Utility/BsNonCopyable.h"
	"bsfUtility/Utility/BsUUID.h"
	"bsfUtility/Utility/BsOctree.h"
	"bsfUtility/Utility/BsDataBlob.h"
	"bsfUtility/Utility/BsLookupTable.h"
	"bsfUtility/Utility/BsSmallVector.h"
	"bsfUtility/Utility/BsDynArray.h"
	"bsfUtility/Utility/BsMinHeap.h"
	"bsfUtility/Utility/BsDenseMap.h"
	"bsfUtility/Utility/BsUSPtr.h"
)

set(BS_UTILITY_SRC_ALLOCATORS
	"bsfUtility/Allocators/BsFrameAlloc.cpp"
	"bsfUtility/Allocators/BsStackAlloc.cpp"
	"bsfUtility/Allocators/BsMemoryAllocator.cpp"
)

set(BS_UTILITY_SRC_REFLECTION
	"bsfUtility/Reflection/BsIReflectable.cpp"
	"bsfUtility/Reflection/BsRTTIField.cpp"
	"bsfUtility/Reflection/BsRTTIType.cpp"
	"bsfUtility/Reflection/BsRTTIPlain.h"
)

set(BS_UTILITY_SRC_DEBUG
	"bsfUtility/Debug/BsBitmapWriter.cpp"
	"bsfUtility/Debug/BsLog.cpp"
	"bsfUtility/Debug/BsDebug.cpp"
)

set(BS_UTILITY_INC_RTTI
	"bsfUtility/RTTI/BsSerializedObjectRTTI.h"
	"bsfUtility/RTTI/BsIReflectableRTTI.h"
	"bsfUtility/RTTI/BsColorGradientRTTI.h"
	"bsfUtility/RTTI/BsStringRTTI.h"
	"bsfUtility/RTTI/BsSmallVectorRTTI.h"
	"bsfUtility/RTTI/BsFlagsRTTI.h"
	"bsfUtility/RTTI/BsStdRTTI.h"
	"bsfUtility/RTTI/BsPathRTTI.h"
	"bsfUtility/RTTI/BsStringIDRTTI.h"
	"bsfUtility/RTTI/BsDataBlobRTTI.h"
	"bsfUtility/RTTI/BsMathRTTI.h"
	"bsfUtility/RTTI/BsRectOffsetRTTI.h"
	"bsfUtility/RTTI/BsColorRTTI.h"
	"bsfUtility/RTTI/BsUUIDRTTI.h"
	"bsfUtility/RTTI/BsRTTISchemaRTTI.h"
	"bsfUtility/RTTI/BsBitLengthRTTI.h"
)

set(BS_UTILITY_INC_ALLOCATORS
	"bsfUtility/Allocators/BsFrameAlloc.h"
	"bsfUtility/Allocators/BsMemAllocProfiler.h"
	"bsfUtility/Allocators/BsMemoryAllocator.h"
	"bsfUtility/Allocators/BsStackAlloc.h"
	"bsfUtility/Allocators/BsStaticAlloc.h"
	"bsfUtility/Allocators/BsGroupAlloc.h"
	"bsfUtility/Allocators/BsFreeAlloc.h"
	"bsfUtility/Allocators/BsPoolAlloc.h"
)

set(BS_UTILITY_INC_THIRDPARTY
	"bsfUtility/ThirdParty/md5.h"
	"bsfUtility/ThirdParty/json.hpp"
	"bsfUtility/ThirdParty/TetGen/tetgen.h"
)

set(BS_UTILITY_INC_ERROR
	"bsfUtility/Error/BsException.h"
	"bsfUtility/Error/BsCrashHandler.h"
)

set(BS_UTILITY_SRC_MATH
	"bsfUtility/Math/BsAABox.cpp"
	"bsfUtility/Math/BsDegree.cpp"
	"bsfUtility/Math/BsMath.cpp"
	"bsfUtility/Math/BsMatrix3.cpp"
	"bsfUtility/Math/BsMatrix4.cpp"
	"bsfUtility/Math/BsPlane.cpp"
	"bsfUtility/Math/BsQuaternion.cpp"
	"bsfUtility/Math/BsRadian.cpp"
	"bsfUtility/Math/BsRay.cpp"
	"bsfUtility/Math/BsSphere.cpp"
	"bsfUtility/Math/BsVector2.cpp"
	"bsfUtility/Math/BsVector2I.cpp"
	"bsfUtility/Math/BsVector3.cpp"
	"bsfUtility/Math/BsVector4.cpp"
	"bsfUtility/Math/BsBounds.cpp"
	"bsfUtility/Math/BsConvexVolume.cpp"
	"bsfUtility/Math/BsTorus.cpp"
	"bsfUtility/Math/BsRect3.cpp"
	"bsfUtility/Math/BsRect2.cpp"
	"bsfUtility/Math/BsRect2I.cpp"
	"bsfUtility/Math/BsLineSegment3.cpp"
	"bsfUtility/Math/BsCapsule.cpp"
	"bsfUtility/Math/BsLine2.cpp"
)

set(BS_UTILITY_INC_TESTING
	"bsfUtility/Testing/BsTestSuite.h"
	"bsfUtility/Testing/BsTestOutput.h"
	"bsfUtility/Testing/BsConsoleTestOutput.h"
)

set(BS_UTILITY_SRC_TESTING
	"bsfUtility/Testing/BsTestSuite.cpp"
	"bsfUtility/Testing/BsTestOutput.cpp"
	"bsfUtility/Testing/BsConsoleTestOutput.cpp"
)

set(BS_UTILITY_SRC_SERIALIZATION
	"bsfUtility/Serialization/BsFileSerializer.cpp"
	"bsfUtility/Serialization/BsBinarySerializer.cpp"
	"bsfUtility/Serialization/BsBinaryDiff.cpp"
	"bsfUtility/Serialization/BsSerializedObject.cpp"
	"bsfUtility/Serialization/BsBinaryCloner.cpp"
	"bsfUtility/Serialization/BsBinaryCompare.cpp"
	"bsfUtility/Serialization/BsIntermediateSerializer.cpp"
)

set(BS_UTILITY_INC_MATH
	"bsfUtility/Math/BsAABox.h"
	"bsfUtility/Math/BsDegree.h"
	"bsfUtility/Math/BsMath.h"
	"bsfUtility/Math/BsMatrix3.h"
	"bsfUtility/Math/BsMatrix4.h"
	"bsfUtility/Math/BsPlane.h"
	"bsfUtility/Math/BsQuaternion.h"
	"bsfUtility/Math/BsRadian.h"
	"bsfUtility/Math/BsRay.h"
	"bsfUtility/Math/BsSphere.h"
	"bsfUtility/Math/BsVector2.h"
	"bsfUtility/Math/BsVector2I.h"
	"bsfUtility/Math/BsVector3.h"
	"bsfUtility/Math/BsVector3I.h"
	"bsfUtility/Math/BsVector4.h"
	"bsfUtility/Math/BsVector4I.h"
	"bsfUtility/Math/BsBounds.h"
	"bsfUtility/Math/BsConvexVolume.h"
	"bsfUtility/Math/BsTorus.h"
	"bsfUtility/Math/BsLineSegment3.h"
	"bsfUtility/Math/BsRect3.h"
	"bsfUtility/Math/BsRect2.h"
	"bsfUtility/Math/BsRect2I.h"
	"bsfUtility/Math/BsCapsule.h"
	"bsfUtility/Math/BsMatrixNxM.h"
	"bsfUtility/Math/BsLine2.h"
	"bsfUtility/Math/BsSIMD.h"
	"bsfUtility/Math/BsRandom.h"
	"bsfUtility/Math/BsComplex.h"
)

set(BS_UTILITY_SRC_ERROR
	"bsfUtility/Error/BsCrashHandler.cpp"
)

set(BS_UTILITY_INC_SERIALIZATION
	"bsfUtility/Serialization/BsBinarySerializer.h"
	"bsfUtility/Serialization/BsFileSerializer.h"
	"bsfUtility/Serialization/BsBinaryDiff.h"
	"bsfUtility/Serialization/BsSerializedObject.h"
	"bsfUtility/Serialization/BsBinaryCloner.h"
	"bsfUtility/Serialization/BsBinaryCompare.h"
	"bsfUtility/Serialization/BsIntermediateSerializer.h"
)

set(BS_UTILITY_SRC_STRING
	"bsfUtility/String/BsStringID.cpp"
	"bsfUtility/String/BsString.cpp"
	"bsfUtility/String/BsUnicode.cpp"
)

set(BS_UTILITY_INC_REFLECTION
	"bsfUtility/Reflection/BsIReflectable.h"
	"bsfUtility/Reflection/BsRTTIField.h"
	"bsfUtility/Reflection/BsRTTIManagedDataBlockField.h"
	"bsfUtility/Reflection/BsRTTIPlainField.h"
	"bsfUtility/Reflection/BsRTTIReflectableField.h"
	"bsfUtility/Reflection/BsRTTIReflectablePtrField.h"
	"bsfUtility/Reflection/BsRTTIType.h"
)

set(BS_UTILITY_INC_WIN32
	"bsfUtility/Private/Win32/BsWin32PlatformUtility.h"
	"bsfUtility/Private/Win32/BsWin32Window.h"
	"bsfUtility/Private/Win32/BsWindows.h"
)

set(BS_UTILITY_SRC_WIN32
	"bsfUtility/Private/Win32/BsWin32FileSystem.cpp"
	"bsfUtility/Private/Win32/BsWin32CrashHandler.cpp"
	"bsfUtility/Private/Win32/BsWin32PlatformUtility.cpp"
	"bsfUtility/Private/Win32/BsWin32Window.cpp"
	"bsfUtility/Private/Win32/BsWin32Resource.rc"
)

set(BS_UTILITY_SRC_UNIX
	"bsfUtility/Private/Unix/BsUnixFileSystem.cpp"
	"bsfUtility/Private/Unix/BsUnixCrashHandler.cpp"
)

set(BS_UTILITY_SRC_LINUX
	"bsfUtility/Private/Linux/BsLinuxPlatformUtility.cpp"
)

set(BS_UTILITY_SRC_MACOS
	"bsfUtility/Private/MacOS/BsMacOSPlatformUtility.cpp"
)

source_group("Threading" FILES ${BS_UTILITY_INC_THREADING} ${BS_UTILITY_SRC_THREADING})
source_group("Prerequisites" FILES ${BS_UTILITY_INC_PREREQUISITES})
source_group("Image" FILES ${BS_UTILITY_INC_IMAGE} ${BS_UTILITY_SRC_IMAGE})
source_group("String" FILES ${BS_UTILITY_INC_STRING} ${BS_UTILITY_SRC_STRING})
source_group("Debug" FILES ${BS_UTILITY_INC_DEBUG} ${BS_UTILITY_SRC_DEBUG})
source_group("FileSystem" FILES ${BS_UTILITY_INC_FILESYSTEM} ${BS_UTILITY_SRC_FILESYSTEM})
source_group("Utility" FILES ${BS_UTILITY_INC_UTILITY} ${BS_UTILITY_SRC_UTILITY})
source_group("RTTI" FILES ${BS_UTILITY_INC_RTTI})
source_group("Allocators" FILES ${BS_UTILITY_INC_ALLOCATORS} ${BS_UTILITY_SRC_ALLOCATORS})
source_group("ThirdParty" FILES ${BS_UTILITY_INC_THIRDPARTY} ${BS_UTILITY_SRC_THIRDPARTY})
source_group("Error" FILES ${BS_UTILITY_INC_ERROR} ${BS_UTILITY_SRC_ERROR})
source_group("Testing" FILES ${BS_UTILITY_INC_TESTING} ${BS_UTILITY_SRC_TESTING})
source_group("Math" FILES ${BS_UTILITY_INC_MATH} ${BS_UTILITY_SRC_MATH})
source_group("Serialization" FILES ${BS_UTILITY_INC_SERIALIZATION} ${BS_UTILITY_SRC_SERIALIZATION})
source_group("Reflection" FILES ${BS_UTILITY_INC_REFLECTION} ${BS_UTILITY_SRC_REFLECTION})
source_group("Win32" FILES ${BS_UTILITY_INC_WIN32} ${BS_UTILITY_SRC_WIN32})

set(BS_UTILITY_SRC
	${BS_UTILITY_INC_THREADING}
	${BS_UTILITY_SRC_THIRDPARTY}
	${BS_UTILITY_INC_PREREQUISITES}
	${BS_UTILITY_INC_IMAGE}
	${BS_UTILITY_INC_STRING}
	${BS_UTILITY_SRC_IMAGE}
	${BS_UTILITY_SRC_UTILITY}
	${BS_UTILITY_INC_DEBUG}
	${BS_UTILITY_INC_FILESYSTEM}
	${BS_UTILITY_SRC_FILESYSTEM}
	${BS_UTILITY_SRC_THREADING}
	${BS_UTILITY_INC_UTILITY}
	${BS_UTILITY_SRC_ALLOCATORS}
	${BS_UTILITY_SRC_REFLECTION}
	${BS_UTILITY_SRC_DEBUG}
	${BS_UTILITY_INC_RTTI}
	${BS_UTILITY_INC_ALLOCATORS}
	${BS_UTILITY_INC_THIRDPARTY}
	${BS_UTILITY_INC_ERROR}
	${BS_UTILITY_SRC_MATH}
	${BS_UTILITY_INC_TESTING}
	${BS_UTILITY_SRC_TESTING}
	${BS_UTILITY_SRC_SERIALIZATION}
	${BS_UTILITY_INC_MATH}
	${BS_UTILITY_SRC_ERROR}
	${BS_UTILITY_INC_SERIALIZATION}
	${BS_UTILITY_SRC_STRING}
	${BS_UTILITY_INC_REFLECTION}
)

if(WIN32)
	list(APPEND BS_UTILITY_SRC ${BS_UTILITY_SRC_WIN32})
	list(APPEND BS_UTILITY_SRC ${BS_UTILITY_INC_WIN32})
endif()

if(UNIX)
	list(APPEND BS_UTILITY_SRC ${BS_UTILITY_SRC_UNIX})

	if(LINUX)
		list(APPEND BS_UTILITY_SRC ${BS_UTILITY_SRC_LINUX})
	elseif(APPLE)
		list(APPEND BS_UTILITY_SRC ${BS_UTILITY_SRC_MACOS})
	endif()
endif()
