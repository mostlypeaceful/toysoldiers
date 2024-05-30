#include "BasePch.hpp"
#include "tXmlFile.hpp"
#include "tinyxml.h"

namespace Sig
{
	///
	/// \section tXmlNode
	///

	tXmlNode::tXmlNode( )
		: mNodePtr( 0 )
	{
	}

	tXmlNode::tXmlNode( tXmlNodeInternalPtr nodeptr )
		: mNodePtr( nodeptr )
	{
	}

	tXmlNode tXmlNode::fGetParent( )
	{
		if( fNull( ) )
			return tXmlNode( );

		TiXmlNode* parent = mNodePtr->Parent( );
		if( !parent )
			return tXmlNode( );

		return tXmlNode( parent->ToElement( ) );
	}

	void tXmlNode::fGetAllChildren( tXmlNodeList& children ) const
	{
		if( fNull( ) )
			return;

		for( TiXmlNode* child = mNodePtr->FirstChild( );
				child;
				child = child->NextSibling( ) )
		{
			TiXmlElement* elem = child->ToElement( );
			if( !elem )
				continue;
			children.fPushBack( tXmlNode( elem ) );
		}
	}

	u32	tXmlNode::fGetChildCount( ) const
	{
		if( fNull( ) )
			return 0;

		u32 count = 0;

		for( TiXmlNode* child = mNodePtr->FirstChild( );
				child;
				child = child->NextSibling( ) )
		{
			TiXmlElement* elem = child->ToElement( );
			if( !elem )
				continue;
			++count;
		}

		return count;
	}

	b32 tXmlNode::fFindChild( tXmlNode& child, const char* nodeName )
	{
		if( fNull( ) )
			return false;

		tXmlNodeInternalPtr nodeptr = mNodePtr->FirstChildElement( nodeName );
		if( !nodeptr )
			return false;

		child = tXmlNode( nodeptr );
		sigassert( !child.fNull( ) );
		return true;
	}

	b32 tXmlNode::fFindChildren( tXmlNodeList& children, const char* nodeName )
	{
		if( fNull( ) )
			return false;

		for( TiXmlNode* child = mNodePtr->FirstChild( );
				child;
				child = child->NextSibling( ) )
		{
			TiXmlElement* elem = child->ToElement( );
			if( !elem )
				continue;
			if( StringUtil::fStricmp( elem->Value( ), nodeName ) == 0 )
				children.fPushBack( tXmlNode( elem ) );
		}

		return children.fCount( ) > 0;
	}

	tXmlNode tXmlNode::fCreateChild( const char* nodeName )
	{
		if( fNull( ) )
			return tXmlNode( );
		
		tXmlNode child( NEW TiXmlElement( nodeName ) );

		mNodePtr->LinkEndChild( child.mNodePtr );

		return child;
	}

	b32	tXmlNode::fGetName( std::string& name ) const
	{
		if( fNull( ) )
			return false;
		name = mNodePtr->Value( );
		return true;
	}

	void tXmlNode::fSetContents( const char* text, b32 cdata )
	{
		if( fNull( ) )
			return;

		TiXmlText* textNode = 0;

		// first look for existing text child
		for( TiXmlNode* child = mNodePtr->FirstChild( );
				child;
				child = child->NextSibling( ) )
		{
			textNode = child->ToText( );
			if( textNode )
				break;
		}

		// if found, reset text, otherwise create new node with text value

		if( textNode )
			textNode->SetValue( text );
		else
			textNode = NEW TiXmlText( text );

		if( cdata )
			textNode->SetCDATA( true );

		mNodePtr->LinkEndChild( textNode );
	}

	b32 tXmlNode::fGetContents( std::stringstream& contents ) const
	{
		if( fNull( ) )
			return false;

		const char* text = mNodePtr->GetText( );
		if( !text )
			return false;

		contents << text;
		return true;
	}

	void tXmlNode::fSetAttribute( const char* attrName, const std::string& value ) const
	{
		if( fNull( ) ) return;
		mNodePtr->SetAttribute( attrName, value.c_str( ) );
	}

	void tXmlNode::fSetAttribute( const char* attrName, s32 value ) const
	{
		if( fNull( ) ) return;
		mNodePtr->SetAttribute( attrName, value );
	}

	void tXmlNode::fSetAttribute( const char* attrName, f32 value ) const
	{
		if( fNull( ) ) return;
		mNodePtr->SetDoubleAttribute( attrName, value );
	}

	b32	tXmlNode::fGetAttribute( const char* attrName, std::string& value )
	{
		if( fNull( ) ) return false;
		const char* attr = mNodePtr->Attribute( attrName );
		if( attr )
			value = attr;
		return attr != 0;
	}

	b32	tXmlNode::fGetAttribute( const char* attrName, s32& value )
	{
		if( fNull( ) ) return false;

		if( mNodePtr->QueryIntAttribute( attrName, &value ) == TIXML_SUCCESS )
			return true;
		return false;
	}

	b32	tXmlNode::fGetAttribute( const char* attrName, u64& value )
	{
		std::string text;
		if( !fGetAttribute( attrName, text ) ) 
			return false;
			
#if defined(platform_ios)
		value = atoll( text.c_str( ) );
#else
		value = _atoi64( text.c_str( ) );
#endif

		return true;
	}

	b32 tXmlNode::fGetAttribute( const char* attrName, f32& value )
	{
		if( fNull( ) ) return false;

		if( mNodePtr->QueryFloatAttribute( attrName, &value ) == TIXML_SUCCESS )
			return true;
		return false;
	}

	///
	/// \section tXmlFile
	///

	tXmlFileData::tXmlFileData( )
	{
		fNewDocument( );
	}

	tXmlFileData::~tXmlFileData( )
	{
	}


	b32 tXmlFileData::fSave( const tFilePathPtr& path, b32 promptToCheckout, b32 displayWarnings )
	{
		if( !mDocPtr->SaveFile( path.fCStr( ) ) )
		{
#if defined( __ToolsPaths__ )
			if( promptToCheckout && ToolsPaths::fPromptToCheckout( path ) )
			{
				if( displayWarnings )
				{
					std::stringstream msgTextSS;

					std::string userList;
					if ( ToolsPaths::fIsCheckedOut( path, userList ) )
						msgTextSS << "The file [" << path.fCStr( ) << "] is checked out.";

					// Prompt when a file is out of date
					if( ToolsPaths::fIsOutOfDate( path ) )
					{
						if ( msgTextSS.str().length() )
							msgTextSS << "\n\n";

						msgTextSS << "The file [" << path.fCStr( ) << "] is out-of-date.";
					}

					if ( msgTextSS.str( ).length( ) )
						const int result = MessageBox( NULL, msgTextSS.str().c_str(), "Warning!", MB_ICONWARNING | MB_OK | MB_APPLMODAL );
				}
				return mDocPtr->SaveFile( path.fCStr( ) )!=0; // attempt to save again
			}
#endif
			return false;
		}

		return true;
	}

	// TODO: There is something going on with the dll export getting somehow mismatched if this dummy argument is removed.
	b32 tXmlFileData::fLoad( const tFilePathPtr& path, b32 dummy )
	{
        if( !mDocPtr->LoadFile( path.fCStr( ) ) ) 
        {
            log_warning( "Error Loading Xml File. Row: " << mDocPtr->ErrorRow( ) << " Column: " << mDocPtr->ErrorCol( ) << " Error: " << mDocPtr->ErrorDesc( ) );
			return false;
        }

		tXmlNodeInternalPtr nodePtr = mDocPtr->FirstChildElement( );

		// store root
		mRoot = tXmlNode( nodePtr );

		return true;
	}

	b32 tXmlFileData::fParse( const char * text )
	{
		mDocPtr->Clear( );
		mDocPtr->Parse( text );
		if( mDocPtr->Error( ) )
		{
			log_warning( "Error parsing Xml text. Row: " << mDocPtr->ErrorRow( ) << " Column: " << mDocPtr->ErrorCol( ) << " Error: " << mDocPtr->ErrorDesc( ) );
			return false;
		}
		
		tXmlNodeInternalPtr nodePtr = mDocPtr->FirstChildElement( );

		// store root
		mRoot = tXmlNode( nodePtr );

		return true;
	}

	tXmlNode tXmlFileData::fCreateRoot( const char* nodeName )
	{
		fNewDocument( );

		mRoot = tXmlNode( NEW TiXmlElement( nodeName ) );
		mDocPtr->LinkEndChild( mRoot.mNodePtr );

		return mRoot;
	}

	void tXmlFileData::fNewDocument( )
	{
		mDocPtr = fNewStrongPtr<TiXmlDocument>( );
		sigassert( mDocPtr );
		mDocPtr->LinkEndChild( NEW TiXmlDeclaration( "1.0", "ISO-8859-1", "" ) );
	}

	void tXmlFileData::fConstructTree( const std::string& contents )
	{
		fNewDocument( );
		mDocPtr->Parse( contents.c_str( ) );
		mRoot = tXmlNode( mDocPtr->FirstChildElement( ) );
	}

}


