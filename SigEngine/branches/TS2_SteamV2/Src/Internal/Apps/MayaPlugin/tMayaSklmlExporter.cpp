#include "MayaPluginPch.hpp"
#include "tExporterToolbox.hpp"
#include "tMayaSklmlExporter.hpp"
#include "MayaUtil.hpp"
#include "WxUtil.hpp"

namespace Sig
{

	///
	/// \section tSklmlMayaPlugin
	///

	b32 tSklmlMayaPlugin::fOnMayaInitialize( MObject& mayaObject, MFnPlugin& mayaPlugin )
	{
		MStatus status = mayaPlugin.registerFileTranslator("sklml", "", tMayaSklmlExporter::fCreate, "", "option1=1", true);

		if( !status )
		{
			status.perror("registerFileTranslator");
			return false;
		}

		return true;
	}

	b32 tSklmlMayaPlugin::fOnMayaShutdown( MObject& mayaObject, MFnPlugin& mayaPlugin )
	{
		MStatus status = mayaPlugin.deregisterFileTranslator("sklml");

		if( !status )
		{
			status.perror("deregisterFileTranslator");
			return false;
		}

		return true;
	}
}

namespace Sig
{
	void* tMayaSklmlExporter::fCreate( )
	{
		// if we return an actual 'new' object,
		// we apparently get a memory leak... i.e.,
		// maya never seems to deallocate the object.
		// since this class doesn't store any state, 
		// it's safe to return the same one over and over
		static tMayaSklmlExporter gExporter;
		return &gExporter;
	}

	tMayaSklmlExporter::tMayaSklmlExporter( )
		: mNumRefFrames( 0 )
	{
	}

	tMayaSklmlExporter::~tMayaSklmlExporter( )
	{
	}

	MStatus tMayaSklmlExporter::writer( const MFileObject& file, const MString& optionsString, FileAccessMode mode )
	{
		MStatus status = MStatus::kFailure;

		// check which objects are to be exported, and invoke the corresponding
		// methods; only 'export all' and 'export selection' are allowed
		if( MPxFileTranslator::kExportAccessMode == mode )
			status = fExportAll( file.expandedFullName( ) );
		else if( MPxFileTranslator::kExportActiveAccessMode == mode )
			status = fExportSelected( file.expandedFullName( ) );

		// report result
		if( status == MStatus::kSuccess )
			MGlobal::displayInfo("Export to " + file.fullName( ) + " successful.");

		// return result
		return status;
	}

	bool tMayaSklmlExporter::haveReadMethod( ) const
	{
		// return false bcz we're not an importer
		return false;
	}

	bool tMayaSklmlExporter::haveWriteMethod( ) const
	{
		// return true bcz we're an exporter
		return true;
	}

	bool tMayaSklmlExporter::haveNamespaceSupport( ) const
	{
		// this is only necessary for importers
		return false;
	}

	bool tMayaSklmlExporter::haveReferenceMethod( ) const
	{
		// currently don't support this, but we could conceivably
		// implement referenced sigml files directly using maya api
		return false;
	}

	MString tMayaSklmlExporter::defaultExtension( ) const
	{
		return MString( "" );
	}

	MString tMayaSklmlExporter::filter( ) const
	{
		return MString( "*" ) + Sklml::fGetFileExtension( );
	}

	bool tMayaSklmlExporter::canBeOpened( ) const
	{
		// the docs say to return:
		//	* true if the class can open and import files
		//	* false if the class can only import files 
		return true;
	}

	MPxFileTranslator::MFileKind tMayaSklmlExporter::identifyFile( const MFileObject& file, const char* buffer, short size ) const
	{
		// only relevant for importer/referencer
		return kNotMyFileType;
	}

	MStatus tMayaSklmlExporter::fExportAll( const MString& path )
	{
		fExportCommonBegin( path );
		MayaUtil::fForEachTransformNode( make_delegate_memfn( MayaUtil::tForEachObject, tMayaSklmlExporter, fVisitNode ) );
		fExportCommonEnd( path );
		return MStatus::kSuccess;
	}

	MStatus tMayaSklmlExporter::fExportSelected( const MString& path )
	{
		fExportCommonBegin( path );
		MayaUtil::fForEachSelectedNode( make_delegate_memfn( MayaUtil::tForEachObject, tMayaSklmlExporter, fVisitNode ) );
		fExportCommonEnd( path );
		return MStatus::kSuccess;
	}

	void tMayaSklmlExporter::fExportCommonBegin( const MString& path )
	{
		log_line( 0, "Begin Sklml ExportAll.............." );

		mNumRefFrames = 0;
		mExportFile.fReset( new Sklml::tFile( ) );
		mExportFile->mModellingPackage = tStringPtr( "Maya.2013" );
		mExportFile->mSrcFile = tFilePathPtr( MFileIO::currentFile( ).asChar( ) );
		mExportFile->mSrcFile = ToolsPaths::fMakeResRelative( mExportFile->mSrcFile );
	}

	void tMayaSklmlExporter::fExportCommonEnd( const MString& path )
	{
		mExportFile->fGenerateMasterIndices( );
		mExportFile->fSaveXml( tFilePathPtr( path.asChar( ) ), true );
		mExportFile.fRelease( );
		mNumRefFrames = 0;

		log_line( 0, "..............End Sklml ExportAll" );
	}

	b32 tMayaSklmlExporter::fVisitNode( MDagPath& path, MObject& component )
	{
		MFnDagNode nodeFn( path );
		MObject object = path.node( );

		b32 found;

		// test if the object is being ignored
		b32 ignore = false;
		found = MayaUtil::fGetBoolAttribute( object, nodeFn, ignore, tExporterToolbox::fNameIgnoreObject( ) );
		if( found && ignore )
		{
			log_line( 0, "Skipping object [" << nodeFn.name( ).asChar( ) << "] because it was specified as IGNORE." );
			return true;
		}

		// test if the object was specified as a reference frame
		b32 refFrame = false;
		found = MayaUtil::fGetBoolAttribute( object, nodeFn, refFrame, tExporterToolbox::fNameReferenceFrame( ) );
		if( found && refFrame )
		{
			// we only support one reference frame per export
			if( mNumRefFrames == 0 )
			{
				log_line( 0, "Exporting reference frame [" << nodeFn.name( ).asChar( ) << "]" );
				fVisitReferenceFrame( path );
			}
			else
				log_warning( 0, "Skipping reference frame [" << nodeFn.name( ).asChar( ) << "]; only 1 reference frame can be exported at a time in a .sklml file." );
			++mNumRefFrames;
		}

		return true;
	}

	void tMayaSklmlExporter::fVisitReferenceFrame( MDagPath& path )
	{
		MDagPath rootNodeDagPath = MayaUtil::fRootBoneFromReferenceFrame( path, tExporterToolbox::fNameRootNodeName( ) );
		if( !rootNodeDagPath.isValid( ) )
		{
			log_warning( 0, "Invalid root node name specified on reference frame." );
			return;
		}

		// Animation Sources
		{
			MFnDagNode nodeFn( path );
			MObject object = path.node( );

			s32 sourceCount = 0;
			MayaUtil::fGetIntAttribute(
				object, nodeFn, sourceCount, tExporterToolbox::fNameAnimationSources( ) );

			if( sourceCount )
			{
				mExportFile->mAnimSourceSkeletons.fSetCapacity( sourceCount );
				for( u32 s = 0; s < (u32)sourceCount; ++s )
				{
					std::string longName;
					tExporterToolbox::fNameAnimationSources( s, longName );

					std::string source;
					b32 found = MayaUtil::fGetStringAttribute( object, nodeFn, source, longName.c_str( ) );
					if( !found )
						continue;

					// Last chance to ensure non-duplicates
					mExportFile->mAnimSourceSkeletons.fFindOrAdd( tFilePathPtr( source.c_str( ) ) );
				}
			}
		}

		fBuildExportSkeleton( path, rootNodeDagPath );
	}

	void tMayaSklmlExporter::fBuildExportSkeleton( MDagPath& refFramePath, MDagPath& rootNodePath )
	{
		sigassert( refFramePath.isValid( ) && rootNodePath.isValid( ) );

		fConvertBoneData( mExportFile->mSkeleton.mRefFrame, refFramePath );
		fBuildExportSkeleton( mExportFile->mSkeleton.mRoot, rootNodePath );
	}

	void tMayaSklmlExporter::fConvertBoneData( Sklml::tBone& sklmlRoot, MDagPath& mayaRoot )
	{
		MFnDagNode mayaNodeFn( mayaRoot );

		// get name
		sklmlRoot.mName = mayaNodeFn.name( ).asChar( );

		// get xform
		sigassert( mayaRoot.hasFn( MFn::kTransform ) );
		MFnTransform xformFn( mayaRoot );

		// convert the xform
		MayaUtil::fConvertMatrix( xformFn, sklmlRoot.mXform );
	}

	void tMayaSklmlExporter::fBuildExportSkeleton( Sklml::tBone& sklmlRoot, MDagPath& mayaRoot )
	{
		fConvertBoneData( sklmlRoot, mayaRoot );

		const u32 childCount = mayaRoot.childCount( );
		for( u32 i = 0; i < childCount; ++i )
		{
			MObject child = mayaRoot.child( i );
			if( child.apiType( ) == MFn::kJoint )
			{
				MDagPath childPath;
				MDagPath::getAPathTo( child, childPath );

				sklmlRoot.mChildren.fPushBack( Sklml::tBonePtr( new Sklml::tBone ) );
				fBuildExportSkeleton( *sklmlRoot.mChildren.fBack( ), childPath );
			}
		}
	}

}

