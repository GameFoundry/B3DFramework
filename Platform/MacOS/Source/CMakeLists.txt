if(NOT APPLE)
	return()
endif()

# Platform packages
find_package(LibUUID REQUIRED)
target_link_libraries(bsf PRIVATE ${LibUUID_LIBRARIES})

# OS frameworks
B3DTargetLinkFramework(bsf CoreServices)
B3DTargetLinkFramework(bsf IOKit)
B3DTargetLinkFramework(bsf AppKit)
B3DTargetLinkFramework(bsf Carbon)
B3DTargetLinkFramework(bsf QuartzCore)
