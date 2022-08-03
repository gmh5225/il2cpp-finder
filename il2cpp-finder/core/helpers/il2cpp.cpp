#include <Windows.h>
#include <unordered_map>
#include <stdexcept>
#include <array>
#include <format>

#include "il2cpp.hpp"

struct Il2CppDomain;
struct Il2CppAssembly;
struct Il2CppImage;
struct MethodInfo;

#define IMPL_IL2CPP_FUNCTION( pVariable, szProcedureName ) \
if ( pVariable = reinterpret_cast< decltype( pVariable ) >( GetProcAddress( hGameAssembly, szProcedureName ) ); !pVariable ) \
	throw std::runtime_error( "Failed to get \"" szProcedureName "\" procedure address." )

namespace IL2CPP
{
	HMODULE hGameAssembly = nullptr;

	Il2CppDomain* ( __cdecl* pfnGetDomain )( ) = nullptr;
	Il2CppAssembly* ( __cdecl* pfnDomainAssemblyOpen )( Il2CppDomain* pDomain, const char* szModuleName ) = nullptr;
	Il2CppImage* ( __cdecl* pfnAssemblyGetImage )( Il2CppAssembly* pAssembly ) = nullptr;
	Il2CppClass* ( __cdecl* pfnImageGetClass )( Il2CppImage* pImage, int nIndex ) = nullptr;
	const char* ( __cdecl* pfnClassGetName )( Il2CppClass* pClass ) = nullptr;
	int( __cdecl* pfnImageGetClassCount )( Il2CppImage* pImage ) = nullptr;
	void** ( __cdecl* pfnClassGetMethodFromName )( Il2CppClass* pClass, const char* szName, int nArgsCount ) = nullptr;

	Il2CppDomain* pDomain = nullptr; 

	std::unordered_map< Hash_t, std::unordered_map< Hash_t, Il2CppClass* > > mapClasses;

	std::array< const char*, 2 > arrAssemblies = {
			"Assembly-CSharp",
			"UnityEngine.CoreModule"
	};

	void Setup( )
	{
		hGameAssembly = GetModuleHandle( TEXT( "GameAssembly.dll" ) );
		if ( !hGameAssembly )
			throw std::runtime_error( "Failed to get \"GameAssembly.dll\" module." );

		IMPL_IL2CPP_FUNCTION( pfnGetDomain, "il2cpp_domain_get" );
		IMPL_IL2CPP_FUNCTION( pfnDomainAssemblyOpen, "il2cpp_domain_assembly_open" );
		IMPL_IL2CPP_FUNCTION( pfnAssemblyGetImage, "il2cpp_assembly_get_image" );
		IMPL_IL2CPP_FUNCTION( pfnImageGetClass, "il2cpp_image_get_class" );
		IMPL_IL2CPP_FUNCTION( pfnClassGetName, "il2cpp_class_get_name" );
		IMPL_IL2CPP_FUNCTION( pfnImageGetClassCount, "il2cpp_image_get_class_count" );
		IMPL_IL2CPP_FUNCTION( pfnClassGetMethodFromName, "il2cpp_class_get_method_from_name" );

		if ( pDomain = pfnGetDomain( ); !pDomain )
			throw std::runtime_error( "Failed to get il2cpp domain." );

		for ( const auto& szAssembly : arrAssemblies )
		{
			Il2CppAssembly* pAssembly = pfnDomainAssemblyOpen( pDomain, szAssembly );
			if ( !pAssembly )
				throw std::runtime_error( std::vformat( "Failed to open \"{}\" assembly.", std::make_format_args( szAssembly ) ) );

			Il2CppImage* pImage = pfnAssemblyGetImage( pAssembly );
			if ( !pImage )
				throw std::runtime_error( std::vformat( "Failed to get image from \"{}\" assembly.", std::make_format_args( szAssembly ) ) );

			int nClassCount = pfnImageGetClassCount( pImage );
			if ( !nClassCount )
				continue;

			Hash_t nAssemblyHashed = Hash::FNV1A( szAssembly );

			for ( int nClassIndex = 0; nClassIndex < nClassCount; nClassIndex++ )
			{
				Il2CppClass* pClass = pfnImageGetClass( pImage, nClassIndex );
				if ( !pClass )
					continue;

				mapClasses[ nAssemblyHashed ][ Hash::FNV1A( pClass->GetName( ) ) ] = pClass;
			}
		}
	}

	Il2CppClass* FindClass( Hash_t nModuleNameHashed, Hash_t nClassNameHashed )
	{
		return mapClasses[ nModuleNameHashed ][ nClassNameHashed ];
	}
}

const char* Il2CppClass::GetName( )
{
	return IL2CPP::pfnClassGetName( this );
}

void* Il2CppClass::FindMethod( const char* szName, int nArgsCount )
{
	void** ppMethod = IL2CPP::pfnClassGetMethodFromName( this, szName, nArgsCount );

	return ppMethod ? *ppMethod : nullptr;
}

#undef IMPL_IL2CPP_FUNCTION