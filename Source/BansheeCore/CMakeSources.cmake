set(BS_BANSHEECORE_INC_COMPONENTS
	"Include/BsCBoxCollider.h"
	"Include/BsCCollider.h"
	"Include/BsCRigidbody.h"
	"Include/BsCSphereCollider.h"
	"Include/BsCCapsuleCollider.h"
	"Include/BsCPlaneCollider.h"
	"Include/BsCMeshCollider.h"
	"Include/BsCJoint.h"
	"Include/BsCFixedJoint.h"
	"Include/BsCHingeJoint.h"
	"Include/BsCDistanceJoint.h"
	"Include/BsCSliderJoint.h"
	"Include/BsCSphericalJoint.h"
	"Include/BsCD6Joint.h"
	"Include/BsCCharacterController.h"
	"Include/BsCAudioSource.h"
	"Include/BsCAudioListener.h"
	"Include/BsCCamera.h"
	"Include/BsCRenderable.h"
	"Include/BsCLight.h"
	"Include/BsCAnimation.h"
	"Include/BsCBone.h"	
	"Include/BsCReflectionProbe.h"
	"Include/BsCSkybox.h"
)

set(BS_BANSHEECORE_INC_PHYSICS
	"Include/BsPhysicsManager.h"
	"Include/BsPhysics.h"
	"Include/BsPhysicsMaterial.h"
	"Include/BsRigidbody.h"
	"Include/BsBoxCollider.h"
	"Include/BsSphereCollider.h"
	"Include/BsCapsuleCollider.h"
	"Include/BsPlaneCollider.h"
	"Include/BsFCollider.h"
	"Include/BsPhysicsMesh.h"
	"Include/BsMeshCollider.h"
	"Include/BsFJoint.h"
	"Include/BsJoint.h"
	"Include/BsFixedJoint.h"
	"Include/BsDistanceJoint.h"
	"Include/BsHingeJoint.h"
	"Include/BsSliderJoint.h"
	"Include/BsSphericalJoint.h"
	"Include/BsD6Joint.h"
	"Include/BsCharacterController.h"
	"Include/BsCollider.h"
	"Include/BsPhysicsCommon.h"
)

set(BS_BANSHEECORE_INC_CORETHREAD
	"Include/BsCoreThreadQueue.h"
	"Include/BsCoreThread.h"
	"Include/BsCoreObjectManager.h"
	"Include/BsCoreObject.h"
	"Include/BsCommandQueue.h"
	"Include/BsCoreObjectCore.h"
)

set(BS_BANSHEECORE_INC_IMPORTER
	"Include/BsSpecificImporter.h"
	"Include/BsImportOptions.h"
	"Include/BsImporter.h"
	"Include/BsTextureImportOptions.h"
	"Include/BsShaderIncludeImporter.h"
	"Include/BsMeshImportOptions.h"
	"Include/BsShaderImportOptions.h"
)

set(BS_BANSHEECORE_INC_SCENE
	"Include/BsComponent.h"
	"Include/BsGameObject.h"
	"Include/BsGameObjectHandle.h"
	"Include/BsGameObjectManager.h"
	"Include/BsSceneObject.h"
	"Include/BsSceneManager.h"
	"Include/BsPrefab.h"
	"Include/BsPrefabDiff.h"
	"Include/BsPrefabUtility.h"
)

set(BS_BANSHEECORE_INC_INPUT
	"Include/BsRawInputHandler.h"
	"Include/BsOSInputHandler.h"
	"Include/BsInputFwd.h"
	"Include/BsInput.h"
)

set(BS_BANSHEECORE_INC_RENDERER
	"Include/BsLight.h"
	"Include/BsRenderable.h"
	"Include/BsRendererManager.h"
	"Include/BsRendererFactory.h"
	"Include/BsRenderer.h"
	"Include/BsRendererMeshData.h"
	"Include/BsParamBlocks.h"
	"Include/BsCamera.h"
	"Include/BsPostProcessSettings.h"
	"Include/BsRendererExtension.h"
	"Include/BsReflectionProbe.h"
	"Include/BsSkybox.h"
)

set(BS_BANSHEECORE_SRC_LOCALIZATION
	"Source/BsHString.cpp"
	"Source/BsStringTable.cpp"
	"Source/BsStringTableManager.cpp"
)

set(BS_BANSHEECORE_SRC_RTTI
	"Source/BsMaterialRTTI.cpp"
)

set(BS_BANSHEECORE_INC_PROFILING
	"Include/BsProfilerCPU.h"
	"Include/BsProfilerGPU.h"
	"Include/BsProfilingManager.h"
	"Include/BsRenderStats.h"
)

set(BS_BANSHEECORE_INC_RENDERAPI
	"Include/BsVideoModeInfo.h"
	"Include/BsVertexDeclaration.h"
	"Include/BsVertexData.h"
	"Include/BsVertexBuffer.h"
	"Include/BsTimerQuery.h"
	"Include/BsTextureView.h"
	"Include/BsSubMesh.h"
	"Include/BsSamplerState.h"
	"Include/BsRenderWindow.h"
	"Include/BsRenderTexture.h"
	"Include/BsRenderTarget.h"
	"Include/BsRasterizerState.h"
	"Include/BsOcclusionQuery.h"
	"Include/BsIndexBuffer.h"
	"Include/BsHardwareBuffer.h"
	"Include/BsGpuProgram.h"
	"Include/BsGpuParams.h"
	"Include/BsGpuParamDesc.h"
	"Include/BsGpuParamBlockBuffer.h"
	"Include/BsGpuParam.h"
	"Include/BsGpuBuffer.h"
	"Include/BsEventQuery.h"
	"Include/BsDepthStencilState.h"
	"Include/BsBlendState.h"
	"Include/BsRenderAPI.h"
	"Include/BsRenderAPICapabilities.h"
	"Include/BsViewport.h"
	"Include/BsCommandBuffer.h"
	"Include/BsGpuPipelineState.h"
	"Include/BsGpuPipelineParamInfo.h"
)

set(BS_BANSHEECORE_INC_RENDERAPI_MANAGERS
	"Include/BsRenderWindowManager.h"
	"Include/BsRenderStateManager.h"
	"Include/BsQueryManager.h"
	"Include/BsMeshManager.h"
	"Include/BsHardwareBufferManager.h"
	"Include/BsGpuProgramManager.h"
	"Include/BsRenderAPIManager.h"
	"Include/BsRenderAPIFactory.h"
	"Include/BsCommandBufferManager.h"
)

set(BS_BANSHEECORE_SRC_CORETHREAD
	"Source/BsCommandQueue.cpp"
	"Source/BsCoreObject.cpp"
	"Source/BsCoreObjectManager.cpp"
	"Source/BsCoreThread.cpp"
	"Source/BsCoreThreadQueue.cpp"
	"Source/BsCoreObjectCore.cpp"
)

set(BS_BANSHEECORE_INC_NOFILTER
	"Include/BsCoreApplication.h"
	"Include/BsCorePrerequisites.h"
)

set(BS_BANSHEECORE_INC_MATERIAL
	"Include/BsTechnique.h"
	"Include/BsShader.h"
	"Include/BsPass.h"
	"Include/BsMaterialManager.h"
	"Include/BsMaterial.h"
	"Include/BsMaterialParam.h"
	"Include/BsShaderManager.h"
	"Include/BsMaterialParams.h"
	"Include/BsShaderDefines.h"
	"Include/BsGpuParamsSet.h"
)

set(BS_BANSHEECORE_INC_RESOURCES
	"Include/BsVertexDataDesc.h"
	"Include/BsTransientMesh.h"
	"Include/BsTextureManager.h"
	"Include/BsTexture.h"
	"Include/BsResources.h"
	"Include/BsResourceManifest.h"
	"Include/BsResourceHandle.h"
	"Include/BsResource.h"
	"Include/BsPixelData.h"
	"Include/BsMeshHeap.h"
	"Include/BsMeshData.h"
	"Include/BsMeshBase.h"
	"Include/BsMesh.h"
	"Include/BsGpuResourceData.h"
	"Include/BsResourceMetaData.h"
	"Include/BsSavedResourceData.h"
	"Include/BsShaderInclude.h"
	"Include/BsResourceListenerManager.h"
	"Include/BsIResourceListener.h"
)

set(BS_BANSHEECORE_SRC_UTILITY
	"Source/BsUtility.cpp"
	"Source/BsMeshUtility.cpp"
	"Source/BsDeferredCallManager.cpp"
	"Source/BsIconUtility.cpp"
	"Source/BsUUID.cpp"
	"Source/BsPixelUtil.cpp"
)

set(BS_BANSHEECORE_INC_TEXT
	"Include/BsTextData.h"
	"Include/BsFontManager.h"
	"Include/BsFontImportOptions.h"
	"Include/BsFontDesc.h"
	"Include/BsFont.h"
)

set(BS_BANSHEECORE_SRC_PROFILING
	"Source/BsProfilerCPU.cpp"
	"Source/BsProfilerGPU.cpp"
	"Source/BsProfilingManager.cpp"
)

set(BS_BANSHEECORE_SRC_COMPONENTS
	"Source/BsCBoxCollider.cpp"
	"Source/BsCCollider.cpp"
	"Source/BsCRigidbody.cpp"
	"Source/BsCSphereCollider.cpp"
	"Source/BsCPlaneCollider.cpp"
	"Source/BsCCapsuleCollider.cpp"
	"Source/BsCMeshCollider.cpp"
	"Source/BsCJoint.cpp"
	"Source/BsCFixedJoint.cpp"
	"Source/BsCHingeJoint.cpp"
	"Source/BsCDistanceJoint.cpp"
	"Source/BsCSliderJoint.cpp"
	"Source/BsCSphericalJoint.cpp"
	"Source/BsCD6Joint.cpp"
	"Source/BsCCharacterController.cpp"
	"Source/BsCAudioSource.cpp"
	"Source/BsCAudioListener.cpp"
	"Source/BsCCamera.cpp"
	"Source/BsCRenderable.cpp"
	"Source/BsCLight.cpp"
	"Source/BsCAnimation.cpp"
	"Source/BsCBone.cpp"	
	"Source/BsCReflectionProbe.cpp"
	"Source/BsCSkybox.cpp"
)

set(BS_BANSHEECORE_SRC_IMPORTER
	"Source/BsImporter.cpp"
	"Source/BsImportOptions.cpp"
	"Source/BsSpecificImporter.cpp"
	"Source/BsTextureImportOptions.cpp"
	"Source/BsShaderIncludeImporter.cpp"
	"Source/BsMeshImportOptions.cpp"
	"Source/BsShaderImportOptions.cpp"
)

set(BS_BANSHEECORE_INC_UTILITY
	"Include/BsCommonTypes.h"
	"Include/BsUtility.h"
	"Include/BsMeshUtility.h"
	"Include/BsDeferredCallManager.h"
	"Include/BsIconUtility.h"
	"Include/BsUUID.h"
	"Include/BsPixelUtil.h"
	"Include/BsPixelVolume.h"
)

set(BS_BANSHEECORE_INC_RTTI
	"Include/BsBlendStateRTTI.h"
	"Include/BsComponentRTTI.h"
	"Include/BsDepthStencilStateRTTI.h"
	"Include/BsFontImportOptionsRTTI.h"
	"Include/BsFontRTTI.h"
	"Include/BsGameObjectHandleRTTI.h"
	"Include/BsGameObjectRTTI.h"
	"Include/BsGpuProgramRTTI.h"
	"Include/BsGpuResourceDataRTTI.h"
	"Include/BsImportOptionsRTTI.h"
	"Include/BsMaterialRTTI.h"
	"Include/BsMeshBaseRTTI.h"
	"Include/BsMeshDataRTTI.h"
	"Include/BsPassRTTI.h"
	"Include/BsPixelDataRTTI.h"
	"Include/BsRasterizerStateRTTI.h"
	"Include/BsResourceHandleRTTI.h"
	"Include/BsResourceManifestRTTI.h"
	"Include/BsResourceRTTI.h"
	"Include/BsSamplerStateRTTI.h"
	"Include/BsSceneObjectRTTI.h"
	"Include/BsShaderRTTI.h"
	"Include/BsTechniqueRTTI.h"
	"Include/BsTextureRTTI.h"
	"Include/BsVertexDataDescRTTI.h"
	"Include/BsVertexDeclarationRTTI.h"
	"Include/BsTextureImportOptionsRTTI.h"
	"Include/BsResourceMetaDataRTTI.h"
	"Include/BsViewportRTTI.h"
	"Include/BsSavedResourceDataRTTI.h"
	"Include/BsShaderIncludeRTTI.h"
	"Include/BsMeshImportOptionsRTTI.h"
	"Include/BsPrefabRTTI.h"
	"Include/BsPrefabDiffRTTI.h"
	"Include/BsStringTableRTTI.h"
	"Include/BsMaterialParamsRTTI.h"
	"Include/BsMeshRTTI.h"
	"Include/BsPhysicsMaterialRTTI.h"
	"Include/BsCBoxColliderRTTI.h"
	"Include/BsCColliderRTTI.h"
	"Include/BsCRigidbodyRTTI.h"
	"Include/BsCSphereColliderRTTI.h"
	"Include/BsCPlaneColliderRTTI.h"
	"Include/BsCCapsuleColliderRTTI.h"
	"Include/BsCMeshColliderRTTI.h"
	"Include/BsCJointRTTI.h"
	"Include/BsCFixedJointRTTI.h"
	"Include/BsCHingeJointRTTI.h"
	"Include/BsCDistanceJointRTTI.h"
	"Include/BsCSliderJointRTTI.h"
	"Include/BsCSphericalJointRTTI.h"
	"Include/BsCD6JointRTTI.h"
	"Include/BsCCharacterControllerRTTI.h"
	"Include/BsShaderImportOptionsRTTI.h"
	"Include/BsPhysicsMeshRTTI.h"
	"Include/BsAudioClipRTTI.h"
	"Include/BsCAudioSourceRTTI.h"
	"Include/BsCAudioListenerRTTI.h"
	"Include/BsAnimationClipRTTI.h"
	"Include/BsAnimationCurveRTTI.h"
	"Include/BsSkeletonRTTI.h"
	"Include/BsCCameraRTTI.h"
	"Include/BsCameraRTTI.h"
	"Include/BsPostProcessSettingsRTTI.h"
	"Include/BsMorphShapesRTTI.h"
	"Include/BsAudioClipImportOptionsRTTI.h"
	"Include/BsCRenderableRTTI.h"
	"Include/BsCLightRTTI.h"
	"Include/BsCAnimationRTTI.h"
	"Include/BsCBoneRTTI.h"
	"Include/BsRenderableRTTI.h"
	"Include/BsLightRTTI.h"	
	"Include/BsReflectionProbeRTTI.h"
	"Include/BsCReflectionProbeRTTI.h"
	"Include/BsSkyboxRTTI.h"
)

set(BS_BANSHEECORE_SRC_RENDERER
	"Source/BsLight.cpp"
	"Source/BsRenderable.cpp"
	"Source/BsRendererManager.cpp"
	"Source/BsRenderer.cpp"
	"Source/BsRendererMeshData.cpp"
	"Source/BsParamBlocks.cpp"
	"Source/BsCamera.cpp"
	"Source/BsPostProcessSettings.cpp"
	"Source/BsRendererExtension.cpp"
	"Source/BsReflectionProbe.cpp"
	"Source/BsSkybox.cpp"
)

set(BS_BANSHEECORE_SRC_RESOURCES
	"Source/BsGpuResourceData.cpp"
	"Source/BsMesh.cpp"
	"Source/BsMeshBase.cpp"
	"Source/BsMeshData.cpp"
	"Source/BsMeshHeap.cpp"
	"Source/BsPixelData.cpp"
	"Source/BsResource.cpp"
	"Source/BsResourceHandle.cpp"
	"Source/BsResourceManifest.cpp"
	"Source/BsResources.cpp"
	"Source/BsTexture.cpp"
	"Source/BsTextureManager.cpp"
	"Source/BsTransientMesh.cpp"
	"Source/BsVertexDataDesc.cpp"
	"Source/BsResourceMetaData.cpp"
	"Source/BsSavedResourceData.cpp"
	"Source/BsShaderInclude.cpp"
	"Source/BsResourceListenerManager.cpp"
	"Source/BsIResourceListener.cpp"
)

set(BS_BANSHEECORE_SRC_MATERIAL
	"Source/BsMaterial.cpp"
	"Source/BsMaterialManager.cpp"
	"Source/BsPass.cpp"
	"Source/BsShader.cpp"
	"Source/BsTechnique.cpp"
	"Source/BsMaterialParam.cpp"
	"Source/BsShaderManager.cpp"
	"Source/BsMaterialParams.cpp"
	"Source/BsShaderDefines.cpp"
	"Source/BsGpuParamsSet.cpp"
)

set(BS_BANSHEECORE_SRC_INPUT
	"Source/BsInput.cpp"
	"Source/BsOSInputHandler.cpp"
)

set(BS_BANSHEECORE_INC_LOCALIZATION
	"Include/BsHString.h"
	"Include/BsStringTable.h"
	"Include/BsStringTableManager.h"
)

set(BS_BANSHEECORE_SRC_TEXT
	"Source/BsFont.cpp"
	"Source/BsFontImportOptions.cpp"
	"Source/BsFontManager.cpp"
	"Source/BsTextData.cpp"
)

set(BS_BANSHEECORE_SRC_RENDERAPI
	"Source/BsBlendState.cpp"
	"Source/BsDepthStencilState.cpp"
	"Source/BsEventQuery.cpp"
	"Source/BsGpuBuffer.cpp"
	"Source/BsGpuParam.cpp"
	"Source/BsGpuParamBlockBuffer.cpp"
	"Source/BsGpuParams.cpp"
	"Source/BsGpuProgram.cpp"
	"Source/BsIndexBuffer.cpp"
	"Source/BsOcclusionQuery.cpp"
	"Source/BsRasterizerState.cpp"
	"Source/BsRenderTarget.cpp"
	"Source/BsRenderTexture.cpp"
	"Source/BsRenderWindow.cpp"
	"Source/BsSamplerState.cpp"
	"Source/BsTextureView.cpp"
	"Source/BsTimerQuery.cpp"
	"Source/BsVertexBuffer.cpp"
	"Source/BsVertexData.cpp"
	"Source/BsVertexDeclaration.cpp"
	"Source/BsVideoModeInfo.cpp"
	"Source/BsRenderAPI.cpp"
	"Source/BsRenderAPICapabilities.cpp"
	"Source/BsViewport.cpp"
	"Source/BsCommandBuffer.cpp"
	"Source/BsGpuPipelineState.cpp"
	"Source/BsGpuPipelineParamInfo.cpp"
)

set(BS_BANSHEECORE_SRC_RENDERAPI_MANAGERS
	"Source/BsGpuProgramManager.cpp"
	"Source/BsHardwareBufferManager.cpp"
	"Source/BsMeshManager.cpp"
	"Source/BsQueryManager.cpp"
	"Source/BsRenderStateManager.cpp"
	"Source/BsRenderWindowManager.cpp"
	"Source/BsRenderAPIManager.cpp"
	"Source/BsCommandBufferManager.cpp"
)

set(BS_BANSHEECORE_SRC_NOFILTER
	"Source/BsCoreApplication.cpp"
)

set(BS_BANSHEECORE_SRC_PHYSICS
	"Source/BsPhysicsManager.cpp"
	"Source/BsPhysics.cpp"
	"Source/BsPhysicsMaterial.cpp"
	"Source/BsCollider.cpp"
	"Source/BsRigidbody.cpp"
	"Source/BsBoxCollider.cpp"
	"Source/BsSphereCollider.cpp"
	"Source/BsPlaneCollider.cpp"
	"Source/BsCapsuleCollider.cpp"
	"Source/BsFCollider.cpp"
	"Source/BsPhysicsMesh.cpp"
	"Source/BsMeshCollider.cpp"
	"Source/BsFJoint.cpp"
	"Source/BsJoint.cpp"
	"Source/BsFixedJoint.cpp"
	"Source/BsDistanceJoint.cpp"
	"Source/BsHingeJoint.cpp"
	"Source/BsSliderJoint.cpp"
	"Source/BsSphericalJoint.cpp"
	"Source/BsD6Joint.cpp"
	"Source/BsCharacterController.cpp"
)

set(BS_BANSHEECORE_SRC_SCENE
	"Source/BsComponent.cpp"
	"Source/BsGameObject.cpp"
	"Source/BsGameObjectHandle.cpp"
	"Source/BsGameObjectManager.cpp"
	"Source/BsSceneObject.cpp"
	"Source/BsSceneManager.cpp"
	"Source/BsPrefab.cpp"
	"Source/BsPrefabDiff.cpp"
	"Source/BsPrefabUtility.cpp"
)

set(BS_BANSHEECORE_INC_AUDIO
	"Include/BsAudio.h"
	"Include/BsAudioClip.h"
	"Include/BsAudioListener.h"
	"Include/BsAudioSource.h"
	"Include/BsAudioClipImportOptions.h"
	"Include/BsAudioUtility.h"
	"Include/BsAudioManager.h"
)

set(BS_BANSHEECORE_SRC_AUDIO
	"Source/BsAudio.cpp"
	"Source/BsAudioClip.cpp"
	"Source/BsAudioListener.cpp"
	"Source/BsAudioSource.cpp"
	"Source/BsAudioClipImportOptions.cpp"
	"Source/BsAudioUtility.cpp"
	"Source/BsAudioManager.cpp"
)

set(BS_BANSHEECORE_INC_ANIMATION
	"Include/BsAnimationCurve.h"
	"Include/BsAnimationClip.h"
	"Include/BsSkeleton.h"
	"Include/BsAnimation.h"
	"Include/BsAnimationManager.h"
	"Include/BsCurveCache.h"
	"Include/BsAnimationUtility.h"
	"Include/BsSkeletonMask.h"
	"Include/BsMorphShapes.h"
)

set(BS_BANSHEECORE_SRC_ANIMATION
	"Source/BsAnimationCurve.cpp"
	"Source/BsAnimationClip.cpp"
	"Source/BsSkeleton.cpp"
	"Source/BsAnimation.cpp"
	"Source/BsAnimationManager.cpp"
	"Source/BsAnimationUtility.cpp"
	"Source/BsSkeletonMask.cpp"
	"Source/BsMorphShapes.cpp"
)

set(BS_BANSHEECORE_INC_PLATFORM
	"Include/BsPlatform.h"
	"Include/BsFolderMonitor.h"
)

set(BS_BANSHEECORE_INC_PLATFORM_WIN32
	"Include/Win32/BsWin32FolderMonitor.h"
	"Include/Win32/BsWin32DropTarget.h"
	"Include/Win32/BsWin32Defs.h"
	"Include/Win32/BSWin32PlatformData.h"
	"Include/Win32/BsWin32Platform.h"
)

set(BS_BANSHEECORE_SRC_PLATFORM
	"Source/BsPlatform.cpp"
)

set(BS_BANSHEECORE_SRC_PLATFORM_WIN32
	"Source/Win32/BsWin32FolderMonitor.cpp"
	"Source/Win32/BsWin32Platform.cpp"
	"Source/Win32/BsWin32BrowseDialogs.cpp"
)

if(WIN32)
	list(APPEND BS_BANSHEECORE_INC_PLATFORM ${BS_BANSHEECORE_INC_PLATFORM_WIN32})
	list(APPEND BS_BANSHEECORE_SRC_PLATFORM ${BS_BANSHEECORE_SRC_PLATFORM_WIN32})
endif()

source_group("Header Files\\Components" FILES ${BS_BANSHEECORE_INC_COMPONENTS})
source_group("Header Files\\Physics" FILES ${BS_BANSHEECORE_INC_PHYSICS})
source_group("Header Files\\CoreThread" FILES ${BS_BANSHEECORE_INC_CORETHREAD})
source_group("Header Files\\Importer" FILES ${BS_BANSHEECORE_INC_IMPORTER})
source_group("Header Files\\Scene" FILES ${BS_BANSHEECORE_INC_SCENE})
source_group("Header Files\\Input" FILES ${BS_BANSHEECORE_INC_INPUT})
source_group("Header Files\\Platform" FILES ${BS_BANSHEECORE_INC_PLATFORM})
source_group("Header Files\\Renderer" FILES ${BS_BANSHEECORE_INC_RENDERER})
source_group("Source Files\\Localization" FILES ${BS_BANSHEECORE_SRC_LOCALIZATION})
source_group("Source Files\\RTTI" FILES ${BS_BANSHEECORE_SRC_RTTI})
source_group("Header Files\\Profiling" FILES ${BS_BANSHEECORE_INC_PROFILING})
source_group("Header Files\\RenderAPI" FILES ${BS_BANSHEECORE_INC_RENDERAPI})
source_group("Header Files\\RenderAPI\\Managers" FILES ${BS_BANSHEECORE_INC_RENDERAPI_MANAGERS})
source_group("Source Files\\CoreThread" FILES ${BS_BANSHEECORE_SRC_CORETHREAD})
source_group("Header Files" FILES ${BS_BANSHEECORE_INC_NOFILTER})
source_group("Header Files\\Material" FILES ${BS_BANSHEECORE_INC_MATERIAL})
source_group("Header Files\\Resources" FILES ${BS_BANSHEECORE_INC_RESOURCES})
source_group("Source Files\\Utility" FILES ${BS_BANSHEECORE_SRC_UTILITY})
source_group("Header Files\\Text" FILES ${BS_BANSHEECORE_INC_TEXT})
source_group("Source Files\\Profiling" FILES ${BS_BANSHEECORE_SRC_PROFILING})
source_group("Source Files\\Components" FILES ${BS_BANSHEECORE_SRC_COMPONENTS})
source_group("Source Files\\Platform" FILES ${BS_BANSHEECORE_SRC_PLATFORM})
source_group("Source Files\\Importer" FILES ${BS_BANSHEECORE_SRC_IMPORTER})
source_group("Header Files\\Utility" FILES ${BS_BANSHEECORE_INC_UTILITY})
source_group("Header Files\\RTTI" FILES ${BS_BANSHEECORE_INC_RTTI})
source_group("Source Files\\Renderer" FILES ${BS_BANSHEECORE_SRC_RENDERER})
source_group("Source Files\\Resources" FILES ${BS_BANSHEECORE_SRC_RESOURCES})
source_group("Source Files\\Material" FILES ${BS_BANSHEECORE_SRC_MATERIAL})
source_group("Source Files\\Input" FILES ${BS_BANSHEECORE_SRC_INPUT})
source_group("Header Files\\Localization" FILES ${BS_BANSHEECORE_INC_LOCALIZATION})
source_group("Source Files\\Text" FILES ${BS_BANSHEECORE_SRC_TEXT})
source_group("Source Files\\RenderAPI" FILES ${BS_BANSHEECORE_SRC_RENDERAPI})
source_group("Source Files\\RenderAPI\\Managers" FILES ${BS_BANSHEECORE_SRC_RENDERAPI_MANAGERS})
source_group("Source Files" FILES ${BS_BANSHEECORE_SRC_NOFILTER})
source_group("Source Files\\Physics" FILES ${BS_BANSHEECORE_SRC_PHYSICS})
source_group("Source Files\\Scene" FILES ${BS_BANSHEECORE_SRC_SCENE})
source_group("Header Files\\Audio" FILES ${BS_BANSHEECORE_INC_AUDIO})
source_group("Source Files\\Audio" FILES ${BS_BANSHEECORE_SRC_AUDIO})
source_group("Header Files\\Animation" FILES ${BS_BANSHEECORE_INC_ANIMATION})
source_group("Source Files\\Animation" FILES ${BS_BANSHEECORE_SRC_ANIMATION})

set(BS_BANSHEECORE_SRC
	${BS_BANSHEECORE_INC_COMPONENTS}
	${BS_BANSHEECORE_INC_PHYSICS}
	${BS_BANSHEECORE_INC_CORETHREAD}
	${BS_BANSHEECORE_INC_IMPORTER}
	${BS_BANSHEECORE_INC_SCENE}
	${BS_BANSHEECORE_INC_INPUT}
	${BS_BANSHEECORE_INC_PLATFORM}
	${BS_BANSHEECORE_INC_RENDERER}
	${BS_BANSHEECORE_SRC_LOCALIZATION}
	${BS_BANSHEECORE_SRC_RTTI}
	${BS_BANSHEECORE_INC_PROFILING}
	${BS_BANSHEECORE_INC_RENDERAPI}
	${BS_BANSHEECORE_SRC_CORETHREAD}
	${BS_BANSHEECORE_INC_NOFILTER}
	${BS_BANSHEECORE_INC_MATERIAL}
	${BS_BANSHEECORE_INC_RESOURCES}
	${BS_BANSHEECORE_SRC_UTILITY}
	${BS_BANSHEECORE_INC_TEXT}
	${BS_BANSHEECORE_SRC_PROFILING}
	${BS_BANSHEECORE_SRC_COMPONENTS}
	${BS_BANSHEECORE_SRC_PLATFORM}
	${BS_BANSHEECORE_SRC_IMPORTER}
	${BS_BANSHEECORE_INC_UTILITY}
	${BS_BANSHEECORE_INC_RTTI}
	${BS_BANSHEECORE_SRC_RENDERER}
	${BS_BANSHEECORE_SRC_RESOURCES}
	${BS_BANSHEECORE_SRC_MATERIAL}
	${BS_BANSHEECORE_SRC_INPUT}
	${BS_BANSHEECORE_INC_LOCALIZATION}
	${BS_BANSHEECORE_SRC_TEXT}
	${BS_BANSHEECORE_SRC_RENDERAPI}
	${BS_BANSHEECORE_SRC_NOFILTER}
	${BS_BANSHEECORE_SRC_PHYSICS}
	${BS_BANSHEECORE_SRC_SCENE}
	${BS_BANSHEECORE_INC_AUDIO}
	${BS_BANSHEECORE_SRC_AUDIO}
	${BS_BANSHEECORE_INC_ANIMATION}
	${BS_BANSHEECORE_SRC_ANIMATION}
	${BS_BANSHEECORE_INC_RENDERAPI_MANAGERS}
	${BS_BANSHEECORE_SRC_RENDERAPI_MANAGERS}
)