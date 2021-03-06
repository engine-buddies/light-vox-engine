//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#if defined(_WIN32) || defined(_WIN64)
#include <wrl.h>
#include <Windows.h>
#include "d3dx12.h"
#include <stdlib.h>  
#include <iostream>

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
using Microsoft::WRL::ComPtr;

#endif



#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
class HrException : public std::runtime_error
{
public:
    HrException(LV_RESULT hr ) : std::runtime_error( s_str ), m_hr( hr )
    {
        sprintf_s( s_str, "HRESULT of 0x%08X", static_cast<UINT>( hr ) );
    }

	LV_RESULT Error() const { return m_hr; }
private:
    char s_str[ 64 ] = {};
    const LV_RESULT m_hr;
};

inline void ThrowIfFailed(LV_RESULT hr )
{
    if ( FAILED( hr ) )
    {
        throw HrException( hr );
    }
}
#else // _DEBUG

inline void ThrowIfFailed( LV_RESULT hr ) { }
#endif

#if defined(_WIN32) || defined(_WIN64)

#ifdef _DEBUG
// Assign a name to the object to aid with debugging.
inline void SetName( ID3D12Object* pObject, LPCWSTR name )
{
    pObject->SetName( name );
}
inline void SetNameIndexed( ID3D12Object* pObject, LPCWSTR name, UINT index )
{
    WCHAR fullName[ 50 ];
    if ( swprintf_s( fullName, L"%s[%u]", name, index ) > 0 )
    {
        pObject->SetName( fullName );
    }
}
inline void SetNameExtra( ID3D12Object* pObject, LPCWSTR name, char* format, ... )
{
    char extra[ 50 ];
    va_list args;
    va_start( args, format );
    vsprintf_s( extra, format, args );
    va_end( args );


    WCHAR fullName[ 50 ];
    if ( swprintf_s( fullName, L"%s-%hs", name, extra ) > 0 )
    {
        pObject->SetName( fullName );
    }
}
#else	
inline void SetName( ID3D12Object*, LPCWSTR ) { }
inline void SetNameIndexed( ID3D12Object*, LPCWSTR, UINT ) { }
inline void SetNameExtra( ID3D12Object* pObject, LPCWSTR name, char* format, ... ) { }
#endif	//if debug

// Naming helper for ComPtr<T>.
// Assigns the name of the variable as the name of the object.
// The indexed variant will include the index in the name of the object.
#define NAME_D3D12_OBJECT(x) SetName((x).Get(), L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed((x)[n].Get(), L#x, n)
#define NAME_D3D12_OBJECT_WITH_NAME(x, n, ...) SetNameExtra((x).Get(), L#x, n, __VA_ARGS__)
#endif	//if windows


//we don't use the following
/*
inline void GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, UINT pathSize)
{
    if (path == nullptr)
    {
        throw std::exception();
    }

    DWORD size = GetModuleFileName(nullptr, path, pathSize);
    if (size == 0 || size == pathSize)
    {
        // Method failed or path was truncated.
        throw std::exception();
    }

    WCHAR* lastSlash = wcsrchr(path, L'\\');
    if (lastSlash)
    {
        *(lastSlash + 1) = L'\0';
    }
}

#ifdef _DEBUG
inline HRESULT ReadDataFromFile( LPCWSTR filename, byte** data, UINT* size )
{
    using namespace Microsoft::WRL;

    CREATEFILE2_EXTENDED_PARAMETERS extendedParams = {};
    extendedParams.dwSize = sizeof( CREATEFILE2_EXTENDED_PARAMETERS );
    extendedParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    extendedParams.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
    extendedParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
    extendedParams.lpSecurityAttributes = nullptr;
    extendedParams.hTemplateFile = nullptr;

    Wrappers::FileHandle file( CreateFile2( filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, &extendedParams ) );
    if ( file.Get() == INVALID_HANDLE_VALUE )
    {
        throw std::exception();
    }

    FILE_STANDARD_INFO fileInfo = {};
    if ( !GetFileInformationByHandleEx( file.Get(), FileStandardInfo, &fileInfo, sizeof( fileInfo ) ) )
    {
        throw std::exception();
    }

    if ( fileInfo.EndOfFile.HighPart != 0 )
    {
        throw std::exception();
    }

    *data = reinterpret_cast<byte*>( malloc( fileInfo.EndOfFile.LowPart ) );
    *size = fileInfo.EndOfFile.LowPart;

    if ( !ReadFile( file.Get(), *data, fileInfo.EndOfFile.LowPart, nullptr, nullptr ) )
    {
        throw std::exception();
    }

    return S_OK;
}



inline UINT CalculateConstantBufferByteSize( UINT byteSize )
{
    // Constant buffer size is required to be aligned.
    return ( byteSize + ( D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1 ) ) & ~( D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1 );
}

//don't need to compile shader because VS does that for us

#ifdef D3D_COMPILE_STANDARD_FILE_INCLUDE
inline Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
    const std::wstring& filename,
    const D3D_SHADER_MACRO* defines,
    const std::string& entrypoint,
    const std::string& target)
{
    UINT compileFlags = 0;
#if defined(_DEBUG) || defined(DBG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr;

    Microsoft::WRL::ComPtr<ID3DBlob> byteCode = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errors;
    hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

    if (errors != nullptr)
    {
        OutputDebugStringA((char*)errors->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    return byteCode;
}
#endif

// Resets all elements in a ComPtr array.
template<class T>
void ResetComPtrArray( T* comPtrArray )
{
    for ( auto &i : *comPtrArray )
    {
        i.Reset();
    }
}


// Resets all elements in a unique_ptr array.
template<class T>
void ResetUniquePtrArray( T* uniquePtrArray )
{
    for ( auto &i : *uniquePtrArray )
    {
        i.reset();
    }
}
*/