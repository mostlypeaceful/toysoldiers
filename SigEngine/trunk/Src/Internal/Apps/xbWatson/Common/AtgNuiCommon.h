//--------------------------------------------------------------------------------------
// AtgNuiCommon.h
//
// Common defines and macros for NUI samples 
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once
#ifndef ATG_NUI_COMMON_H
#define ATG_NUI_COMMON_H

#include <NuiApi.h>
#include "AtgUtil.h"

namespace ATG
{
    VOID NuiPrintError( HRESULT hResult, const CHAR* szFunctionName );
};

#endif // ATG_NUI_COMMON_H