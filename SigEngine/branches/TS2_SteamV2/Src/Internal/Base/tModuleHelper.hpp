//------------------------------------------------------------------------------
// \file tModuleHelper.hpp - 31 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tModuleHelper__
#define __tModuleHelper__

namespace Sig
{
	///
	/// \class tSymbolsSignature
	/// \brief 
	struct tSymbolsSignature
	{
		tSymbolsSignature( ) : mAge( 0 ) { fZeroOut( mGuid ); }

		struct { u32 D1; u16 D2, D3; tFixedArray< u8, 8 > D4; } mGuid;
		u32 mAge;
		tFilePathPtr mPath;
	};

	///
	/// \class tModule
	/// \brief 
	struct tModule
	{
		tModule( ) : mBaseAddress( 0 ), mSize( 0 ), mTimeStamp( 0 ) { }
		std::string			mName;
		u64					mBaseAddress;
		u32					mSize;
		u32					mTimeStamp;
		tSymbolsSignature	mSymbolsSignature;
	};

	///
	/// \class tModuleHelper
	/// \brief Captures the currently loaded set of modules
	class base_export tModuleHelper
	{
	public:

		tModuleHelper( b32 refresh = true );

		void fRefreshModules( );

		void fSave( tGrowableArray< byte > & buffer );
		const byte * fLoad( const byte * data, u32 dataLength );

		u32	fModuleCount( ) const { return mModules.fCount( ); }
		const tModule & fModule( u32 m ) const { return mModules[ m ]; }

	private:

		tDynamicArray< tModule > mModules;
	};
}

#endif//__tModuleHelper__
