#ifndef __ExporterUtil_hpp__
#define __ExporterUtil_hpp__
#include "MaxInclude.hpp"
#include "tXmlFile.hpp"

namespace Sig { namespace MaxPlugin { namespace ExporterUtil
{
	void fExportUserProps( tXmlFile& output_file, tXmlNode node, Interface* maxIface, INode* inode );
	void fExportKeyFrames( tXmlFile& output_file, tXmlNode node, Interface* maxIface, INode* inode );
	void fExportVerts( tXmlFile& output_file, tXmlNode node, Mesh* mesh );
	void fExportVertTris( tXmlFile& output_file, tXmlNode node, Mesh* mesh );
	void fExportMtlIds( tXmlFile& output_file, tXmlNode node, Mesh* mesh );
	void fExportSmoothingGroups( tXmlFile& output_file, tXmlNode node, Mesh* mesh );
	void fExportTriNormals( tXmlFile& output_file, tXmlNode node, Mesh* mesh );
	void fExportUvwVerts( tXmlFile& output_file, tXmlNode node, Mesh* mesh, s32 ithChannel );
	void fExportUvwTris( tXmlFile& output_file, tXmlNode node, Mesh* mesh, s32 ithChannel );
	void fExportUvwChannel( tXmlFile& output_file, tXmlNode node, Mesh* mesh, s32 ithChannel );
	void fExportUvws( tXmlFile& output_file, tXmlNode node, Mesh* mesh );
	void fExportVertColors( tXmlFile& output_file, tXmlNode node, Mesh* mesh );
	void fExportVertAlphas( tXmlFile& output_file, tXmlNode node, Mesh* mesh );
	void fExportAuxChannels( tXmlFile& output_file, tXmlNode node, Mesh* mesh );
	void fExportBoneWeights( tXmlFile& output_file, tXmlNode node, Mesh* mesh, INode* inode );

}}}


#endif//__ExporterUtil_hpp__

