set(BS_PHYSX_INC_NOFILTER
	"BsPhysXPrerequisites.h"
	"BsPhysX.h"
	"BsPhysXMaterial.h"
	"BsPhysXRigidbody.h"
	"BsPhysXBoxCollider.h"
	"BsFPhysXCollider.h"
	"BsPhysXSphereCollider.h"
	"BsPhysXPlaneCollider.h"
	"BsPhysXCapsuleCollider.h"
	"BsPhysXMesh.h"
	"BsPhysXMeshCollider.h"
	"BsFPhysXJoint.h"
	"BsPhysXFixedJoint.h"
	"BsPhysXDistanceJoint.h"
	"BsPhysXHingeJoint.h"
	"BsPhysXSliderJoint.h"
	"BsPhysXSphericalJoint.h"
	"BsPhysXD6Joint.h"
	"BsPhysXCharacterController.h"
)

set(BS_PHYSX_SRC_NOFILTER
	"BsPhysXPlugin.cpp"
	"BsPhysX.cpp"
	"BsPhysXMaterial.cpp"
	"BsPhysXRigidbody.cpp"
	"BsPhysXBoxCollider.cpp"
	"BsFPhysXCollider.cpp"
	"BsPhysXSphereCollider.cpp"
	"BsPhysXPlaneCollider.cpp"
	"BsPhysXCapsuleCollider.cpp"
	"BsPhysXMesh.cpp"
	"BsPhysXMeshCollider.cpp"
	"BsFPhysXJoint.cpp"
	"BsPhysXFixedJoint.cpp"
	"BsPhysXDistanceJoint.cpp"
	"BsPhysXHingeJoint.cpp"
	"BsPhysXSliderJoint.cpp"
	"BsPhysXSphericalJoint.cpp"
	"BsPhysXD6Joint.cpp"
	"BsPhysXCharacterController.cpp"
)

set(BS_PHYSX_INC_RTTI
	"RTTI/BsPhysXMeshRTTI.h"
)

if(WIN32)
	set(BS_PHYSX_WIN32RES
	"BsPhysXWin32Resource.rc"
	)
else()
	set(BS_PHYSX_WIN32RES )
endif()

source_group("" FILES ${BS_PHYSX_INC_NOFILTER} ${BS_PHYSX_SRC_NOFILTER} ${BS_PHYSX_WIN32RES})
source_group("RTTI" FILES ${BS_PHYSX_INC_RTTI})

set(BS_PHYSX_SRC
	${BS_PHYSX_INC_NOFILTER}
	${BS_PHYSX_SRC_NOFILTER}
	${BS_PHYSX_INC_RTTI}
	${BS_PHYSX_WIN32RES}
)