//------------------------------------------------------------------------------
// \file tDirectResourceLoader.hpp - 11 Aug 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tDirectResourceLoader__
#define __tDirectResourceLoader__
#include "tResourceLoader.hpp"
#include "tAsyncFileReader.hpp"

namespace Sig
{
	///
	/// \class tDirectResourceLoader
	/// \brief Handle the loading of the header information for files want to
	///		load data straight into a VRAM buffer
	class tDirectResourceLoader : public tResourceLoader
	{
		define_dynamic_cast( tDirectResourceLoader, tResourceLoader );

	public:

		typedef tDelegate< void ( const tResource & theResource ) > tOnChildReadsComplete;

	protected:
		enum tLoadState
		{
			cLoadStateOpening,
			cLoadStateHeaderSize,
			cLoadStateHeader,
			cLoadStateFinished,
			cLoadStateChildReads,
			cLoadStateChildReadsFinished,
		};
	protected:
		u32 mHeaderSize;
		u32 mHeaderSizeOffset;
		tLoadState mLoadState;
		tAsyncFileReaderPtr mFileReader;
		tGrowableArray<tAsyncFileReaderPtr> mChildReads;
		tOnChildReadsComplete mChildReadsCompleteCb;


	public:
		tDirectResourceLoader( u32 headerSizeOffset, tResource * resource );
		virtual ~tDirectResourceLoader( ) { } //mFileReader is implicitly released and the file is closed here.

		//------------------------------------------------------------------------------
		// tResourceLoader Overrides
		//------------------------------------------------------------------------------
		virtual void		fInitiate( );
		virtual tLoadResult	fUpdate( );
		virtual s32			fGetLoadStage( );

		
		void fSetChildReadsCompleteCB( tOnChildReadsComplete cb ) { mChildReadsCompleteCb = cb; }
		tAsyncFileReader * fCreateChildReader( );

	protected:

		virtual void		fOnOpenSuccess( );
		virtual void		fOnOpenFailure( );
		virtual void		fOnReadHeaderSizeSuccess( );
		virtual void		fOnReadHeaderSuccess( );
		virtual void		fOnReadFailure( );
		
		virtual void		fCleanupAfterSuccess( );
		virtual void		fCleanupAfterFailure( );

		virtual tAsyncFileReaderPtr fCreateChildReaderInternal( );
	};

}

#endif//__tDirectResourceLoader__
