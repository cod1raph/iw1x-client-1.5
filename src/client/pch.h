#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

// Windows
#include <Windows.h>
#include <atlbase.h>
#include <dbghelp.h>
#include <TlHelp32.h>
#include <wincrypt.h>

// C++
#include <cassert>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>

// Submodules
#include <gsl/gsl>
#include <MinHook.h>

/*
Size found using Ghidra
IMAGE_NT_HEADERS32 -> IMAGE_OPTIONAL_HEADER32: SizeOfImage - SizeOfHeaders
*/
// CD:		0x1897000
// Steam:	0x1899C00
// Currently using the bigger value to be able to inform that Steam is currently unsupported.
constexpr auto BINARY_PAYLOAD_SIZE = 0x1899C00;

constexpr auto MOD_NAME = "iw1x-1.5";

#pragma warning(disable: 26440) // Disable "Function '' can be declared 'noexcept'" warning