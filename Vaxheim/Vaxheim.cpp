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
	std::cout << "exiting..." << std::endl;
	for (auto p : g_patchList) {
		p->unpatch();
	}
	Sleep(2000);
	return true;
}
void _exitHandler() {
	exitHandler();
	terminate();
}

int main() {
	enableDebugPriviliges();
	std::cout << "Enabled Debug Privileges" << std::endl;

	hookExitHandler((PHANDLER_ROUTINE)exitHandler);
	atexit(_exitHandler);
	set_terminate(_exitHandler);
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

	// ShipIsWindControllActive = 0x2703D397880
	// "\x55\x48\x8B\x00\x48\x83\xEC\x00\x48\x89\x00\x00\x33\xC0\x48\x89\x00\x00\x48\x89\x00\x00\x48\x89\x00\x00\x48\x8B\x00\x00\x48\x8B\x00\x00\x48\x8B\x00\x48\x83\xC2\x00\x48\x8B\x00\x83\x38\x00\x48\x8D\x00\x00\x00\x00\x00\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\x00\xE9\x00\x00\x00\x00\x48\x8B\x00\x00\x48\x8B\x00\x83\x39\x00\x48\x8B\x00\x00\x00\x00\x00\x48\x8B", "xxx?xxx?xx??xxxx??xx??xx??xx??xx??xx?xxx?xx?xx?xx?????xx????????xx?x????xx??xx?xx?xx?????xx"
	// "55 48 8B ? 48 83 EC ? 48 89 ? ? 33 C0 48 89 ? ? 48 89 ? ? 48 89 ? ? 48 8B ? ? 48 8B ? ? 48 8B ? 48 83 C2 ? 48 8B ? 83 38 ? 48 8D ? ? ? ? ? 49 BB ? ? ? ? ? ? ? ? 41 FF ? E9 ? ? ? ? 48 8B ? ? 48 8B ? 83 39 ? 48 8B ? ? ? ? ? 48 8B"
	Patch * ShipIsWindControllActive = new Patch{
		valheimHandle,
		"\x55\x48\x8B\x00\x48\x83\xEC\x00\x48\x89\x00\x00\x33\xC0\x48\x89\x00\x00\x48\x89\x00\x00\x48\x89\x00\x00\x48\x8B\x00\x00\x48\x8B\x00\x00\x48\x8B\x00\x48\x83\xC2\x00\x48\x8B\x00\x83\x38\x00\x48\x8D\x00\x00\x00\x00\x00\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\x00\xE9\x00\x00\x00\x00\x48\x8B\x00\x00\x48\x8B\x00\x83\x39\x00\x48\x8B\x00\x00\x00\x00\x00\x48\x8B",
		"xxx?xxx?xx??xxxx??xx??xx??xx??xx??xx?xxx?xx?xx?xx?????xx????????xx?x????xx??xx?xx?xx?????xx",
		(BYTE*)_ASM_RET_TRUE,
		_ASM_RET_TRUE_SIZE,
		RAWSTR(ShipIsWindControllActive)
	};
	g_patchList.push_back(ShipIsWindControllActive);

	// AttackGetLevelDamageFactor
	// Address of signature = 0x1F208A776B0
	// "\x48\x83\xEC\x00\x48\x89\x00\x00\xF3\x0F\x00\x00\x00\x00\x00\x00\xF3\x0F\x00\x00\x48\x8B\x00\x00\x48\x8B", "xxx?xx??xx??????xx??xx??xx"
	// "48 83 EC ? 48 89 ? ? F3 0F ? ? ? ? ? ? F3 0F ? ? 48 8B ? ? 48 8B"
	// float at +0x70
	float newAttackMultiplier = 100.f;
	Patch * AttackGetLevelDamageFactor = new Patch{
		valheimHandle,
		"\x48\x83\xEC\x00\x48\x89\x00\x00\xF3\x0F\x00\x00\x00\x00\x00\x00\xF3\x0F\x00\x00\x48\x8B\x00\x00\x48\x8B",
		"xxx?xx??xx??????xx??xx??xx",
		(BYTE*)(float*)&newAttackMultiplier,
		sizeof(float),
		RAWSTR(AttackGetLevelDamageFactor)
	};
	AttackGetLevelDamageFactor->m_offset = 0x70;
	g_patchList.push_back(AttackGetLevelDamageFactor);

	// InventoryIsTeleportable = 0x1DAEA21DE70
	// "\x55\x48\x8B\x00\x48\x83\xEC\x00\x48\x89\x00\x00\x33\xC0\x48\x89\x00\x00\x48\x89\x00\x00\x48\x89\x00\x00\x48\x8B\x00\x00\x48\x8B\x00\x00\x48\x8B\x00\x48\x83\xC2\x00\x48\x8B\x00\x83\x38\x00\x48\x8D", "xxx?xxx?xx??xxxx??xx??xx??xx??xx??xx?xxx?xx?xx?xx"
	// "55 48 8B ? 48 83 EC ? 48 89 ? ? 33 C0 48 89 ? ? 48 89 ? ? 48 89 ? ? 48 8B ? ? 48 8B ? ? 48 8B ? 48 83 C2 ? 48 8B ? 83 38 ? 48 8D"
	Patch * InventoryIsTeleportable = new Patch{
		valheimHandle,
		"\x55\x48\x8B\x00\x48\x83\xEC\x00\x48\x89\x00\x00\x33\xC0\x48\x89\x00\x00\x48\x89\x00\x00\x48\x89\x00\x00\x48\x8B\x00\x00\x48\x8B\x00\x00\x48\x8B\x00\x48\x83\xC2\x00\x48\x8B\x00\x83\x38\x00\x48\x8D",
		"xxx?xxx?xx??xxxx??xx??xx??xx??xx??xx?xxx?xx?xx?xx",
		(BYTE*)_ASM_RET_TRUE,
		_ASM_RET_TRUE_SIZE,
		RAWSTR(InventoryIsTeleportable)
	};
	g_patchList.push_back(InventoryIsTeleportable);

	// MonsterAIIsSleeping = 0x1DAEA16FE40
	// "\x55\x48\x8B\x00\x48\x83\xEC\x00\x48\x89\x00\x00\x48\x8B\x00\x48\x8B\x00\x00\x48\x8B\x00\x83\x38\x00\x48\x8D\x00\x00\x00\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\x00\x85\xC0\x75\x00\x33\xC0\xEB\x00\x48\x8B\x00\x00\x48\x8B\x00\x83\x39\x00\x48\x8B\x00\x00\x44\x0F", "xxx?xxx?xx??xx?xx??xx?xx?xx???xx????????xx?xxx?xxx?xx??xx?xx?xx??xx"
	// "55 48 8B ? 48 83 EC ? 48 89 ? ? 48 8B ? 48 8B ? ? 48 8B ? 83 38 ? 48 8D ? ? ? 49 BB ? ? ? ? ? ? ? ? 41 FF ? 85 C0 75 ? 33 C0 EB ? 48 8B ? ? 48 8B ? 83 39 ? 48 8B ? ? 44 0F"
	Patch * MonsterAIIsSleeping = new Patch{
		valheimHandle,
		"\x55\x48\x8B\x00\x48\x83\xEC\x00\x48\x89\x00\x00\x48\x8B\x00\x48\x8B\x00\x00\x48\x8B\x00\x83\x38\x00\x48\x8D\x00\x00\x00\x49\xBB\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\x00\x85\xC0\x75\x00\x33\xC0\xEB\x00\x48\x8B\x00\x00\x48\x8B\x00\x83\x39\x00\x48\x8B\x00\x00\x44\x0F",
		"xxx?xxx?xx??xx?xx??xx?xx?xx???xx????????xx?xxx?xxx?xx??xx?xx?xx??xx",
		(BYTE*)_ASM_RET_TRUE,
		_ASM_RET_TRUE_SIZE,
		RAWSTR(MonsterAIIsSleeping)
	};
	g_patchList.push_back(MonsterAIIsSleeping);

	// InventoryMoveInventoryToGrave = 0x1DA099A1F50
	// "\x55\x48\x8B\x00\x48\x81\xEC\x00\x00\x00\x00\x48\x89\x00\x00\x48\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x48\x8B\x00\x48\x8B\x00\x33\xC0\x48\x89\x00\x00\x48\x89\x00\x00\x48\x89\x00\x00\x48\x8B", "xxx?xxx????xx??xx??xx??xx??xx?xx?xxxx??xx??xx??xx"
	// "55 48 8B ? 48 81 EC ? ? ? ? 48 89 ? ? 48 89 ? ? 4C 89 ? ? 4C 89 ? ? 48 8B ? 48 8B ? 33 C0 48 89 ? ? 48 89 ? ? 48 89 ? ? 48 8B"
	Patch * InventoryMoveInventoryToGrave = new Patch{
		valheimHandle,
		"\x55\x48\x8B\x00\x48\x81\xEC\x00\x00\x00\x00\x48\x89\x00\x00\x48\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x48\x8B\x00\x48\x8B\x00\x33\xC0\x48\x89\x00\x00\x48\x89\x00\x00\x48\x89\x00\x00\x48\x8B",
		"xxx?xxx????xx??xx??xx??xx??xx?xx?xxxx??xx??xx??xx",
		(BYTE*)_ASM_RET,
		_ASM_RET_SIZE,
		RAWSTR(InventoryMoveInventoryToGrave)
	};
	g_patchList.push_back(InventoryMoveInventoryToGrave);

	// InventoryAddItem = 0x210A832A280
	// "\x55\x48\x8B\x00\x48\x81\xEC\x00\x00\x00\x00\x48\x89\x00\x00\x48\x89\x00\x00\x48\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x8B\x00\x4C\x8B\x00\x4D\x8B\x00\x49\x8B\x00\x48\x8B\x00\x00\x49\x63\x00\x00\x44\x3B", "xxx?xxx????xx??xx??xx??xx??xx??xx??xx??xx?xx?xx?xx?xx??xx??xx"
	// "55 48 8B ? 48 81 EC ? ? ? ? 48 89 ? ? 48 89 ? ? 48 89 ? ? 4C 89 ? ? 4C 89 ? ? 4C 89 ? ? 4C 89 ? ? 4C 8B ? 4C 8B ? 4D 8B ? 49 8B ? 48 8B ? ? 49 63 ? ? 44 3B"
	// "remove" at +2ef
	Patch * InventoryAddItem = new Patch{
		valheimHandle,
		"\x55\x48\x8B\x00\x48\x81\xEC\x00\x00\x00\x00\x48\x89\x00\x00\x48\x89\x00\x00\x48\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x89\x00\x00\x4C\x8B\x00\x4C\x8B\x00\x4D\x8B\x00\x49\x8B\x00\x48\x8B\x00\x00\x49\x63\x00\x00\x44\x3B", 
		"xxx?xxx????xx??xx??xx??xx??xx??xx??xx??xx?xx?xx?xx?xx??xx??xx",
		(BYTE*)"\x90\x90\x90",
		3,
		RAWSTR(InventoryAddItem)
	};
	InventoryAddItem->m_offset = 0x2ef;
	g_patchList.push_back(InventoryAddItem);

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
			if (GetAsyncKeyState(VK_NUMPAD5) & 0x1) {
				ShipIsWindControllActive->toggle();
			}
			if (GetAsyncKeyState(VK_NUMPAD6) & 0x1) {
				AttackGetLevelDamageFactor->toggle();
			}
			if (GetAsyncKeyState(VK_NUMPAD7) & 0x1) {
				InventoryIsTeleportable->toggle();
			}
			if (GetAsyncKeyState(VK_NUMPAD8) & 0x1) {
				MonsterAIIsSleeping->toggle();
			}
			if (GetAsyncKeyState(VK_NUMPAD9) & 0x1) {
				InventoryMoveInventoryToGrave->toggle();
			}
			if (GetAsyncKeyState(VK_NUMPAD0) & 0x1) {
				InventoryAddItem->toggle();
			}
			
		}

	}

	return 0;
}