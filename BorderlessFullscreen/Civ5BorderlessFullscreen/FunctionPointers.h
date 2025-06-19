#pragma once
#include "pch.h"
#include <dxgi.h>

typedef HRESULT(STDMETHODCALLTYPE *CreateSwapChain_t)(
    IDXGIFactory *pFactory, IUnknown *pDevice,
    DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain
);

typedef HRESULT(STDMETHODCALLTYPE *Present_t)(IDXGISwapChain *, UINT, UINT);
