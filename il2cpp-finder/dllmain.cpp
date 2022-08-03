#include <Windows.h>
#include <thread>
#include <format>

#include "core/helpers/hash.hpp"
#include "core/helpers/il2cpp.hpp"

void OnDllAttach( HMODULE hModule )
{
	DisableThreadLibraryCalls( hModule );

	try
	{
		IL2CPP::Setup( );

		Il2CppClass* pClass = IL2CPP::FindClass( "UnityEngine.CoreModule"_fnv1a, "Camera"_fnv1a );
		if ( !pClass )
			throw std::runtime_error( "Failed to get class." );

		auto pfnGetMain = pClass->FindMethod< void* >( "get_main" );

		MessageBoxA( nullptr, std::vformat( "pfnGetMain addr: {:X}", std::make_format_args( reinterpret_cast< std::uintptr_t >( pfnGetMain ) ) ).c_str( ), PROJECT_NAME, MB_OK );
	}
	catch ( std::exception& exInfo )
	{
		MessageBoxA( nullptr, std::vformat( "Something fucked up:\n\n{}", std::make_format_args( exInfo.what( ) ) ).c_str( ), PROJECT_NAME, MB_OK | MB_ICONERROR );
	}
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved )
{
	if ( dwReason == DLL_PROCESS_ATTACH )
		std::thread( OnDllAttach, hModule ).detach( );

	return TRUE;
}

