#include <Windows.h>
#include <Psapi.h>>

#include <algorithm>

#include "ProcessQueryService.hpp"
#include "ProcessHandle.hpp"
#include "Win32Error.hpp"

#pragma comment(lib, "Psapi.lib")

/// <summary>
/// Defines a compile-time constant for the PID vector size.
/// </summary>
constexpr auto PID_VECT_SIZE = 1024;

/// <summary>
/// Retrieves the list of process IDs by calling EnumProcesses and resizing the internal buffer until it is large enough. Throws a Win32Error if EnumProcesses fails.
/// </summary>
/// <returns>A std::vector<DWORD> containing the process IDs of currently running processes. The vector is resized to match the number of IDs returned.</returns>
std::vector<DWORD> ProcessQueryService::enumerateProcessIds() const
{
	std::vector<DWORD> pids(PID_VECT_SIZE);

	while (true)
	{
		DWORD bytesReturned = 0;
		if (!::EnumProcesses(
			pids.data(),
			static_cast<DWORD>(pids.size() * sizeof(DWORD)), &bytesReturned))
		{
			throw Win32Error("EnumProcesses failed.");
		}

		const std::size_t count = bytesReturned / sizeof(DWORD);

		if (count < pids.size())
		{
			pids.resize(count);
			break;
		}

		// The buffer was full. Resize and try again.
		pids.resize(pids.size() * 2);
	}

	return pids;
}

/// <summary>
/// Retrieves runtime information about a process identified by its PID. Returns a ProcessInfo when the process can be opened and memory info retrieved; otherwise returns std::nullopt.
/// </summary>
/// <param name="pid">The process identifier (DWORD) to query. If pid is 0 or the process cannot be opened or queried, the function returns std::nullopt.</param>
/// <returns>std::optional<ProcessInfo> containing the process information (pid, name, workingSetBytes, privateBytes) on success; std::nullopt if the PID is 0, access is denied/cannot open the process, or memory information could not be obtained. The function is noexcept and does not throw.</returns>
std::optional<ProcessInfo>ProcessQueryService::queryProcess(DWORD pid) const noexcept
{
	if (pid == 0)
	{
		return std::nullopt;
	}

	auto handleOpt = ProcessHandle::open(pid);

	if (!handleOpt)
	{
		// This means access was denied. System process, etc.
		return std::nullopt;
	}

	PROCESS_MEMORY_COUNTERS_EX pmc{};

	if (!::GetProcessMemoryInfo(
		handleOpt->get(),
		reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
		sizeof(pmc)))
	{
		return std::nullopt;
	}

	ProcessInfo info;
	info.pid = pid;
	info.name = tryGetProcessName(handleOpt->get());
	info.workingSetBytes = static_cast<Bytes>(pmc.WorkingSetSize);
	info.privateBytes = static_cast<Bytes>(pmc.PrivateUsage);

	return info;
}

/// <summary>
/// Retrieves the name of the specified process. Attempts to use the module base name first and falls back to the full process image name; if both attempts fail, returns "<unknown>".
/// </summary>
/// <param name="process">Handle to the process to query. Must refer to a valid process and have sufficient access rights for name/query operations.</param>
/// <returns>A std::wstring containing the process name (module base name or full image path). Returns "<unknown>" if the name could not be determined.</returns>
std::wstring ProcessQueryService::tryGetProcessName(HANDLE process) const noexcept
{
	wchar_t buffer[MAX_PATH];

	if (::GetModuleBaseNameW(process, nullptr, buffer, static_cast<DWORD>(std::size(buffer))))
	{
		return buffer;
	}

	DWORD size = static_cast<DWORD>(std::size(buffer));

	if (::QueryFullProcessImageNameW(process, 0, buffer, &size))
	{
		return buffer;
	}

	return L"<unknown>";
}

/// <summary>
/// Enumerates process IDs, queries each process for information, and returns a collection of the gathered ProcessInfo objects.
/// </summary>
/// <returns>A std::vector<ProcessInfo> containing the ProcessInfo entries for processes that could be successfully queried. Processes for which queryProcess() returns no data are omitted.</returns>
std::vector<ProcessInfo> ProcessQueryService::collectProcesses() const
{
	const auto pids = enumerateProcessIds();

	std::vector<ProcessInfo> result;
	result.reserve(pids.size());

	for (DWORD pid : pids)
	{
		if (auto info = queryProcess(pid))
		{
			result.push_back(std::move(*info));
		}
	}

	return result;
}