#include "BasePch.hpp"
#include "tXmlDeserializer.hpp"

namespace Sig
{

	template<>
	void fSerializeXmlObject<tXmlDeserializer,int>( tXmlDeserializer& s, int& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );
		contents >> object;
	}

	template<>
	void fSerializeXmlObject<tXmlDeserializer,u16>( tXmlDeserializer& s, u16& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );
		contents >> object;
	}

	template<>
	void fSerializeXmlObject<tXmlDeserializer,u32>( tXmlDeserializer& s, u32& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );
		contents >> object;
	}

	template<>
	void fSerializeXmlObject<tXmlDeserializer,u64>( tXmlDeserializer& s, u64& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );
		contents >> object;
	}

	template<>
	void fSerializeXmlObject<tXmlDeserializer,f32>( tXmlDeserializer& s, f32& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );
		contents >> object;
	}

	template<>
	void fSerializeXmlObject<tXmlDeserializer,f64>( tXmlDeserializer& s, f64& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );
		contents >> object;
	}

	template<>
	void fSerializeXmlObject<tXmlDeserializer,std::string>( tXmlDeserializer& s, std::string& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );
		object = contents.str( );
	}

	template<>
	void fSerializeXmlObject<tXmlDeserializer,tStringPtr>( tXmlDeserializer& s, tStringPtr& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );
		object = tStringPtr( contents.str( ).c_str( ) );
	}

	template<>
	void fSerializeXmlObject<tXmlDeserializer,tFilePathPtr>( tXmlDeserializer& s, tFilePathPtr& object )
	{
		std::stringstream contents;
		s.fCurRoot( ).fGetContents( contents );
		object = tFilePathPtr( contents.str( ).c_str( ) );
	}

}

