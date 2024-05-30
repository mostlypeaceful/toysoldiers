#ifndef __tXmlSerializeMath__
#define __tXmlSerializeMath__
#include "tXmlSerializer.hpp"

namespace Sig
{

	template<class t>
	void fSerializeXmlObject( tXmlSerializer& s, Math::tVector1<t>& object )
	{
		std::stringstream ss;
		ss << object.x;
		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ) );
	}

	template<class t>
	void fSerializeXmlObject( tXmlSerializer& s, Math::tVector2<t>& object )
	{
		std::stringstream ss;
		ss << object.x << ' ' << object.y;
		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ) );
	}

	template<class t>
	void fSerializeXmlObject( tXmlSerializer& s, Math::tVector3<t>& object )
	{
		std::stringstream ss;
		ss << object.x << ' ' << object.y << ' ' << object.z;
		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ) );
	}

	template<class t>
	void fSerializeXmlObject( tXmlSerializer& s, Math::tVector4<t>& object )
	{
		std::stringstream ss;
		ss << object.x << ' ' << object.y << ' ' << object.z << ' ' << object.w;
		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ) );
	}

	template<class t>
	void fSerializeXmlObject( tXmlSerializer& s, tGrowableArray< Math::tVector1<t> >& object )
	{
		s.fCurRoot( ).fSetAttribute( "count", object.fCount( ) );

		std::stringstream ss;

		for( u32 i = 0; i < object.fCount( ); ++i )
			ss << object[i].x << " ";

		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ), true );
	}

	template<class t>
	void fSerializeXmlObject( tXmlSerializer& s, tGrowableArray< Math::tVector2<t> >& object )
	{
		s.fCurRoot( ).fSetAttribute( "count", object.fCount( ) );

		std::stringstream ss;

		for( u32 i = 0; i < object.fCount( ); ++i )
			ss << object[i].x << ' ' << object[i].y << "  ";

		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ), true );
	}

	template<class t>
	void fSerializeXmlObject( tXmlSerializer& s, tGrowableArray< Math::tVector3<t> >& object )
	{
		s.fCurRoot( ).fSetAttribute( "count", object.fCount( ) );

		std::stringstream ss;

		for( u32 i = 0; i < object.fCount( ); ++i )
			ss << object[i].x << ' ' << object[i].y << ' ' << object[i].z << "  ";

		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ), true );
	}

	template<class t>
	void fSerializeXmlObject( tXmlSerializer& s, tGrowableArray< Math::tVector4<t> >& object )
	{
		s.fCurRoot( ).fSetAttribute( "count", object.fCount( ) );

		std::stringstream ss;

		for( u32 i = 0; i < object.fCount( ); ++i )
			ss << object[i].x << ' ' << object[i].y << ' ' << object[i].z << ' ' << object[i].w << "  ";

		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ), true );
	}

	template<class t>
	void fSerializeXmlObject( tXmlSerializer& s, Math::tSphere<t>& object )
	{
		std::stringstream ss;
		ss << object.mCenter.x << ' ' << object.mCenter.y << ' ' << object.mCenter.z << ' ' << object.mRadius;
		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ) );
	}

	template<class t>
	void fSerializeXmlObject( tXmlSerializer& s, Math::tAabb<t>& object )
	{
		std::stringstream ss;
		ss << object.mMin.x << ' ' << object.mMin.y << ' ' << object.mMin.z << ' ' << object.mMax.x << ' ' << object.mMax.y << ' ' << object.mMax.z;
		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ) );
	}

	template<class t>
	void fSerializeXmlObject( tXmlSerializer& s, Math::tMatrix3<t>& object )
	{
		std::stringstream ss;

		Math::tVector3<t>	x = object.fXAxis( ),
							y = object.fYAxis( ),
							z = object.fZAxis( ),
							t = object.fGetTranslation( );

		ss << x.x << ' ' << x.y << ' ' << x.z << ' ';
		ss << y.x << ' ' << y.y << ' ' << y.z << ' ';
		ss << z.x << ' ' << z.y << ' ' << z.z << ' ';
		ss << t.x << ' ' << t.y << ' ' << t.z;

		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ) );
	}

}


#endif//__tXmlSerializeMath__
