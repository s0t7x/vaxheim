#pragma once
#include <iostream>
#include <Windows.h>
#include <string>
#include "ArrayOfBytes.h"

#define PRINTFUNCHEADER() { std::string t; t = __FUNCDNAME__; std::cout << name << "::" << t.substr(0,t.find("@@")) << "..."; }

class Patch
{
public:
	HANDLE processHandle;
	const char * signaturePattern;
	const char * signatureMask;

	uintptr_t addrRangeStart = 0x0;
	uintptr_t addrRangeSize = 0x3FFFFFFFFFFF;

	uintptr_t m_addr = NULL;
	uintptr_t m_offset = 0;

	BYTE * dataToPatch;
	BYTE * originalBytes;
	size_t size;

	std::string name = { typeid(this).name() };

	bool isPatched = false;

	inline void scan() {
		PRINTFUNCHEADER();
		uintptr_t result = aobScan(processHandle, signaturePattern, signatureMask, (char*)addrRangeStart, addrRangeSize);
		if (result) m_addr = result;
		std::cout << result << std::endl;
	}

	inline void patch() {
		if (!m_addr || isPatched) return;

		PRINTFUNCHEADER();

		originalBytes = (BYTE *)malloc(size);
		ReadProcessMemory(processHandle, (LPCVOID)(m_addr + m_offset), (LPVOID)originalBytes, size, nullptr);

		size_t cb = 0;
		WriteProcessMemory(processHandle, (LPVOID)(m_addr + m_offset), dataToPatch, size, &cb);
		if (cb == size)
			isPatched = true;

		std::cout << (isPatched ? "PATCHED" : "FAILED") << std::endl;
	}

	inline void unpatch() {
		if (!m_addr || !isPatched) return;

		PRINTFUNCHEADER();

		size_t cb;
		WriteProcessMemory(processHandle, (LPVOID)(m_addr + m_offset), (LPCVOID)originalBytes, size, &cb);
		if (cb == size) {
			free(originalBytes);
			isPatched = false;
		}
		std::cout << (isPatched ? "FAILED" : "UNPATCHED") << std::endl;
	};

	inline void scanAndPatch() {
		if (!m_addr) scan();
		patch();
	}

	inline void toggle() {
		if (isPatched)
			unpatch();
		else
			patch();//scanAndPatch();
		std::cout << name << "::" << (isPatched ? "ACTIVATED" : "DEACTIVATED") << std::endl;
	}

	inline Patch(HANDLE processHandle, const char * signaturePattern, const char * signatureMask, BYTE * data, size_t size, std::string name = "", uintptr_t offset = 0x0) {
		this->processHandle = processHandle;
		this->signaturePattern = signaturePattern;
		this->signatureMask = signatureMask;
		this->dataToPatch = data;
		this->size = size;
		if (name.length() > 1) this->name = name;
		this->m_offset = offset;
		//scan();
	}

	inline ~Patch() {
		if (m_addr && isPatched)
			unpatch();
	}
};

