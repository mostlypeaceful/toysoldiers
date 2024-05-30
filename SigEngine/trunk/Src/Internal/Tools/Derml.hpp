#ifndef __Derml__
#define __Derml__
#include "tShadeNode.hpp"

namespace Sig
{
	class tXmlSerializer;
	class tXmlDeserializer;
}

namespace Sig { namespace Derml
{
	tools_export const char* fGetFileExtension( );
	tools_export b32 fIsDermlFile( const tFilePathPtr& path );
	tools_export tFilePathPtr fDermlPathToMtlb( const tFilePathPtr& path );

	struct tConnection
	{
		u32 mInput;
		u32 mIndex;
		u32 mOutput;

		tConnection( ) : mInput( ~0 ), mIndex( ~0 ), mOutput( ~0 ) { }
		tConnection( u32 i, u32 idx, u32 o ) : mInput( i ), mIndex( idx ), mOutput( o ) { }
	};

	typedef tGrowableArray< tConnection >  tConnectionList;
	typedef tGrowableArray< tShadeNodePtr >  tNodeList;

	class tools_export tFile
	{
	public:
		u32					mVersion;
		u32					mNextUniqueNodeId;
		HlslGen::tVshStyle	mGeometryStyle;
		tNodeList			mNodes;
		tConnectionList		mConnections;
	public:
		tFile( );

		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		void fSaveXml( tXmlSerializer & out );

		b32 fLoadXml( const tFilePathPtr& path );
		b32 fLoadXml( tXmlDeserializer & in );

		void fCollectOutputs( tShadeNode::tOutputArray& outputs ) const;
		void fGetConnectionList( const tShadeNodePtr& shadeNode, tConnectionList& connexList ) const;
		void fCullUnconnected( tNodeList& nodesOut ) const;

	private:
		void fCollectInputs( tShadeNodePtr node, tNodeList& collected ) const;
		void fFixupAfterLoad( );
	};

	class tools_export tMtlFile
	{
	public:
		tFilePathPtr mShaderPath;
		tNodeList mNodes;
		u64 mShaderLastModifiedStamp;
	public:
		tMtlFile( );
		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );
		void fSerialize( tXmlSerializer& ser );
		void fSerialize( tXmlDeserializer& des );
		void fClone( tMtlFile& cloneOut, const tFilePathPtr& scratchFilePath );
		void fFromShaderFile( const tFile& dermlFile, const tFilePathPtr& shaderFilePath );
		b32 fUpdateAgainstShaderFile( Derml::tFile& dermlFile, b32 * loadSuccess = NULL ); // returns true if the material updated (i.e., the shader had changed)
		b32 fUpdateAgainstPreviousNodes( const tNodeList& oldNodes ); // returns true if the material updated (i.e., the shader had changed)
		b32 fIsShaderModified( u64* newStampOut = 0 ) const;
		u64 fShaderLastModified( ) const;
	private:
		tShadeNodePtr fFindByUniqueId( u32 uid ) const;
		u32 fFindByName( const char* name ) const;
	};

}}

#endif//__Derml__
