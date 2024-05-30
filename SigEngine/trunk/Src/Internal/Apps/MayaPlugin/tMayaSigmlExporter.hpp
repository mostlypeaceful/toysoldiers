#ifndef __tMayaSigmlExporter__
#define __tMayaSigmlExporter__
#include "tMayaPlugin.hpp"
#include "Sigml.hpp"

// we default to converting maya materials to phong (for now at least)
#include "tPhongMaterialGen.hpp"

class MFnLambertShader;

namespace Sig { namespace Sigml
{
	class tHeightFieldMeshObject;
}}

namespace Sig
{

	///
	/// \brief Registers a custom maya exporter plugin capable of exporting a maya
	/// scene to a .mshml file.
	class tSigmlMayaPlugin : public iMayaPlugin
	{
	public:
		virtual b32		fOnMayaInitialize( MObject& mayaObject, MFnPlugin& mayaPlugin );
		virtual b32		fOnMayaShutdown( MObject& mayaObject, MFnPlugin& mayaPlugin );
	};

	class tPhongMaterialGen;

	///
	/// \brief Helper class for detecting meshes that are "truly" instanced; i.e.,
	/// Maya considers meshes instanced if their geometry is the same, even if their
	/// materials are different (or material groupings). We only want to call meshes
	/// instanced if their completely identical, which is more of a pain to detect.
	class tMayaMeshInstance : tUncopyable, public tRefCounter
	{
	private:
		struct tMaterialSlice
		{
			MObject				mShaderObject;
			tDynamicArray<u32>	mPolygonIds;
		};
	private:
		MObject								mMeshObject;
		tGrowableArray< tMaterialSlice >	mMaterialSlices;
	public:
		tMayaMeshInstance( const MDagPath& path, const MFnMesh& meshFn, const MObjectArray& polygonSets, const MObjectArray& polygonComponents );
		b32 operator==( const tMayaMeshInstance& other ) const;
	};
	typedef tRefCounterPtr< tMayaMeshInstance > tMayaMeshInstancePtr;

	class tMayaMaterialInstance : tUncopyable, public tRefCounter
	{
	private:
		tGrowableArray<MObject> mMaterialObjects;
	public:
		tMayaMaterialInstance( const MObject& mtlObject );
		void fAddObject( const MObject& mtlObject );
		b32 fHasObject( const MObject& other ) const;
	};
	typedef tRefCounterPtr< tMayaMaterialInstance > tMayaMaterialInstancePtr;


	///
	/// \brief Overrides the Maya API file translator type in order to
	/// provide exporting to the Sigml file format (for geometry).
	class tMayaSigmlExporter : public MPxFileTranslator 
	{
	public:
		enum tMode
		{
			cModeMesh,
			cModeSkin,
			cModeCount
		};

	private:
		const tMode									mMode;
		tStrongPtr<Sigml::tFile>					mExportFile;
		tGrowableArray<tMayaMeshInstancePtr>		mMeshInstances;
		tGrowableArray<tMayaMaterialInstancePtr>	mMaterialInstances;
		tGrowableArray<MObject>						mSkinClusters;

	public:

		static void* fCreateMeshExporter( );
		static void* fCreateSkinExporter( );
		static void fConvertRenderStates( Sigml::tMaterialRenderOptions& mtlOut, MObject& shaderNode );

		explicit tMayaSigmlExporter( tMode mode );
		virtual ~tMayaSigmlExporter( );

		virtual MStatus	reader( const MFileObject& file, const MString& optionsString, FileAccessMode mode);
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

		MStatus fImport( const MString& path );
		MStatus fImportSigml( const Sigml::tFile& sigmlFile, const Math::tMat3f& curMatrix );
		MStatus fImportObject( const Sigml::tFile& parentFile, const Sigml::tObject& sigmlObject, const Math::tMat3f& localToWorld );
		MStatus fImportSceneRef( const Sigml::tFile& parentFile, const Sigml::tSigmlReferenceObject& sigmlObject, const Math::tMat3f& localToWorld );
		MStatus fImportGeomObject( const Sigml::tFile& parentFile, const Sigml::tGeomObject& sigmlObject, const Math::tMat3f& localToWorld );
		MStatus fImportHeightField( const Sigml::tFile& parentFile, const Sigml::tHeightFieldMeshObject& sigmlObject, const Math::tMat3f& localToWorld );

		MStatus fExportAll( const MString& path );
		MStatus fExportSelected( const MString& path );
		void	fExportCommonBegin( const MString& path );
		void	fExportCommonEnd( const MString& path );

		b32		fVisitNode( MDagPath& path, MObject& component );
		void	fVisitObject( Sigml::tObject* sigmlObject, MDagPath& path, MFnDagNode& nodeFn, MObject& mobject );
		void	fVisitGeometry( MDagPath& path, MFnDagNode& nodeFn, MObject& mobject );
		void	fExtractMaterial( Sigml::tSubMesh* sigmlSubMesh, const MObject& polygonSet );
		b32		fConvertMaterial( Sigml::tMaterialPtr& mtlOut, MObject& shaderNode );
		b32		fConvertPhongBasedMaterial( Sigml::tMaterialPtr& mtlOut, MObject& shaderNode );
		b32		fConvertDermlBasedMaterial( Sigml::tMaterialPtr& mtlOut, MObject& shaderNode );
		b32		fConvertMayaLambertMaterial( tPhongMaterialGen& phongMtl, MFnLambertShader& shaderFn );
		void	fConvertMayaColorPlug( 
					MFnLambertShader& shaderFn, 
					const char* colorName, 
					tFilePathPtr& texPath, 
					Math::tVec4f& color,
					tPhongMaterialGen::tUvParameters& uvParams );
		void	fConvertMayaBumpMap( 
					MFnLambertShader& shaderFn, 
					tFilePathPtr& texPath, 
					f32& bumpDepth,
					tPhongMaterialGen::tUvParameters& uvParams );
		void	tMayaSigmlExporter::fConvertUvParameters( 
					MFnDependencyNode& texNodeFn, 
					Math::tVec2f& repeatUv, 
					Math::tVec2f& offsetUv, 
					Math::tVec2u& mirrorUv, 
					Math::tVec2u& wrapUv,
					std::string& uvSetName );
		void	fAcquireGameProperties( Sigml::tGeomObject* sigmlObject, MDagPath& path, MFnDagNode& nodeFn, MObject& mobject );
		void	fAcquireObjectMesh( 
					Sigml::tGeomObject* sigmlObject, 
					f32 highLodRatio,
					f32 mediumLodRatio,
					f32 lowLodTarget, 
					MDagPath& path, 
					MFnDagNode& nodeFn, 
					MObject& mobject );
		Sigml::tMeshPtr	fExtractGeometry( 
					MDagPath& path, 
					MFnMesh& meshFn, 
					const MObjectArray& polygonSets, 
					const MObjectArray& polygonComponents );
		void	fExtractGeometry( 
					Sigml::tMesh* geoMeshPtr,
					MDagPath& path, 
					MFnMesh& meshFn, 
					const MObjectArray& polygonSets, 
					const MObjectArray& polygonComponents );
		Sigml::tSkinPtr fBuildSkin( MDagPath& path, MFnMesh& meshFn );
		void	fBuildPotentialSkinClusterList( );
		MObject fFindSkinCluster( MFnMesh& fnMesh );
	};

}


#endif//__tMayaSigmlExporter__
