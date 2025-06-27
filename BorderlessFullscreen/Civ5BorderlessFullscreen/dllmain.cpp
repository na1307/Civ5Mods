#include "pch.h"
#include <corecrt_wstdio.h>
#include "FunctionPointers.h"
#include "../../minhook/include/MinHook.h"

#pragma comment(lib, "dxgi.lib")

#if _DEBUG
#pragma comment(lib, "../lib/Debug/libMinHook.x86.lib")
#else
#pragma comment(lib, "../lib/Release/libMinHook.x86.lib")
#endif

CreateSwapChain_t oCreateSwapChain = nullptr;
Present_t oPresent = nullptr;
IDXGISwapChain *swapChain = nullptr;

HRESULT STDMETHODCALLTYPE hkPresent(IDXGISwapChain *pSwapChain, UINT syncInterval, UINT flags) {
	BOOL isFullscreen = FALSE;
	IDXGIOutput *pOutput = nullptr;

	if (SUCCEEDED(pSwapChain->GetFullscreenState(&isFullscreen, &pOutput)) && isFullscreen && FAILED(pSwapChain->SetFullscreenState(FALSE, nullptr))) {
		MessageBoxW(nullptr, L"Windowing Failed", L"Error", MB_ICONERROR);
	}

	if (pOutput) {
		pOutput->Release();
	}

	return oPresent(pSwapChain, syncInterval, flags);
}

HRESULT STDMETHODCALLTYPE hkCreateSwapChain(
	IDXGIFactory *pFactory, IUnknown *pDevice,
	DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain
) {
	HRESULT hr = oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);

	if (SUCCEEDED(hr) && ppSwapChain && *ppSwapChain) {
		if (swapChain) {
			swapChain->Release();
		}

		swapChain = *ppSwapChain;
		swapChain->AddRef();

		if (!oPresent) {
			void **vtbl = *reinterpret_cast<void ***>(swapChain);
			void *presentAddr = vtbl[8];
			auto mhc = MH_CreateHook(presentAddr, &hkPresent, reinterpret_cast<void **>(&oPresent));

			if (mhc != MH_OK) {
				wchar_t asdf[10];
				swprintf_s(asdf, L"%d", mhc);
				MessageBoxW(nullptr, asdf, nullptr, MB_ICONERROR);

				return 1;
			}

			auto mhe = MH_EnableHook(presentAddr);

			if (mhe != MH_OK) {
				wchar_t asdf[10];
				swprintf_s(asdf, L"%d", mhe);
				MessageBoxW(nullptr, asdf, nullptr, MB_ICONERROR);

				return 1;
			}
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

			//while (true) {
			//	Sleep(30000);
			//}

			return 0;
		}, nullptr, 0, nullptr);

		CloseHandle(sct);
	} else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();
	}

	return TRUE;
}
