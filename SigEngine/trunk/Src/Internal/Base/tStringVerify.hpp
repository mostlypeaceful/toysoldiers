//------------------------------------------------------------------------------
// \file tStringVerify.hpp - 12 Dec 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tStringVerify__
#define __tStringVerify__
#include "XtlUtil.hpp"

namespace Sig
{
	class tLocalizedString;

	///
	/// \class tStringVerifyOpBase
	/// \brief 
	class tStringVerifyOpBase : public tRefCounter
	{
	public:

		static const u32 cMaxStringCount;

	public:

		virtual ~tStringVerifyOpBase( ) { }

		u32 fTextCount( ) const { return mStrings.fCount( ); }
		const wchar_t* fText( u32 index );
		u32 fTextLength( u32 index );

		void fAddString( const tLocalizedString& text );
		void fAddString( const wchar_t text[], u32 length = ~0 );

		virtual b32 fBeginVerify( ) = 0;
		virtual b32 fIsComplete( ) const = 0;
		virtual b32 fIsTextValid( u32 index ) const = 0;

	protected:

		tGrowableArray< u32 > mStrings;
		tGrowableArray< wchar_t > mStorage;
	};

#if defined( platform_xbox360 )

	///
	/// \class tStringVerifyOp
	/// \brief 
	class tStringVerifyOp : public tStringVerifyOpBase, protected XtlUtil::tOverlappedOp
	{
	public:

		virtual b32 fBeginVerify( );
		virtual b32 fIsComplete( ) const;
		virtual b32 fIsTextValid( u32 index ) const;

	private:

		tDynamicArray<STRING_DATA> mInputs;
		tDynamicBuffer mResults;
	};
	
#else

	///
	/// \class tStringVerifyOp
	/// \brief 
	class tStringVerifyOp : public tStringVerifyOpBase
	{
	public:

		virtual b32 fBeginVerify( ) { return false; }
		virtual b32 fIsComplete( ) const { return true; }
		virtual b32 fIsTextValid( u32 index ) const { return true; }
	};

#endif

	typedef tRefCounterPtr< tStringVerifyOp > tStringVerifyOpPtr;

} // ::Sig


#endif//__tStringVerify__
