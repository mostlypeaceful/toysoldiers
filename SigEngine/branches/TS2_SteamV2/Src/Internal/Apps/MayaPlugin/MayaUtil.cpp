#include "MayaPluginPch.hpp"
#include "MayaUtil.hpp"
#include "Maya/MFnTypedAttribute.h"

namespace Sig { namespace MayaUtil
{
	MDagPath fFindNodeByName( const char* searchName, MFn::Type type )
	{
		MItDag itDag( MItDag::kBreadthFirst, type );

		for( ; !itDag.isDone( ); itDag.next( ) )
		{
			MDagPath path;
			itDag.getPath( path );
			MString name = MFnDagNode( path ).name( );
			if( _stricmp( name.asChar( ), searchName ) == 0 )
				return path;
		}

		return MDagPath( );
	}

	MDagPath fGetRootNode( )
	{
		return fFindNodeByName( "world", MFn::kInvalid );
	}

	MDagPath fGetSceneDataNode( b32 createIfNotFound )
	{
		const char* nodeName = "SigEngineSceneExportNode";

		MDagPath o = fFindNodeByName( nodeName, MFn::kInvalid );
		if( o.isValid( ) )
			return o;

		if( createIfNotFound )
		{
			MString createGroupNodeCommand = MString( "group -em -w -n " + MString( nodeName ) );
			MGlobal::executeCommand( createGroupNodeCommand );

			o = fFindNodeByName( nodeName, MFn::kInvalid );
			sigassert( o.isValid( ) );
		}

		return o;
	}

	MDagPath fRootBoneFromReferenceFrame( MDagPath& path, const char* nameOfRootNodeNameProperty )
	{
		MFnDagNode nodeFn( path );
		MObject object = path.node( );

		std::string rootNodeName = "";
		const b32 found = fGetStringAttribute( object, nodeFn, rootNodeName, nameOfRootNodeNameProperty );
		MDagPath rootNodeDagPath;
		if( found && rootNodeName.length( ) > 0 )
			rootNodeDagPath = MayaUtil::fFindNodeByName( rootNodeName.c_str( ) );
		return rootNodeDagPath;
	}

	MObject fFirstShaderFromTransform( MDagPath& transform )
	{
		MFnMesh meshFn( transform );

		MObjectArray shaders;
		MIntArray dummy;
		meshFn.getConnectedShaders( 0, shaders, dummy );

		if( shaders.length( ) > 0 )
		{
			MFnDependencyNode fnNode( shaders[ 0 ] );
			MPlug shaderPlug = fnNode.findPlug( "surfaceShader" );
			if( !shaderPlug.isNull( ) )
			{
				MPlugArray connectedPlugs;

				//get all the plugs that are connected as the destination of this 
				//surfaceShader plug so we can find the surface shaderNode
				MStatus status;
				shaderPlug.connectedTo( connectedPlugs, true, false, &status );

				if( status != MStatus::kFailure && connectedPlugs.length( ) == 1 )
					return connectedPlugs[ 0 ].node( );
			}
		}

		return MObject( );
	}

	b32 fIsTransformNode( MObject& mobject, MString* nameOut )
	{
		MFnDagNode nodeFn( mobject );
		MDagPath path;
		nodeFn.getPath( path );
		if( nameOut )
			*nameOut = nodeFn.name( );
		return path.hasFn( MFn::kTransform );
	}

	u32 fForEachTransformNode( const tForEachObject& cb )
	{
		MItDag itDag( MItDag::kDepthFirst, MFn::kTransform );

		MDagPath path;

		for( ; !itDag.isDone( ); itDag.next( ) )
		{
			itDag.getPath( path );
			cb( path, path.node( ) );
		}

		return MStatus::kSuccess;
	}

	u32 fForEachSelectedNode( const tForEachObject& cb )
	{
		MSelectionList list;
		MGlobal::getActiveSelectionList( list ); 

		MDagPath path;
		MObject component;

		for( u32 index = 0; index < list.length(); ++index )
		{ 
			list.getDagPath( index, path );
			list.getDependNode( index, component );
			cb( path, component );
		}

		return list.length( );
	}

	u32 fGetSelectedNodeCount( )
	{
		MSelectionList list;
		MGlobal::getActiveSelectionList( list );

		return list.length( );
	}

	void fConvertMatrix( const MFnTransform& tmFn, Math::tMat3f& sigTm, f32 unitConversion )
	{
		// get maya matrix
		MMatrix mm = tmFn.transformation( ).asMatrix( );

		// transpose into sig matrix
		for( u32 row=0; row < 3; ++row )
			for( u32 col=0; col < 4; ++col )
				sigTm( row, col ) = mm( col, row );

		// scale cm -> m for translation
		sigTm.fSetTranslation( sigTm.fGetTranslation( ) * unitConversion );

		if( tmFn.parentCount( ) > 0 )
		{
			MObject parentObj = tmFn.parent( 0 );
			if( parentObj.hasFn( MFn::kTransform ) )
			{
				Math::tMat3f parentSigTm;
				MFnTransform parentTmFn( parentObj );
				fConvertMatrix( parentTmFn, parentSigTm );
				sigTm = parentSigTm * sigTm;
			}
		}
	}

	u32 fConvertFramesPerSecond( MTime::Unit& mayaUnit )
	{
		u32 o;

		switch( mayaUnit )
		{
		case MTime::kSeconds:		o = 1;		break;
		case MTime::kGames:			o = 15; 	break;
		case MTime::kFilm:			o = 24; 	break;
		case MTime::kPALFrame:		o = 25; 	break;
		case MTime::kNTSCFrame:		o = 30; 	break;
		case MTime::kShowScan:		o = 48; 	break;
		case MTime::kPALField:		o = 50; 	break;
		case MTime::kNTSCField:		o = 60; 	break;
		case MTime::k2FPS:			o = 2;		break;
		case MTime::k3FPS:			o = 3;		break;
		case MTime::k4FPS:			o = 4;		break;
		case MTime::k5FPS:			o = 5;		break;
		case MTime::k6FPS:			o = 6;		break;
		case MTime::k8FPS:			o = 8;		break;
		case MTime::k10FPS:			o = 10; 	break;
		case MTime::k12FPS:			o = 12; 	break;
		case MTime::k16FPS:			o = 16; 	break;
		case MTime::k20FPS:			o = 20; 	break;
		case MTime::k40FPS:			o = 40; 	break;
		case MTime::k75FPS:			o = 75; 	break;
		case MTime::k80FPS:			o = 80; 	break;
		case MTime::k100FPS:		o = 100;	break;
		case MTime::k120FPS:		o = 120;	break;
		default:
			{
				log_warning( 0, "Unhandled maya time unit type. Setting to default (30fps)." );
				mayaUnit	= MTime::kNTSCFrame;
				o			= 30;
			}
			break;
		}

		return o;
	}

	void fSetSceneAttribute( const char* attrName, const std::string& stringValue )
	{
		std::string test;
		if( fGetSceneAttribute( attrName, test ) && stringValue == test )
			return; // identical

		MDagPath root = fGetSceneDataNode( );
		MFnDependencyNode nodeFn( root.node( ) );
		MObject object = root.node( );

		fUpdateStringAttribute(
			object,
			nodeFn,
			stringValue,
			attrName );
	}

	b32 fGetSceneAttribute( const char* attrName, std::string& stringValue )
	{
		MDagPath sceneDataNode = fGetSceneDataNode( false );
		if( !sceneDataNode.isValid( ) )
			return false;

		MFnDependencyNode nodeFn( sceneDataNode.node( ) );
		MObject object = sceneDataNode.node( );

		return fGetStringAttribute( object, nodeFn, stringValue, attrName );
	}

	void fUpdateStringAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		const std::string& value,
		const char* longName_ )
	{
		MString longName( longName_ );
		MString shortName( longName_ );

		// first get the attribute (in case it had already been added), and remove it
		MObject attr = nodeFn.attribute( longName );
		if( !attr.isNull( ) )
			nodeFn.removeAttribute( attr );

		// Create the attribute
		MFnTypedAttribute attrFn;
		attr = attrFn.create( longName, shortName, MFnData::kString, MObject::kNullObj );

		// add the attribute
		nodeFn.addAttribute( attr, MFnDependencyNode::kLocalDynamicAttr );

		// now get the attribute (just to be sure)
		attr = nodeFn.attribute( longName );

		// now update it
		attrFn.setObject( attr );

		// update attribute parameters
		attrFn.setWritable( true );
		attrFn.setReadable( true );
		attrFn.setKeyable( false );

		// set actual string value
		MPlug plug( object, attr );
		plug.setValue( value.c_str( ) );
	}

	void fUpdateFloatAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		f32 value,
		const char* longName_, 
		f32 defValue, f32 minValue, f32 maxValue, f32 increment, u32 precision )
	{
		MString longName( longName_ );
		MString shortName( longName_ );

		// create the attribute (in case it hasn't been added yet)
		MFnNumericAttribute attrFn;
		MObject attr = attrFn.create( longName, shortName, precision > 0 ? MFnNumericData::kFloat : MFnNumericData::kInt, defValue );

		// add the attribute
		nodeFn.addAttribute( attr, MFnDependencyNode::kLocalDynamicAttr );

		// now get the attribute (in case it had already been added)
		attr = nodeFn.attribute( longName );

		// now update it
		attrFn.setObject( attr );

		// update attribute parameters
		attrFn.setWritable( true );
		attrFn.setReadable( true );
		attrFn.setKeyable( false );
		attrFn.setMin( minValue );
		attrFn.setMax( maxValue );

		// finally, update the attribute's value itself
		MPlug attrPlug( object, attr );
		attrPlug.setFloat( value );
	}

	void fUpdateIntAttribute(
		MObject & object,
		MFnDependencyNode & nodeFn,
		s32 value,
		const char * longName_,
		s32 defValue, s32 minValue, s32 maxValue )
	{
		MString longName( longName_ );
		MString shortName( longName_ );

		// create the attribute (in case it hasn't been added yet)
		MFnNumericAttribute attrFn;
		MObject attr = attrFn.create( longName, shortName, MFnNumericData::kInt, defValue );

		// add the attribute
		nodeFn.addAttribute( attr, MFnDependencyNode::kLocalDynamicAttr );

		// now get the attribute (in case it had already been added)
		attr = nodeFn.attribute( longName );

		// now update it
		attrFn.setObject( attr );

		// update attribute parameters
		attrFn.setWritable( true );
		attrFn.setReadable( true );
		attrFn.setKeyable( false );

		if( minValue < maxValue )
		{
			attrFn.setMin( minValue );
			attrFn.setMax( maxValue );
		}

		// finally, update the attribute's value itself
		MPlug attrPlug( object, attr );
		attrPlug.setInt( value );
	}

	void fUpdateBoolAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		b32 value,
		const char* longName_, 
		b32 defValue )
	{
		MString longName( longName_ );
		MString shortName( longName_ );

		// create the attribute (in case it hasn't been added yet)
		MFnNumericAttribute attrFn;
		MObject attr = attrFn.create( longName, shortName, MFnNumericData::kBoolean, defValue );

		// add the attribute
		nodeFn.addAttribute( attr, MFnDependencyNode::kLocalDynamicAttr );

		// now get the attribute (in case it had already been added)
		attr = nodeFn.attribute( longName );

		// now update it
		attrFn.setObject( attr );

		// update attribute parameters
		attrFn.setWritable( true );
		attrFn.setReadable( true );
		attrFn.setKeyable( false );

		// finally, update the attribute's value itself
		MPlug attrPlug( object, attr );
		attrPlug.setBool( value!=0 );
	}

	void fUpdateEnumAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		u32 value,
		const char* longName_, 
		const std::string enumNames[], 
		u32 numEnumNames,
		u32 defValue )
	{
		MString longName( longName_ );
		MString shortName( longName_ );

		// first get the attribute (in case it had already been added), and remove it
		MObject attr = nodeFn.attribute( longName );
		if( !attr.isNull( ) )
			nodeFn.removeAttribute( attr );

		// create the attribute (in case it hasn't been added yet)
		MFnEnumAttribute attrFn;
		attr = attrFn.create( longName, shortName, defValue );

		// add the enum fields
		for( u32 i = 0; i < numEnumNames; ++i )
			attrFn.addField( MString( enumNames[ i ].c_str( ) ), i );

		// add the attribute
		nodeFn.addAttribute( attr, MFnDependencyNode::kLocalDynamicAttr );

		// now update it
		attrFn.setObject( attr );

		// update attribute parameters
		attrFn.setWritable( true );
		attrFn.setReadable( true );
		attrFn.setKeyable( false );

		// finally, update the attribute's value itself
		MPlug attrPlug( object, attr );
		attrPlug.setInt( value );
	}

	b32 fGetStringAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		std::string& value,
		const char* longName_ )
	{
		MString longName( longName_ );

		// get the attribute
		MObject attr = nodeFn.attribute( longName );
		if( attr.isNull( ) )
			return false; // attribute hasn't been set

		// extract the attribute's value
		MPlug plug( object, attr );
		value = plug.asString( ).asChar( );

		// return that we found the attribute
		return true;
	}

	b32 fGetFloatAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		f32& value,
		const char* longName_ )
	{
		MString longName( longName_ );

		// get the attribute
		MObject attr = nodeFn.attribute( longName );
		if( attr.isNull( ) )
			return false; // attribute hasn't been set

		// extract the attribute's value
		MPlug attrPlug( object, attr );
		value = attrPlug.asFloat( );

		// return that we found the attribute
		return true;
	}

	b32 fGetIntAttribute(
		MObject & object,
		MFnDependencyNode & nodeFn,
		s32 & value,
		const char * longName_ )
	{
		MString longName( longName_ );

		// get the attribute
		MObject attr = nodeFn.attribute( longName );
		if( attr.isNull( ) )
			return false; // attribute hasn't been set

		// extract the attribute's value
		MPlug attrPlug( object, attr );
		value = attrPlug.asInt( );

		// return that we found the attribute
		return true;
	}

	b32 fGetBoolAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		b32& value,
		const char* longName_ )
	{
		MString longName( longName_ );

		// get the attribute
		MObject attr = nodeFn.attribute( longName );
		if( attr.isNull( ) )
			return false; // attribute hasn't been set

		// extract the attribute's value
		MPlug attrPlug( object, attr );
		value = attrPlug.asBool( );

		// return that we found the attribute
		return true;
	}

	b32 fGetEnumAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		u32& value,
		const char* longName_ )
	{
		MString longName( longName_ );

		// get the attribute
		MObject attr = nodeFn.attribute( longName );
		if( attr.isNull( ) )
			return false; // attribute hasn't been set

		// extract the attribute's value
		MPlug attrPlug( object, attr );
		value = attrPlug.asInt( );

		// return that we found the attribute
		return true;
	}

	b32 fGetEnumAttributeString( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		u32& valueInt,
		std::string& valueString,
		const char* longName_ )
	{
		MString longName( longName_ );

		// get the attribute
		MObject attr = nodeFn.attribute( longName );
		if( attr.isNull( ) )
			return false; // attribute hasn't been set

		// extract the attribute's value
		MPlug attrPlug( object, attr );
		valueInt = attrPlug.asInt( );
		MFnEnumAttribute attrFn( attr );
		valueString = attrFn.fieldName( valueInt ).asChar( );

		// return that we found the attribute
		return true;
	}

}}
