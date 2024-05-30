#ifndef __MayaPluginPch__
#define __MayaPluginPch__

#include "ToolsPch.hpp"

#include <maya/MGlobal.h>
#include <maya/M3dView.h>
#include <maya/MSimple.h> 
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MObject.h>
#include <maya/MItDag.h>
#include <maya/MDagPath.h>
#include <maya/MItSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MFnSet.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFileIO.h>
#include <maya/MIOStream.h>
#include <maya/MFStream.h>
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MEventMessage.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTransform.h>
#include <maya/MPxFileTranslator.h>
#include <maya/MQuaternion.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MMatrix.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MDagPathArray.h>
#include <maya/MItGeometry.h>
#include <maya/MAnimControl.h>
#include <maya/MCommandResult.h>

#endif//__MayaPluginPch__
