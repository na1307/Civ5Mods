#include "pch.h"
#include <corecrt_wstdio.h>
#include "CreateSwapChain_t.h"

#pragma comment(lib, "dxgi.lib")

#if _DEBUG
#pragma comment(lib, "../lib/Debug/libMinHook.x86.lib")
#else
#pragma comment(lib, "../lib/Release/libMinHook.x86.lib")
#endif

CreateSwapChain_t oCreateSwapChain = nullptr;

HRESULT STDMETHODCALLTYPE hkCreateSwapChain(
	IDXGIFactory *pFactory, IUnknown *pDevice,
	DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain
) {
	HRESULT hr = oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);

	if (SUCCEEDED(hr) && ppSwapChain && *ppSwapChain) {
		auto fshr = (*ppSwapChain)->SetFullscreenState(FALSE, nullptr);

		if (fshr != S_OK) {
			MessageBoxW(nullptr, fshr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE ? L"Not Available" : L"Unknown", L"Error", MB_ICONHAND);
			MessageBoxW(nullptr, L"Windowed Failed", L"Error", MB_ICONHAND);

			return 1;
		}
	}

	return hr;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		auto mhi = MH_Initialize();

		if (mhi != MH_OK) {
			wchar_t asdf[10];
			swprintf_s(asdf, L"%d", mhi);
			MessageBoxW(nullptr, asdf, nullptr, MB_ICONERROR);

			return FALSE;
		}

		auto sct = CreateThread(nullptr, 0, [](LPVOID) -> DWORD {
			IDXGIFactory *pFactory = nullptr;
			HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void **>(&pFactory));

			if (hr) {
				DWORD le = GetLastError();
				wchar_t asdf[10];
				swprintf_s(asdf, L"%d", le);
				MessageBoxW(nullptr, asdf, nullptr, MB_ICONERROR);

				return 1;
			}

			void **vtable = *reinterpret_cast<void ***>(pFactory);
			void *createSwapChainAddr = vtable[10];
			pFactory->Release();

			auto mhc = MH_CreateHook(
				createSwapChainAddr,
				&hkCreateSwapChain,
				reinterpret_cast<void **>(&oCreateSwapChain)
			);

			if (mhc != MH_OK) {
				wchar_t asdf[10];
				swprintf_s(asdf, L"%d", mhc);
				MessageBoxW(nullptr, asdf, nullptr, MB_ICONERROR);

				return 1;
			}

			auto mhe = MH_EnableHook(createSwapChainAddr);

			if (mhe != MH_OK) {
				wchar_t asdf[10];
				swprintf_s(asdf, L"%d", mhe);
				MessageBoxW(nullptr, asdf, nullptr, MB_ICONERROR);

				return 1;
			}

			return 0;
		}, nullptr, 0, nullptr);

		CloseHandle(sct);
	} else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();
	}

	return TRUE;
}
