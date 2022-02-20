#pragma once
#include "package.h"

class MemoryManager
{
public:
    HWND hWnd;
    DWORD pId;
    HANDLE hProcess;

    MemoryManager(const char* sWindowName)
    {
        this->hWnd = FindWindowA(NULL, sWindowName);

        if (!this->hWnd)
        {
            std::cout << "Window " << sWindowName << " was not found. :: " << this->hWnd << std::endl;
            exit(EXIT_FAILURE);
        }

        GetWindowThreadProcessId(hWnd, &this->pId);

        if (!this->pId)
        {
            std::cout << "Could not get process Id for " << sWindowName << " :: " << this->pId << std::endl;
            exit(EXIT_FAILURE);
        }

        this->hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, this->pId);

        if (!this->hProcess)
        {
            std::cout << "Could not open the process " << this->pId << " aka " << sWindowName << " :: " << this->hProcess << std::endl;
            exit(EXIT_FAILURE);
        }

        std::cout << "Found window. HWND :: " << this->hWnd << " pId :: " << this->pId << " hProcess :: " << this->hProcess << std::endl;
    }

    DWORD getModuleBaseAddress(const char* sModuleName)
    {
        HANDLE moduleSnapshot = INVALID_HANDLE_VALUE;
        moduleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, this->pId);

        if (moduleSnapshot == INVALID_HANDLE_VALUE)
        {
            std::cout << "Failed to take a snapshot of the modules." << std::endl;
            CloseHandle(moduleSnapshot);
            return NULL;
        }

        MODULEENTRY32 moduleEntry;
        moduleEntry.dwSize = sizeof(MODULEENTRY32);

        if (Module32First(moduleSnapshot, &moduleEntry))
        {
            if (!strcmp(sModuleName, (const char*)moduleEntry.szModule))
            {
                CloseHandle(moduleSnapshot);
                return (DWORD)moduleEntry.modBaseAddr;
            }
        }

        while (Module32Next(moduleSnapshot, &moduleEntry))
        {
            if (!strcmp(sModuleName, (const char*)moduleEntry.szModule))
            {
                CloseHandle(moduleSnapshot);
                return (DWORD)moduleEntry.modBaseAddr;
            }
        }

        std::cout << sModuleName << " was not found." << std::endl;
        CloseHandle(moduleSnapshot);
        exit(EXIT_FAILURE);
        // return NULL;
    }

    template<class type>
    type read(DWORD address)
    {
        type buffer;
        ReadProcessMemory(this->hProcess, (void*)address, &buffer, sizeof(type), NULL);
        return buffer;
    }

    template<class type>
    void write(DWORD address, type value)
    {
        WriteProcessMemory(this->hProcess, (void*)address, &value, sizeof(type), NULL);
    }
};