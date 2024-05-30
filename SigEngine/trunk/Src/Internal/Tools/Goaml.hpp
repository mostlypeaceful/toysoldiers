#ifndef __Goaml__
#define __Goaml__
#include "DerivedAINodes.hpp"
#include "tDAGNodeCanvas.hpp"

namespace Sig
{
	class tXmlSerializer;
	class tXmlDeserializer;
}

namespace Sig { namespace Goaml
{
	tools_export const char* fGetFileExtension( );
	tools_export b32 fIsGoamlFile( const tFilePathPtr& path );
	tools_export tFilePathPtr fGoamlPathToNutb( const tFilePathPtr& path );
	tools_export tFilePathPtr fGoamlPathToNut( const tFilePathPtr& path );

	struct tConnection
	{
		u32 mInput;
		u32 mOutput;
		u32 mInputIndex;
		u32 mOutputIndex;
		tEditablePropertyTable mProps;

		tConnection( ) 
			: mInput( ~0 )
			, mOutput( ~0 )
			, mInputIndex( ~0 )
			, mOutputIndex( ~0 )
		{ }
		tConnection( u32 i, u32 idi, u32 o, u32 ido, tAIConnectionData* data )
			: mInput( i )
			, mOutput( o )
			, mInputIndex( idi )
			, mOutputIndex( ido )
			, mProps( data->mProps )
		{ }
	};

	typedef tGrowableArray< tConnection >  tConnectionList;
	typedef tGrowableArray< tAINodePtr >  tNodeList;

	class tools_export tFile
	{
	public:
		u32					mVersion;
		u32					mNextUniqueNodeId;
		tNodeList			mNodes;
		tConnectionList		mConnections;
		tFilePathPtr		mMoMapFile;
		tFilePathPtrList	mExplicitDependencies;

	public:
		tFile( );
		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );

		const tFilePathPtr& fMoMap( ) const { return mMoMapFile; }
		void fSetMoMap( const tFilePathPtr& path ) { mMoMapFile = path; }

		// AlreadyConstructed means not serialized yet. still looking at the file in memory, only these will only work if alreadyConstructed
		std::string fBuildScript( const std::string& uniqueFileName, b32 alreadyConstructed = false, const tDAGNodeCanvas::tDAGNodeList& onlyThese = tDAGNodeCanvas::tDAGNodeList() );
		void fCollect( tDAGNodeCanvas* canvas, tDAGNodeCanvas::tDAGNodeList& nodes, tDAGNodeCanvas::tDAGNodeConnectionList& conn, b32 alreadyConstructed ) const;
		void fSortXMLNodes( );
		void fQuickSortXMLNodes( );

	private:
		void fQuickSortXMLNodes( s32 left, s32 right );
	};

}}

#endif//__Goaml__
