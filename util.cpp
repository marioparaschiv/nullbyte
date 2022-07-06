#include <windows.h>
#include <iostream>
#include <string>
#include <napi.h>
#include <windows.h>
#include <TlHelp32.h>
#include "util.h"

#define INRANGE(x,a,b) (x >= a && x <= b) 
#define getBits( x ) (INRANGE(x,'0','9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xa))
#define getByte( x ) (getBits(x[0]) << 4 | getBits(x[1]))

bool Util::compareBytes(const unsigned char* bytes, const char* pattern) {
    for (; *pattern; *pattern != ' ' ? ++bytes : bytes, ++pattern) {
        if (*pattern == ' ' || *pattern == '?') {
            continue;
        }

        if (*bytes != getByte(pattern)) {
            return false;
        }

        ++pattern;
    }

    return true;
}

std::vector<MEMORY_BASIC_INFORMATION> Util::getRegions(HANDLE hProcess) {
    std::vector<MEMORY_BASIC_INFORMATION> regions;

    MEMORY_BASIC_INFORMATION region;
    DWORD64 address;

    for (address = 0; VirtualQueryEx(hProcess, (LPVOID)address, &region, sizeof(region)) == sizeof(region); address += region.RegionSize) {
        regions.push_back(region);
    }

    return regions;
}

std::vector<PROCESSENTRY32> Util::getProcesses(char** errorMessage) {
    HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    PROCESSENTRY32 pEntry;

    if (hProcessSnapshot == INVALID_HANDLE_VALUE) {
        *errorMessage = "method failed to take snapshot of the process";
    }

    pEntry.dwSize = sizeof(pEntry);

    if (!Process32First(hProcessSnapshot, &pEntry)) {
        CloseHandle(hProcessSnapshot);
        *errorMessage = "method failed to retrieve the first process";
    }

    std::vector<PROCESSENTRY32> processes;

    do {
        processes.push_back(pEntry);
    } while (Process32Next(hProcessSnapshot, &pEntry));

    CloseHandle(hProcessSnapshot);
    return processes;
}

struct Pair {
    HANDLE handle;
    PROCESSENTRY32 process;
};

Util::Pair Util::openProcess(DWORD processId, char** errorMessage) {
    PROCESSENTRY32 process;
    HANDLE handle = NULL;

    std::vector<PROCESSENTRY32> processes = Util::getProcesses(errorMessage);

    for (std::vector<PROCESSENTRY32>::size_type i = 0; i != processes.size(); i++) {
        if (processId == processes[i].th32ProcessID) {
            handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processes[i].th32ProcessID);
            process = processes[i];
            break;
        }
    }

    if (handle == NULL) {
        *errorMessage = "unable to find process";
    }

    return {
       handle,
       process,
    };
}

bool Util::findPattern(HANDLE handle, uintptr_t memoryBase, unsigned char* byteBase, DWORD memorySize, const char* pattern, short flags, uint32_t patternOffset, uintptr_t* pAddress) {
    auto maxOffset = memorySize - 0x1000;

    for (uintptr_t offset = 0; offset < maxOffset; ++offset) {
        if (compareBytes(byteBase + offset, pattern)) {
            uintptr_t address = memoryBase + offset + patternOffset;

            if (flags & 0x1) {
                ReadProcessMemory(handle, LPCVOID(address), &address, sizeof(uintptr_t), nullptr);
            }

            if (flags & 0x2) {
                address -= memoryBase;
            }

            *pAddress = address;

            return true;
        }
    }

    return false;
};

bool Util::searchPattern(HANDLE handle, std::vector<MODULEENTRY32> modules, DWORD64 searchAddress, const char* pattern, short flags, uint32_t patternOffset, uintptr_t* pAddress) {
    for (std::vector<MODULEENTRY32>::size_type i = 0; i != modules.size(); i++) {
        uintptr_t baseAddress = (uintptr_t)modules[i].modBaseAddr;
        DWORD baseSize = modules[i].modBaseSize;

        if (searchAddress != 0 && (searchAddress < baseAddress || searchAddress >(baseAddress + baseSize))) {
            continue;
        }

        std::vector<unsigned char> moduleBytes = std::vector<unsigned char>(baseSize);
        ReadProcessMemory(handle, (LPVOID)baseAddress, &moduleBytes[0], baseSize, nullptr);
        unsigned char* byteBase = const_cast<unsigned char*>(&moduleBytes.at(0));

        if (Util::findPattern(handle, baseAddress, byteBase, baseSize, pattern, flags, patternOffset, pAddress)) {
            return true;
        }
    }

    return false;
}

std::vector<MODULEENTRY32> Util::getModules(DWORD processId, char** errorMessage) {
    HANDLE hModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    MODULEENTRY32 mEntry;

    if (hModuleSnapshot == INVALID_HANDLE_VALUE) {
        *errorMessage = "method failed to take snapshot of the modules";
    }

    mEntry.dwSize = sizeof(mEntry);

    if (!Module32First(hModuleSnapshot, &mEntry)) {
        CloseHandle(hModuleSnapshot);
        *errorMessage = "method failed to retrieve the first module";
    }

    std::vector<MODULEENTRY32> modules;

    do {
        modules.push_back(mEntry);
    } while (Module32Next(hModuleSnapshot, &mEntry));

    CloseHandle(hModuleSnapshot);

    return modules;
}

MODULEENTRY32 Util::findModule(const char* moduleName, DWORD processId, char** errorMessage) {
    MODULEENTRY32 module;
    bool found = false;

    std::vector<MODULEENTRY32> moduleEntries = getModules(processId, errorMessage);

    for (std::vector<MODULEENTRY32>::size_type i = 0; i != moduleEntries.size(); i++) {
        if (!strcmp(moduleEntries[i].szModule, moduleName)) {
            module = moduleEntries[i];
            found = true;
            break;
        }
    }

    if (!found) {
        *errorMessage = "unable to find module";
    }

    return module;
}

int Util::findPatternByModule(HANDLE handle, std::string moduleName, std::string pattern, short flags, uint32_t patternOffset) {
    uintptr_t address = 0;

    char* err = "";
    MODULEENTRY32 module = Util::findModule(moduleName.c_str(), GetProcessId(handle), &err);

    uintptr_t baseAddress = (uintptr_t)module.modBaseAddr;
    DWORD baseSize = module.modBaseSize;

    std::vector<unsigned char> moduleBytes = std::vector<unsigned char>(baseSize);
    ReadProcessMemory(handle, (LPVOID)baseAddress, &moduleBytes[0], baseSize, nullptr);
    unsigned char* byteBase = const_cast<unsigned char*>(&moduleBytes.at(0));

    Util::findPattern(handle, baseAddress, byteBase, baseSize, pattern.c_str(), flags, patternOffset, &address);

    return address;
}

std::vector<std::string> Util::split(char* phrase, std::string delimiter) {
    std::vector<std::string> list;
    std::string s = std::string(phrase);
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        list.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    list.push_back(s);
    return list;
}