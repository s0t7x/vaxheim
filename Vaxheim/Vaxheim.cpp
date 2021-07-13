#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <string>
#include <tlhelp32.h>
#include <Psapi.h>
#include <regex>

#define _ASM_RET_TRUE_SIZE 3
#define _ASM_RET_TRUE "\xB0\x01\xC3"
#define _ASM_RET_FALSE_SIZE 3
#define _ASM_RET_FALSE "\xB0\x00\xC3"
#define _ASM_RET_SIZE 1
#define _ASM_RET "\xC3"


HANDLE g_process_handle = nullptr;

void EnableDebugPriv()
{
	sizeof(_ASM_RET_TRUE);
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tkp;

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luid;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, false, &tkp, sizeof(tkp), NULL, NULL);

	CloseHandle(hToken);
}

uintptr_t getPIDByName(std::string name) {
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	uintptr_t pid = 0;

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (_stricmp(entry.szExeFile, name.c_str()) == 0)
			{
				pid = entry.th32ProcessID;
			}
		}
	}

	CloseHandle(snapshot);
	return pid;
}

char* ScanBasic(const char* pattern, const char* mask, char* begin, intptr_t size)
{
	intptr_t patternLen = strlen(mask);

	for (int i = 0; i < size; i++)
	{
		bool found = true;
		for (int j = 0; j < patternLen; j++)
		{
			if (mask[j] != '?' && pattern[j] != *(char*)((intptr_t)begin + i + j))
			{
				found = false;
				break;
			}
		}
		if (found)
		{
			return (begin + i);
		}
	}
	return nullptr;
}

char* ScanEx(const char* pattern, const char* mask, char* begin, intptr_t size, HANDLE hProc)
{
	char* match{ nullptr };
	SIZE_T bytesRead;
	DWORD oldprotect;
	char* buffer{ nullptr };
	MEMORY_BASIC_INFORMATION mbi;
	mbi.RegionSize = 0x1000;//

	VirtualQueryEx(hProc, (LPCVOID)begin, &mbi, sizeof(mbi));

	for (char* curr = begin; curr < begin + size; curr += mbi.RegionSize)
	{
		if (!VirtualQueryEx(hProc, curr, &mbi, sizeof(mbi))) continue;
		if (mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS) continue;

		delete[] buffer;
		buffer = new char[mbi.RegionSize];

		if (VirtualProtectEx(hProc, mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &oldprotect))
		{
			ReadProcessMemory(hProc, mbi.BaseAddress, buffer, mbi.RegionSize, &bytesRead);
			VirtualProtectEx(hProc, mbi.BaseAddress, mbi.RegionSize, oldprotect, &oldprotect);

			char* internalAddr = ScanBasic(pattern, mask, buffer, (intptr_t)bytesRead);

			if (internalAddr != nullptr)
			{
				//calculate from internal to external
				//std::cout << (uintptr_t)curr << " + " << (uintptr_t)internalAddr << " - " << (uintptr_t)buffer << std::endl;
				match = curr + (internalAddr - buffer);
				break;
			}
		}
	}
	delete[] buffer;
	return match;
}

void patch(HANDLE process, uintptr_t addr, CONST void far * bytes, size_t size, void * originalBytes = nullptr) {
	if (!addr) return;
	if (originalBytes != nullptr) {
		ReadProcessMemory(process, (LPCVOID)addr, originalBytes, size, nullptr);
	}
	WriteProcessMemory(process, (LPVOID)addr, bytes, size, NULL);
	return;
}

bool shouldExit = false;

bool exitHandler() {
	shouldExit = true;
	Sleep(2000);
	return true;
}

int main()
{
	EnableDebugPriv();

	std::cout << "Searching for 'valheim.exe'..." << std::endl;
	uintptr_t pidValheim = getPIDByName("valheim.exe");
	std::cout << "pidValheim: " << std::dec << pidValheim << std::endl;

	std::cout << "Create Handle to 'valheim.exe'..." << std::endl;
	HANDLE hValheim = OpenProcess(PROCESS_ALL_ACCESS, NULL, pidValheim);
	
	std::cout << "Looking for first known signature..." << std::endl;
	// Player:IsDebugFlying = 0x1F09BDF4BF0
	// "\x55\x48\x8B\xEC\x48\x83\xEC\x00\x48\x89\x00\x00\x48\x8B\x00\x48\x8B\x00\x00\x00\x00\x00\x48\x8B\x00\x83\x38\x00\x66\x90\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\xD3\x85\xC0\x74\x09\x0F\xB6\x00\x00", "xxxxxxx?xx??xx?xx?????xx?xx?xxxx????????xxxxxxxxx??"
	// "55 48 8B EC 48 83 EC ? 48 89 ? ? 48 8B ? 48 8B ? ? ? ? ? 48 8B ? 83 38 ? 66 90 49 BB ? ? ? ? ? ? ? ? 41 FF D3 85 C0 74 09 0F B6 ? ?"
	BYTE * originalPlayerIsDebugFlying[_ASM_RET_TRUE_SIZE];
	uintptr_t dwPlayerIsDebugFlying = (uintptr_t)ScanEx(
		"\x55\x48\x8B\xEC\x48\x83\xEC\x00\x48\x89\x00\x00\x48\x8B\x00\x48\x8B\x00\x00\x00\x00\x00\x48\x8B\x00\x83\x38\x00\x66\x90\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\xD3\x85\xC0\x74\x09\x0F\xB6\x00\x00",
		"xxxxxxx?xx??xx?xx?????xx?xx?xxxx????????xxxxxxxxx??",
		0x0, 0x7FFFFFFFFFFF,
		hValheim
	);
	std::cout << "dwPlayerIsDebugFlying: 0x" << std::hex << dwPlayerIsDebugFlying << std::endl;

	std::cout << "Calculating address range..." << std::endl;
	char * addrRangeStart = (char*)((dwPlayerIsDebugFlying - 0x1000000000) & 0xFFFFFFFF00000000);
	intptr_t addrRangeSize = 0x1FFFFFFFFF;
	std::cout << "addrRangeStart: 0x" << std::hex << (uintptr_t)addrRangeStart << std::endl;
	std::cout << "addrRangeEnd: 0x" << std::hex << (uintptr_t)(addrRangeStart) + addrRangeSize << std::endl;

	std::cout << "Looking for signatures in address range (this may take a while)..." << std::endl;
	// PlayerInGodMode = 0x219000A6C60
	// "\x48\x83\xEC\x00\x48\x89\x00\x00\x48\x8B\x00\x0F\xB6\x80\x51\x00\x00\x00\x48\x83\xC4\x00\xC3", "xxx?xx??xx?xxxx???xxx?x"
	// "48 83 EC ? 48 89 ? ? 48 8B ? 0F B6 80 51 ? ? ? 48 83 C4 ? C3"
	BYTE * originalPlayerInGodMode[_ASM_RET_TRUE_SIZE];
	uintptr_t dwPlayerInGodMode = (uintptr_t)ScanEx(
		"\x48\x83\xEC\x00\x48\x89\x00\x00\x48\x8B\x00\x0F\xB6\x80\x51\x00\x00\x00\x48\x83\xC4\x00\xC3",
		"xxx?xx??xx?xxxx???xxx?x",
		addrRangeStart, addrRangeSize,
		hValheim
	);
	std::cout << "dwPlayerInGodMode: 0x" << std::hex << dwPlayerInGodMode << std::endl;

	// PlayerHaveStamina
	// "\x55\x48\x8B\x00\x48\x83\xEC\x00\x48\x89\x00\x00\x48\x8B\x00\xF3\x0F\x00\x00\x00\x48\x8B\x86\x00\x00\x00\x00\x48\x8B\x00\x83\x38\x00\x48\x8D\x64\x24\x00\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\xD3\x85\xC0\x0F\x84\x00\x00\x00\x00\x48\x8B\x00\x00\x00\x00\x00\x48\x8B\x00\x83\x38\x00\x48\x8D\x00\x00\x00\x90\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\x00\x85\xC0\x0F\x85", 
	// "xxx?xxx?xx??xx?xx???xxx????xx?xx?xxxx?xx????????xxxxxxx????xx?????xx?xx?xx???xxx????????xx?xxxx"
	// "55 48 8B ? 48 83 EC ? 48 89 ? ? 48 8B ? F3 0F ? ? ? 48 8B 86 ? ? ? ? 48 8B ? 83 38 ? 48 8D 64 24 ? 49 BB ? ? ? ? ? ? ? ? 41 FF D3 85 C0 0F 84 ? ? ? ? 48 8B ? ? ? ? ? 48 8B ? 83 38 ? 48 8D ? ? ? 90 49 BB ? ? ? ? ? ? ? ? 41 FF ? 85 C0 0F 85"
	BYTE * originalPlayerHaveStamina[_ASM_RET_TRUE_SIZE];
	uintptr_t dwPlayerHaveStamina = (uintptr_t)ScanEx(
		"\x55\x48\x8B\x00\x48\x83\xEC\x00\x48\x89\x00\x00\x48\x8B\x00\xF3\x0F\x00\x00\x00\x48\x8B\x00\x00\x00\x00\x00\x48\x8B\x00\x83\x38\x00\x48\x8D\x00\x00\x00\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\x00\x85\xC0\x0F\x84\x00\x00\x00\x00\x48\x8B\x00\x00\x00\x00\x00\x48\x8B\x00\x83\x38\x00\x48\x8D\x00\x00\x00\x90\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\x00\x85\xC0\x0F\x85",
		"xxx?xxx?xx??xx?xx???xx?????xx?xx?xx???xx????????xx?xxxx????xx?????xx?xx?xx???xxx????????xx?xxxx",
		addrRangeStart, addrRangeSize,
		hValheim
	);
	std::cout << "dwPlayerHaveStamina: 0x" << std::hex << dwPlayerHaveStamina << std::endl;

	// PlayerHaveRequirementsPiece = 0x238431FA1D0 (Building)
	// "\x55\x48\x8B\x00\x48\x81\xEC\x00\x00\x00\x00\x48\x89\x00\x00\x48\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x8B\x00\x48\x8B\x00\x49\x8B\x00\x48\x8B\x00\x00\x48\x8D\x00\x00\x00\x90\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\x00\x85\xC0\x0F\x84\x00\x00\x00\x00\x83\xFE\x01\x74\x05",
	// "xxx?xxx????xx??xx??xx??xx??xx??xx??xx?xx?xx?xx??xx???xxx????????xx?xxxx????xxxxx"
	// "55 48 8B ? 48 81 EC ? ? ? ? 48 89 ? ? 48 89 ? ? 4C 89 ? ? 4C 89 ? ? 4C 89 ? ? 4C 89 ? ? 4C 8B ? 48 8B ? 49 8B ? 48 8B ? ? 48 8D ? ? ? 90 49 BB ? ? ? ? ? ? ? ? 41 FF ? 85 C0 0F 84 ? ? ? ? 83 FE 01 74 05"
	BYTE * originalPlayerHaveRequirementsPiece[_ASM_RET_TRUE_SIZE];
	uintptr_t dwPlayerHaveRequirementsPiece = (uintptr_t)ScanEx(
		"\x55\x48\x8B\x00\x48\x81\xEC\x00\x00\x00\x00\x48\x89\x00\x00\x48\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x8B\x00\x48\x8B\x00\x49\x8B\x00\x48\x8B\x00\x00\x48\x8D\x00\x00\x00\x90\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\x00\x85\xC0\x0F\x84\x00\x00\x00\x00\x83\xFE\x01\x74\x05",
		"xxx?xxx????xx??xx??xx??xx??xx??xx??xx?xx?xx?xx??xx???xxx????????xx?xxxx????xxxxx",
		addrRangeStart, addrRangeSize,
		hValheim
	);
	std::cout << "dwPlayerHaveRequirementsPiece: 0x" << std::hex << dwPlayerHaveRequirementsPiece << std::endl;

	// PlayerHaveRequirementsRecipe (Workbench)
	// "\x55\x48\x8B\x00\x48\x83\xEC\x00\x48\x89\x00\x00\x48\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x8B\x00\x4C\x8B\x00\x49\x8B\x00\x49\x8B\x00\x85\xFF", "xxx?xxx?xx??xx??xx??xx??xx?xx?xx?xx?xx"
	// "55 48 8B ? 48 83 EC ? 48 89 ? ? 48 89 ? ? 4C 89 ? ? 4C 89 ? ? 4C 8B ? 4C 8B ? 49 8B ? 49 8B ? 85 FF"
	BYTE * originalPlayerHaveRequirementsRecipe[_ASM_RET_TRUE_SIZE];
	uintptr_t dwPlayerHaveRequirementsRecipe = (uintptr_t)ScanEx(
		"\x55\x48\x8B\x00\x48\x83\xEC\x00\x48\x89\x00\x00\x48\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x8B\x00\x4C\x8B\x00\x49\x8B\x00\x49\x8B\x00\x85\xFF",
		"xxx?xxx?xx??xx??xx??xx??xx?xx?xx?xx?xx",
		addrRangeStart, addrRangeSize,
		hValheim
	);
	std::cout << "dwPlayerHaveRequirementsRecipe: 0x" << std::hex << dwPlayerHaveRequirementsRecipe << std::endl;
	
	bool creativeMode = false;
	bool godMode = false;

	SetConsoleCtrlHandler((PHANDLER_ROUTINE)exitHandler, true);

	std::cout << "Loaded!\n" << std::endl;

	std::cout << "DEL - Toggle God Mode (No Damage, Infinite Stamina)" << std::endl;
	std::cout << "END - Toogle Creative Mode (Fly, Free Build, Free Craft)" << std::endl;
	while (!shouldExit) {
		if (GetAsyncKeyState(VK_END) & 0x1) {
			if (creativeMode) {
				patch(hValheim, dwPlayerIsDebugFlying, originalPlayerIsDebugFlying, _ASM_RET_TRUE_SIZE);
				patch(hValheim, dwPlayerHaveRequirementsPiece, originalPlayerHaveRequirementsPiece, _ASM_RET_TRUE_SIZE);
				patch(hValheim, dwPlayerHaveRequirementsRecipe, originalPlayerHaveRequirementsRecipe, _ASM_RET_TRUE_SIZE);
				//Beep(444, 10);
			}
			else {
				patch(hValheim, dwPlayerIsDebugFlying, _ASM_RET_TRUE, _ASM_RET_TRUE_SIZE, originalPlayerIsDebugFlying);
				patch(hValheim, dwPlayerHaveRequirementsPiece, _ASM_RET_TRUE, _ASM_RET_TRUE_SIZE, originalPlayerHaveRequirementsPiece);
				patch(hValheim, dwPlayerHaveRequirementsRecipe, _ASM_RET_TRUE, _ASM_RET_TRUE_SIZE, originalPlayerHaveRequirementsRecipe);
				//Beep(444 * 2, 10);
			}
			creativeMode = !creativeMode;
			std::cout << "Creative Mode: " << (creativeMode ? "ON" : "OFF") << std::endl;
		}
		if (GetAsyncKeyState(VK_DELETE) & 0x1) {
			if (godMode) {
				patch(hValheim, dwPlayerInGodMode, originalPlayerInGodMode, _ASM_RET_TRUE_SIZE);
				patch(hValheim, dwPlayerHaveStamina, originalPlayerHaveStamina, _ASM_RET_TRUE_SIZE);
			}
			else {
				patch(hValheim, dwPlayerInGodMode, _ASM_RET_TRUE, _ASM_RET_TRUE_SIZE, originalPlayerInGodMode);
				patch(hValheim, dwPlayerHaveStamina, _ASM_RET_TRUE, _ASM_RET_TRUE_SIZE, originalPlayerHaveStamina);
			}
			godMode = !godMode;
			std::cout << "God Mode: " << (godMode ? "ON" : "OFF") << std::endl;
		}
		Sleep(20);
	}

	std::cout << "Removing patches..." << std::endl;

	patch(hValheim, dwPlayerIsDebugFlying, originalPlayerIsDebugFlying, _ASM_RET_TRUE_SIZE);
	patch(hValheim, dwPlayerHaveRequirementsPiece, originalPlayerHaveRequirementsPiece, _ASM_RET_TRUE_SIZE);
	patch(hValheim, dwPlayerHaveRequirementsRecipe, originalPlayerHaveRequirementsRecipe, _ASM_RET_TRUE_SIZE);
	patch(hValheim, dwPlayerInGodMode, originalPlayerInGodMode, _ASM_RET_TRUE_SIZE);
	patch(hValheim, dwPlayerHaveStamina, originalPlayerHaveStamina, _ASM_RET_TRUE_SIZE);

	Sleep(2000);
}
