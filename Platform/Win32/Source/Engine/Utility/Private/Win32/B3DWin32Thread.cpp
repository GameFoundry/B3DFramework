//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <cstdlib>  // mbstowcs
#include <limits>   // std::numeric_limits
#include <vector>
#undef max

#include "Threading/B3DThread.h"
#include "Debug/B3DDebug.h"
#include "Utility/B3DScopeGuard.h"

using namespace b3d;

namespace
{
	constexpr size_t MaxCoreCount = std::numeric_limits<decltype(CPUCore::Windows.IndexWithinGroup)>::max() + 1L;
	constexpr size_t MaxGroupCount = std::numeric_limits<decltype(CPUCore::Windows.CoreGroup)>::max() + 1;
	static_assert(sizeof(KAFFINITY) * 8ULL <= MaxCoreCount, "GPUCore::Windows.IndexWithinGroup data type is too small to fit all affinities.");

#define B3D_CHECK_WIN32(Expr)																									\
	do																															\
	{																															\
		auto result = Expr;																										\
		if(result != TRUE)																										\
		{																														\
			B3D_LOG(Error, LogGeneric, "Win32 operation '{0}' failed with error: {1}", #Expr, (i32)GetLastError());				\
			B3D_ASSERT(false);																									\
		}																														\
	} while (false)

	struct Win32ProcessorGroup
	{
		u32 LogicalCoreCount;
		KAFFINITY AffinityMask;
	};

	struct Win32ProcessorGroups
	{
		TInlineArray<Win32ProcessorGroup, MaxGroupCount> Groups;
	};

	/** Returns information about all logical processor groups. */
	const Win32ProcessorGroups& GetProcessorGroups()
	{
		static Win32ProcessorGroups groups = []
		{
			Win32ProcessorGroups output = {};

			SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX processorInformation[32] = {};
			DWORD processorInformationSize = sizeof(processorInformation);

			B3D_CHECK_WIN32(GetLogicalProcessorInformationEx(RelationGroup, processorInformation, &processorInformationSize));

			const u32 processorCount = processorInformationSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
			for (u32 processorIndex = 0; processorIndex < processorCount; processorIndex++)
			{
				if (processorInformation[processorIndex].Relationship != RelationGroup)
					continue;

				auto groupCount = processorInformation[processorIndex].Group.ActiveGroupCount;
				for (auto groupIndex = 0; groupIndex < groupCount; groupIndex++)
				{
					auto const& groupInformation = processorInformation[processorIndex].Group.GroupInfo[groupIndex];

					output.Groups.Add(Win32ProcessorGroup{ groupInformation.ActiveProcessorCount, groupInformation.ActiveProcessorMask });
					B3D_ASSERT(output.Groups.size() <= MaxGroupCount && "Group index overflow");
				}
			}

			return output;
		}();

		return groups;
	}
}  // namespace

ThreadCoreMask ThreadCoreMask::CreateAnyThreadMask()
{
	ThreadCoreMask output;

	const auto& processorGroups = GetProcessorGroups();
	for (size_t groupIndex = 0; groupIndex < processorGroups.Groups.Size(); groupIndex++)
	{
		const auto& group = processorGroups.Groups[groupIndex];

		CPUCore core;
		core.Windows.CoreGroup = (u8)groupIndex;

		for (unsigned int coreIndex = 0; coreIndex < group.LogicalCoreCount; coreIndex++)
		{
			if ((group.AffinityMask >> coreIndex) & 1)
			{
				core.Windows.IndexWithinGroup = (u8)coreIndex;
				output.mCores.Add(core);
			}
		}
	}

	return output;
}

ThreadCoreMask AnyOfThreadAffinityPolicy::GetMaskForThread(u32 threadIndex) const
{
	const size_t availableCoreCount = mAvailableCores.GetCoreCount();
	if (availableCoreCount == 0)
		return mAvailableCores;

	const u8 groupIndex = mAvailableCores[threadIndex % mAvailableCores.GetCoreCount()].Windows.CoreGroup;

	TInlineArray<CPUCore, 32> coresInGroup;
	coresInGroup.reserve(availableCoreCount);

	for(size_t coreIndex = 0; coreIndex < mAvailableCores.GetCoreCount(); coreIndex++)
	{
		const CPUCore core = mAvailableCores[coreIndex];

		if (core.Windows.CoreGroup == groupIndex)
			coresInGroup.Add(core);
	}
	return ThreadCoreMask(coresInGroup);
}

class Thread::Implementation
{
public:
	Implementation(Function<void()>&& workerFunction, _PROC_THREAD_ATTRIBUTE_LIST* attributes)
		: WorkerFunction(std::move(workerFunction))
		, ThreadHandle(CreateRemoteThreadEx(GetCurrentProcess(), nullptr, 0, &Implementation::Run, this, 0, attributes, &ThreadId))
	{
		if (ThreadHandle == nullptr)
			B3D_LOG(Error, LogGeneric, "CreateRemoteThreadEx() failed with error: {0}", (i32)GetLastError());
	}
	~Implementation() { if (ThreadHandle != nullptr) CloseHandle(ThreadHandle); }

	Implementation(const Implementation&) = delete;
	Implementation(Implementation&&) = delete;
	Implementation& operator=(const Implementation&) = delete;
	Implementation& operator=(Implementation&&) = delete;

	void Join() const { if (ThreadHandle != nullptr) WaitForSingleObject(ThreadHandle, INFINITE); }

	static DWORD WINAPI Run(void* self)
	{
		Implementation* implementation = static_cast<Implementation*>(self);

		// Query the id directly rather than reading the out-param the parent is concurrently writing.
		Thread::CurrentId = (u32)::GetCurrentThreadId();

		implementation->WorkerFunction();
		return 0;
	}

	const Function<void()> WorkerFunction;
	const HANDLE ThreadHandle;
	DWORD ThreadId = 0;
};

Thread::Thread(const ThreadCoreMask& affinity, Function<void()>&& workerFunction)
{
	SIZE_T attributeListSize = 0;
	InitializeProcThreadAttributeList(nullptr, 1, 0, &attributeListSize);
	B3D_ASSERT(attributeListSize > 0 && "InitializeProcThreadAttributeList() did not give a size");

	std::vector<u8> attributeListBuffer(attributeListSize);
	LPPROC_THREAD_ATTRIBUTE_LIST attributes = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attributeListBuffer.data());
	B3D_CHECK_WIN32(InitializeProcThreadAttributeList(attributes, 1, 0, &attributeListSize));
	B3D_SCOPE_CLEANUP(DeleteProcThreadAttributeList(attributes));

	GROUP_AFFINITY groupAffinity = {};

	const size_t coreCount = affinity.GetCoreCount();
	if (coreCount > 0)
	{
		groupAffinity.Group = affinity[0].Windows.CoreGroup;

		for (size_t coreIndex = 0; coreIndex < coreCount; coreIndex++)
		{
			const CPUCore& core = affinity[coreIndex];
			B3D_ASSERT(groupAffinity.Group == core.Windows.CoreGroup && "Cannot create thread that uses multiple affinity groups");
			groupAffinity.Mask |= (1ULL << core.Windows.IndexWithinGroup);
		}

		B3D_CHECK_WIN32(UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_GROUP_AFFINITY, &groupAffinity, sizeof(groupAffinity), nullptr, nullptr));
	}

	m = B3DNew<Implementation>(std::move(workerFunction), attributes);
}

Thread::~Thread()
{
	if(m != nullptr)
		B3DDelete(m);
}

void Thread::WaitUntilComplete()
{
	if (!m)
		return;

	m->Join();
}

u32 Thread::GetId() const
{
	if (!m)
		return 0;

	return m->ThreadId;
}

void Thread::SetName(const char* format, ...)
{
	static auto fnSetThreadDescription = reinterpret_cast<HRESULT(WINAPI*)(HANDLE, PCWSTR)>(GetProcAddress(GetModuleHandleA("kernelbase.dll"), "SetThreadDescription"));
	if (fnSetThreadDescription == nullptr)
		return;

	char name[1024];
	va_list vararg;
	va_start(vararg, format);
	vsnprintf(name, sizeof(name), format, vararg);
	va_end(vararg);

	wchar_t wname[1024];
	mbstowcs(wname, name, 1023);
	wname[1023] = L'\0';
	fnSetThreadDescription(GetCurrentThread(), wname);
}

u32 Thread::GetLogicalCoreCount()
{
	u32 coreCount = 0;
	const Win32ProcessorGroups& processorGroups = GetProcessorGroups();
	for (size_t groupIndex = 0; groupIndex < processorGroups.Groups.Size(); groupIndex++)
	{
		const Win32ProcessorGroup& group = processorGroups.Groups[groupIndex];
		coreCount += group.LogicalCoreCount;
	}

	return coreCount;
}

Thread::Thread(Thread&& rhs) : m(rhs.m)
{
	rhs.m = nullptr;
}

Thread& Thread::operator=(Thread&& rhs)
{
	if (this == &rhs)
		return *this;

	if (m)
	{
		m->Join();
		B3DDelete(m);
		m = nullptr;
	}

	m = rhs.m;

	rhs.m = nullptr;
	return *this;
}
