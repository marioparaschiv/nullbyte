#pragma once
#ifndef UTIL_H
#define UTIL_H
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <vector>

namespace Util {
	bool compareBytes(const unsigned char* bytes, const char* pattern);
	std::vector<MEMORY_BASIC_INFORMATION> getRegions(HANDLE hProcess);
	std::vector<PROCESSENTRY32> getProcesses(char** errorMessage);

	struct Pair {
		HANDLE handle;
		PROCESSENTRY32 process;
	};

	Util::Pair openProcess(DWORD processId, char** errorMessage);
	bool findPattern(HANDLE handle, uintptr_t memoryBase, unsigned char* byteBase, DWORD memorySize, const char* pattern, short flags, uint32_t patternOffset, uintptr_t* pAddress);
	bool searchPattern(HANDLE handle, std::vector<MODULEENTRY32> modules, DWORD64 searchAddress, const char* pattern, short flags, uint32_t patternOffset, uintptr_t* pAddress);
	std::vector<MODULEENTRY32> getModules(DWORD processId, char** errorMessage);
	MODULEENTRY32 findModule(const char* moduleName, DWORD processId, char** errorMessage);
	int findPatternByModule(HANDLE handle, std::string moduleName, std::string pattern, short flags, uint32_t patternOffset);
	std::vector<std::string> split(char* phrase, std::string delimiter);
};

#endif
#pragma once