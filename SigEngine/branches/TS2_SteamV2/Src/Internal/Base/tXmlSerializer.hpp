#ifndef __tXmlSerializer__
#define __tXmlSerializer__
#include "tXmlSerializerBase.hpp"

namespace Sig
{

	///
	/// \brief Facilitates saving an object to an xml data file.
	///
	/// Works in conjunction with tXmlDeserializer. Expects that an object
	/// has a single templatized method that looks like this:
	/// template<class tSerializer> void fSerializeXml( tSerializer& s );
	/// Or, alternatively, you can specialize the template function fSerializeXmlObject
	/// for the given data type (so that you can extend library types).
	class tXmlSerializer : public tXmlSerializerBase
	{
	public:

		virtual b32 fIn( ) const { return false; }
		virtual b32 fOut( ) const { return true; }

		///
		/// \brief Serialize an object and all its sub-objects to an xml file.
		template<class t>
		void fSave( const char * rootName, t& object )
		{
			mCurRoot = mXmlFile.fCreateRoot( rootName );
			fSerializeXmlObject( *this, object );
		}

		///
		/// \brief Serialize an object and all its sub-objects to an xml file on disk.
		template<class t>
		b32 fSave( const tFilePathPtr& path, const char* rootName, t& object, b32 promptToCheckout, b32 displayWarnings = false )
		{
			fSave( rootName, object );
			return mXmlFile.fSave( path, promptToCheckout, displayWarnings );
		}	

		///
		/// \brief Serialize data as an attribute on the current root node.
		template<class t>
		void fAsAttribute( const char* name, t& object )
		{
			sigassert( !mCurRoot.fNull( ) );
			mCurRoot.fSetAttribute( name, object );
		}

		///
		/// \brief General form for serializing an object.
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
			mCurRoot = mCurRoot.fCreateChild( name );
			fSerializeXmlObject( *this, object );
			mCurRoot = mCurRoot.fGetParent( );
		}

	};

	template<class t, bool isRttiSubClass>
	class tXmlPolymorphicSaver
	{
	public:
		static void fAddRttiClassId( tXmlNode& node, t& polymorphicObject )
		{
			// in the generic case, we do nothing; this is
			// for types that do not derive from tSerializableBaseClass
		}
	};

	template<class t>
	class tXmlPolymorphicSaver<t,true>
	{
	public:
		static void fAddRttiClassId( tXmlNode& node, t& polymorphicObject )
		{
			// this is the specialized version for types that
			// derive from tSerializableBaseClass
			const Rtti::tClassId cid = polymorphicObject.fClassId( );
			std::stringstream ss;
			ss << std::hex << cid;
			node.fSetAttribute( "cid", ss.str( ) );
		}
	};

	template<class t>
	class tXmlBuiltInFixedArraySaver
	{
	public:
		static void fSave( tXmlSerializer& s, t& object )
		{
			// in the generic case, we recurse on each sub object
			for( u32 i = 0; i < object.fCount( ); ++i )
			{
				s.fCurRoot( ) = s.fCurRoot( ).fCreateChild( "i" );
				fSerializeXmlObject( s, object[i] );
				s.fCurRoot( ) = s.fCurRoot( ).fGetParent( );
			}
		}
	};


	template<class t>
	class tXmlBuiltInFixedGrowingArraySaver
	{
	public:
		static void fSave( tXmlSerializer& s, t& object )
		{
			// in the generic case, we recurse on each sub object
			s.fCurRoot( ).fSetAttribute( "count", object.fCount( ) );
			for( u32 i = 0; i < object.fCount( ); ++i )
			{
				s.fCurRoot( ) = s.fCurRoot( ).fCreateChild( "i" );
				fSerializeXmlObject( s, object[i] );
				s.fCurRoot( ) = s.fCurRoot( ).fGetParent( );
			}
		}
	};

	template<class t, bool isBuiltIn>
	class tXmlBuiltInGrowableArraySaver
	{
	public:
		static void fSave( tXmlSerializer& s, tGrowableArray<t>& object )
		{
			// in the generic case, we recurse on each sub object
			s.fCurRoot( ).fSetAttribute( "count", object.fCount( ) );
			for( u32 i = 0; i < object.fCount( ); ++i )
			{
				s.fCurRoot( ) = s.fCurRoot( ).fCreateChild( "i" );
				fSerializeXmlObject( s, object[i] );
				s.fCurRoot( ) = s.fCurRoot( ).fGetParent( );
			}
		}
	};

	template<class t>
	class tXmlBuiltInGrowableArraySaver<t,true>
	{
	public:
		static void fSave( tXmlSerializer& s, tGrowableArray<t>& object )
		{
			// this is the specialized version for built in types; basically an optimization
			s.fCurRoot( ).fSetAttribute( "count", object.fCount( ) );
			std::stringstream ss;
			for( u32 i = 0; i < object.fCount( ); ++i )
				ss << object[i] << " ";
			s.fCurRoot( ).fSetContents( ss.str( ).c_str( ), object.fCount( ) > 0 );
		}
	};


	template<class t, bool isBuiltIn>
	class tXmlBuiltInDynamicArraySaver
	{
	public:
		static void fSave( tXmlSerializer& s, tDynamicArray<t>& object )
		{
			// in the generic case, we recurse on each sub object
			s.fCurRoot( ).fSetAttribute( "count", object.fCount( ) );
			for( u32 i = 0; i < object.fCount( ); ++i )
			{
				s.fCurRoot( ) = s.fCurRoot( ).fCreateChild( "i" );
				fSerializeXmlObject( s, object[i] );
				s.fCurRoot( ) = s.fCurRoot( ).fGetParent( );
			}
		}
	};

	template<class t>
	class tXmlBuiltInDynamicArraySaver<t,true>
	{
	public:
		static void fSave( tXmlSerializer& s, tDynamicArray<t>& object )
		{
			// this is the specialized version for built in types; basically an optimization
			s.fCurRoot( ).fSetAttribute( "count", object.fCount( ) );
			std::stringstream ss;
			for( u32 i = 0; i < object.fCount( ); ++i )
				ss << object[i] << " ";
			s.fCurRoot( ).fSetContents( ss.str( ).c_str( ), object.fCount( ) > 0 );
		}
	};

	template<>
	class tXmlBuiltInDynamicArraySaver<byte,true>
	{
	public:
		static void fSave( tXmlSerializer& s, tDynamicArray<byte>& object )
		{
			// this is the specialized version for built in types; basically an optimization
			s.fCurRoot( ).fSetAttribute( "count", object.fCount( ) );
			std::stringstream ss;
			for( u32 i = 0; i < object.fCount( ); ++i )
				ss << (u32)object[i] << " ";
			s.fCurRoot( ).fSetContents( ss.str( ).c_str( ), object.fCount( ) > 0 );
		}
	};


	template<class t>
	void fSerializeXmlObject( tXmlSerializer& s, tStrongPtr<t>& object )
	{
		s.fCurRoot( ) = s.fCurRoot( ).fCreateChild( "p" );
		tXmlPolymorphicSaver< const t, can_convert( t, Rtti::tSerializableBaseClass ) >::fAddRttiClassId( s.fCurRoot( ), *object );
		fSerializeXmlObject( s, *object );
		s.fCurRoot( ) = s.fCurRoot( ).fGetParent( );
	}

	template<class t>
	void fSerializeXmlObject( tXmlSerializer& s, tRefCounterPtr<t>& object )
	{
		s.fCurRoot( ) = s.fCurRoot( ).fCreateChild( "p" );
		tXmlPolymorphicSaver< const t, can_convert( t, Rtti::tSerializableBaseClass ) >::fAddRttiClassId( s.fCurRoot( ), *object );
		fSerializeXmlObject( s, *object );
		s.fCurRoot( ) = s.fCurRoot( ).fGetParent( );
	}

	template<class t, int N>
	void fSerializeXmlObject( tXmlSerializer& s, tFixedArray<t,N>& object )
	{
		tXmlBuiltInFixedArraySaver< tFixedArray<t,N> >::fSave( s, object );
	}

	template<class t, int N>
	void fSerializeXmlObject( tXmlSerializer& s, tFixedGrowingArray<t,N>& object )
	{
		tXmlBuiltInFixedGrowingArraySaver< tFixedGrowingArray<t,N> >::fSave( s, object );
	}

	template<class t>
	void fSerializeXmlObject( tXmlSerializer& s, tDynamicArray<t>& object )
	{
		tXmlBuiltInDynamicArraySaver< t, tIsBuiltInType<t>::cIs >::fSave( s, object );
	}

	template<class t>
	void fSerializeXmlObject( tXmlSerializer& s, tGrowableArray<t>& object )
	{
		tXmlBuiltInGrowableArraySaver< t, tIsBuiltInType<t>::cIs >::fSave( s, object );
	}

	template<>
	base_export void fSerializeXmlObject<tXmlSerializer,int>( tXmlSerializer& s, int& object );

	template<>
	base_export void fSerializeXmlObject<tXmlSerializer,u16>( tXmlSerializer& s, u16& object );

	template<>
	base_export void fSerializeXmlObject<tXmlSerializer,u32>( tXmlSerializer& s, u32& object );

	template<>
	base_export void fSerializeXmlObject<tXmlSerializer,u64>( tXmlSerializer& s, u64& object );

	template<>
	base_export void fSerializeXmlObject<tXmlSerializer,f32>( tXmlSerializer& s, f32& object );

	template<>
	base_export void fSerializeXmlObject<tXmlSerializer,f64>( tXmlSerializer& s, f64& object );

	template<>
	base_export void fSerializeXmlObject<tXmlSerializer,std::string>( tXmlSerializer& s, std::string& object );

	template<>
	base_export void fSerializeXmlObject<tXmlSerializer,tStringPtr>( tXmlSerializer& s, tStringPtr& object );

	template<>
	base_export void fSerializeXmlObject<tXmlSerializer,tFilePathPtr>( tXmlSerializer& s, tFilePathPtr& object );
}

#endif//__tXmlSerializer__
