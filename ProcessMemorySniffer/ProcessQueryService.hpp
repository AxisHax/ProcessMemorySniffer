#pragma once

#include <Windows.h>

#include <vector>
#include <optional>
#include <string>

#include "ProcessInfo.hpp"

/// <summary>
/// Service for enumerating running processes and collecting information about them.
/// </summary>
class ProcessQueryService
{
public:
	[[nodiscard]] std::vector<ProcessInfo> collectProcesses() const;

private:
	[[nodiscard]] std::vector<DWORD> enumerateProcessIds() const;

	[[nodiscard]] std::optional<ProcessInfo> queryProcess(DWORD pid) const noexcept;

	[[nodiscard]] std::wstring tryGetProcessName(HANDLE process) const noexcept;
};