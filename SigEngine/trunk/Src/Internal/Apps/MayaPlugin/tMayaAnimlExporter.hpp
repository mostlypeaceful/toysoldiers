#ifndef __tMayaAnimlExporter__
#define __tMayaAnimlExporter__
#include "tMayaPlugin.hpp"
#include "Animl.hpp"

namespace Sig
{

	///
	/// \brief Registers a custom maya exporter plugin capable of exporting a maya
	/// scene to a .animl file.
	class tAnimlMayaPlugin : public iMayaPlugin
	{
	public:
		virtual b32		fOnMayaInitialize( MObject& mayaObject, MFnPlugin& mayaPlugin );
		virtual b32		fOnMayaShutdown( MObject& mayaObject, MFnPlugin& mayaPlugin );
	};

	///
	/// \brief Overrides the Maya API file translator type in order to
	/// provide exporting to the Animl file format (for skeletal animations).
	class tMayaAnimlExporter : public MPxFileTranslator 
	{
		tStrongPtr<Animl::tFile>	mExportFile;
		MTime::Unit					mTimeUnit;
		MTime						mCurrentTime;
		u32							mNumRefFrames;

	public:

		static void* fCreate( );

		virtual ~tMayaAnimlExporter( );

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

		tMayaAnimlExporter( );

		MStatus fExportAll( const MString& path );
		MStatus fExportSelected( const MString& path );
		void	fExportCommonBegin( const MString& path );
		void	fExportCommonEnd( const MString& path );
		b32		fVisitNode( MDagPath& path, MObject& component );
		void	fVisitReferenceFrame( MDagPath& path );
		void	fBuildMayaBoneList( tGrowableArray< MDagPath >& bones, MDagPath& mayaRoot );
		void	fConvertStaticBoneData( Animl::tBone& animlBone, MDagPath& mayaRoot, u32 numKeys );
		void	fConvertDynamicBoneData( Animl::tBone& animlBone, MDagPath& mayaRoot, u32 frameNum );
	};

}


#endif//__tMayaAnimlExporter__
