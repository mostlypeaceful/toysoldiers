#ifndef __tMayaSklmlExporter__
#define __tMayaSklmlExporter__
#include "tMayaPlugin.hpp"
#include "Sklml.hpp"

namespace Sig
{
	///
	/// \brief Registers a custom maya exporter plugin capable of exporting a maya
	/// scene to a .sklml file.
	class tSklmlMayaPlugin : public iMayaPlugin
	{
	public:
		virtual b32		fOnMayaInitialize( MObject& mayaObject, MFnPlugin& mayaPlugin );
		virtual b32		fOnMayaShutdown( MObject& mayaObject, MFnPlugin& mayaPlugin );
	};

	///
	/// \brief Overrides the Maya API file translator type in order to
	/// provide exporting to the Sklml file format (for animated skeletons).
	class tMayaSklmlExporter : public MPxFileTranslator 
	{
		tStrongPtr<Sklml::tFile> mExportFile;
		u32 mNumRefFrames;

	public:

		static void* fCreate( );

		virtual ~tMayaSklmlExporter( );

		virtual MStatus writer( const  MFileObject & file, const MString & optionsString, FileAccessMode mode );
		virtual bool haveReadMethod( ) const;
		virtual bool haveWriteMethod( ) const;
		virtual bool haveNamespaceSupport( ) const;
		virtual bool haveReferenceMethod( ) const;
		virtual MString defaultExtension( ) const;
		virtual MString filter( ) const;
		virtual bool canBeOpened( ) const;
		virtual MPxFileTranslator::MFileKind identifyFile( const MFileObject& file, const char* buffer, short size ) const;

	private:

		tMayaSklmlExporter( );

		MStatus fExportAll( const MString& path );
		MStatus fExportSelected( const MString& path );
		void	fExportCommonBegin( const MString& path );
		void	fExportCommonEnd( const MString& path );
		b32		fVisitNode( MDagPath& path, MObject& component );
		void	fVisitReferenceFrame( MDagPath& path );
		void	fBuildExportSkeleton( MDagPath& refFramePath, MDagPath& rootNodePath );
		void	fConvertBoneData( Sklml::tBone& sklmlRoot, MDagPath& mayaRoot );
		void	fBuildExportSkeleton( Sklml::tBone& sklmlRoot, MDagPath& mayaRoot );
	};

}


#endif//__tMayaSklmlExporter__
