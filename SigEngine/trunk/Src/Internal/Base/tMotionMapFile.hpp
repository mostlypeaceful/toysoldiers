#ifndef __tMotionMapFile__
#define __tMotionMapFile__

namespace Sig
{
	// This structure is simply a serialized easy to use c++ version of the graph in Siganim.
	class base_export tMotionMapFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		declare_lip_version( );
		implement_rtti_serializable_base_class( tMotionMapFile, 0xCB91A25F );
	public:
		static const char*	fGetFileExtension( );
		static tFilePathPtr fPathToMomapb( const char* path );
		static tFilePathPtr fPathToMomapb( const tFilePathPtr& path );
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathAddB( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathSubB( path ); }
	public:
		struct tAnimTrackData
		{
			declare_reflector( );

			tLoadInPlaceStringPtr*	mName;
			f32						mTimeScale;

			tAnimTrackData( ) 
				: mName( NULL )
				, mTimeScale( 1.f ) 
			{ }

			tAnimTrackData( tNoOpTag ) 
			{ }
		};

		struct tBlendTrackData
		{
			declare_reflector( );

			tLoadInPlaceStringPtr* mName;	
			f32 mACurve;
			f32 mBCurve;
			f32 mTimeScale;
			f32 mDigitalThresold;

			b8 mDigital;
			b8 mOneShot;

			tBlendTrackData( tLoadInPlaceStringPtr* name, f32 aCurve, f32 bCurve, f32 timeScale, f32 digitalThreshold, b32 digital, b32 oneShot )
				: mName( name )
				, mACurve( aCurve )
				, mBCurve( bCurve )
				, mTimeScale( timeScale )
				, mDigitalThresold( digitalThreshold )
				, mDigital( digital )
				, mOneShot( oneShot )
			{ }

			tBlendTrackData( tNoOpTag ) 
			{ }
		};

		struct tTrack
		{
			declare_reflector( );

			tDynamicArray< tLoadInPlacePtrWrapper< tTrack > > mChildren;		

			// Only one of these pointers will be set if any.
			tAnimTrackData* mAnim;
			tBlendTrackData* mBlend;

			tTrack( ) 
				: mAnim( NULL )
				, mBlend( NULL ) 
			{ }

			tTrack( tNoOpTag ) 
				: mChildren( cNoOpTag )
			{ }	
		};

		//struct tContext
		//{
		//	declare_reflector( );

		//	u32 mKey;

		//	tContext( u32 key = ~0 ) : mKey( key ) { }
		//	tContext( tNoOpTag ) { }	
		//};

		tMotionMapFile( )
			: mRoot( NULL )
			, mNumBlendTracks( 0 )
			, mNumAnimTracks( 0 )
		{ }

		tMotionMapFile( tNoOpTag )
			: tLoadInPlaceFileBase( cNoOpTag )
			//, mContexts( cNoOpTag )
		{ }

		tTrack* mRoot;
		u32 mNumBlendTracks;
		u32 mNumAnimTracks;
		//tDynamicArray< tContext > mContexts;
	};

}

#endif//__tMotionMapFile__
