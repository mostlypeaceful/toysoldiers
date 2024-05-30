//------------------------------------------------------------------------------
// \file tCallStack.hpp - 21 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tCallStack__
#define __tCallStack__

namespace Sig
{
	struct tCallStackData
	{
		static const u32 cMaxDepth = 32;

		tCallStackData( ) : mDepth( 0 ) { }

		u32 mDepth;
		tFixedArray< void *, cMaxDepth > mAddresses;
	};

	///
	/// \class tCallStack
	/// \brief 
	class tCallStack
	{	

	public:

		// Construction captures up to cMaxDepth addresses
		tCallStack( u32 depthToCapture );

		const tCallStackData & fData( ) const { return mData; }

	private:

		void fCaptureCallStackForPlatform( );

	private:

		tCallStackData mData;
	};
}

#endif//__tCallStack__
