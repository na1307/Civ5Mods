#include "pch.h"

#include <memory>
#include <string>
#include <windows.h>
#include <psapi.h>
#include "sqlite3.h"
#include "../../minhook/include/MinHook.h"

#pragma managed(push, off)

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "sqlite3.lib")

#if _DEBUG
#pragma comment(lib, "../lib/Debug/libMinHook.x86.lib")
#else
#pragma comment(lib, "../lib/Release/libMinHook.x86.lib")
#endif

typedef void(__fastcall *GameStart_t)(int);

char *gameStartAddr = nullptr;
GameStart_t oGameStart = nullptr;

char *ScanIn(const char *pattern, const char *mask, const int length, char *begin, const unsigned int size) {
	for (unsigned int i = 0; i < size - length; i++) {
		bool found = true;

		for (unsigned int j = 0; j < length; j++) {
			if (mask[j] != '?' && pattern[j] != *(begin + i + j)) {
				found = false;
				break;
			}
		}

		if (found) {
			return begin + i;
		}
	}

	return nullptr;
}

char *ScanEx(const char *pattern, const char *mask, const int length, char *begin, char *end, const HANDLE hProc) {
	char *currentChunk = begin;
	char *match = nullptr;
	SIZE_T bytesRead;

	while (currentChunk < end) {
		MEMORY_BASIC_INFORMATION mbi;

		//return nullptr if VirtualQuery fails
		if (!VirtualQueryEx(hProc, currentChunk, &mbi, sizeof(mbi))) {
			return nullptr;
		}

		std::unique_ptr<char[]> buffer(new char[mbi.RegionSize]);

		if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS) {
			DWORD oldprotect;

			if (VirtualProtectEx(hProc, mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &oldprotect)) {
				ReadProcessMemory(hProc, mbi.BaseAddress, buffer.get(), mbi.RegionSize, &bytesRead);
				VirtualProtectEx(hProc, mbi.BaseAddress, mbi.RegionSize, oldprotect, &oldprotect);

				const char *internalAddress = ScanIn(pattern, mask, length, buffer.get(), bytesRead);

				if (internalAddress != nullptr) {
					//calculate from internal to external
					const uintptr_t offsetFromBuffer = internalAddress - buffer.get();
					match = currentChunk + offsetFromBuffer;
					break;
				}
			}
		}

		currentChunk = currentChunk + mbi.RegionSize;
	}

	return match;
}

void __fastcall hkGameStart(int i) {
	oGameStart(i);
}

DWORD WINAPI InitHook(LPVOID) {
	//Sleep(60000);

	if (MH_Initialize() != MH_OK) {
		return 1;
	}

	constexpr auto pattern = "\x55\x8B\xEC\x83\xE4\xF8\xA1\x00\x00\x00\x00\x83\xEC\x28";
	constexpr auto mask = "xxxxxxx????xxx";
	const auto cp = GetCurrentProcess();
	const auto m = GetModuleHandleW(nullptr);
	MODULEINFO mInfo;
	GetModuleInformation(cp, m, &mInfo, sizeof(MODULEINFO));
	const auto end = reinterpret_cast<char *>(m + mInfo.SizeOfImage);
	gameStartAddr = ScanEx(pattern, mask, 14, reinterpret_cast<char *>(m), end, cp);

	if (!gameStartAddr) {
		return 1;
	}

	if (MH_CreateHook(gameStartAddr, &hkGameStart, reinterpret_cast<void **>(&oGameStart)) != MH_OK) {
		return 1;
	}

	if (MH_EnableHook(gameStartAddr) != MH_OK) {
		return 1;
	}

	return 0;
}

int callback(void *, int argc, char **argv, char **) {
	for (int i = 0; i < argc; i++) {
		OutputDebugStringA(argv[i]);
		OutputDebugStringW(L"\n");
	}

	return 0;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		auto ih = CreateThread(nullptr, 0, InitHook, nullptr, 0, nullptr);
		CloseHandle(ih);

		//sqlite3 *db;

		//int exitcode = sqlite3_open("C:\\Users\\na130\\Documents\\My Games\\Sid Meier's Civilization 5\\cache\\Civ5ModsDatabase.db", &db);

		//if (exitcode) {
		//	OutputDebugStringW(std::to_wstring(exitcode).c_str());
		//	OutputDebugStringW(L"\n");

		//	return FALSE;
		//}

		//char *em = nullptr;
		//exitcode = sqlite3_exec(db, "SELECT EvaluatedPath FROM ModFiles WHERE Import = 1", callback, nullptr, &em);

		//if (exitcode) {
		//	OutputDebugStringA(em);
		//	OutputDebugStringW(L"\n");
		//	sqlite3_free(em);
		//}

		//sqlite3_close(db);
	} else if (reason == DLL_PROCESS_DETACH) {
		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();
	}

	return TRUE;
}
