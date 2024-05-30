#ifndef __tSixFxKeyframe__
#define __tSixFxKeyframe__

namespace Sig
{
namespace FX
{

	class tKeyframe : public Rtti::tSerializableBaseClass, public tRefCounter
	{
		declare_null_reflector( );

	public:
		tKeyframe( f32 x )
			: mX( x )
		{
		}

		tKeyframe( )
			: mX( 0.f )
		{
		}
		
		tKeyframe( tKeyframe* rhs )
			: mX( rhs->mX )
		{
		}

		virtual ~tKeyframe( ) {  }

		template<class t>
		const t& fValue( ) const
		{
			static const t gDefault = t( );
			const t* vptr = ( const t* )fValuePtr( Rtti::fGetClassId< t > ( ) );
			return vptr ? *vptr : gDefault;
		}

		template<class t>
		b32 fSetValue( const t& v )
		{
			t* vptr = ( t* )fValuePtr( Rtti::fGetClassId< t > ( ) );
			if( vptr )
			{
				*vptr = v;
				return true;
			}
			return false;
		}

		template<class t>
		b32 fEqual( const t& v ) const
		{
			const t* vptr = ( const t* )fValuePtr( Rtti::fGetClassId< t > ( ) );
			return vptr ? Sig::fEqual( *vptr, v ) : false;
		}

		const f32	fX( ) const { return mX; }
		void		fSetX( const f32 val ) { mX = val; }

		virtual Rtti::tClassId fGetID( ) const { return 0; }

	protected:

		virtual void* fValuePtr( Rtti::tClassId cid ) const { return 0; }
		
	private:

		f32	mX;
	};

	typedef tRefCounterPtr< tKeyframe > tKeyframePtr;


	template<class t>
	class tTemplateKeyframe : public tKeyframe
	{
		define_class_pool_new_delete( tTemplateKeyframe, 256 );
	public:
		tTemplateKeyframe( )
			: tKeyframe( 0.f )
		{
			fZeroValue( mValue );
		}

		tTemplateKeyframe( const f32 x, const t& value )
			: tKeyframe( x ), mValue( value )
		{
		}

		explicit tTemplateKeyframe( tTemplateKeyframe* rhs )
			: tKeyframe( rhs->fX( ) ), mValue( rhs->fValue( ) )
		{
		}
	
		virtual Rtti::tClassId fGetID( ) const { return gCid; }

	protected:

		virtual void* fValuePtr( Rtti::tClassId cid ) const { if( cid == gCid ) return ( void* )&mValue; return 0; }

	private:
		static const Rtti::tClassId gCid;

		t mValue;

		template< typename t >
		void fZeroValue( t& val )
		{
			val = t( 0.f );
		}

		template< >
		void fZeroValue<Math::tQuatf>( Math::tQuatf& val )
		{
			val = Math::tQuatf::cIdentity;
		}
	};

	template<class t>
	const Rtti::tClassId tTemplateKeyframe< t >::gCid = Rtti::fGetClassId< t >( );


	#define define_derived_keyframe( className, t, guid ) \
	class className : public tTemplateKeyframe< t > \
	{ \
		declare_null_reflector( ); \
		implement_rtti_serializable_base_class( className, guid ); \
	public:\
		className( f32 x, const t& value ) : tTemplateKeyframe< t >( x, value ) { }\
		className( ) { }\
	};

	define_derived_keyframe( tFxKeyframeF32, f32, 0xBA30908B );
	define_derived_keyframe( tFxKeyframeV2f, Math::tVec2f, 0x73A6A8 );
	define_derived_keyframe( tFxKeyframeV3f, Math::tVec3f, 0x509B60CD );
	define_derived_keyframe( tFxKeyframeV4f, Math::tVec4f, 0x4FB2F06E );
	define_derived_keyframe( tFxKeyframeQuatf, Math::tQuatf, 0x37CC8829 );

	

}
}

#endif	//__tSixFxKeyframe__

