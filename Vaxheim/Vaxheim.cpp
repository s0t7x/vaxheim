#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <Windows.h>
#include <vector>
#include "Helper.h"
#include "Patch.h"

#define _ASM_RET "\xC3"
#define _ASM_RET_SIZE 1
#define _ASM_RET_TRUE "\xB0\x01\xC3"
#define _ASM_RET_TRUE_SIZE 3
#define _ASM_RET_FALSE "\xB0\x00\xC3"
#define _ASM_RET_FALSE_SIZE 3

#define RAWSTR(x) #x

std::vector<Patch*> g_patchList;

bool g_shouldExit = false;
bool exitHandler() {
	g_shouldExit = true;
	for (auto p : g_patchList) {
		p->unpatch();
	}
	Sleep(2000);
	return true;
}

int main() {
	enableDebugPriviliges();
	std::cout << "Enabled Debug Privileges" << std::endl;

	hookExitHandler((PHANDLER_ROUTINE)exitHandler);
	std::cout << "Hooked Exit Handler" << std::endl;

	uintptr_t valheimPID = NULL;
	valheimPID = getPIDByProcessName("valheim.exe");
	
	if (!valheimPID) {
		startViaSteam();
		Sleep(3000);
	}
	while (!valheimPID) {
		valheimPID = getPIDByProcessName("valheim.exe");
	}
	std::cout << "valheimPID: " << valheimPID << std::endl;

	HANDLE valheimHandle = NULL;
	while (!valheimHandle) {
		valheimHandle = OpenProcess(PROCESS_ALL_ACCESS, NULL, valheimPID);
	}

	Patch * PlayerInGodMode = new Patch{
		valheimHandle,
		"\x48\x83\xEC\x00\x48\x89\x00\x00\x48\x8B\x00\x0F\xB6\x80\x51\x00\x00\x00\x48\x83\xC4\x00\xC3",
		"xxx?xx??xx?xxxx???xxx?x",
		(BYTE*)_ASM_RET_TRUE,
		_ASM_RET_TRUE_SIZE,
		RAWSTR(PlayerInGodMode)
	};
	g_patchList.push_back(PlayerInGodMode);

	Patch * PlayerHaveStamina = new Patch{
		valheimHandle,
		"\x55\x48\x8B\x00\x48\x83\xEC\x00\x48\x89\x00\x00\x48\x8B\x00\xF3\x0F\x00\x00\x00\x48\x8B\x00\x00\x00\x00\x00\x48\x8B\x00\x83\x38\x00\x48\x8D\x00\x00\x00\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\x00\x85\xC0\x0F\x84\x00\x00\x00\x00\x48\x8B\x00\x00\x00\x00\x00\x48\x8B\x00\x83\x38\x00\x48\x8D\x00\x00\x00\x90\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\x00\x85\xC0\x0F\x85",
		"xxx?xxx?xx??xx?xx???xx?????xx?xx?xx???xx????????xx?xxxx????xx?????xx?xx?xx???xxx????????xx?xxxx",
		(BYTE*)_ASM_RET_TRUE,
		_ASM_RET_TRUE_SIZE,
		RAWSTR(PlayerHaveStamina)
	};
	g_patchList.push_back(PlayerHaveStamina);

	Patch * PlayerIsDebugFlying = new Patch{
		valheimHandle,
		"\x55\x48\x8B\xEC\x48\x83\xEC\x00\x48\x89\x00\x00\x48\x8B\x00\x48\x8B\x00\x00\x00\x00\x00\x48\x8B\x00\x83\x38\x00\x66\x90\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\xD3\x85\xC0\x74\x09\x0F\xB6\x00\x00",
		"xxxxxxx?xx??xx?xx?????xx?xx?xxxx????????xxxxxxxxx??",
		(BYTE*)_ASM_RET_TRUE,
		_ASM_RET_TRUE_SIZE,
		RAWSTR(PlayerIsDebugFlying)
	};
	g_patchList.push_back(PlayerIsDebugFlying);

	Patch * PlayerHaveRequirementsPiece = new Patch{
		valheimHandle,
		"\x55\x48\x8B\x00\x48\x81\xEC\x00\x00\x00\x00\x48\x89\x00\x00\x48\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x8B\x00\x48\x8B\x00\x49\x8B\x00\x48\x8B\x00\x00\x48\x8D\x00\x00\x00\x90\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\x00\x85\xC0\x0F\x84\x00\x00\x00\x00\x83\xFE\x01\x74\x05",
		"xxx?xxx????xx??xx??xx??xx??xx??xx??xx?xx?xx?xx??xx???xxx????????xx?xxxx????xxxxx",
		(BYTE*)_ASM_RET_TRUE,
		_ASM_RET_TRUE_SIZE,
		RAWSTR(PlayerHaveRequirementsPiece)
	};
	Patch * PlayerHaveRequirementsRecipe = new Patch{
		valheimHandle,
		"\x55\x48\x8B\x00\x48\x83\xEC\x00\x48\x89\x00\x00\x48\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x8B\x00\x4C\x8B\x00\x49\x8B\x00\x49\x8B\x00\x85\xFF",
		"xxx?xxx?xx??xx??xx??xx??xx?xx?xx?xx?xx",
		(BYTE*)_ASM_RET_TRUE,
		_ASM_RET_TRUE_SIZE,
		RAWSTR(PlayerHaveRequirementsRecipe)
	};
	g_patchList.push_back(PlayerHaveRequirementsPiece);
	g_patchList.push_back(PlayerHaveRequirementsRecipe);

	std::cout << "Initialized!\n" << std::endl;

	while (!g_shouldExit) {
		
		if (GetAsyncKeyState(VK_MENU) & 0x8000) {
			if (GetAsyncKeyState(VK_NUMPAD1) & 0x1) {
				PlayerInGodMode->toggle();
			}
			if(GetAsyncKeyState(VK_NUMPAD2) & 0x1) {
				PlayerHaveStamina->toggle();
			}
			if (GetAsyncKeyState(VK_NUMPAD3) & 0x1) {
				PlayerIsDebugFlying->toggle();
			}
			if (GetAsyncKeyState(VK_NUMPAD4) & 0x1) {
				PlayerHaveRequirementsRecipe->toggle();
				if (PlayerHaveRequirementsRecipe->isPatched) {
					PlayerHaveRequirementsPiece->scanAndPatch();
				}
				else {
					PlayerHaveRequirementsPiece->unpatch();
				}
			}
		}
		//PlayerHaveStamina->scanAndPatch();
		//std::cout << PlayerHaveStamina->m_addr << std::endl;
		//std::cout << PlayerHaveStamina->isPatched << std::endl;
		//std::cin.get();
	}

	return EXIT_SUCCESS;
}