#ifndef __tAniMapFile__
#define __tAniMapFile__

#include "tLoadInPlaceFileBase.hpp"

namespace Sig
{

	class base_export tAniMapFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		declare_lip_version( );
		implement_rtti_serializable_base_class( tAniMapFile, 0xE3DDD58D );
	public:
		static const char*	fGetFileExtension( );
		static tFilePathPtr fPathToAnimab( const char* path );
		static tFilePathPtr fPathToAnimab( const tFilePathPtr& path );
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathAddB( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathSubB( path ); }

	public:
		tAniMapFile( ) 
		{ }

		tAniMapFile( tNoOpTag ) 
			: tLoadInPlaceFileBase( cNoOpTag )
			, mMappings( cNoOpTag )
			, mContexts( cNoOpTag )
		{ }

	public:
		struct tAnimRef
		{
			declare_reflector( );
			tLoadInPlaceResourcePtr*	mAnimPack;
			tLoadInPlaceStringPtr*		mAnimName;

			f32 mTimeScale;

			tAnimRef( )
			{ }

			tAnimRef( tLoadInPlaceResourcePtr* animPack, tLoadInPlaceStringPtr* animName, f32 timeScale ) 
				: mAnimPack( animPack )
				, mAnimName( animName ) 
				, mTimeScale( timeScale )
			{ }

			tAnimRef( tNoOpTag ) 
			{ }
		};

		// These are references to an enum in GameFlags. The enum values will be used as the context switch values.
		struct tContextRef
		{
			declare_reflector( );
			u32 mEnumTypeValue; // These will need to be rebuild when the index changes in GameFlags.

			tContextRef( u32 enumTypeValue = ~0 ) : mEnumTypeValue( enumTypeValue ) { }
			tContextRef( tNoOpTag ) { }

			b32 operator == ( const tContextRef& other ) const { return mEnumTypeValue == other.mEnumTypeValue; }
		};

		struct tContextSwitch
		{
			declare_reflector( );
			u32 mIndex;		// This index is into an array of context values, one per tContextRef. If this context switch is used to represent a context value. it will be the enum value (from GameFlags).
			tDynamicArray< tLoadInPlacePtrWrapper< tContextSwitch > >	mBranches;
			tDynamicArray< tLoadInPlacePtrWrapper< tAnimRef > >			mLeaves;

			tContextSwitch( u32 index = ~0 ) : mIndex( index ) { }
			tContextSwitch( tNoOpTag ) : mBranches( cNoOpTag ), mLeaves( cNoOpTag ) { }
		};

		struct tMapping
		{
			declare_reflector( );
			tLoadInPlaceStringPtr*	mName; // name of anim track input from momap.
			tContextSwitch*			mRoot;
			tDynamicArray<u32>		mContextsIndexed; // indexes into context array for those relevant to this tree.

			tMapping( ) : mName( NULL ), mRoot( NULL ) { }
			tMapping( tNoOpTag ) : mContextsIndexed( cNoOpTag ) { }
		};

		tDynamicArray< tMapping > mMappings;
		tDynamicArray< tContextRef > mContexts;

	public:
		const tMapping* fFind( const tStringPtr& name ) const
		{
			for( u32 i = 0; i < mMappings.fCount( ); ++i )
			{
				if( mMappings[ i ].mName->fGetStringPtr( ) == name )
					return &mMappings[ i ];
			}

			return NULL;
		}
	};

}

#endif//__tAniMapFile__
