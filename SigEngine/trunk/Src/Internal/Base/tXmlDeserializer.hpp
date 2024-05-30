#ifndef __tXmlDeserializer__
#define __tXmlDeserializer__
#include "tXmlSerializerBase.hpp"

namespace Sig
{

	///
	/// \brief Deserialize an object from an xml file that had been 
	/// serialized using tXmlSerializer.
	class base_export tXmlDeserializer : public tXmlSerializerBase
	{
	public:

		virtual b32 fIn( ) const { return true; }
		virtual b32 fOut( ) const { return false; }

		///
		/// \brief Deserialize an object using a prespecified xml root
		template<class t>
		b32 fLoad( const char * rootName, t & object )
		{
			// Assumes mCurRoot set externally
			std::string name;
			mCurRoot.fGetName( name );
			if( StringUtil::fStricmp( name.c_str( ), rootName ) == 0 )
			{
				fSerializeXmlObject( *this, object );
				return true;
			}

			return false;
		}

		///
		/// \brief Deserialize an object and all its sub-objects from an xml file on disk
		template<class t>
		b32 fLoad( const tFilePathPtr& path, const char* rootName, t& object )
		{
			if( !mXmlFile.fLoad( path ) )
				return false;

			mCurRoot = mXmlFile.fGetRoot( );
			return fLoad( rootName, object );
		}

		///
		/// \brief Deserialize an object and all its sub-objects from an xml file from buffer
		template<class t>
		b32 fParse( const char* data, const char* rootName, t& object )
		{
			if( !mXmlFile.fParse( data ) )
				return false;

			mCurRoot = mXmlFile.fGetRoot( );
			return fLoad( rootName, object );
		}

		

		///
		/// \brief Deserialize data from an attribute on the current root node.
		template<class t>
		void fAsAttribute( const char* name, t& object )
		{
			sigassert( !mCurRoot.fNull( ) );
			mCurRoot.fGetAttribute( name, object );
		}

		///
		/// \brief General form for deserializing an object.
		///
		/// This version assumes the object type passed has a
		/// templatized method called fSerializeXml, or else that you've
		/// implemented a specialization of the fSerializeXmlObject function.
		/// If not, you'll get a compile error. This also implies that objects that
		/// take this route are considered internal nodes (non-leaf
		/// nodes). This method should be specialized for all types of leaf
		/// nodes, and hence "real" concrete data should go to one of the
		/// specialized versions of this method.
		template<class t>
		void operator( )( const char* name, t& object )
		{
			sigassert( !mCurRoot.fNull( ) );
			if( mCurRoot.fFindChild( mCurRoot, name ) )
			{
				fSerializeXmlObject( *this, object );
				mCurRoot = mCurRoot.fGetParent( );
			}
		}

	};

	template<class t, bool isRttiSubClass>
	class tXmlPolymorphicLoader
	{
	public:
		static t* fNewObject( tXmlNode& node )
		{
			// in the generic case, we just return a new object of type t;
			// this is for types that do not derive from tSerializableBaseClass
			return NEW t;
		}
	};

	template<class t>
	class tXmlPolymorphicLoader<t,true>
	{
	public:
		static t* fNewObject( tXmlNode& node )
		{
			// this is the specialized version for types that
			// derive from tSerializableBaseClass
			std::string cidText;
			node.fGetAttribute( "cid", cidText );

			Rtti::tClassId cid=Rtti::cInvalidClassId;

			// convert cid from a hex string
			std::stringstream ss;
			ss << cidText;
			ss >> std::hex >> cid;

			sigassert( cid != Rtti::cInvalidClassId );
			return ( t* )Rtti::fNewClass( cid );
		}
	};


	template<class t>
	class tXmlBuiltInFixedArrayLoader
	{
	public:
		static void fLoad( tXmlDeserializer& s, t& object )
		{
			// in the generic case, we recurse on each sub object
			tXmlNode savedRoot = s.fCurRoot( );

			tXmlNodeList children;
			s.fCurRoot( ).fGetAllChildren( children );

			for( u32 i = 0; i < children.fCount( ); ++i )
			{
				s.fCurRoot( ) = children[i];
				fSerializeXmlObject( s, object[i] );
			}

			s.fCurRoot( ) = savedRoot;
		}
	};


	template<class t>
	class tXmlBuiltInFixedGrowingArrayLoader
	{
	public:
		static void fLoad( tXmlDeserializer& s, t& object )
		{
			// in the generic case, we recurse on each sub object
			tXmlNode savedRoot = s.fCurRoot( );

			tXmlNodeList children;
			s.fCurRoot( ).fGetAllChildren( children );

			object.fSetCount( children.fCount( ) );
			for( u32 i = 0; i < object.fCount( ); ++i )
			{
				s.fCurRoot( ) = children[i];
				fSerializeXmlObject( s, object[i] );
			}

			s.fCurRoot( ) = savedRoot;
		}
	};

	template<class t, bool isBuiltIn>
	class tXmlBuiltInGrowableArrayLoader
	{
	public:
		static void fLoad( tXmlDeserializer& s, tGrowableArray<t>& object )
		{
			// in the generic case, we recurse on each sub object
			tXmlNode savedRoot = s.fCurRoot( );

			tXmlNodeList children;
			s.fCurRoot( ).fGetAllChildren( children );

			object.fSetCount( children.fCount( ) );
			for( u32 i = 0; i < object.fCount( ); ++i )
			{
				s.fCurRoot( ) = children[i];
				fSerializeXmlObject( s, object[i] );
			}

			s.fCurRoot( ) = savedRoot;
		}
	};

	template<class t>
	class tXmlBuiltInGrowableArrayLoader<t,true>
	{
	public:
		static void fLoad( tXmlDeserializer& s, tGrowableArray<t>& object )
		{
			// this is the specialized version for built in types; basically an optimization
			u32 count = 0;
			s.fCurRoot( ).fGetAttribute( "count", count );
			object.fSetCount( count );

			std::stringstream ss;
			s.fCurRoot( ).fGetContents( ss );

			for( u32 i = 0; i < object.fCount( ); ++i )
				ss >> object[ i ];
		}
	};


	template<class t, bool isBuiltIn>
	class tXmlBuiltInDynamicArrayLoader
	{
	public:
		static void fLoad( tXmlDeserializer& s, tDynamicArray<t>& object )
		{
			// in the generic case, we recurse on each sub object
			tXmlNode savedRoot = s.fCurRoot( );

			tXmlNodeList children;
			s.fCurRoot( ).fGetAllChildren( children );

			object.fNewArray( children.fCount( ) );
			for( u32 i = 0; i < object.fCount( ); ++i )
			{
				s.fCurRoot( ) = children[i];
				fSerializeXmlObject( s, object[i] );
			}

			s.fCurRoot( ) = savedRoot;
		}
	};

	template<class t>
	class tXmlBuiltInDynamicArrayLoader<t,true>
	{
	public:
		static void fLoad( tXmlDeserializer& s, tDynamicArray<t>& object )
		{
			// this is the specialized version for built in types; basically an optimization
			u32 count = 0;
			s.fCurRoot( ).fGetAttribute( "count", count );
			object.fNewArray( count );

			std::stringstream ss;
			s.fCurRoot( ).fGetContents( ss );

			for( u32 i = 0; i < object.fCount( ); ++i )
				ss >> object[ i ];
		}
	};

	template<>
	class tXmlBuiltInDynamicArrayLoader<byte,true>
	{
	public:
		static void fLoad( tXmlDeserializer& s, tDynamicArray<byte>& object )
		{
			// this is the specialized version for built in types; basically an optimization
			u32 count = 0;
			s.fCurRoot( ).fGetAttribute( "count", count );
			object.fNewArray( count );

			std::stringstream ss;
			s.fCurRoot( ).fGetContents( ss );

			for( u32 i = 0; i < object.fCount( ); ++i )
			{
				u32 o; ss >> o;
				object[ i ] = (byte)o;
			}
		}
	};


	template<class t>
	void fSerializeXmlObject( tXmlDeserializer& s, tStrongPtr<t>& object )
	{
		if( s.fCurRoot( ).fFindChild( s.fCurRoot( ), "p" ) )
		{
			t* newObject = tXmlPolymorphicLoader< t, can_convert( t, Rtti::tSerializableBaseClass ) >::fNewObject( s.fCurRoot( ) );
			object = tStrongPtr<t>( newObject );
			fSerializeXmlObject( s, *object );
			s.fCurRoot( ) = s.fCurRoot( ).fGetParent( );
		}
	}

	template<class t>
	void fSerializeXmlObject( tXmlDeserializer& s, tRefCounterPtr<t>& object )
	{
		if( s.fCurRoot( ).fFindChild( s.fCurRoot( ), "p" ) )
		{
			t* newObject = tXmlPolymorphicLoader< t, can_convert( t, Rtti::tSerializableBaseClass ) >::fNewObject( s.fCurRoot( ) );
			object = tRefCounterPtr<t>( newObject );
			fSerializeXmlObject( s, *object );
			s.fCurRoot( ) = s.fCurRoot( ).fGetParent( );
		}
	}

	template<class t, int N>
	void fSerializeXmlObject( tXmlDeserializer& s, tFixedArray<t,N>& object )
	{
		tXmlBuiltInFixedArrayLoader< tFixedArray<t,N> >::fLoad( s, object );
	}

	template<class t, int N>
	void fSerializeXmlObject( tXmlDeserializer& s, tFixedGrowingArray<t,N>& object )
	{
		tXmlBuiltInFixedGrowingArrayLoader< tFixedGrowingArray<t,N> >::fLoad( s, object );
	}

	template<class t>
	void fSerializeXmlObject( tXmlDeserializer& s, tGrowableArray<t>& object )
	{
		tXmlBuiltInGrowableArrayLoader< t, tIsBuiltInType<t>::cIs >::fLoad( s, object );
	}

	template<class t>
	void fSerializeXmlObject( tXmlDeserializer& s, tDynamicArray<t>& object )
	{
		tXmlBuiltInDynamicArrayLoader< t, tIsBuiltInType<t>::cIs >::fLoad( s, object );
	}

	template<>
	base_export void fSerializeXmlObject<tXmlDeserializer,int>( tXmlDeserializer& s, int& object );

	template<>
	base_export void fSerializeXmlObject<tXmlDeserializer,u16>( tXmlDeserializer& s, u16& object );

	template<>
	base_export void fSerializeXmlObject<tXmlDeserializer,u32>( tXmlDeserializer& s, u32& object );

	template<>
	base_export void fSerializeXmlObject<tXmlDeserializer,u64>( tXmlDeserializer& s, u64& object );

	template<>
	base_export void fSerializeXmlObject<tXmlDeserializer,f32>( tXmlDeserializer& s, f32& object );

	template<>
	base_export void fSerializeXmlObject<tXmlDeserializer,f64>( tXmlDeserializer& s, f64& object );

	template<>
	base_export void fSerializeXmlObject<tXmlDeserializer,std::string>( tXmlDeserializer& s, std::string& object );

	template<>
	base_export void fSerializeXmlObject<tXmlDeserializer,tStringPtr>( tXmlDeserializer& s, tStringPtr& object );

	template<>
	base_export void fSerializeXmlObject<tXmlDeserializer,tFilePathPtr>( tXmlDeserializer& s, tFilePathPtr& object );
}

#endif//__tXmlDeserializer__
