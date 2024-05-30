//------------------------------------------------------------------------------
// \file tSync.h - 09 Nov 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tSync__
#define __tSync__
#include "tGameArchive.hpp"
#include "Threads\tCriticalSection.hpp"

#if defined( target_game )
	#define sync_system_enabled
	#if defined ( build_release ) || defined( build_playtest ) || defined( build_profile )
		#define sync_system_detect_only
	#endif
#endif

namespace Sig
{
	struct tCallStackData;

	///
	/// \class tSync
	/// \brief 
	class base_export tSync
	{
	public:

		struct tFrame
		{
			tFrame( ) : mHash( 0 ) { }

			u32 mHash;

#ifndef sync_system_detect_only
			tDynamicArray<byte> mBuffer;
			tDynamicArray<byte> mCallstackBuffer;
#endif
		};

		enum tSyncCategory
		{
			cSCUnspecified = 0,
			cSCRandom =			( 1 << 0 ),
			cSCInput =			( 1 << 1 ),
			cSCSceneGraph =		( 1 << 2 ),
			cSCPhysics =		( 1 << 3 ),
			cSCAI =				( 1 << 4 ),
			cSCPlayer =			( 1 << 5 ),
			cSCNPC =			( 1 << 6 ),
			cSCUnit =			( 1 << 7 ),
			cSCVehicle =		( 1 << 8 ),
			cSCProjectile =		( 1 << 9 ),
			cSCProximity =		( 1 << 10 ),
			cSCLogic =			( 1 << 11 ),
			cSCStats =			( 1 << 12 ),
			cSCLevel =			( 1 << 13 ),
			cSCUser =			( 1 << 14 ),
			cSCDebris =			( 1 << 15 ),
			cSCCamera =			( 1 << 16 ),
			cSCRaycast =		( 1 << 17 ),
			cSCThread =			( 1 << 18 ),
			cSCSpatial =		( 1 << 19 ),
			cSCParticles =		( 1 << 20 ),
		};

	public:

		declare_global_object( tSync );

		static b32 fFramesEqual( 
			const tFrame & a, 
			const tFrame & b, 
			std::string & unequalInfo, 
			const tFilePathPtr & desyncFolder = tFilePathPtr( )  );
		static b32 fFilesEqual( 
			const tFilePathPtr & file1Path, 
			const tFilePathPtr & file2Path, 
			u32 prependCount = 5,
			u32 appendCount = 5 );

		static b32 fSyncEnabled( );

		void fInitialize( 
			const char * title, // Title of the application
			u32 buildId			// Build id of the application
		);

		void fRegisterThread( b32 mainThread = false );
		void fDeregisterThread( );

		u32 fBuildId( ) const { return mBuildId; }
		const std::string & fTitle( ) const { return mTitle; }

		void fStartSync( const tFilePathPtr & logPath, const tFilePathPtr & compareTo );
		void fEndSync( );

		void fSetAllowedCategories( u32 allowedCategoriesMask ) { mAllowedCategories = allowedCategoriesMask; }

		b32 fMidSync( ) const { return mRunning; }

		void fMarkFrame( tFrame * frame = NULL, b32 verboseFrame = false );

		// Log functions
		#define declare_synclog( t ) void fLog( const t & value, const char * context, const char * file, const char * function, u32 line, u32 category )

		declare_synclog( u32 );
		declare_synclog( s32 );
		declare_synclog( u64 );
		declare_synclog( s64 );
		declare_synclog( f32 );
		declare_synclog( Math::tVec2u );
		declare_synclog( Math::tVec3u );
		declare_synclog( Math::tVec4u );
		declare_synclog( Math::tVec2f );
		declare_synclog( Math::tVec3f );
		declare_synclog( Math::tVec4f );
		declare_synclog( Math::tMat3f );
		declare_synclog( Math::tMat4f );
		declare_synclog( Math::tQuatf );
		declare_synclog( Math::tAabbf );

		#undef declare_synclog

		template<class t>
		const t & fLogWithReturn( const t & value, const char * context, const char * file, const char * function, u32 line, u32 category )
		{
			#ifdef sync_system_enabled
				fLog( value, context, file, function, line, category );
			#endif
			return value;
		}

#ifndef sync_system_detect_only

		///
		/// \class tSyncStrPtrStr
		/// \brief 
		class tSyncStrPtrStr : public tSharedStringBase
		{
			define_class_pool_new_delete(tSyncStrPtrStr,256);
		public:
			static tSharedStringTable& fAccessTable( );
			static tSharedStringBase* fNewInstance( const char* str ) { return NEW tSyncStrPtrStr( str ); }
			explicit tSyncStrPtrStr( const char* str ) : tSharedStringBase( str ) { }
			~tSyncStrPtrStr( ) { fOnDestroy( fAccessTable( ) ); }
		};

		typedef tRefCounterPtr< tSyncStrPtrStr > tSyncStrPtrStrPtr;

		///
		/// \class tSyncStrPtr
		/// \brief 
		class base_export tSyncStrPtr
		{
			sig_make_loggable( tSyncStrPtr, ( fCStr( ) ? fCStr( ) : "(null)" ) );
		private:
			static Threads::tCriticalSection mCritSect;
			tSyncStrPtrStrPtr mStringRep;

		public:

			inline tSyncStrPtr( ) { }

			explicit tSyncStrPtr( const char* str );
			explicit tSyncStrPtr( const std::string& str );

			~tSyncStrPtr( );
			tSyncStrPtr(const tSyncStrPtr& other);
			tSyncStrPtr& operator=(const tSyncStrPtr& other);

			inline const char*	fCStr( ) const		{ return mStringRep.fNull( ) ? "" : mStringRep->fCStr( ); }
			inline u32			fLength( ) const	{ return mStringRep.fNull( ) ? 0  : mStringRep->fLength( ); }
			inline b32			fNull( ) const		{ return mStringRep.fNull( ); }
			inline b32			fExists( ) const	{ return mStringRep.fNull( ) ? false : mStringRep->fExists( ); }

			inline tHashTablePtrInt fGetHashValue( ) const { return ( tHashTablePtrInt )( size_t )mStringRep.fGetRawPtr( ); }	

			#define overload_comparison_operator( opType ) \
				inline b32 operator opType( const tSyncStrPtr& other ) const { return mStringRep opType other.mStringRep; }
				overload_comparison_operator( == );
				overload_comparison_operator( != );
				overload_comparison_operator( < );
				overload_comparison_operator( <= );
				overload_comparison_operator( > );
				overload_comparison_operator( >= );
			#undef overload_comparison_operator

			static const tSyncStrPtr cNullPtr;

		};

#endif // sync_system_detect_only

	private:

#ifndef sync_system_detect_only

		///
		/// \class tSyncEventData
		/// \brief 
		struct tSyncEventData
		{
			u32 mType;
			u32 mLine;
			u32 mContextId;
			u32 mFileId;
			u32 mFunctionId;
			u32 mDataId; // ( pageId << 16 ) | dataOffset, or value
		};

		///
		/// \class tSyncEvent
		/// \brief 
		struct tSyncEvent : public tSyncEventData
		{
			static const u32 cCallStackDepth = 32;

			inline tSyncEvent( ) { mCallStack[ 0 ] = NULL; }

			tFixedArray< void *, cCallStackDepth > mCallStack;

			b32 operator==( const tSyncEvent & e ) const;
			b32 operator!=( const tSyncEvent & e ) const;
		};

		///
		/// \class tFrameHeader
		/// \brief 
		class tFrameHeader : public tRefCounter
		{
			static const u32 cNullStringId = ~0;

		public:

			template< class t >
			void fPushEvent( 
				const t & value, 
				const char * context, 
				const char * file, 
				const char * function,
				u32 line );

			void fWrite( tGrowableArray< byte > & out ) const;
			const byte * fRead( tPlatformId source, const byte * data, u32 dataLen  );

			void fWriteStacks( tGrowableArray< byte > & out ) const;
			const byte * fReadStacks( tPlatformId source, const byte * data, u32 dataLen );

			void fReset( );

			std::string fInfo( tPlatformId source, const tSyncEvent & e ) const;

			u32  fEventCount( ) const { return mEvents.fCount( ); }
			const tSyncEvent & fEvent( u32 idx ) const { return mEvents[ idx ]; }

			const tSyncStrPtr & fContext( const tSyncEvent & e ) const
			{ 
				return e.mContextId == cNullStringId ? tSyncStrPtr::cNullPtr : mContexts[ e.mContextId ]; 
			}

			const tSyncStrPtr & fFile( const tSyncEvent & e ) const
			{
				return e.mFileId == cNullStringId ? tSyncStrPtr::cNullPtr : mFiles[ e.mFileId ];
			}

			const tSyncStrPtr & fFunction( const tSyncEvent & e ) const
			{
				return e.mFunctionId == cNullStringId ? tSyncStrPtr::cNullPtr : mFunctions[ e.mFunctionId ];
			}

			u32 fDataSize( const tSyncEvent & e ) const;
			const byte * fData( const tSyncEvent & e ) const;

		private:

			struct tDataPage : public tRefCounter
			{
				static const u32 cMaxDataSize = 4096;

				tDataPage( ) : mFilled( 0 ) { }

				u32 fSpaceLeft( ) const { return mData.fCount( ) - mFilled; }
				void fWrite( const byte * data, u32 size ) 
				{ 
					sigassert( size <= fSpaceLeft( ) );
					fMemCpy( mData.fBegin( ) + mFilled, data, size );
					mFilled += size;
				}

				u32 mFilled;
				tFixedArray<byte, cMaxDataSize> mData;
			};

			typedef tRefCounterPtr< tDataPage > tDataPagePtr;
			typedef tGrowableArray< tSyncStrPtr > tStrTable;

		private:

			u32 fFindOrAdd( const char * str, tStrTable & table );
			u32 fPushData( const byte * data, u32 size );

		private:

			tGrowableArray< tSyncStrPtr >	mContexts;
			tGrowableArray< tSyncStrPtr >	mFiles;
			tGrowableArray< tSyncStrPtr >	mFunctions;
			tGrowableArray< tDataPagePtr >	mDataPages;
			tGrowableArray< tSyncEvent >	mEvents;

			tGrowableArray< tDataPagePtr >  mUnusedPages;
		};

		typedef tRefCounterPtr< tFrameHeader > tFrameHeaderPtr;

		///
		/// \class tSyncReader
		/// \brief For reading and analyzing syncs
		class tSyncReader
		{
		public:
			
			tSyncReader( );
			~tSyncReader( );

			const std::string & fTitle( ) const { return mTitle; }
			u32 fBuildId( ) const { return mBuildId; }
			
			b32 fOpen( const tFilePathPtr & path );
			void fOpen( const byte * data, u32 dataLen, const byte * csData, u32 csDataLen );
			b32 fIsOpen( ) const;

			void fClose( );

			b32 fEventsAreEqual( const tSyncReader & other ) const;

			const tSyncEvent & fEvent( ) const { return mEvent; }
			std::string fEventInfo( ) const { return mFrameHeaders[ mHeaderIdx ].fInfo( mPlatformId, mEvent ); }

			b32 fAdvance( );

		private:

			void fReadFrame( const byte * data, u32 dataLen, const byte * csData, u32 csDataLen );

		private:

			std::string mTitle;
			u32 mBuildId;
			tPlatformId mPlatformId;

			u32 mHeaderIdx;
			u32 mEventIdx;
			tSyncEvent mEvent;

			tGrowableArray< tFrameHeader > mFrameHeaders;
			tGrowableArray< byte > mBuffer;
			class tFileReader * mReader;
		};

		///
		/// \class tSyncWriter
		/// \brief For writing and creating syncs
		class tSyncWriter
		{
		public:

			tSyncWriter( );
			~tSyncWriter( );

			b32 fHasData( ) const { return mBuffer.fCount( ) && *(u32*)mBuffer.fBegin( ); }
			const tGrowableArray<byte> & fBuffer( ) const { return mBuffer; }

			b32 fOpen( const tFilePathPtr & path, const std::string & title, u32 buildId );
			void fClose( );

			void fWrite( const tFrameHeader & header );
			void fFlush( );

		private:

			tGrowableArray<byte> mBuffer;
			class tFileWriter * mWriter;
		};
#else
		struct tFrameHeader : public tRefCounter
		{
			inline tFrameHeader( ) : mHash( 0 ) { }
			inline void fReset( ) { mHash = 0; }

			template< class t >
			void fPushEvent( 
				const t & value, 
				const char * context, 
				const char * file, 
				const char * function,
				u32 line );

			u32 mHash;
		};

		typedef tRefCounterPtr< tFrameHeader > tFrameHeaderPtr;

#endif // sync_system_detect_only

	private:

		tSync( );
		~tSync( );

#ifndef sync_system_detect_only

		static void fDecompress( 
			const byte * data, u32 dataLength, tDynamicBuffer & decomped );
		static void fSaveDesync( 
			const tFilePathPtr & desyncFolder, 
			const tSyncReader & reader, u64 stamp, u32 idx );
#endif

	private:

		u32			mBuildId;
		std::string	mTitle;

		b32 mRunning;
		u32 mAllowedCategories;

		u32 mFrameHeaderTLS;
		Threads::tCriticalSection mWriteFramesSection;

		tGrowableArray< tFrameHeaderPtr > mWriteFrames;

#ifndef sync_system_detect_only

		tSyncWriter mWriter;
		tSyncReader mReader;

#endif
		
	};

	//------------------------------------------------------------------------------
	// Helper macros for logging synced events
	//------------------------------------------------------------------------------
#ifdef sync_system_enabled

	#define sync_enabled( ) ::Sig::tSync::fSyncEnabled( )
	#define sync_init( title, build ) ::Sig::tSync::fInstance( ).fInitialize( title, build )
	#define sync_register_thread( isMain ) ::Sig::tSync::fInstance( ).fRegisterThread( isMain )
	#define sync_deregister_thread( ) ::Sig::tSync::fInstance( ).fDeregisterThread( )
	#define sync_start( logPath, comparePath ) ::Sig::tSync::fInstance( ).fStartSync( logPath, comparePath )
	#define sync_log_start( path ) sync_start( path, tFilePathPtr( ) )
	#define sync_compare_start( path ) sync_start( tFilePathPtr( ), path )
	#define sync_end( ) ::Sig::tSync::fInstance( ).fEndSync( )
	#define sync_allowed_categories( catmask ) ::Sig::tSync::fInstance( ).fSetAllowedCategories( catmask )
	#define sync_in_progress( ) ::Sig::tSync::fInstance( ).fMidSync( )
	#define sync_frame( ) ::Sig::tSync::fInstance( ).fMarkFrame( )
	#define sync_frame_f( framePtr, verbose ) ::Sig::tSync::fInstance( ).fMarkFrame( framePtr, verbose )

	#define sync_d_event( context, value ) ::Sig::tSync::fInstance( ).fLog( ( value ), ( context ), __FILE__, __FUNCTION__, __LINE__, tSync::cSCUnspecified )
	#define sync_d_event_v( value ) ::Sig::tSync::fInstance( ).fLog( ( value ), #value, __FILE__, __FUNCTION__, __LINE__, tSync::cSCUnspecified )
	#define sync_d_event_r( context, value ) ::Sig::tSync::fInstance( ).fLogWithReturn( ( value ), ( context ), __FILE__, __FUNCTION__, __LINE__, tSync::cSCUnspecified )
	#define sync_d_line( ) ::Sig::tSync::fInstance( ).fLog( 0u, NULL, __FILE__, __FUNCTION__, __LINE__, tSync::cSCUnspecified )
	#define sync_d_event_c( context, value, cat ) ::Sig::tSync::fInstance( ).fLog( ( value ), ( context ), __FILE__, __FUNCTION__, __LINE__, cat )
	#define sync_d_event_v_c( value, cat ) ::Sig::tSync::fInstance( ).fLog( ( value ), #value, __FILE__, __FUNCTION__, __LINE__, cat )
	#define sync_d_event_r_c( context, value, cat ) ::Sig::tSync::fInstance( ).fLogWithReturn( ( value ), ( context ), __FILE__, __FUNCTION__, __LINE__, cat )
	#define sync_d_line_c( cat ) ::Sig::tSync::fInstance( ).fLog( 0u, NULL, __FILE__, __FUNCTION__, __LINE__, cat )


	#ifndef sync_system_detect_only

		#define sync_event( context, value ) ::Sig::tSync::fInstance( ).fLog( ( value ), ( context ), __FILE__, __FUNCTION__, __LINE__, tSync::cSCUnspecified )
		#define sync_event_v( value ) ::Sig::tSync::fInstance( ).fLog( ( value ), #value, __FILE__, __FUNCTION__, __LINE__, tSync::cSCUnspecified )
		#define sync_event_r( context, value ) ::Sig::tSync::fInstance( ).fLogWithReturn( ( value ), ( context ), __FILE__, __FUNCTION__, __LINE__, tSync::cSCUnspecified )
		#define sync_line( ) ::Sig::tSync::fInstance( ).fLog( 0u, NULL, __FILE__, __FUNCTION__, __LINE__, tSync::cSCUnspecified )
		#define sync_event_c( context, value, cat ) ::Sig::tSync::fInstance( ).fLog( ( value ), ( context ), __FILE__, __FUNCTION__, __LINE__, cat )
		#define sync_event_v_c( value, cat ) ::Sig::tSync::fInstance( ).fLog( ( value ), #value, __FILE__, __FUNCTION__, __LINE__, cat )
		#define sync_event_r_c( context, value, cat ) ::Sig::tSync::fInstance( ).fLogWithReturn( ( value ), ( context ), __FILE__, __FUNCTION__, __LINE__, cat )
		#define sync_line_c( cat ) ::Sig::tSync::fInstance( ).fLog( 0u, NULL, __FILE__, __FUNCTION__, __LINE__, cat )

	#else
		
		#define sync_event( context, value ) (void)0
		#define sync_event_v( value ) (void)0
		#define sync_event_r( context, value ) ( value )
		#define sync_line( ) (void)0
		#define sync_event_c( context, value, cat ) (void)0
		#define sync_event_v_c( value, cat ) (void)0
		#define sync_event_r_c( context, value, cat ) ( value )
		#define sync_line_c( cat ) (void)0

	#endif

#else

	#define sync_enabled( ) ( false )
	#define sync_init( title, build ) (void)0
	#define sync_register_thread( isMain ) (void)0
	#define sync_deregister_thread( ) (void)0
	#define sync_start( logPath, comparePath ) (void)0
	#define sync_log_start( path ) (void)0
	#define sync_compare_start( path ) (void)0
	#define sync_end( ) (void)0
	#define sync_allowed_categories( catmask ) (void)0
	#define sync_in_progress( ) ( false )
	#define sync_frame( ) (void)0
	#define sync_frame_f( framePtr, verbose ) (void)0
	
	#define sync_event( context, value ) (void)0
	#define sync_event_v( value ) (void)0
	#define sync_event_r( context, value ) ( value )
	#define sync_line( ) (void)0
	#define sync_event_c( context, value, cat ) (void)0
	#define sync_event_v_c( value, cat ) (void)0
	#define sync_event_r_c( context, value, cat ) ( value )
	#define sync_line_c( cat ) (void)0

	#define sync_d_event( context, value ) (void)0
	#define sync_d_event_v( value ) (void)0
	#define sync_d_event_r( context, value ) ( value )
	#define sync_d_line( ) (void)0
	#define sync_d_event_c( context, value, cat ) (void)0
	#define sync_d_event_v_c( value, cat ) (void)0
	#define sync_d_event_r_c( context, value, cat ) ( value )
	#define sync_d_line_c( cat ) (void)0

#endif // sync_system_enabled
}

#endif//__tSync__
