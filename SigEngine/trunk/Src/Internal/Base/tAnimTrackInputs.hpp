#ifndef __tAnimInputs__
#define __tAnimInputs__

namespace Sig { namespace Anim
{
	enum tInputType
	{
		cFloat,
		cInt,
		cVec3,
		cVec4,
	};

	enum tInputFlags
	{
		cHorizontal, //for ui
		cVertical,   //for ui
	};

	struct tTrackInputDesc
	{
		u32 mType;
		u32 mCount;
		f32 mMin;
		f32 mMax;
		const char* mName;

		tTrackInputDesc( u32 type = ~0, u32 count = 0, f32 min = 0.f, f32 max = 1.f, const char* name = "NoName" )
			: mType( type )
			, mCount( count )
			, mMin( min )
			, mMax( max )
			, mName( name )
		{ }
	};

	class tTrackInput
	{
	public:
		tTrackInput( )
			: mData( NULL )
			, mType( ~0 )
		{ }

		tTrackInput( const f32& data )			: mData( &data ), mType( cFloat ) { }
		tTrackInput( const u32& data )			: mData( &data ), mType( cInt ) { }
		tTrackInput( const Math::tVec3f& data ) : mData( &data ), mType( cVec3 ) { }
		tTrackInput( const Math::tVec4f& data ) : mData( &data ), mType( cVec4 ) { }

		const u32& fInt( ) const
		{
			sigassert( mType == cInt );
			return *reinterpret_cast<const u32*>( mData );
		}

		const f32& fFloat( ) const
		{
			sigassert( mType == cFloat );
			return *reinterpret_cast<const f32*>( mData );
		}

		const Math::tVec4f& fVec4( ) const
		{
			sigassert( mType == cVec4 );
			return *reinterpret_cast<const Math::tVec4f*>( mData );
		}

		const Math::tVec3f& fVec3( ) const
		{
			sigassert( mType == cVec3 );
			return *reinterpret_cast<const Math::tVec3f*>( mData );
		}

	private:
		const void* mData;
		u32			mType;

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

} }


#endif//__tAnimInputs__

