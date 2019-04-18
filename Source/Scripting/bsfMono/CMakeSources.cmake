set(BS_MONO_INC_NOFILTER
	"BsMonoPrerequisites.h"
	"BsMonoAssembly.h"
	"BsMonoClass.h"
	"BsMonoField.h"
	"BsMonoManager.h"
	"BsMonoMethod.h"
	"BsMonoProperty.h"
	"BsMonoUtil.h"
	"BsScriptMeta.h"
	"BsMonoArray.h"
)

set(BS_MONO_SRC_NOFILTER
	"BsMonoAssembly.cpp"
	"BsMonoClass.cpp"
	"BsMonoField.cpp"
	"BsMonoManager.cpp"
	"BsMonoMethod.cpp"
	"BsMonoProperty.cpp"
	"BsScriptMeta.cpp"
	"BsMonoUtil.cpp"
	"BsMonoArray.cpp"
)

source_group("" FILES ${BS_MONO_INC_NOFILTER} ${BS_MONO_SRC_NOFILTER})

set(BS_MONO_SRC
	${BS_MONO_INC_NOFILTER}
	${BS_MONO_SRC_NOFILTER}
)