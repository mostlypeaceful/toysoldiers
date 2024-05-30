#ifndef __tXmlDeserializeMath__
#define __tXmlDeserializeMath__
#include "tXmlDeserializer.hpp"

namespace Sig
{

	template<class t>
	void fSerializeXmlObject( tXmlDeserializer& s, Math::tVector1<t>& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );
		contents >> object.x;
	}

	template<class t>
	void fSerializeXmlObject( tXmlDeserializer& s, Math::tVector2<t>& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );
		contents >> object.x >> object.y;
	}

	template<class t>
	void fSerializeXmlObject( tXmlDeserializer& s, Math::tVector3<t>& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );
		contents >> object.x >> object.y >> object.z;
	}

	template<class t>
	void fSerializeXmlObject( tXmlDeserializer& s, Math::tVector4<t>& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );
		contents >> object.x >> object.y >> object.z >> object.w;
	}

	template<class t>
	void fSerializeXmlObject( tXmlDeserializer& s, tGrowableArray< Math::tVector1<t> >& object )
	{
		u32 count = 0;
		s.fCurRoot( ).fGetAttribute( "count", count );
		object.fSetCount( count );

		std::stringstream ss;
		s.fCurRoot( ).fGetContents( ss );

		for( u32 i = 0; i < object.fCount( ); ++i )
			ss >> object[i].x;
	}

	template<class t>
	void fSerializeXmlObject( tXmlDeserializer& s, tGrowableArray< Math::tVector2<t> >& object )
	{
		u32 count = 0;
		s.fCurRoot( ).fGetAttribute( "count", count );
		object.fSetCount( count );

		std::stringstream ss;
		s.fCurRoot( ).fGetContents( ss );

		for( u32 i = 0; i < object.fCount( ); ++i )
			ss >> object[i].x >> object[i].y;
	}

	template<class t>
	void fSerializeXmlObject( tXmlDeserializer& s, tGrowableArray< Math::tVector3<t> >& object )
	{
		u32 count = 0;
		s.fCurRoot( ).fGetAttribute( "count", count );
		object.fSetCount( count );

		std::stringstream ss;
		s.fCurRoot( ).fGetContents( ss );

		for( u32 i = 0; i < object.fCount( ); ++i )
			ss >> object[i].x >> object[i].y >> object[i].z;
	}

	template<class t>
	void fSerializeXmlObject( tXmlDeserializer& s, tGrowableArray< Math::tVector4<t> >& object )
	{
		u32 count = 0;
		s.fCurRoot( ).fGetAttribute( "count", count );
		object.fSetCount( count );

		std::stringstream ss;
		s.fCurRoot( ).fGetContents( ss );

		for( u32 i = 0; i < object.fCount( ); ++i )
			ss >> object[i].x >> object[i].y >> object[i].z >> object[i].w;
	}

	template<class t>
	void fSerializeXmlObject( tXmlDeserializer& s, Math::tSphere<t>& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );
		contents >> object.mCenter.x >> object.mCenter.y >> object.mCenter.z >> object.mRadius;
	}

	template<class t>
	void fSerializeXmlObject( tXmlDeserializer& s, Math::tAabb<t>& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );
		contents >> object.mMin.x >> object.mMin.y >> object.mMin.z >> object.mMax.x >> object.mMax.y >> object.mMax.z;
	}

	template<class t>
	void fSerializeXmlObject( tXmlDeserializer& s, Math::tMatrix3<t>& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );

		Math::tVector3<t> x,y,z,t;

		contents >> x.x >> x.y >> x.z;
		contents >> y.x >> y.y >> y.z;
		contents >> z.x >> z.y >> z.z;
		contents >> t.x >> t.y >> t.z;

		object.fXAxis( x );
		object.fYAxis( y );
		object.fZAxis( z );
		object.fSetTranslation( t );
	}
}


#endif//__tXmlDeserializeMath__
