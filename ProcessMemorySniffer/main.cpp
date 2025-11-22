#include "ProcessMemorySniffer.hpp"

/// <summary>
/// Windows wide-character program entry point that invokes runSniffer with an argument of 10.
/// </summary>
/// <returns>The integer result returned by runSniffer(10), typically used as the program's exit code.</returns>
int wmain()
{
	return runSniffer(10);
}