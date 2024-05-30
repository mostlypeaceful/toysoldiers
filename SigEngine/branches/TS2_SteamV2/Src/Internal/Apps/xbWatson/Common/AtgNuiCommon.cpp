//--------------------------------------------------------------------------------------
// AtgNuiCommon.cpp
//
// Common defines and macros for NUI samples 
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "stdafx.h"
#include "AtgNuiCommon.h"

namespace ATG
{
    //--------------------------------------------------------------------------------------
    // Outputs verbose error message from NUI errors
    //--------------------------------------------------------------------------------------
    VOID NuiPrintError( HRESULT hResult, const CHAR* szFunctionName )
    {
        switch (hResult)
        {
        case E_INVALIDARG:
            ATG_PrintError( "%s failed with E_INVALIDARG\n", szFunctionName );
            break;

        case E_NUI_ALREADY_INITIALIZED:
            ATG_PrintError( "%s failed with E_NUI_ALREADY_INITIALIZED\n", szFunctionName );
            break;

        case E_NUI_DATABASE_NOT_FOUND:
            ATG_PrintError( "%s failed with E_NUI_DATABASE_NOT_FOUND\n", szFunctionName );
            break;

        case E_NUI_DATABASE_VERSION_MISMATCH:
            ATG_PrintError( "%s failed with E_NUI_DATABASE_VERSION_MISMATCH\n", szFunctionName );
            break;

        case E_OUTOFMEMORY:
            ATG_PrintError( "%s failed with E_OUTOFMEMORY\n", szFunctionName );
            break;

        case E_NUI_DEVICE_NOT_READY:
            ATG_PrintError( "%s failed with E_NUI_DEVICE_NOT_READY\n", szFunctionName );
            break;

        case E_NUI_DEVICE_NOT_CONNECTED:
            ATG_PrintError( "%s failed with E_NUI_DEVICE_NOT_CONNECTED\n", szFunctionName );
            break;

        case E_NUI_FEATURE_NOT_INITIALIZED:
            ATG_PrintError( "%s failed with E_NUI_FEATURE_NOT_INITIALIZED\n", szFunctionName );
            break;

        case E_NUI_IMAGE_STREAM_IN_USE:
            ATG_PrintError( "%s failed with E_NUI_IMAGE_STREAM_IN_USE\n", szFunctionName );
            break;

        default:
            ATG_PrintError( "%s failed with 0x%x\n", szFunctionName, (UINT)hResult );
        }
    }

};


