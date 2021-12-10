#pragma once

#include <fstream>
#include <memory>
#include <vector>
#include <io.h>

#include <Windows.h>

#include <d3d12.h>
#include <d3d12shader.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <DirectXMath.h>
using namespace DirectX;

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "d3d12.lib" )
#pragma comment( lib, "dxgi.lib" )

#define SCREEN_WIDTH	(960)			// ウインドウの幅
#define SCREEN_HEIGHT	(540)			// ウインドウの高さ

HWND GetWindow();
