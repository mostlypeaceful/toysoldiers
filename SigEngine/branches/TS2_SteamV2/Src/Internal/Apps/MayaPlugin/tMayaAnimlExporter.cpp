#include "MayaPluginPch.hpp"
#include "tExporterToolbox.hpp"
#include "tMayaAnimlExporter.hpp"
#include "MayaUtil.hpp"
#include "WxUtil.hpp"
#include <maya\MFnPlugin.h>
#include "Math/Math.hpp"

namespace Sig
{
	///
	/// \section tAnimlMayaPlugin
	///

	b32 tAnimlMayaPlugin::fOnMayaInitialize( MObject& mayaObject, MFnPlugin& mayaPlugin )
	{
		MStatus status = mayaPlugin.registerFileTranslator("animl", "", tMayaAnimlExporter::fCreate, "", "option1=1", true);

		if( !status )
		{
			status.perror("registerFileTranslator");
			return false;
		}

		return true;
	}

	b32 tAnimlMayaPlugin::fOnMayaShutdown( MObject& mayaObject, MFnPlugin& mayaPlugin )
	{
		MStatus status = mayaPlugin.deregisterFileTranslator("animl");

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
	///
	/// \section tMayaAnimlExporter
	///

	void* tMayaAnimlExporter::fCreate( )
	{
		// if we return an actual 'new' object,
		// we apparently get a memory leak... i.e.,
		// maya never seems to deallocate the object.
		// since this class doesn't store any state, 
		// it's safe to return the same one over and over
		static tMayaAnimlExporter gExporter;
		return &gExporter;
	}

	tMayaAnimlExporter::tMayaAnimlExporter( )
		: mTimeUnit( MTime::kNTSCFrame )
		, mNumRefFrames( 0 )
	{
	}

	tMayaAnimlExporter::~tMayaAnimlExporter( )
	{
	}

	MStatus tMayaAnimlExporter::writer( const MFileObject& file, const MString& optionsString, FileAccessMode mode )
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

	bool tMayaAnimlExporter::haveReadMethod( ) const
	{
		// return false bcz we're not an importer
		return false;
	}

	bool tMayaAnimlExporter::haveWriteMethod( ) const
	{
		// return true bcz we're an exporter
		return true;
	}

	bool tMayaAnimlExporter::haveNamespaceSupport( ) const
	{
		// this is only necessary for importers
		return false;
	}

	bool tMayaAnimlExporter::haveReferenceMethod( ) const
	{
		// currently don't support this, but we could conceivably
		// implement referenced sigml files directly using maya api
		return false;
	}

	MString tMayaAnimlExporter::defaultExtension( ) const
	{
		return MString( "" );
	}

	MString tMayaAnimlExporter::filter( ) const
	{
		return MString( "*" ) + Animl::fGetFileExtension( );
	}

	bool tMayaAnimlExporter::canBeOpened( ) const
	{
		// the docs say to return:
		//	* true if the class can open and import files
		//	* false if the class can only import files 
		return true;
	}

	MPxFileTranslator::MFileKind tMayaAnimlExporter::identifyFile( const MFileObject& file, const char* buffer, short size ) const
	{
		// only relevant for importer/referencer
		return kNotMyFileType;
	}

	MStatus tMayaAnimlExporter::fExportAll( const MString& path )
	{
		fExportCommonBegin( path );
		MayaUtil::fForEachTransformNode( make_delegate_memfn( MayaUtil::tForEachObject, tMayaAnimlExporter, fVisitNode ) );
		fExportCommonEnd( path );
		return MStatus::kSuccess;
	}

	MStatus tMayaAnimlExporter::fExportSelected( const MString& path )
	{
		fExportCommonBegin( path );
		MayaUtil::fForEachSelectedNode( make_delegate_memfn( MayaUtil::tForEachObject, tMayaAnimlExporter, fVisitNode ) );
		fExportCommonEnd( path );
		return MStatus::kSuccess;
	}

	void tMayaAnimlExporter::fExportCommonBegin( const MString& path )
	{
		log_line( 0, "Begin Animl ExportAll.............." );

		mExportFile.fReset( new Animl::tFile( ) );
		mExportFile->mModellingPackage = tStringPtr( "Maya.2013" );
		mExportFile->mSrcFile = tFilePathPtr( MFileIO::currentFile( ).asChar( ) );
		mExportFile->mSrcFile = ToolsPaths::fMakeResRelative( mExportFile->mSrcFile );

		const MTime::Unit minUnit = MAnimControl::minTime().unit( );
		const MTime::Unit maxUnit = MAnimControl::maxTime().unit( );
		if( minUnit != maxUnit )
			log_warning( 0, "Different units for start and end of animation timeline! Not sure how this can happen." );
		mTimeUnit = minUnit;
		mExportFile->mFramesPerSecond = MayaUtil::fConvertFramesPerSecond( mTimeUnit );

		mCurrentTime = MAnimControl::currentTime( ); // save current time
		mNumRefFrames = 0; // reset, we'll count as we find them
	}

	void tMayaAnimlExporter::fExportCommonEnd( const MString& path )
	{
		mExportFile->fSaveXml( tFilePathPtr( path.asChar( ) ), true );
		mExportFile.fRelease( );

		// reset back to current
		MAnimControl::setCurrentTime( mCurrentTime );

		log_line( 0, "..............End Animl ExportAll" );
	}

	b32 tMayaAnimlExporter::fVisitNode( MDagPath& path, MObject& component )
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

	void tMayaAnimlExporter::fVisitReferenceFrame( MDagPath& path )
	{
		MDagPath rootNodeDagPath = MayaUtil::fRootBoneFromReferenceFrame( path, tExporterToolbox::fNameRootNodeName( ) );
		if( !rootNodeDagPath.isValid( ) )
		{
			log_warning( 0, "Invalid root node name specified on reference frame." );
			return;
		}

		// compute number of key frames, and animation range
		const int startTime = ( int )( MAnimControl::minTime().as( mTimeUnit ) );
		const int endTime   = ( int )( MAnimControl::maxTime().as( mTimeUnit ) );
		const u32 numKeys	= endTime - startTime + 1;
		mExportFile->mTotalFrames = numKeys;

		// get the list of maya dag paths corresponding to all the bones
		tGrowableArray< MDagPath > boneList;
		fBuildMayaBoneList( boneList, rootNodeDagPath );

		// convert the static bone data for all bones and reference frame
		fConvertStaticBoneData( mExportFile->mSkeleton.mRefFrame, path, numKeys );
		mExportFile->mSkeleton.mBones.fSetCount( boneList.fCount( ) );
		for( u32 i = 0; i < boneList.fCount( ); ++i )
		{
			mExportFile->mSkeleton.mBones[ i ].fReset( new Animl::tBone );
			fConvertStaticBoneData( *mExportFile->mSkeleton.mBones[ i ], boneList[ i ], numKeys );
		}

		// now we convert time-dependent data for all bones and reference frame
		for( int i = startTime; i <= endTime; ++i )
		{
			// set the current time
			MAnimControl::setCurrentTime( MTime( i, mTimeUnit ) );

			const u32 frameNum = i - startTime;

			fConvertDynamicBoneData( mExportFile->mSkeleton.mRefFrame, path, frameNum );
			for( u32 ibone = 0; ibone < boneList.fCount( ); ++ibone )
				fConvertDynamicBoneData( *mExportFile->mSkeleton.mBones[ ibone ], boneList[ ibone ], frameNum );
		}
	}

	void tMayaAnimlExporter::fBuildMayaBoneList( tGrowableArray< MDagPath >& bones, MDagPath& mayaRoot )
	{
		bones.fPushBack( mayaRoot );

		const u32 childCount = mayaRoot.childCount( );
		for( u32 i = 0; i < childCount; ++i )
		{
			MObject child = mayaRoot.child( i );
			if( child.apiType( ) == MFn::kJoint )
			{
				MDagPath childPath;
				MDagPath::getAPathTo( child, childPath );
				fBuildMayaBoneList( bones, childPath );
			}
		}
	}

	namespace 
	{
		const u32 gMayaRotateOrderCnt = 6;
		const std::string gMayaRotateOrders[ gMayaRotateOrderCnt ] =				{ "xyz", "yzx", "zxy", "zyx", "yxz", "xzy" };
		const Math::tEulerOrder gEulerRotateOrders[ gMayaRotateOrderCnt ] = { Math::cEulerOrderXYZ, Math::cEulerOrderYZX, Math::cEulerOrderZXY, Math::cEulerOrderZYX, Math::cEulerOrderYXZ, Math::cEulerOrderXZY };

		Math::tEulerOrder fGetEulerOrder( const std::string& order )
		{
			for( u32 i = 0; i < gMayaRotateOrderCnt; ++i )
				if( gMayaRotateOrders[ i ] == order ) return gEulerRotateOrders[ i ];

			return Math::cEulerOrderXYZ;
		}
	}

	void tMayaAnimlExporter::fConvertStaticBoneData( Animl::tBone& animlBone, MDagPath& mayaRoot, u32 numKeys )
	{
		MFnDagNode mayaNodeFn( mayaRoot );

		// get name
		animlBone.mName = mayaNodeFn.name( ).asChar( );
		animlBone.mKeyFrames.fNewArray( numKeys );

		MFnDagNode nodeFn( mayaRoot );
		MObject object = mayaRoot.node( );
		if( !MayaUtil::fGetBoolAttribute( object, nodeFn, animlBone.mExclude, tExporterToolbox::fNameExcludeBone( ) ) )
			animlBone.mExclude = false;
		if( !MayaUtil::fGetBoolAttribute( object, nodeFn, animlBone.mAdditive, tExporterToolbox::fNameAdditiveBone( ) ) )
			animlBone.mAdditive = false;
		f32 ikPriority = 0.f;
		if( MayaUtil::fGetFloatAttribute( object, nodeFn, ikPriority, tExporterToolbox::fNameIKBonePriority( ) ) )
			animlBone.mIKPriority = ( u32 )ikPriority;

		b32 useMinX=false,useMinY=false,useMinZ=false;
		b32 useMaxX=false,useMaxY=false,useMaxZ=false;
		std::string rotateOrder( "" );
		u32 roID = 0;

		MayaUtil::fGetBoolAttribute( object, nodeFn, useMinX, "minRotXLimitEnable" );
		MayaUtil::fGetBoolAttribute( object, nodeFn, useMinY, "minRotYLimitEnable" );
		MayaUtil::fGetBoolAttribute( object, nodeFn, useMinZ, "minRotZLimitEnable" );
		MayaUtil::fGetBoolAttribute( object, nodeFn, useMaxX, "maxRotXLimitEnable" );
		MayaUtil::fGetBoolAttribute( object, nodeFn, useMaxY, "maxRotYLimitEnable" );
		MayaUtil::fGetBoolAttribute( object, nodeFn, useMaxZ, "maxRotZLimitEnable" );
		MayaUtil::fGetEnumAttributeString( object, nodeFn, roID, rotateOrder, "rotateOrder" );

		animlBone.mIKRotLimitOrder = fGetEulerOrder( rotateOrder );

		if( useMinX ) MayaUtil::fGetFloatAttribute( object, nodeFn, animlBone.mIKRotLimits.mMin.x, "minRotXLimit" );
		if( useMinY ) MayaUtil::fGetFloatAttribute( object, nodeFn, animlBone.mIKRotLimits.mMin.y, "minRotYLimit" );
		if( useMinZ ) MayaUtil::fGetFloatAttribute( object, nodeFn, animlBone.mIKRotLimits.mMin.z, "minRotZLimit" );
		if( useMaxX ) MayaUtil::fGetFloatAttribute( object, nodeFn, animlBone.mIKRotLimits.mMax.x, "maxRotXLimit" );
		if( useMaxY ) MayaUtil::fGetFloatAttribute( object, nodeFn, animlBone.mIKRotLimits.mMax.y, "maxRotYLimit" );
		if( useMaxZ ) MayaUtil::fGetFloatAttribute( object, nodeFn, animlBone.mIKRotLimits.mMax.z, "maxRotZLimit" );

		//log_line( 0, animlBone.mName << ": ikPri=" << animlBone.mIKPriority << ", min=" << animlBone.mIKRotLimits.mMin << ", max=" << animlBone.mIKRotLimits.mMax << ", order=" << rotateOrder << "(" << animlBone.mIKRotLimitOrder << ")" );
	}

	void tMayaAnimlExporter::fConvertDynamicBoneData( Animl::tBone& animlBone, MDagPath& mayaRoot, u32 frameNum )
	{
		MFnDagNode mayaNodeFn( mayaRoot );

		// get xform
		sigassert( mayaRoot.hasFn( MFn::kTransform ) );
		MFnTransform xformFn( mayaRoot );

		// convert the xform
		MayaUtil::fConvertMatrix( xformFn, animlBone.mKeyFrames[ frameNum ] );
	}

}

