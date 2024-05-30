#ifndef __tMayaDermlMaterial__
#define __tMayaDermlMaterial__

#undef profile //conflicts with maya lib
#include <maya/MPxHardwareShader.h>
#include <maya/MVaryingParameter.h>
#include <maya/MTypeId.h>
#include "tMayaPlugin.hpp"
#include "Derml.hpp"
#include "tMaterialPreviewBundle.hpp"

namespace Sig
{

	///
	/// \brief Registers a custom maya material plugin for interfacing
	/// with the .derml shader material system.
	class tDermlMayaPlugin : public iMayaPlugin
	{
	public:
		virtual b32		fOnMayaInitialize( MObject& mayaObject, MFnPlugin& mayaPlugin );
		virtual b32		fOnMayaShutdown( MObject& mayaObject, MFnPlugin& mayaPlugin );
	};

	class tMayaDermlMaterial : public MPxHardwareShader
	{
	public:
		static const MTypeId cId;
	public:
		static void* creator( );
		static MStatus initialize( );
	private:
		Derml::tMtlFile mMaterialFile;
		tMaterialPreviewBundlePtr mPreviewBundle;
		MVaryingParameter mMayaVtxStructure;
		u32 mTextureCacheDirtyStamp;

	public: // overrides from maya base classes
		tMayaDermlMaterial();
		virtual void postConstructor();
		virtual ~tMayaDermlMaterial(); 
		virtual MStatus populateRequirements( const MPxHardwareShader::ShaderContext& context, MGeometryRequirements& requirements );
		virtual MStatus render( MGeometryList& iterator );
		virtual MStatus renderSwatchImage( MImage& image );
		virtual const MRenderProfile& profile( );
		virtual void copyInternalData( MPxNode* pSrc );
		virtual bool setInternalValueInContext( const MPlug& plug, const MDataHandle& handle, MDGContext& context );
		virtual bool getInternalValueInContext( const MPlug& plug, MDataHandle& handle, MDGContext& context );
	public: // interface for sig maya plugin with MatEd
		static Gfx::tDevicePtr fGetMayaDevice( );
		static tMayaDermlMaterial* fFromMayaObject( MObject& mobj, MDagPath* path = 0, b32* wasFromTransform = 0 );
		const Derml::tMtlFile& fMaterialFile( ) const { return mMaterialFile; }
		const tMaterialPreviewBundlePtr& fPreviewBundle( ) const { return mPreviewBundle; }
		void fCloneMaterialFile( Derml::tMtlFile& o );
		b32 fChangeShader( Derml::tFile& dermlFile, const tFilePathPtr& shaderPath );
		void fUpdateAgainstShaderFile( );
	protected:
		void fSetPreviewDevice( const Gfx::tDevicePtr& device );
		void fFillContext( MGeometryList& iterator, Gfx::tRenderContext& renderContext, Gfx::tRenderInstance& renderInstance, Math::tMat3f& objectToWorld, Gfx::tCamera& camera );
		void fGenerateShadersFromCurrentMtlFile( );
		void fGenerateShaders( const Derml::tFile& dermlFile );
		void fUpdateMayaVtxStructure( );
	};

}

#endif//__tMayaDermlMaterial__
