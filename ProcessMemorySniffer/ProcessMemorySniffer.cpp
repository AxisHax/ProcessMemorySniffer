#define NOMINMAX
#include <Windows.h>

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>

#include "ProcessMemorySniffer.hpp"
#include "ProcessQueryService.hpp"
#include "ProcessInfo.hpp"
#include "Win32Error.hpp"

#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN

/// <summary>
/// Constant representing the maximum allowed length for a name, in characters.
/// </summary>
constexpr int MAX_NAME_LEN = 28;

/// <summary>
/// Prints a table of the top processes sorted by working set (physical RAM) to the wide output stream.
/// </summary>
/// <param name="processes">A vector of ProcessInfo structures describing processes. If empty, a message is printed and the function returns.</param>
/// <param name="topN">Maximum number of top entries to print. If greater than the number of processes, it is clamped to the available size.</param>
static void printTopByWorkingSet(const std::vector<ProcessInfo>& processes, std::size_t topN)
{
	if (processes.empty())
	{
		std::wcout << L"No processes available.\n";
		return;
	}

	auto sorted = processes;
	std::sort(sorted.begin(), sorted.end(),
		[](const ProcessInfo& a, const ProcessInfo& b)
		{
			return a.workingSetBytes > b.workingSetBytes;
		});

	topN = std::min(topN, sorted.size());

	std::wcout << L"Top " << topN << L" processes by working set (physical ram):\n\n";
	std::wcout << std::left
		<< std::setw(8) << L"PID"
		<< std::setw(30) << L"Process"
		<< std::setw(16) << L"WorkingSet (MB)"
		<< std::setw(16) << L"Private (MB)"
		<< L"\n";

	std::wcout.setf(std::ios::fixed);
	std::wcout << std::setprecision(2);

	for (std::size_t i = 0; i < topN; i++)
	{
		const auto& p = sorted[i];

		const double wsMB = static_cast<double>(p.workingSetBytes) / (1024.0 * 1024.0);
		const double privMB = static_cast<double>(p.privateBytes) / (1024.0 * 1024.0);

		std::wstring displayName = p.name;

		if (displayName.size() > MAX_NAME_LEN)
		{
			displayName = displayName.substr(0, MAX_NAME_LEN - 1) + L"...";
		}

		std::wcout << std::left
			<< std::setw(8) << p.pid
			<< std::setw(30) << displayName
			<< std::setw(16) << wsMB
			<< std::setw(16) << privMB
			<< L"\n";
	}
}

/// <summary>
/// Collects processes and prints the top processes by working set. Returns EXIT_SUCCESS on success or EXIT_FAILURE if an exception occurs.
/// </summary>
/// <param name="topN">The requested number of top processes to print. Note: this parameter is currently ignored because a local constexpr topN = 15 shadows it, so the function always prints the top 15 processes.</param>
/// <returns>EXIT_SUCCESS if processing and printing complete without exceptions; EXIT_FAILURE if a Win32 error or other std::exception is thrown.</returns>
int runSniffer(std::size_t topN)
{
	try
	{
		ProcessQueryService service;
		const auto processes = service.collectProcesses();
		printTopByWorkingSet(processes, topN);
	}
	catch (const Win32Error& ex)
	{
		std::cerr << "Win32 error: " << ex.what() << "\n";
		return EXIT_FAILURE;
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << "\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}