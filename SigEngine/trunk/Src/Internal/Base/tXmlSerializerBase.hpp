#ifndef __tXmlSerializerBase__
#define __tXmlSerializerBase__
#include "tXmlFile.hpp"

namespace Sig
{
	class tXmlSerializer;
	class tXmlDeserializer;

	///
	/// \brief Default serialization function for objects. 
	///
	/// Can be specialized for "library" types, so that the 
	/// type doesn't have to define the fSerializeXml method.
	template<class tSerializer, class t>
	void fSerializeXmlObject( tSerializer& s, t& object )
	{
		object.fSerializeXml( s );
	}

	class base_export tXmlSerializerBase
	{
	protected:

		tXmlFileData	mXmlFile;
		tXmlNode	mCurRoot;

	public:

		virtual ~tXmlSerializerBase( ) { }

		virtual b32 fIn( ) const = 0; // returns true if we're a deserializer/loader (in)
		virtual b32 fOut( ) const = 0; // returns true if we're a serializer/saver (out)

		inline tXmlNode& fCurRoot( ) { return mCurRoot; }
	};

}


#endif//__tXmlSerializerBase__
