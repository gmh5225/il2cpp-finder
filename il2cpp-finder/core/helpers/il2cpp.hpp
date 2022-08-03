#pragma once

#include "hash.hpp"

struct Il2CppClass;

namespace IL2CPP
{
	void Setup( );

	Il2CppClass* FindClass( Hash_t nModuleNameHashed, Hash_t nClassNameHashed );
}

struct Il2CppClass
{
	const char* GetName( );

	void* FindMethod( const char* szName, int nArgsCount );

	template< typename tReturnValue, typename... tArgList >
	auto FindMethod( const char* szName )
	{
		tReturnValue ( __thiscall * pfnInvokable )( void*, tArgList... ) = reinterpret_cast< decltype( pfnInvokable ) >( FindMethod( szName, sizeof...( tArgList ) ) );

		return pfnInvokable;
	}
};