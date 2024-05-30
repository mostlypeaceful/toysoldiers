#include "MayaPluginPch.hpp"
#include "tExporterToolbox.hpp"
#include "Editor/tEditablePropertyTypes.hpp"
#include "tHeightFieldMeshObject.hpp"
#include "tMayaSigmlExporter.hpp"
#include "MayaUtil.hpp"
#include "WxUtil.hpp"

// we use the real default material for anything we don't recognize
#include "tDefaultMaterialGen.hpp"

// for custom sig material types
#include "tMayaDermlMaterial.hpp"
#include "tShadeMaterialGen.hpp"

// stl
#include <limits>

#include <maya\MFnPlugin.h>
#include <maya\MFloatPointArray.h>

namespace Sig
{
	///
	/// \section tSigmlMayaPlugin
	///

	b32 tSigmlMayaPlugin::fOnMayaInitialize( MObject& mayaObject, MFnPlugin& mayaPlugin )
	{
		MStatus status;

		status = mayaPlugin.registerFileTranslator("mshml", "", tMayaSigmlExporter::fCreateMeshExporter, "", "option1=1", true);
		if( !status )
		{
			status.perror("registerFileTranslator");
			return false;
		}

		status = mayaPlugin.registerFileTranslator("skin", "", tMayaSigmlExporter::fCreateSkinExporter, "", "option1=1", true);
		if( !status )
		{
			status.perror("registerFileTranslator");
			return false;
		}

		return true;
	}

	b32 tSigmlMayaPlugin::fOnMayaShutdown( MObject& mayaObject, MFnPlugin& mayaPlugin )
	{
		MStatus status;
		
		status = mayaPlugin.deregisterFileTranslator("skin");
		if( !status )
		{
			status.perror("deregisterFileTranslator");
			return false;
		}

		status = mayaPlugin.deregisterFileTranslator("mshml");
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

	namespace
	{
		///
		/// \brief Utility function to map a triangle of absolute vertex ids
		/// to a triangle of maya's "face-relative" ids (i.e., where the indices are
		/// in the range [0, numVertsOnPolygon-1]); this is because Maya doesn't always
		/// use tris.
		void fMapAbsoluteTriVertexIdsToFaceRelative( 
			MFnMesh& meshFn, 
			const u32 polygonIndex,
			const Math::tVec3u& absoluteTriVerts,
			Math::tVec3u& faceRelativeTriVerts )
		{
			MIntArray absIds;
			meshFn.getPolygonVertices( polygonIndex, absIds );

			for( u32 ivtx = 0; ivtx < 3; ++ivtx )
			{
				// invalidate current vertex id
				faceRelativeTriVerts[ ivtx ] = ~0;

				const u32 toFind = absoluteTriVerts[ ivtx ];
				for( u32 ifind = 0; ifind < absIds.length( ); ++ifind )
				{
					if( absIds[ ifind ] == toFind )
					{
						faceRelativeTriVerts[ ivtx ] = ifind;
						break;
					}
				}

				// make sure we found it
				sigassert( faceRelativeTriVerts[ ivtx ] < absIds.length( ) );
			}
		}

		///
		/// \brief Taking a fully constructed mesh, compute derivative spatial
		/// values such as bounding volumes.
		void fComputeDerivativeMeshSpatialValues( Sigml::tMesh* geoMeshPtr )
		{
			geoMeshPtr->mAabb.fInvalidate( );

			// first pass to build aabb
			for( u32 ivtx = 0; ivtx < geoMeshPtr->mVertices.fCount( ); ++ivtx )
				geoMeshPtr->mAabb |= geoMeshPtr->mVertices[ ivtx ];

			// second pass to build sphere
			geoMeshPtr->mBoundingSphere.mCenter = geoMeshPtr->mAabb.fComputeCenter( );
			geoMeshPtr->mBoundingSphere.mRadius = 0.f;
			for( u32 ivtx = 0; ivtx < geoMeshPtr->mVertices.fCount( ); ++ivtx )
			{
				const f32 d = ( geoMeshPtr->mVertices[ ivtx ] - geoMeshPtr->mBoundingSphere.mCenter ).fLengthSquared( );
				if( d > geoMeshPtr->mBoundingSphere.mRadius )
					geoMeshPtr->mBoundingSphere.mRadius = d;
			}
			geoMeshPtr->mBoundingSphere.mRadius = std::sqrt( geoMeshPtr->mBoundingSphere.mRadius );
		}

		///
		/// \brief Finds the maya shader object associated with the polygon set object.
		MObject fFindMayaShaderObject( const MObject& setNode )
		{
			// find the shading node for the given shading group set node (i.e., polygon grouping)

			MFnDependencyNode fnNode( setNode );
			MPlug shaderPlug = fnNode.findPlug( "surfaceShader" );

			if( !shaderPlug.isNull( ) )
			{
				MPlugArray connectedPlugs;

				//get all the plugs that are connected as the destination of this 
				//surfaceShader plug so we can find the surface shaderNode
				MStatus status;
				shaderPlug.connectedTo( connectedPlugs, true, false, &status );

				if( status != MStatus::kFailure && connectedPlugs.length( ) == 1 )
				{
					return connectedPlugs[ 0 ].node( );
				}
			}
			
			return MObject::kNullObj;
		}
	}

	tMayaMeshInstance::tMayaMeshInstance( 
		const MDagPath& path, 
		const MFnMesh& meshFn, 
		const MObjectArray& polygonSets, 
		const MObjectArray& polygonComponents )
		: mMeshObject( meshFn.object( ) )
	{
		mMaterialSlices.fSetCapacity( polygonSets.length( ) );

		for( u32 iset = 0; iset < polygonSets.length( ); ++iset )
		{
			// create polygon iterator
			MObject polyComponent = polygonComponents[ iset ];
			MItMeshPolygon itMeshPolygon( path, polyComponent );

			if( itMeshPolygon.count( ) == 0 )
				continue; // degenerate, no polys

			// if this is not the first set, and the number of polys is equal
			// to the total poly count of the mesh, then this is a redundant set;
			// not really sure why it gets in here (only happens sometimes :{)
			if( iset > 0 && itMeshPolygon.count( ) == meshFn.numPolygons( ) )
				continue;

			// increase material slices array by one
			mMaterialSlices.fGrowCount( 1 );

			// get the maya material/shader object
			mMaterialSlices.fBack( ).mShaderObject = fFindMayaShaderObject( polygonSets[ iset ] );

			// get all the polygon ids
			mMaterialSlices.fBack( ).mPolygonIds.fNewArray( itMeshPolygon.count( ) );
			u32 ipoly = 0;
			for( itMeshPolygon.reset( ); !itMeshPolygon.isDone( ); itMeshPolygon.next( ) )
			{
				// now iterate over all the triangles in this polygon
				const u32 polygonIndex = itMeshPolygon.index( );
				mMaterialSlices.fBack( ).mPolygonIds[ ipoly++ ] = polygonIndex;
			}
		}
	}

	b32 tMayaMeshInstance::operator==( const tMayaMeshInstance& other ) const
	{
		// do the quick checks first in hopes of earlying out ( O(k) )
		if( mMeshObject != other.mMeshObject )
			return false;
		if( mMaterialSlices.fCount( ) != other.mMaterialSlices.fCount( ) )
			return false;

		// slightly longer test  ( O(n) )
		for( u32 islice = 0; islice < mMaterialSlices.fCount( ); ++islice )
		{
			if( mMaterialSlices[ islice ].mShaderObject != other.mMaterialSlices[ islice ].mShaderObject )
				return false;
		}

		// do the longest/most expensive tests lasts  ( O(n * m) )
		for( u32 islice = 0; islice < mMaterialSlices.fCount( ); ++islice )
		{
			for( u32 ipoly = 0; ipoly < mMaterialSlices[ islice ].mPolygonIds.fCount( ); ++ipoly )
			{
				if( mMaterialSlices[ islice ].mPolygonIds[ ipoly ] != other.mMaterialSlices[ islice ].mPolygonIds[ ipoly ] )
					return false;
			}
		}

		// everything checked out, apparently we're equal
		return true;
	}

	tMayaMaterialInstance::tMayaMaterialInstance( const MObject& mtlObject )
	{
		fAddObject( mtlObject );
	}

	void tMayaMaterialInstance::fAddObject( const MObject& mtlObject )
	{
		mMaterialObjects.fPushBack( mtlObject );
	}

	b32 tMayaMaterialInstance::fHasObject( const MObject& other ) const
	{
		return mMaterialObjects.fFind( other )!=0;
	}


	void* tMayaSigmlExporter::fCreateMeshExporter( )
	{
		// if we return an actual 'new' object,
		// we apparently get a memory leak... i.e.,
		// maya never seems to deallocate the object.
		// since this class doesn't store any state, 
		// it's safe to return the same one over and over
		static tMayaSigmlExporter gExporter( cModeMesh );
		return &gExporter;
	}

	void* tMayaSigmlExporter::fCreateSkinExporter( )
	{
		// if we return an actual 'new' object,
		// we apparently get a memory leak... i.e.,
		// maya never seems to deallocate the object.
		// since this class doesn't store any state, 
		// it's safe to return the same one over and over
		static tMayaSigmlExporter gExporter( cModeSkin );
		return &gExporter;
	}

	tMayaSigmlExporter::tMayaSigmlExporter( tMode mode )
		: mMode( mode )
	{
	}

	tMayaSigmlExporter::~tMayaSigmlExporter( )
	{
	}

	MStatus	tMayaSigmlExporter::reader( const MFileObject& file, const MString& optionsString, FileAccessMode mode)
	{
		return fImport( file.expandedFullName( ) );
	}

	MStatus tMayaSigmlExporter::writer( const MFileObject& file, const MString& optionsString, FileAccessMode mode )
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

	bool tMayaSigmlExporter::haveReadMethod( ) const
	{
		// return true bcz we import sigmls
		return true;
	}

	bool tMayaSigmlExporter::haveWriteMethod( ) const
	{
		// return true bcz we export mshmls and skin files
		return true;
	}

	bool tMayaSigmlExporter::haveNamespaceSupport( ) const
	{
		// this is only necessary for importers
		return false;
	}

	bool tMayaSigmlExporter::haveReferenceMethod( ) const
	{
		// currently don't support this, but we could conceivably
		// implement referenced sigml files directly using maya api
		return false;
	}

	MString tMayaSigmlExporter::defaultExtension( ) const
	{
		return MString( "" );
	}

	MString tMayaSigmlExporter::filter( ) const
	{
		return MString( "*.mshml;*.skin" );
	}

	bool tMayaSigmlExporter::canBeOpened( ) const
	{
		// the docs say to return:
		//	* true if the class can open and import files
		//	* false if the class can only import files 
		return true;
	}

	MPxFileTranslator::MFileKind tMayaSigmlExporter::identifyFile( const MFileObject& file, const char* buffer, short size ) const
	{
		// only relevant for importer/referencer
		const MString path = file.expandedFullName( );
		if( StringUtil::fCheckExtension( path.asChar( ), ".mshml" ) )
			return kIsMyFileType; // import mshml files
		if( StringUtil::fCheckExtension( path.asChar( ), ".sigml" ) )
			return kIsMyFileType; // import sigml files
		return kNotMyFileType;
	}

	MStatus tMayaSigmlExporter::fImport( const MString& path )
	{
		log_line( 0, "Importing file: " << path.asChar( ) );

		Sigml::tFile sigmlFile;
		if( !sigmlFile.fLoadXml( tFilePathPtr( path.asChar( ) ) ) )
		{
			log_warning( "Error trying to import [" << path.asChar( ) << "] - the file either doesn't exist or is corrupt." );
			return MStatus::kFailure;
		}

		Math::tMat3f curMatrix = Math::tMat3f::cIdentity;
		return fImportSigml( sigmlFile, curMatrix );
	}

	MStatus tMayaSigmlExporter::fImportSigml( const Sigml::tFile& sigmlFile, const Math::tMat3f& curMatrix )
	{
		for( u32 i = 0; i < sigmlFile.mObjects.fCount( ); ++i )
		{
			const MStatus status = fImportObject( sigmlFile, *sigmlFile.mObjects[ i ], curMatrix );
			if( status.error( ) )
				log_warning( "Error importing object: " << status.errorString( ).asChar( ) );
		}

		return MStatus::kSuccess;
	}

	MStatus tMayaSigmlExporter::fImportObject( const Sigml::tFile& parentFile, const Sigml::tObject& sigmlObject, const Math::tMat3f& localToWorld )
	{
		if( sigmlObject.fClassId( ) == Sigml::tSigmlReferenceObject::cClassId )
		{
			const Sigml::tSigmlReferenceObject& sceneRef = static_cast<const Sigml::tSigmlReferenceObject&>( sigmlObject );
			return fImportSceneRef( parentFile, sceneRef, localToWorld );
		}
		else if( sigmlObject.fClassId( ) == Sigml::tGeomObject::cClassId )
		{
			const Sigml::tGeomObject& geomObject = static_cast<const Sigml::tGeomObject&>( sigmlObject );
			return fImportGeomObject( parentFile, geomObject, localToWorld );
		}
		else if( sigmlObject.fClassId( ) == Sigml::tHeightFieldMeshObject::cClassId )
		{
			const Sigml::tHeightFieldMeshObject& hfObject = static_cast<const Sigml::tHeightFieldMeshObject&>( sigmlObject );
			return fImportHeightField( parentFile, hfObject, localToWorld );
		}

		return MStatus::kSuccess;
	}

	MStatus tMayaSigmlExporter::fImportSceneRef( const Sigml::tFile& parentFile, const Sigml::tSigmlReferenceObject& sigmlObject, const Math::tMat3f& localToWorld )
	{
		Sigml::tFile sigmlFile;
		if( !sigmlFile.fLoadXml( ToolsPaths::fMakeResAbsolute( sigmlObject.mReferencePath ) ) )
		{
			log_warning( "Error importing sub-scene [" << sigmlObject.mReferencePath << "] - the file either doesn't exist or is corrupt." );
			return MStatus::kFailure;
		}
		const Math::tMat3f curMatrix = localToWorld * sigmlObject.mXform;
		return fImportSigml( sigmlFile, curMatrix );
	}

	MStatus tMayaSigmlExporter::fImportGeomObject( const Sigml::tFile& parentFile, const Sigml::tGeomObject& sigmlObject, const Math::tMat3f& localToWorld )
	{
		Math::tMat3f curMatrix = localToWorld * sigmlObject.mXform;
		curMatrix.fSetTranslation( 100.f * curMatrix.fGetTranslation( ) ); // * 100 bcz of SigEngine=>Maya coord difference

		const Sigml::tMesh& sigmlMesh = *parentFile.mMeshes[ sigmlObject.mMeshIndex ];

		Sigml::tTriArray sigmlMeshVertexTris;
		sigmlMesh.fComputeTriArraySubMeshUnion( sigmlMeshVertexTris );

		// array of vertices
		MFloatPointArray verts;

		// array of vertex counts for each polygon. For example the cube would have 6 faces, 
		// each of which had 4 verts, so the polygonCounts would be {4,4,4,4,4,4}.
		MIntArray polyCounts;
		
		// array of vertex connections for each polygon. For example, in the cube, we have 4 vertices for every face, 
		// so we list the vertices for face0, face1, etc consecutively in the array. These are specified by indexes in the vertexArray:
		// e.g. for the cube: { 0, 1, 2, 3, 4, 5, 6, 7, 3, 2, 6, 5, 0, 3, 5, 4,0, 4, 7, 1, 1, 7, 6, 2 } 
		MIntArray polyConnects;

		// uv coords
		MFloatArray uCoords;
		MFloatArray vCoords;

		verts.setLength( sigmlMesh.mVertices.fCount( ) );
		for( u32 i = 0; i < sigmlMesh.mVertices.fCount( ); ++i )
		{
			const Math::tVec3f v = 100.f * sigmlMesh.mVertices[ i ]; // * 100 bcz of SigEngine=>Maya coord difference
			verts[ i ] = MFloatPoint( v.x, v.y, v.z );
		}

		polyCounts.setLength( sigmlMeshVertexTris.fCount( ) );
		polyConnects.setLength( sigmlMeshVertexTris.fCount( ) * 3 );
		for( u32 i = 0; i < sigmlMeshVertexTris.fCount( ); ++i )
		{
			polyCounts[ i ] = 3;

			const u32 baseConnectIndex = i * 3;
			for( u32 j = 0; j < 3; ++j )
				polyConnects[ baseConnectIndex + j ] = sigmlMeshVertexTris[ i ].fAxis( j );
		}

		const int vertCount = verts.length( );
		const int polyCount = polyCounts.length( );

		MStatus status = MStatus::kSuccess;

		MFnMesh meshFn;
		MObject meshObj = meshFn.create( vertCount, polyCount, verts, polyCounts, polyConnects, /*uCoords, vCoords,*/ MObject::kNullObj, &status );

		const Math::tVec4f row3 = Math::tVec4f(0.f,0.f,0.f,1.f);
		float mm[4][4]={0};

		for( u32 i = 0; i < 4; ++i )
		{
			mm[ 0 ][ i ] = curMatrix( 0, i );
			mm[ 1 ][ i ] = curMatrix( 1, i );
			mm[ 2 ][ i ] = curMatrix( 2, i );
			mm[ 3 ][ i ] = row3[ i ];
		}

		MFnTransform transformFn( meshObj );
		transformFn.set( MTransformationMatrix( MMatrix( mm ).transpose( ) ) );

		if( status.error( ) )
			log_warning( "Error creating mesh node" );

		return status;
	}

	MStatus tMayaSigmlExporter::fImportHeightField( const Sigml::tFile& parentFile, const Sigml::tHeightFieldMeshObject& sigmlObject, const Math::tMat3f& localToWorld )
	{
		Math::tMat3f curMatrix = localToWorld * sigmlObject.mXform;
		curMatrix.fSetTranslation( 100.f * curMatrix.fGetTranslation( ) ); // * 100 bcz of SigEngine=>Maya coord difference

		// array of vertices
		MFloatPointArray verts;

		// array of vertex counts for each polygon. For example the cube would have 6 faces, 
		// each of which had 4 verts, so the polygonCounts would be {4,4,4,4,4,4}.
		MIntArray polyCounts;
		
		// array of vertex connections for each polygon. For example, in the cube, we have 4 vertices for every face, 
		// so we list the vertices for face0, face1, etc consecutively in the array. These are specified by indexes in the vertexArray:
		// e.g. for the cube: { 0, 1, 2, 3, 4, 5, 6, 7, 3, 2, 6, 5, 0, 3, 5, 4,0, 4, 7, 1, 1, 7, 6, 2 } 
		MIntArray polyConnects;


		tGrowableArray< Math::tVec3f > rawVerts;
		tGrowableArray< Math::tVec3u > rawTris;
		sigmlObject.fDumpRawTriangles( rawVerts, rawTris );

		verts.setLength( rawVerts.fCount( ) );
		for( u32 i = 0; i < rawVerts.fCount( ); ++i )
		{
			const Math::tVec3f v = 100.f * rawVerts[ i ]; // * 100 bcz of SigEngine=>Maya coord difference
			verts[ i ] = MFloatPoint( v.x, v.y, v.z );
		}

		polyCounts.setLength( rawTris.fCount( ) );
		polyConnects.setLength( rawTris.fCount( ) * 3 );
		for( u32 i = 0; i < rawTris.fCount( ); ++i )
		{
			polyCounts[ i ] = 3;

			const u32 baseConnectIndex = i * 3;
			for( u32 j = 0; j < 3; ++j )
				polyConnects[ baseConnectIndex + j ] = rawTris[ i ].fAxis( j );
		}

		const int vertCount = verts.length( );
		const int polyCount = polyCounts.length( );

		MStatus status = MStatus::kSuccess;

		MFnMesh meshFn;
		MObject meshObj = meshFn.create( vertCount, polyCount, verts, polyCounts, polyConnects, MObject::kNullObj, &status );

		const Math::tVec4f row3 = Math::tVec4f(0.f,0.f,0.f,1.f);
		float mm[4][4]={0};
		for( u32 i = 0; i < 4; ++i )
		{
			mm[ 0 ][ i ] = curMatrix( 0, i );
			mm[ 1 ][ i ] = curMatrix( 1, i );
			mm[ 2 ][ i ] = curMatrix( 2, i );
			mm[ 3 ][ i ] = row3[ i ];
		}

		MFnTransform transformFn( meshObj );
		transformFn.set( MTransformationMatrix( MMatrix( mm ).transpose( ) ) );

		if( status.error( ) )
			log_warning( "Error creating mesh node" );

		return status;
	}

	MStatus tMayaSigmlExporter::fExportAll( const MString& path )
	{
		fExportCommonBegin( path );
		MayaUtil::fForEachTransformNode( make_delegate_memfn( MayaUtil::tForEachObject, tMayaSigmlExporter, fVisitNode ) );
		fExportCommonEnd( path );
		return MStatus::kSuccess;
	}

	MStatus tMayaSigmlExporter::fExportSelected( const MString& path )
	{
		fExportCommonBegin( path );
		MayaUtil::fForEachSelectedNode( make_delegate_memfn( MayaUtil::tForEachObject, tMayaSigmlExporter, fVisitNode ) );
		fExportCommonEnd( path );
		return MStatus::kSuccess;
	}

	void tMayaSigmlExporter::fExportCommonBegin( const MString& path )
	{
		log_line( 0, "Begin Sigml ExportAll.............." );

		mExportFile.fReset( new Sigml::tFile( ) );
		mExportFile->mModellingPackage = tStringPtr( "Maya.2013" );
		mExportFile->mSrcFile = tFilePathPtr( MFileIO::currentFile( ).asChar( ) );
		mExportFile->mSrcFile = ToolsPaths::fMakeResRelative( mExportFile->mSrcFile );

		// build list of potential skin clusters
		fBuildPotentialSkinClusterList( );
	}

	void tMayaSigmlExporter::fExportCommonEnd( const MString& path )
	{
		mExportFile->fSaveXml( tFilePathPtr( path.asChar( ) ), true );
		mExportFile.fRelease( );
		mMeshInstances.fDeleteArray( );
		mMaterialInstances.fDeleteArray( );
		mSkinClusters.fDeleteArray( );

		log_line( 0, "..............End Sigml ExportAll" );
	}

	b32 tMayaSigmlExporter::fVisitNode( MDagPath& path, MObject& component )
	{
		MFnDagNode nodeFn( path );
		MObject object = path.node( );

		// test if the object is being ignored
		b32 ignore = false;
		const b32 found = MayaUtil::fGetBoolAttribute( object, nodeFn, ignore, tExporterToolbox::fNameIgnoreObject( ) );
		if( found && ignore )
		{
			log_line( 0, "Skipping object [" << nodeFn.name( ).asChar( ) << "] because it was specified as IGNORE." );
			return true;
		}

		if( path.hasFn( MFn::kMesh ) )
		{
			log_line( 0, "Exporting object [" << nodeFn.name( ).asChar( ) << "] of type Geometry." );
			fVisitGeometry( path, nodeFn, object );
		}
		else
		{
			log_line( 0, "Skipping object [" << nodeFn.name( ).asChar( ) << "] of type UNKNOWN." );
		}

		return true;
	}

	void tMayaSigmlExporter::fVisitObject( Sigml::tObject* sigmlObject, MDagPath& path, MFnDagNode& nodeFn, MObject& object )
	{
		// get name
		sigmlObject->mName = nodeFn.name( ).asChar( );

		// get parents' names
		for( u32 i = 0; i < nodeFn.parentCount( ); ++i )
		{
			MString name;
			if( MayaUtil::fIsTransformNode( nodeFn.parent( i ), &name ) && name != MString("world") )
			{
				// only count transform nodes, and just to be safe explicitly disallow the world node
				sigmlObject->mParentNames.fPushBack( name.asChar( ) );
			}
		}

		// get children's names
		for( u32 i = 0; i < nodeFn.childCount( ); ++i )
		{
			MString name;
			if( MayaUtil::fIsTransformNode( nodeFn.child( i ), &name ) )
			{
				// only count transform nodes
				sigmlObject->mChildNames.fPushBack( name.asChar( ) );
			}
		}

		// get xform
		sigassert( path.hasFn( MFn::kTransform ) );
		MStatus status;
		MFnTransform xformFn( path, &status );

		// convert the xform
		MayaUtil::fConvertMatrix( xformFn, sigmlObject->mXform );
	}

	void tMayaSigmlExporter::fVisitGeometry( MDagPath& path, MFnDagNode& nodeFn, MObject& mobject )
	{
		u32 dummyIndex=0;
		std::string objType;
		const b32 found = MayaUtil::fGetEnumAttributeString( mobject, nodeFn, dummyIndex, objType, tExporterToolbox::fNameGeomType( ) );
		if( !found || objType == tExporterToolbox::fNameGeomTypeNone( ) )
			return; // no type specified

		Sigml::tGeomObjectPtr object( new Sigml::tGeomObject );

		// handle common object properties
		fVisitObject( object.fGetRawPtr( ), path, nodeFn, mobject );

		// store object type
		object->mGeomType = objType;

		// store state type
		std::string stateType;
		if( MayaUtil::fGetEnumAttributeString( mobject, nodeFn, dummyIndex, stateType, tExporterToolbox::fNameStateType( ) ) )
			object->mStateType = stateType;
		
		s32 stateMask = 0;		
		if( MayaUtil::fGetIntAttribute( mobject, nodeFn, stateMask, tExporterToolbox::fNameStateMask( ) ) )
			object->mStateMask = stateMask;

		// LOD
		f32 highLodRatio = 1.0f, mediumLodRatio = 0.5f, lowLodRatio = 0.25f;
		MayaUtil::fGetFloatAttribute( mobject, nodeFn, highLodRatio, tExporterToolbox::fNameHighLodRatio( ) );
		MayaUtil::fGetFloatAttribute( mobject, nodeFn, mediumLodRatio, tExporterToolbox::fNameMediumLodRatio( ) );
		MayaUtil::fGetFloatAttribute( mobject, nodeFn, lowLodRatio, tExporterToolbox::fNameLowLodRatio( ) );

		// extract any game properties
		fAcquireGameProperties( object.fGetRawPtr( ), path, nodeFn, mobject );

		// extract the raw geometry, converting maya data to meshes, submeshes, and materials
		fAcquireObjectMesh( object.fGetRawPtr( ), highLodRatio, mediumLodRatio, lowLodRatio, path, nodeFn, mobject );

		// add object to Sigml object list
		mExportFile->mObjects.fPushBack( Sigml::tObjectPtr( object.fGetRawPtr( ) ) );
	}

	void tMayaSigmlExporter::fExtractMaterial( Sigml::tSubMesh* sigmlSubMesh, const MObject& polygonSet )
	{
		// actually get shader data from maya
		MObject shaderNode = fFindMayaShaderObject( polygonSet );

		// look for existing equivalent material
		for( u32 i = 0; i < mMaterialInstances.fCount( ); ++i )
		{
			if( mMaterialInstances[ i ]->fHasObject( shaderNode ) )
			{
				// already exists
				sigmlSubMesh->mMtlIndex = i;
				return;
			}
		}

		Sigml::tMaterialPtr geoMtl;
		const u32 newIndex = mExportFile->mMaterials.fCount( );

		if( !fConvertMaterial( geoMtl, shaderNode ) || geoMtl.fNull( ) )
		{
			geoMtl.fReset( new tDefaultMaterialGen( ) );
			geoMtl->mName = "NullMaterial";
		}

		sigassert( !geoMtl.fNull( ) );

		// before adding material to list, we now try to do a full material comparison; this
		// is a bit more expensive, but will eliminate materials that are functionally equivalent,
		// but which the artist neglected to consolidate into an actual single material instance
		for( u32 i = 0; i < mExportFile->mMaterials.fCount( ); ++i )
		{
			if( mExportFile->mMaterials[ i ]->fIsEquivalent( *geoMtl ) )
			{
				// we found a functionally equivalent material, add the shaderNode so if we
				// encounter this same node again we'll succeed in the quick test above
				mMaterialInstances[ i ]->fAddObject( shaderNode );
				sigmlSubMesh->mMtlIndex = i;
				return;
			}
		}


		// pre-condition
		sigassert( mExportFile->mMaterials.fCount( ) == mMaterialInstances.fCount( ) );

		mExportFile->mMaterials.fPushBack( geoMtl );
		mMaterialInstances.fPushBack( tMayaMaterialInstancePtr( new tMayaMaterialInstance( shaderNode ) ) );

		// post-condition
		sigassert( mExportFile->mMaterials.fCount( ) == mMaterialInstances.fCount( ) );

		sigmlSubMesh->mMtlIndex = newIndex;
	}

	b32 tMayaSigmlExporter::fConvertMaterial( Sigml::tMaterialPtr& mtlOut, MObject& shaderNode )
	{
		if( shaderNode == MObject::kNullObj )
			return false;

		// first try to convert to phong material...
		b32 success = true;
		if( !fConvertPhongBasedMaterial( mtlOut, shaderNode ) )
		{
			// wasn't a phong, try to treat it as a derml based material
			success = fConvertDermlBasedMaterial( mtlOut, shaderNode );
		}

		if( !success )
			mtlOut.fRelease( );
		return success;
	}

	b32 tMayaSigmlExporter::fConvertPhongBasedMaterial( Sigml::tMaterialPtr& mtlOut, MObject& shaderNode )
	{
		tPhongMaterialGen* phongMtl = new tPhongMaterialGen;
		mtlOut.fReset( phongMtl );
		b32 success = true;

		switch( shaderNode.apiType( ) )
		{
		case MFn::kLambert:
			{
				MFnLambertShader shaderFn( shaderNode );
				success = fConvertMayaLambertMaterial( *phongMtl, shaderFn );

				// force no spec
				phongMtl->mSpecSize = 0.f;
			}
			break;
		case MFn::kPhong:
			{
				MFnPhongShader shaderFn( shaderNode );
				const f32 cosPower = shaderFn.cosPower( );
				success = fConvertMayaLambertMaterial( *phongMtl, shaderFn );

				// convert phong spec values
				phongMtl->mSpecSize = fClamp( ( cosPower - 2.f ) / 98.f, 0.f, 1.f );
			}
			break;
		case MFn::kBlinn:
			{
				MFnBlinnShader shaderFn( shaderNode );
				const f32 eccentricity = shaderFn.eccentricity( );
				const f32 rolloff = shaderFn.specularRollOff( );
				success = fConvertMayaLambertMaterial( *phongMtl, shaderFn );

				// convert blinn spec values
				phongMtl->mSpecColor.x *= rolloff;
				phongMtl->mSpecColor.y *= rolloff;
				phongMtl->mSpecColor.z *= rolloff;
				phongMtl->mSpecSize = 1.f - eccentricity;
			}
			break;
		default:
			// not recognized as a phong-style shader
			success = false;
			break;
		}

		if( success ) // conversion was successful, so grab common material attributes
			fConvertRenderStates( *phongMtl, shaderNode );
		else
			mtlOut.fRelease( );
		return success;
	}

	b32	tMayaSigmlExporter::fConvertDermlBasedMaterial( Sigml::tMaterialPtr& mtlOut, MObject& shaderNode )
	{
		// see if this material is one of our custom "derml" material
		tMayaDermlMaterial* mayaDermlMtl = tMayaDermlMaterial::fFromMayaObject( shaderNode );
		if( !mayaDermlMtl )
			return false; // unrecognized

		tShadeMaterialGen* shadeMtl = new tShadeMaterialGen;
		mtlOut.fReset( shadeMtl );

		Derml::tMtlFile mtlFile;
		mayaDermlMtl->fCloneMaterialFile( mtlFile );
		if( !shadeMtl->fFromDermlMtlFile( mtlFile ) )
			mtlOut.fRelease( );

		fConvertRenderStates( *shadeMtl, shaderNode );

		return !mtlOut.fNull( );
	}

	void tMayaSigmlExporter::fConvertRenderStates( Sigml::tMaterialRenderOptions& mtlOut, MObject& shaderNode )
	{
		MFnDependencyNode shaderNodeFn( shaderNode );

		f32 tmp=0.f;
		MayaUtil::fGetBoolAttribute( shaderNode, shaderNodeFn, mtlOut.mTwoSided, tExporterToolbox::fNameTwoSided( ) );
		MayaUtil::fGetBoolAttribute( shaderNode, shaderNodeFn, mtlOut.mFlipBackFaceNormal, tExporterToolbox::fNameFlipBackFaceNormal( ) );
		MayaUtil::fGetBoolAttribute( shaderNode, shaderNodeFn, mtlOut.mTransparency, tExporterToolbox::fNameTransparency( ) );
		MayaUtil::fGetFloatAttribute( shaderNode, shaderNodeFn, tmp, tExporterToolbox::fNameAlphaCutOut( ) );
		mtlOut.mAlphaCutOut = ( u32 )tmp;
		MayaUtil::fGetBoolAttribute( shaderNode, shaderNodeFn, mtlOut.mAdditive, tExporterToolbox::fNameAdditive( ) );
		MayaUtil::fGetEnumAttribute( shaderNode, shaderNodeFn, mtlOut.mZBufferTest, tExporterToolbox::fNameZBufferTest( ) );
		MayaUtil::fGetEnumAttribute( shaderNode, shaderNodeFn, mtlOut.mZBufferWrite, tExporterToolbox::fNameZBufferWrite( ) );
		MayaUtil::fGetBoolAttribute( shaderNode, shaderNodeFn, mtlOut.mFaceX, tExporterToolbox::fNameFaceX( ) );
		MayaUtil::fGetBoolAttribute( shaderNode, shaderNodeFn, mtlOut.mFaceY, tExporterToolbox::fNameFaceY( ) );
		MayaUtil::fGetBoolAttribute( shaderNode, shaderNodeFn, mtlOut.mFaceZ, tExporterToolbox::fNameFaceZ( ) );
		MayaUtil::fGetBoolAttribute( shaderNode, shaderNodeFn, mtlOut.mXparentDepthPrepass, tExporterToolbox::fNameXparentDepthPrepass( ) );
		MayaUtil::fGetFloatAttribute( shaderNode, shaderNodeFn, mtlOut.mSortOffset, tExporterToolbox::fNameSortOffset( ) );
	}

	b32 tMayaSigmlExporter::fConvertMayaLambertMaterial( tPhongMaterialGen& phongMtl, MFnLambertShader& shaderFn )
	{
		phongMtl.mName = shaderFn.name( ).asChar( );
		fConvertMayaColorPlug( shaderFn, "color", phongMtl.mDiffuseMapPath, phongMtl.mDiffuseColor, phongMtl.mDiffuseUvParams );
		fConvertMayaColorPlug( shaderFn, "specularColor", phongMtl.mSpecColorMapPath, phongMtl.mSpecColor, phongMtl.mSpecColorUvParams );
		fConvertMayaColorPlug( shaderFn, "incandescence", phongMtl.mEmissiveMapPath, phongMtl.mEmissiveColor, phongMtl.mEmissiveUvParams );

		fConvertMayaColorPlug( shaderFn, "transparency", phongMtl.mOpacityMapPath, phongMtl.mOpacityColor, phongMtl.mOpacityUvParams );
		if( phongMtl.mOpacityMapPath.fLength( ) == 0 )
		{
			phongMtl.mOpacityColor.x = 1.f - phongMtl.mOpacityColor.x;
			phongMtl.mOpacityColor.y = 1.f - phongMtl.mOpacityColor.y;
			phongMtl.mOpacityColor.z = 1.f - phongMtl.mOpacityColor.z;
		}

		fConvertMayaBumpMap( shaderFn, phongMtl.mNormalMapPath, phongMtl.mBumpDepth, phongMtl.mNormalUvParams );
		return true;
	}

	void tMayaSigmlExporter::fConvertMayaColorPlug( 
		MFnLambertShader& shaderFn, 
		const char* colorName, 
		tFilePathPtr& texPath, 
		Math::tVec4f& color,
		tPhongMaterialGen::tUvParameters& uvParams )
	{
		MStatus status = MS::kSuccess;

		MPlug colorPlug = shaderFn.findPlug( colorName, &status );
		if( status == MS::kFailure )
			return;

		MItDependencyGraph itDG(colorPlug, MFn::kFileTexture,
								MItDependencyGraph::kUpstream, 
								MItDependencyGraph::kBreadthFirst,
								MItDependencyGraph::kNodeLevel, 
								&status);

		if( status == MS::kFailure )
			return;

		//disable automatic pruning so that we can locate a specific plug 
		itDG.disablePruningOnFilter();

		// retrieve the filename
		if( !itDG.isDone( ) )
		{
			MObject textureNode = itDG.thisNode( );
			MFnDependencyNode texNodeFn( textureNode );
			MPlug filenamePlug = texNodeFn.findPlug( "fileTextureName" );
			MString textureName("");
			filenamePlug.getValue( textureName );
			texPath = ToolsPaths::fMakeResRelative( tFilePathPtr( textureName.asChar( ) ) );
			fConvertUvParameters( texNodeFn, uvParams.mRepeatUv, uvParams.mOffsetUv, uvParams.mMirrorUv, uvParams.mWrapUv, uvParams.mUvSetName );
			color = Math::tVec4f(1.f);
		}
		else
		{
			MPlug p;

			MString r = colorName;
			r += "R";
			MString g = colorName;
			g += "G";
			MString b = colorName;
			b += "B";
			MString a = colorName;
			a += "A";

			// get the color value
			p = shaderFn.findPlug(r.asChar());
			p.getValue(color.x);
			p = shaderFn.findPlug(g.asChar());
			p.getValue(color.y);
			p = shaderFn.findPlug(b.asChar());
			p.getValue(color.z);
			p = shaderFn.findPlug(a.asChar());
			p.getValue(color.w);
		}
	}

	void tMayaSigmlExporter::fConvertMayaBumpMap( 
		MFnLambertShader& shaderFn, 
		tFilePathPtr& texPath, 
		f32& bumpDepthFloat,
		tPhongMaterialGen::tUvParameters& uvParams )
	{
		bumpDepthFloat = 0.f;

		// get a plug to the normalCamera attribute on the material
		MPlug bumpPlug = shaderFn.findPlug("normalCamera");

		// get connections to the attribute
		MPlugArray connections;
		bumpPlug.connectedTo( connections, true, false );

		// loop through each one to find a bump2d node
		for( u32 i = 0; i < connections.length( ); ++i )
		{
			if( connections[ i ].node().apiType( ) != MFn::kBump )
				continue;

			// attach a function set to the 2d bump node
			MFnDependencyNode fnBump( connections[ i ].node( ) );

			// get the bump depth value from the node
			MPlug bumpDepth = fnBump.findPlug( "bumpDepth" );
			bumpDepth.getValue( bumpDepthFloat );

			// we now have the fun and joy of actually finding
			// the file node that is connected to the bump map
			// node itself. This is going to involve checking
			// the attribute connections to the bumpValue attribute.
			MPlug bumpValue = fnBump.findPlug("bumpValue");
			MPlugArray bumpValConnections;

			bumpValue.connectedTo( bumpValConnections, true, false );
			for( u32 j = 0; j < bumpValConnections.length( ); ++j )
			{
				if( bumpValConnections[ i ].node( ).apiType( ) != MFn::kFileTexture )
					continue;

				// retrieve the filename
				MFnDependencyNode texNodeFn( bumpValConnections[ i ].node( ) );
				MPlug filenamePlug = texNodeFn.findPlug( "fileTextureName" );
				MString textureName("");
				if( filenamePlug.getValue( textureName ) == MStatus::kFailure )
					continue; // failed to get bump map file name, skip

				texPath = ToolsPaths::fMakeResRelative( tFilePathPtr( textureName.asChar( ) ) );
				fConvertUvParameters( texNodeFn, uvParams.mRepeatUv, uvParams.mOffsetUv, uvParams.mMirrorUv, uvParams.mWrapUv, uvParams.mUvSetName );
				return;
			}
		}
	}

	void tMayaSigmlExporter::fConvertUvParameters( 
		MFnDependencyNode& texNodeFn, 
		Math::tVec2f& repeatUv, 
		Math::tVec2f& offsetUv, 
		Math::tVec2u& mirrorUv, 
		Math::tVec2u& wrapUv,
		std::string& uvSetName )
	{
		// TODOHACK get the uv set name.
		uvSetName = ""; // make it default to the first/default uv set

		MPlug plg;
		bool b;

		// UV mirroring
		plg = texNodeFn.findPlug("mu");
		plg.getValue( b ); mirrorUv.x = (b!=0);
		plg = texNodeFn.findPlug("mv");
		plg.getValue( b ); mirrorUv.y = (b!=0);

		// UV wrapping
		plg = texNodeFn.findPlug("wu");
		plg.getValue( b ); wrapUv.x = (b!=0);
		plg = texNodeFn.findPlug("wv");
		plg.getValue( b ); wrapUv.y = (b!=0);

		// The UV repeat. Determines how many times texture
		// repeats in the given u and v directions
		plg = texNodeFn.findPlug("reu");
		plg.getValue( repeatUv.x );
		plg = texNodeFn.findPlug("rev");
		plg.getValue( repeatUv.y );

		// The UV offset. Determines the offset of the texture
		// from the 0,0 u and v texture origin.
		plg = texNodeFn.findPlug("ofu");
		plg.getValue( offsetUv.x );
		plg = texNodeFn.findPlug("ofv");
		plg.getValue( offsetUv.y );
	}

	void tMayaSigmlExporter::fAcquireGameProperties( Sigml::tGeomObject* sigmlObject, MDagPath& path, MFnDagNode& nodeFn, MObject& mobject )
	{
		// Store game flags
		s32 flagCount = 0;
		if( MayaUtil::fGetIntAttribute( mobject, nodeFn, flagCount, tExporterToolbox::fNameGameTags( ) ) )
		{
			const tProjectFile & projectFile = tProjectFile::fInstance( );

			for( s32 i = 0; i < flagCount; ++i )
			{
				std::stringstream ss;
				ss << tExporterToolbox::fNameGameTags( ) << i;
				std::string longName = ss.str( );

				std::string flag;
				if( MayaUtil::fGetStringAttribute( mobject, nodeFn, flag, longName.c_str( ) ) )
				{
					if( const tProjectFile::tGameTag * tag = projectFile.fFindTagByName( flag ) )
					{
						std::stringstream ss;
						ss << Sigml::tObject::fEditablePropGameTagName( ) << tag->mKey;
						sigmlObject->mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyProjectFileTag( ss.str( ), false ) ) );
					}
				}
			}
		}

		// TODO REFACTOR

		//u32 ithGameAttribute;

		//ithGameAttribute = 0;
		//while( true )
		//{
		//	std::string gamePropName = StringUtil::fAppend( "GameString", ithGameAttribute++ );

		//	std::string str;
		//	if( !MayaUtil::fGetStringAttribute( mobject, nodeFn, str, gamePropName.c_str( ) ) )
		//		break;

		//	std::string strName = str, strValue="";

		//	const char* equals = StringUtil::fStrStrI( str.c_str( ), "=" );
		//	if( equals )
		//	{
		//		strName = StringUtil::fEatWhiteSpace( std::string( str.c_str( ), equals ).c_str( ) );
		//		strValue = StringUtil::fEatWhiteSpace( std::string( equals + 1, str.c_str( ) + str.length( ) ).c_str( ) );
		//	}

		//	sigmlObject->mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyString( Sigml::tObjectProperties::fEditablePropGamePropsName( ) + strName, strValue ) ) );
		//}
	}

	void tMayaSigmlExporter::fAcquireObjectMesh( 
		Sigml::tGeomObject* sigmlObject, 
		f32 highLodRatio,
		f32 mediumLodRatio,
		f32 lowLodRatio, 
		MDagPath& path, 
		MFnDagNode& nodeFn, 
		MObject& mobject )
	{
		sigassert( fInBounds( highLodRatio, 0.f, 1.f ) );
		sigassert( fInBounds( mediumLodRatio, 0.f, 1.f ) );
		sigassert( fInBounds( lowLodRatio, 0.f, 1.f ) );

		sigassert( path.hasFn( MFn::kMesh ) );
		MFnMesh meshFn( path );

		// Have to make the path include the shape below it so that
		// we can determine if the underlying shape node is instanced.
		// By default, dag paths only include transform nodes.
		path.extendToShape( );

		// If the shape is instanced then we need to determine which
		// instance this path refers to.
		const b32 isInstanced = path.isInstanced( )!=0;
		const u32 instanceNum = isInstanced ? path.instanceNumber( ) : 0;

		// Get the connected sets and members - these will be used to determine per-face material groupings.
		MObjectArray polygonSets;
		MObjectArray polygonComponents;
		meshFn.getConnectedSetsAndMembers( instanceNum, polygonSets, polygonComponents, true );
		sigassert( polygonSets.length( ) == polygonComponents.length( ) );

		// before we convert to our sigml mesh format, we need to do the overly-complex task of
		// determining whether this is an instanced mesh, and whether we've already added
		// this particular instanced mesh. The trick here is that "instanced" means not
		// only that the geometry is the same (which is easy to detect), but also that
		// the material groupings (read: submeshes, materials, index buffers) on this
		// mesh are the same...

		// first we create our "maya mesh instance" object which encapsulates all the messiness
		tMayaMeshInstancePtr mayaMeshInstance( new tMayaMeshInstance( path, meshFn, polygonSets, polygonComponents ) );

		// now we look and see if we have an equivalent mesh instance
		for( u32 imeshinst = 0; imeshinst < mMeshInstances.fCount( ); ++imeshinst )
		{
			if( *mMeshInstances[ imeshinst ] == *mayaMeshInstance )
			{
				// Correct the mesh instance to use the lowest lod 0 target ( Prefer more geometry )
				// We do this because the geom Maya objects are where the targets are stored
				// but they may be instancing meshes
				if( mExportFile->mMeshes[ imeshinst ]->mHighLodRatio != highLodRatio )
					log_warning( "Different high Lod ratio value! " << mExportFile->mMeshes[ imeshinst ]->mHighLodRatio << " will now become " << highLodRatio );
				mExportFile->mMeshes[ imeshinst ]->mHighLodRatio = highLodRatio;

				if( mExportFile->mMeshes[ imeshinst ]->mMediumLodRatio != mediumLodRatio )
					log_warning( "Different medium Lod ratio value! " << mExportFile->mMeshes[ imeshinst ]->mMediumLodRatio << " will now become " << mediumLodRatio );
				mExportFile->mMeshes[ imeshinst ]->mMediumLodRatio = mediumLodRatio;

				if( mExportFile->mMeshes[ imeshinst ]->mLowLodRatio != lowLodRatio )
					log_warning( "Different low Lod ratio value! " << mExportFile->mMeshes[ imeshinst ]->mLowLodRatio << " will now become " << lowLodRatio );
				mExportFile->mMeshes[ imeshinst ]->mLowLodRatio = lowLodRatio;

				// we've got a match, early out
				sigmlObject->mMeshIndex = imeshinst;
				mayaMeshInstance.fRelease( );
				break;
			}
		}

		if( !mayaMeshInstance.fNull( ) )
		{
			// pre-condition (1 to 1 correspondence required)
			sigassert( mMeshInstances.fCount( ) == mExportFile->mMeshes.fCount( ) );

			// we didn't find an existing mesh instance, so we need to create a new one
			Sigml::tMeshPtr geoMeshPtr = fExtractGeometry( path, meshFn, polygonSets, polygonComponents );
			
			// Apply the lod targets from the first referenced geometry
			geoMeshPtr->mHighLodRatio = highLodRatio;
			geoMeshPtr->mMediumLodRatio = mediumLodRatio;
			geoMeshPtr->mLowLodRatio = lowLodRatio;

			// store the sigml mesh on the sigml file object
			mExportFile->mMeshes.fPushBack( geoMeshPtr );

			// store Maya mesh instance pointer so we can detect later whether we have an equivalent mesh
			mMeshInstances.fPushBack( mayaMeshInstance );

			// post-condition (1 to 1 correspondence required)
			sigassert( mMeshInstances.fCount( ) == mExportFile->mMeshes.fCount( ) );

			// store the index of the mesh we just added
			sigmlObject->mMeshIndex = mExportFile->mMeshes.fCount( ) - 1;
		}
	}

	Sigml::tMeshPtr tMayaSigmlExporter::fExtractGeometry( 
			MDagPath& path, 
			MFnMesh& meshFn, 
			const MObjectArray& polygonSets, 
			const MObjectArray& polygonComponents )
	{
		Sigml::tMeshPtr geoMeshPtr( new Sigml::tMesh( ) );

		// get skin (may return a null skin ptr if no skin)
		geoMeshPtr->mSkin = fBuildSkin( path, meshFn );

		fExtractGeometry( geoMeshPtr.fGetRawPtr( ), path, meshFn, polygonSets, polygonComponents );

		return geoMeshPtr;
	}

	void tMayaSigmlExporter::fExtractGeometry( 
			Sigml::tMesh* geoMeshPtr,
			MDagPath& path, 
			MFnMesh& meshFn, 
			const MObjectArray& polygonSets, 
			const MObjectArray& polygonComponents )
	{
		// get current uv set name; this is used for extracting tangents and binormals
		MString currentUvSetName, currentColorSetName;
		meshFn.getCurrentUVSetName( currentUvSetName );
		meshFn.getCurrentColorSetName( currentColorSetName );

		// extract geometric surface data (verts, normals, tangents, binormals)
		MPointArray			mverts;
		MFloatVectorArray	mnormals;
		MFloatVectorArray	mtangents;
		MFloatVectorArray	mbinormals;
		meshFn.getPoints(	mverts,		MSpace::kObject );
		meshFn.getNormals(	mnormals,	MSpace::kObject );
		meshFn.getTangents(	mtangents,	MSpace::kObject, &currentUvSetName );
		meshFn.getBinormals(mbinormals,	MSpace::kObject, &currentUvSetName );

		// pass geometric surface data to sigml mesh representation
		geoMeshPtr->mVertices.fSetCount( mverts.length( ) );
		for( u32 i = 0; i < mverts.length( ); ++i )
			geoMeshPtr->mVertices[ i ] = Math::tVec3f( mverts[i].x, mverts[i].y, mverts[i].z ) / 100.f; // convert cm -> m
		geoMeshPtr->mNormals.fSetCount( mnormals.length( ) );
		for( u32 i = 0; i < mnormals.length( ); ++i )
			geoMeshPtr->mNormals[ i ] = Math::tVec3f( mnormals[i].x, mnormals[i].y, mnormals[i].z ).fNormalizeSafe( );
		geoMeshPtr->mTangents.fSetCount( mtangents.length( ) );
		for( u32 i = 0; i < mtangents.length( ); ++i )
			geoMeshPtr->mTangents[ i ] = Math::tVec3f( mtangents[i].x, mtangents[i].y, mtangents[i].z ).fNormalizeSafe( );
		geoMeshPtr->mBinormals.fSetCount( mbinormals.length( ) );
		for( u32 i = 0; i < mbinormals.length( ); ++i )
			geoMeshPtr->mBinormals[ i ] = Math::tVec3f( mbinormals[i].x, mbinormals[i].y, mbinormals[i].z ).fNormalizeSafe( );

		// get all the uv sets from maya and simultaneously pass it to the sigml mesh
		MStringArray muvsetnames;
		meshFn.getUVSetNames( muvsetnames );
		geoMeshPtr->mUvwSetVerts.fSetCount( muvsetnames.length( ) );
		for( u32 iuvset = 0; iuvset < muvsetnames.length( ); ++iuvset )
		{
			// set whether this is the "default" set
			geoMeshPtr->mUvwSetVerts[ iuvset ].mDefault = ( muvsetnames[ iuvset ] == currentUvSetName )!=0;

			// copy the set name
			geoMeshPtr->mUvwSetVerts[ iuvset ].mSetName = muvsetnames[ iuvset ].asChar( );

			// extract the actual uv verts
			MFloatArray mus, mvs;
			meshFn.getUVs( mus, mvs, &muvsetnames[ iuvset ] );

			// better be as many Us as there are Vs
			sigassert( mus.length( ) == mvs.length( ) );

			// copy to sigml set
			geoMeshPtr->mUvwSetVerts[ iuvset ].mUvws.fSetCount( mus.length( ) );
			for( u32 iuv = 0; iuv < mus.length( ); ++iuv )
				geoMeshPtr->mUvwSetVerts[ iuvset ].mUvws[ iuv ] = Math::tVec3f( mus[ iuv ], mvs[ iuv ], 0.f );
		}

		// get all the color sets from maya and simultaneously pass it to the sigml mesh
		MStringArray mcolorsetnames;
		meshFn.getColorSetNames( mcolorsetnames );
		geoMeshPtr->mRgbaSetVerts.fSetCount( mcolorsetnames.length( ) );
		for( u32 icolorset = 0; icolorset < mcolorsetnames.length( ); ++icolorset )
		{
			// set whether this is the "default" set
			geoMeshPtr->mRgbaSetVerts[ icolorset ].mDefault = ( mcolorsetnames[ icolorset ] == currentColorSetName )!=0;

			// copy the set name
			geoMeshPtr->mRgbaSetVerts[ icolorset ].mSetName = mcolorsetnames[ icolorset ].asChar( );

			// extract the actual uv verts
			MColorArray mcolors;
			MColor mdefcolor( 1.f, 1.f, 1.f, 1.f );
			meshFn.getFaceVertexColors( mcolors, &mcolorsetnames[ icolorset ], &mdefcolor );

			// copy to sigml set
			geoMeshPtr->mRgbaSetVerts[ icolorset ].mRgbas.fSetCount( mcolors.length( ) );
			for( u32 icolor = 0; icolor < mcolors.length( ); ++icolor )
			{
				geoMeshPtr->mRgbaSetVerts[ icolorset ].mRgbas[ icolor ] = 
					Math::tVec4f( mcolors[ icolor ].r, mcolors[ icolor ].g, mcolors[ icolor ].b, mcolors[ icolor ].a );
			}
		}

		// extract polygon and triangle information
		MIntArray			mtriCounts;
		MIntArray			mtriVerts;
		meshFn.getTriangles(mtriCounts, mtriVerts );

		// iterate through the "sets"; these are really material groupings
		for( u32 iset = 0; iset < polygonSets.length( ); ++iset )
		{
			// create polygon iterator
			MObject polyComponent = polygonComponents[ iset ];
			MItMeshPolygon itMeshPolygon( path, polyComponent );

			// check for a degenerate mesh (no polys) and skip if it is
			if( itMeshPolygon.count( ) == 0 )
				continue;

			// if this is not the first set, and the number of polys is equal
			// to the total poly count of the mesh, then this is a redundant set;
			// not really sure why it gets in here (only happens sometimes :{)
			if( iset > 0 && itMeshPolygon.count( ) == meshFn.numPolygons( ) )
				continue;

			// create sigml submesh object, and start its triangle capacities with a reasonable value (to avoid lots of allocs)
			Sigml::tSubMeshPtr geoSubMeshPtr( new Sigml::tSubMesh );
			geoSubMeshPtr->mVertexTris.fSetCapacity( 2*itMeshPolygon.count( ) );
			geoSubMeshPtr->mNormalTris.fSetCapacity( 2*itMeshPolygon.count( ) );
			geoSubMeshPtr->mTangentBinormalTris.fSetCapacity( 2*itMeshPolygon.count( ) );
			geoSubMeshPtr->mUvwSetTris.fSetCount( geoMeshPtr->mUvwSetVerts.fCount( ) );
			for( u32 iuvset = 0; iuvset < geoSubMeshPtr->mUvwSetTris.fCount( ); ++iuvset )
			{
				geoSubMeshPtr->mUvwSetTris[ iuvset ].mSetName = geoMeshPtr->mUvwSetVerts[ iuvset ].mSetName;
				geoSubMeshPtr->mUvwSetTris[ iuvset ].mUvwTris.fSetCapacity( 2*itMeshPolygon.count( ) );
			}
			geoSubMeshPtr->mRgbaSetTris.fSetCount( geoMeshPtr->mRgbaSetVerts.fCount( ) );
			for( u32 icolorset = 0; icolorset < geoSubMeshPtr->mRgbaSetTris.fCount( ); ++icolorset )
			{
				geoSubMeshPtr->mRgbaSetTris[ icolorset ].mSetName = geoMeshPtr->mRgbaSetVerts[ icolorset ].mSetName;
				geoSubMeshPtr->mRgbaSetTris[ icolorset ].mRgbaTris.fSetCapacity( 2*itMeshPolygon.count( ) );
			}

			// iterate over every polygon in the mesh
			for( itMeshPolygon.reset( ); !itMeshPolygon.isDone( ); itMeshPolygon.next( ) )
			{
				// now iterate over all the triangles in this polygon
				const s32 polygonIndex = itMeshPolygon.index( );
				const u32 numTrisForPoly = mtriCounts[ polygonIndex ];
				for( u32 itri = 0; itri < numTrisForPoly; ++itri )
				{
					// get the triangle vertex indices
					Math::tVec3u triVertexIds;
					meshFn.getPolygonTriangleVertices( polygonIndex, itri, (s32*)&triVertexIds[0] );

					// add this triangle-vertex-triple to our submesh; 
					// these indices point into the parent mesh's vertex array
					geoSubMeshPtr->mVertexTris.fPushBack( triVertexIds );

					// backwards map the absolute vertex ids to face-relative vertex ids (i.e., [0,numVertsOnPoly-1];
					// we need this to lookup the ids of the normals/uvs/colors
					Math::tVec3u triFaceRelativeVertexIds;
					fMapAbsoluteTriVertexIdsToFaceRelative( meshFn, polygonIndex, triVertexIds, triFaceRelativeVertexIds );

					// now, given the vertex indices, we can get the normal indices
					Math::tVec3u triNormalIds;
					MIntArray mnormalids;
					meshFn.getFaceNormalIds( polygonIndex, mnormalids );
					for( u32 ivtx = 0; ivtx < 3; ++ivtx )
						triNormalIds[ivtx] = mnormalids[ triFaceRelativeVertexIds[ ivtx ] ];

					// add this triangle-normal-triple to our submesh;
					// these indices point into the parent mesh's normal array;
					geoSubMeshPtr->mNormalTris.fPushBack( triNormalIds );

					Math::tVec3u triTangentBinormalIds;
					for( u32 ivtx = 0; ivtx < 3; ++ivtx )
						triTangentBinormalIds[ivtx] = meshFn.getTangentId( polygonIndex, triVertexIds[ ivtx ] );

					// add this triangle-tangent/binormal-triple to our submesh;
					// these indices point into the parent mesh's tangent and binormal array;
					geoSubMeshPtr->mTangentBinormalTris.fPushBack( triTangentBinormalIds );

					// get the Uvw tris for each Uvw set
					for( u32 iuvset = 0; iuvset < geoSubMeshPtr->mUvwSetTris.fCount( ); ++iuvset )
					{
						Math::tVec3u triUvwIds;
						for( u32 ivtx = 0; ivtx < 3; ++ivtx )
						{
							MString muvsetname = geoSubMeshPtr->mUvwSetTris[ iuvset ].mSetName.c_str( );
							meshFn.getPolygonUVid( 
								polygonIndex, 
								triFaceRelativeVertexIds[ ivtx ], 
								reinterpret_cast<s32&>( triUvwIds[ ivtx ] ), 
								&muvsetname );
						}
						geoSubMeshPtr->mUvwSetTris[ iuvset ].mUvwTris.fPushBack( triUvwIds );
					}

					// get the Rgba tris for each Rgba set
					for( u32 icolorset = 0; icolorset < geoSubMeshPtr->mRgbaSetTris.fCount( ); ++icolorset )
					{
						Math::tVec3u triRgbaIds;
						for( u32 ivtx = 0; ivtx < 3; ++ivtx )
						{
							MString mcolorsetname = geoSubMeshPtr->mRgbaSetTris[ icolorset ].mSetName.c_str( );
							meshFn.getFaceVertexColorIndex( 
								polygonIndex, 
								triFaceRelativeVertexIds[ ivtx ], 
								reinterpret_cast<s32&>( triRgbaIds[ ivtx ] ), 
								&mcolorsetname );
						}
						geoSubMeshPtr->mRgbaSetTris[ icolorset ].mRgbaTris.fPushBack( triRgbaIds );
					}
				}
			}

			// now that we've got all the triangles for this sub-mesh,
			// we need to extract the material information
			fExtractMaterial( geoSubMeshPtr.fGetRawPtr( ), polygonSets[iset] );

			// store this submesh on the mesh
			geoMeshPtr->mSubMeshes.fPushBack( geoSubMeshPtr );
		}

		fComputeDerivativeMeshSpatialValues( geoMeshPtr );
	}

	Sigml::tSkinPtr tMayaSigmlExporter::fBuildSkin( MDagPath& path, MFnMesh& meshFn )
	{
		// search for a skin cluster corresponding to this mesh
		MObject skinObj = fFindSkinCluster( meshFn );
		if( skinObj.isNull( ) )
			return Sigml::tSkinPtr( );

		log_line( 0, "Exporting skin weights for mesh [" << meshFn.name( ).asChar( ) << "]" );

		// the mesh is skinned, so retrieve the skinning data...
		MFnSkinCluster fnSkin( skinObj );

		// allocate skin object
		Sigml::tSkinPtr sigmlSkin( new Sigml::tSkin( ) );

		// get the list of joints affecting the mesh, and store off their names
		MDagPathArray joints;
		fnSkin.influenceObjects( joints );
		sigmlSkin->mBoneNames.fNewArray( joints.length( ) );
		for( u32 i = 0; i < joints.length( ); ++i )
			sigmlSkin->mBoneNames[ i ] = joints[ i ].partialPathName( ).asChar( );

		const u32 userBoneCount = sigmlSkin->mBoneNames.fCount( );
		if( userBoneCount > Gfx::tMaterial::cMaxBoneCount )
			log_warning( "Skin contains more bones than is allowed: your bone count (" << userBoneCount << "), max bone count (" << Gfx::tMaterial::cMaxBoneCount << ")" );
		
		// iterate through each vertex in the mesh, and retrieve the influences for each one (bone index, bone weight)
		MItGeometry iVertex( path );
		const u32 numVerts = iVertex.count( );
		sigmlSkin->mVertices.fNewArray( numVerts );
		for( u32 i = 0; i < numVerts; ++i, iVertex.next( ) )
		{
			sigassert( !iVertex.isDone( ) );

			MFloatArray wgts;
			u32 influenceCount = 0;
			fnSkin.getWeights( path, iVertex.currentItem( ), wgts, influenceCount );

			const u32 wgtsLen = wgts.length( );
			if( wgtsLen != joints.length( ) )
				log_warning( "wgtsLen = " << wgts.length( ) << ", joints.length( ) = " << joints.length( ) );

			for( u32 iw = 0; iw < wgtsLen; ++iw )
				if( wgts[ iw ] > 0.f ) // only store weights that are relevant (non-zero)
					sigmlSkin->mVertices[ i ].fPushBack( Sigml::tSkin::tWeight( iw, wgts[ iw ] ) );
		}

		return sigmlSkin;
	}

	void tMayaSigmlExporter::fBuildPotentialSkinClusterList( )
	{
		mSkinClusters.fSetCount( 0 );
		for( MItDependencyNodes itDep( MFn::kInvalid ); !itDep.isDone( ); itDep.next( ) )
		{
			MObject item = itDep.item();
			if( item.apiType( ) == MFn::kSkinClusterFilter )
				mSkinClusters.fPushBack( item );
		}

		log_line( 0, "Found [" << mSkinClusters.fCount( ) << "] skin clusters" );
	}

	MObject tMayaSigmlExporter::fFindSkinCluster( MFnMesh& fnMesh )
	{
		for( u32 i = 0; i < mSkinClusters.fCount( ); ++i )
		{
			MFnSkinCluster skin( mSkinClusters[ i ] );
			MPlug outputPlug = skin.findPlug( "outputGeometry" );

			if( outputPlug.numElements( ) == 0 )
			{
				log_warning( "found a skin cluster with no outputs [" << skin.name( ).asChar( ) << "]" );
				continue; // hmm, no outputs
			}

			MItDependencyGraph itDep( outputPlug.elementByPhysicalIndex( 0 ), MFn::kMesh, MItDependencyGraph::kDownstream );

			MFnMesh test( itDep.thisNode( ) );
			if( test.object( ) == fnMesh.object( ) )
				return mSkinClusters[ i ];
		}

		return MObject( );
	}


}

