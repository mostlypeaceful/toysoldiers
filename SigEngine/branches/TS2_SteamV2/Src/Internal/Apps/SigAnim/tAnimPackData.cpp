//------------------------------------------------------------------------------
// \file tAnimPackData.cpp - 26 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigAnimPch.hpp"
#include "tAnimPackData.hpp"
#include "FileSystem.hpp"

namespace Sig
{
	tAnimPackData::tAnimPackData( const tResourcePtr& resource )
		: mResource( resource ), mDirty( false )
	{
		mAnipkPath = Anipk::fAnibPathToAnipk( mResource->fGetPath( ) );

		mLongLabel = StringUtil::fTrimOversizePath( mAnipkPath.fCStr( ), 48, mAnipkPath.fLength( ) );

		mLabelWithSize = StringUtil::fTrimOversizePath( mAnipkPath.fCStr( ), 22, mAnipkPath.fLength( ) );
		wxString label;
		label.Printf( "%s ~ %.1fmb", mLabelWithSize.c_str( ), 
			FileSystem::fGetFileSize( ToolsPaths::fMakeGameAbsolute( mResource->fGetPath( ) ) )/(1024.f*1024.f) );
		mLabelWithSize = label.c_str( );

		mAnipkFile.fLoadXml( ToolsPaths::fMakeResAbsolute( mAnipkPath ) );
	}
}
