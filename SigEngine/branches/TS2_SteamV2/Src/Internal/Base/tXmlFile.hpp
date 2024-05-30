#ifndef __tXmlFile__
#define __tXmlFile__
#include "tStrongPtr.hpp"
#include "tByteFile.hpp"

///
/// \section Forward declarations from tinyxml.
///

class TiXmlElement;
class TiXmlDocument;

namespace Sig
{


	///
	/// \section Internal types.
	///

	class tXmlNode;
	class tXmlFileData;
	typedef tGrowableArray<tXmlNode> tXmlNodeList;
	typedef TiXmlElement* tXmlNodeInternalPtr;
	typedef tStrongPtr<TiXmlDocument> tXmlDocumentInternalPtr;

	///
	/// \brief Represents a node in an xml tree.
	class base_export tXmlNode
	{
		friend class tXmlFileData;

	public:

		tXmlNode( );
		inline b32	fNull( ) const { return mNodePtr==0; }

		tXmlNode	fCreateChild( const char* nodeName );
		b32			fFindChild( tXmlNode& child, const char* nodeName );
		b32			fFindChildren( tXmlNodeList& children, const char* nodeName );
		tXmlNode	fGetParent( );
		void		fGetAllChildren( tXmlNodeList& children ) const;
		u32			fGetChildCount( ) const;
		b32			fGetName( std::string& name ) const;

		void		fSetContents( const char* text, b32 cdata = false );
		b32			fGetContents( std::stringstream& contents ) const;

		void		fSetAttribute( const char* attrName, const std::string& value ) const;
		void		fSetAttribute( const char* attrName, s32 value ) const;
		void		fSetAttribute( const char* attrName, u32 value ) const { fSetAttribute( attrName, (s32)value ); }
		void		fSetAttribute( const char* attrName, f32 value ) const;

		b32			fGetAttribute( const char* attrName, std::string& value );
		b32			fGetAttribute( const char* attrName, u64& value );
		b32			fGetAttribute( const char* attrName, s32& value );
		b32			fGetAttribute( const char* attrName, u32& value ) { return fGetAttribute( attrName, reinterpret_cast<s32&>( value ) ); }
		b32			fGetAttribute( const char* attrName, u16& value ) { s32 o = 0; const b32 r = fGetAttribute( attrName, o ); value = ( u16 )o; return r; }
		b32			fGetAttribute( const char* attrName, s16& value ) { s32 o = 0; const b32 r = fGetAttribute( attrName, o ); value = ( s16 )o; return r; }
		b32			fGetAttribute( const char* attrName, u8& value ) { s32 o = 0; const b32 r = fGetAttribute( attrName, o ); value = ( u8 )o; return r; }
		b32			fGetAttribute( const char* attrName, s8& value ) { s32 o = 0; const b32 r = fGetAttribute( attrName, o ); value = ( s8 )o; return r; }
		b32			fGetAttribute( const char* attrName, f32& value );

	protected:

		tXmlNode( tXmlNodeInternalPtr nodeptr );

		tXmlNodeInternalPtr	mNodePtr;
	};

	///
	/// \brief Represents an entire xml file (the root of an xml tree).
	class base_export tXmlFileData : public tUncopyable
	{
		friend class tXmlNode;

	public:
		tXmlFileData( );
		~tXmlFileData( );

		b32 fSave( const tFilePathPtr& path, b32 promptToCheckout, b32 displayWarnings = false );

		// TODO: There is something going on with the dll export getting somehow mismatched if this dummy argument is removed.
		b32 fLoad( const tFilePathPtr& path, b32 dummy = false );

		b32 fParse( const char * text );

		tXmlNode fCreateRoot( const char* nodeName );

		inline tXmlNode fGetRoot( ) const { return mRoot; }

		// from a string in memory
		void fConstructTree( const std::string& contents );

	private:
		void fNewDocument( );

		tXmlDocumentInternalPtr		mDocPtr;
		tXmlNode					mRoot;
	};
	
	/*
		tXmlFile is an actual load in place resource file. Meant to be used in Game.

		Warning if you add any other data here, you should not be dependent on tByteFile, and should become your own unique resource.
	*/
	class base_export tXmlFile : public tByteFile
	{
		declare_reflector( );
		implement_rtti_serializable_base_class(tXmlFile, 0x806571E6);

	public:
		static const u32		cVersion;

	public:
		tXmlFile( ) { }
		tXmlFile( tNoOpTag ) : tByteFile( cNoOpTag ) { }

		const char* fBegin( ) const { return (const char*)fData( ).fBegin( ); }
		u32 fCount( ) const { return fData( ).fCount( ); }
	};

	template<>
	class tResourceConvertPath<tXmlFile>
	{
	public:
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathAddB( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathSubB( path ); }
	};
}


#endif//__tXmlFile__
