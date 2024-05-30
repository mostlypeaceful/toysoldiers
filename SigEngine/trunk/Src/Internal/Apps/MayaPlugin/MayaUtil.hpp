#ifndef __MayaUtil__
#define __MayaUtil__
#include "tDelegate.hpp"

namespace Sig { namespace MayaUtil
{
	typedef tDelegate<b32 ( MDagPath& path, MObject& component )> tForEachObject;

	///
	/// \brief Find a maya node by name.
	MDagPath fFindNodeByName( const char* name, MFn::Type type = MFn::kTransform );

	///
	/// \brief Get the root maya node.
	MDagPath fGetRootNode( );

	///
	/// \brief Get the node to which attributes can be attached that are global to the scene.
	MDagPath fGetSceneDataNode( b32 createIfNotFound = true );

	///
	/// \brief Get the root bone that corresponds to the specified reference frame node.
	MDagPath fRootBoneFromReferenceFrame( MDagPath& path, const char* nameOfRootNodeNameProperty );

	///
	/// \brief Find the first shader object on the transform node
	MObject fFirstShaderFromTransform( MDagPath& transform );

	///
	/// \brief Query whether an object is a transform node and retrieve its name at the same time.
	b32 fIsTransformNode( MObject& mobject, MString* nameOut = 0 );

	///
	/// \brief Process 'cb' on each node in the scene (visit with a delegate).
	/// \note If 'cb' returns false, the function will stop visiting further
	/// nodes and return immediately.
	/// \return The number of nodes.
	u32 fForEachTransformNode( const tForEachObject& cb );

	///
	/// \brief Process 'cb' on each selected node (visit with a delegate).
	/// \note If 'cb' returns false, the function will stop visiting further
	/// nodes and return immediately.
	/// \return The number of selected nodes.
	u32 fForEachSelectedNode( const tForEachObject& cb );

	///
	/// \brief Returns the number of selected nodes
	u32 fGetSelectedNodeCount( );

	///
	/// \brief Convert a maya-style matrix to a sig-style matrix. Besides general data-structure transfer,
	/// this function also converts units from cm to m.
	void fConvertMatrix( const MFnTransform& tmFn, Math::tMat3f& sigTm, f32 unitConversion = 1.f/100.f/*cm -> m*/ );

	///
	/// \brief Get the number of frames per second from a maya time unit enum value.
	u32 fConvertFramesPerSecond( MTime::Unit& mayaUnit );

	///
	/// \brief Set a named string attribute on the scene; this value will get saved with the scene.
	void fSetSceneAttribute( const char* attrName, const std::string& stringValue );

	///
	/// \brief Get a named string attribute from the scene; this value will have been loaded from the scene.
	/// \return true if the value was found, false if it wasn't.
	b32 fGetSceneAttribute( const char* attrName, std::string& stringValue );

	void fUpdateStringAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		const std::string& value,
		const char* longName_ );

	void fUpdateFloatAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		f32 value,
		const char* longName_,
		f32 defValue, f32 minValue, f32 maxValue, f32 increment, u32 precision );

	void fUpdateIntAttribute(
		MObject & object,
		MFnDependencyNode & nodeFn,
		s32 value,
		const char * longName_,
		s32 defValue, s32 minValue, s32 maxValue );

	void fUpdateBoolAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		b32 value,
		const char* longName_, 
		b32 defValue );

	void fUpdateEnumAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		u32 value,
		const char* longName_, 
		const std::string enumNames[], 
		u32 numEnumNames,
		u32 defValue );

	b32 fGetStringAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		std::string& value,
		const char* longName_ );

	b32 fGetFloatAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		f32& value,
		const char* longName_ );

	b32 fGetIntAttribute(
		MObject & object,
		MFnDependencyNode & nodeFn,
		s32 & value,
		const char * longName_ );

	b32 fGetBoolAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		b32& value,
		const char* longName_ );

	b32 fGetEnumAttribute( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		u32& value,
		const char* longName_ );

	b32 fGetEnumAttributeString( 
		MObject& object,
		MFnDependencyNode& nodeFn,
		u32& valueInt,
		std::string& valueString,
		const char* longName_ );
}}


#endif//__MayaUtil__
