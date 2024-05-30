#ifndef __GameArchiveUtils__
#define __GameArchiveUtils__

#include "EndianUtil.hpp"

namespace Sig
{
	class tGameArchiveSave;
	class tGameArchiveLoad;

	template<class t, bool isRttiSubClass>
	class tGameArchivePolymorphicSaver
	{
	public:
		static void fSaveRttiClassId( tGameArchiveSave& archive, t& polymorphicObject )
		{
			// in the generic case, we do nothing; this is
			// for types that do not derive from tSerializableBaseClass
		}
	};
	template<class t>
	class tGameArchivePolymorphicSaver<t,true>
	{
	public:
		static void fSaveRttiClassId( tGameArchiveSave& archive, t& polymorphicObject )
		{
			// this is the specialized version for types that
			// derive from tSerializableBaseClass
			const Rtti::tClassId cid = polymorphicObject.fClassId( );
			archive.fSaveLoad( cid );
		}
	};
	template<class t, bool isRttiSubClass>
	class tGameArchivePolymorphicLoader
	{
	public:
		static t* fNewObject( tGameArchiveLoad& archive )
		{
			// in the generic case, we just return a new object of type t;
			// this is for types that do not derive from tSerializableBaseClass
			return NEW t;
		}
	};
	template<class t>
	class tGameArchivePolymorphicLoader<t,true>
	{
	public:
		static t* fNewObject( tGameArchiveLoad& archive )
		{
			// this is the specialized version for types that
			// derive from tSerializableBaseClass

			Rtti::tClassId cid=Rtti::cInvalidClassId;
			archive.fSaveLoad( cid );
			
			if( cid != Rtti::cInvalidClassId )
				return ( t* )Rtti::fNewClass( cid );
			else
				return NULL;
		}
	};
	template<class t, class u>
	void fGameArchiveStandardSave( tGameArchiveSave& archive, tPair<t,u>& object )
	{
		archive.fSaveLoad( object.mA );
		archive.fSaveLoad( object.mB );
	}
	template<class t, class u>
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, tPair<t,u>& object )
	{
		archive.fSaveLoad( object.mA );
		archive.fSaveLoad( object.mB );
	}
	template<class t>
	void fGameArchiveStandardSave( tGameArchiveSave& archive, t& object )
	{
		if( archive.fFailed() )
			return;
		object.fSaveLoad( archive );
	}
	template<class t>
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, t& object )
	{
		if( archive.fFailed() )
			return;
		object.fSaveLoad( archive );
	}
	template<class t>
	void fGameArchiveStandardSave( tGameArchiveSave& archive, tRefCounterPtr<t>& object )
	{
		if( archive.fFailed() )
			return;
		if( object )
		{
			tGameArchivePolymorphicSaver< const t, can_convert( t, Rtti::tSerializableBaseClass ) >::fSaveRttiClassId( archive, *object );
			fGameArchiveStandardSave( archive, *object );
		}
		else
		{
			archive.fSaveLoad( Rtti::cInvalidClassId );
		}
	}
	template<class t>
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, tRefCounterPtr<t>& object )
	{
		if( archive.fFailed() )
			return;
		t* newObj = tGameArchivePolymorphicLoader< t, can_convert( t, Rtti::tSerializableBaseClass ) >::fNewObject( archive );
		object.fReset( newObj );
		if( newObj )
			fGameArchiveStandardLoad( archive, *object );
	}
	template<class t, int N>
	void fGameArchiveStandardSave( tGameArchiveSave& archive, tFixedArray<t, N>& objects )
	{
		if( tIsBuiltInType<t>::cIs ) 
		{
			archive.fPut( objects.fBegin( ), objects.fTotalSizeOf( ) );
		}
		else
		{
			for( u32 i = 0; i < N && !archive.fFailed(); ++i )
				archive.fSaveLoad( objects[ i ] );
		}
	}
	template<class t, int N>
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, tFixedArray<t, N>& objects )
	{
		if( !archive.fSanityCheckArrayAlloc<t>(N) )
			return;
		if( tIsBuiltInType<t>::cIs )
		{
			archive.fGet( objects.fBegin( ), objects.fTotalSizeOf( ) );
			if( archive.fFailed() )
				return;
			EndianUtil::fSwapForTargetPlatform( objects.fBegin( ), sizeof( t ), archive.fSavedPlatform( ), N );
		}
		else
		{
			for( u32 i = 0; i < N && !archive.fFailed(); ++i )
				archive.fSaveLoad( objects[ i ] );
		}
	}
	template<class t>
	void fGameArchiveStandardSave( tGameArchiveSave& archive, tDynamicArray<t>& objects )
	{
		const u32 count = objects.fCount( );
		archive.fSaveLoad( count );
		
		if( tIsBuiltInType<t>::cIs )
		{
			archive.fPut( objects.fBegin( ), objects.fTotalSizeOf( ) );
		}
		else
		{
			for( u32 i = 0; i < count; ++i )
				archive.fSaveLoad( objects[ i ] );
		}
	}
	template<class t>
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, tDynamicArray<t>& objects )
	{
		if( archive.fFailed() )
			return;
		u32 count = 0;
		archive.fSaveLoad( count );
		if( !archive.fSanityCheckArrayAlloc<t>(count) )
			return;
		objects.fNewArray( count );

		if( tIsBuiltInType<t>::cIs )
		{
			archive.fGet( objects.fBegin( ), objects.fTotalSizeOf( ) );
			if( archive.fFailed() )
				return;
			EndianUtil::fSwapForTargetPlatform( objects.fBegin( ), sizeof( t ), archive.fSavedPlatform( ), count );
		}
		else
		{
			for( u32 i = 0; i < count && !archive.fFailed(); ++i )
				archive.fSaveLoad( objects[ i ] );
		}
	}
	template<class t>
	void fGameArchiveStandardSave( tGameArchiveSave& archive, tGrowableArray<t>& objects )
	{
		u32 count = objects.fCount( );
		archive.fSaveLoad( count );

		if( tIsBuiltInType<t>::cIs )
		{
			archive.fPut( objects.fBegin( ), objects.fTotalSizeOf( ) );
		}
		else
		{
			for( u32 i = 0; i < objects.fCount( ); ++i )
				archive.fSaveLoad( objects[ i ] );
		}
	}
	template<class t>
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, tGrowableArray<t>& objects )
	{
		u32 count = 0;
		archive.fSaveLoad( count );
		if( !archive.fSanityCheckArrayAlloc<t>(count) )
			return;
		objects.fSetCount( count );

		if( tIsBuiltInType<t>::cIs )
		{
			archive.fGet( objects.fBegin( ), objects.fTotalSizeOf( ) );
			if( archive.fFailed() )
				return;
			EndianUtil::fSwapForTargetPlatform( objects.fBegin( ), sizeof( t ), archive.fSavedPlatform( ), count );
		}
		else
		{
			for( u32 i = 0; i < objects.fCount( ) && !archive.fFailed(); ++i )
				archive.fSaveLoad( objects[ i ] );
		}
	}

	template<bool isBuiltIn>
	class tGameArchiveSaveLoad
	{
	public:
		template< class t >
		static void fSave( tGameArchiveSave& archive, t& object )
		{
			fGameArchiveStandardSave( archive, object );
		}
		template< class t >
		static void fLoad( tGameArchiveLoad& archive, t& object )
		{
			fGameArchiveStandardLoad( archive, object );
		}
	};
	template<>
	class tGameArchiveSaveLoad<true>
	{
	public:
		template< class t >
		static void fSave( tGameArchiveSave& archive, t& object )
		{
			archive.fPut( &object, sizeof( object ) );
		}
		template< class t >
		static void fLoad( tGameArchiveLoad& archive, t& object )
		{
			archive.fGet( &object, sizeof( object ) );
			if( archive.fFailed() )
				return;
			EndianUtil::fSwapForTargetPlatform( &object, sizeof( object ), archive.fSavedPlatform( ), 1 );
		}
	};
}

#endif//__GameArchiveUtils__
