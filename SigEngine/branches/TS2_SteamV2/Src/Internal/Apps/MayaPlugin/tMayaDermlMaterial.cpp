#include "MayaPluginPch.hpp"
#include "tMayaDermlMaterial.hpp"
#include "tMayaMatEdWindow.hpp"
#include "tMayaSigmlExporter.hpp"
#include "MayaUtil.hpp"
#include "FileSystem.hpp"
#include "Gfx/tRenderContext.hpp"
#include <maya/MFnPlugin.h>
#include <maya/MHWShaderSwatchGenerator.h>
#include <maya/MGeometryList.h>
#include <maya/MRenderProfile.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringArrayData.h>
#include <maya/MD3D9Renderer.h>
#include <maya/MImage.h>
#include <maya/MGeometryManager.h>
#include <maya/MGeometryRequirements.h>
#include <maya/MVaryingParameterList.h>
#include <maya/MFnLight.h>

namespace Sig
{
	///
	/// \section tDermlMayaPlugin
	///

	b32 tDermlMayaPlugin::fOnMayaInitialize( MObject& mayaObject, MFnPlugin& mayaPlugin )
	{
		MString userClassify = MString( "shader/surface/utility" );
		
		// Don't initialize swatches in batch mode
		if (MGlobal::mayaState() != MGlobal::kBatch)
		{
			static MString swatchName( "dermlRenderSwatchGen" );
			MSwatchRenderRegister::registerSwatchRender( swatchName, MHWShaderSwatchGenerator::createObj );
			userClassify = MString( "shader/surface/utility/:swatch/" + swatchName );
		}

		const MStatus status = mayaPlugin.registerNode("dermlShader",
			tMayaDermlMaterial::cId,
			tMayaDermlMaterial::creator,
			tMayaDermlMaterial::initialize,
			MPxNode::kHardwareShader,
			&userClassify );

		if( !status )
		{
			status.perror("mayaPlugin.registerNode:dermlShader");
			return false;
		}

		return true;
	}

	b32 tDermlMayaPlugin::fOnMayaShutdown( MObject& mayaObject, MFnPlugin& mayaPlugin )
	{
		const MStatus status = mayaPlugin.deregisterNode( tMayaDermlMaterial::cId );

		if( !status )
		{
			status.perror("mayaPlugin.deregisterNode:dermlShader");
			return false;
		}

		return true;
	}
}

namespace Sig
{
	namespace
	{
		static const tFilePathPtr cScratchFilePath = ToolsPaths::fCreateTempEngineFilePath( ".derml", tFilePathPtr("maya"), "scratch" );
		static MRenderProfile gProfile;
		static MObject gMtlContentsAttribute;

		static Math::tMat3f fMMatrixToMat3f( const MMatrix& mtm )
		{
			return Math::tMat3f( (float)mtm.matrix[0][0], (float)mtm.matrix[1][0], (float)mtm.matrix[2][0], (float)mtm.matrix[3][0], 
							     (float)mtm.matrix[0][1], (float)mtm.matrix[1][1], (float)mtm.matrix[2][1], (float)mtm.matrix[3][1], 
							     (float)mtm.matrix[0][2], (float)mtm.matrix[1][2], (float)mtm.matrix[2][2], (float)mtm.matrix[3][2] );
		}
		static Math::tMat4f fMMatrixToMat4f( const MMatrix& mtm )
		{
			return Math::tMat4f( (float)mtm.matrix[0][0], (float)mtm.matrix[1][0], (float)mtm.matrix[2][0], (float)mtm.matrix[3][0], 
							     (float)mtm.matrix[0][1], (float)mtm.matrix[1][1], (float)mtm.matrix[2][1], (float)mtm.matrix[3][1], 
							     (float)mtm.matrix[0][2], (float)mtm.matrix[1][2], (float)mtm.matrix[2][2], (float)mtm.matrix[3][2],
							     (float)mtm.matrix[0][3], (float)mtm.matrix[1][3], (float)mtm.matrix[2][3], (float)mtm.matrix[3][3]);
		}
	}

	const MTypeId tMayaDermlMaterial::cId( 0x9BB027D8 );

	void* tMayaDermlMaterial::creator( )
	{
		return new tMayaDermlMaterial( );
	}

	MStatus tMayaDermlMaterial::initialize( )
	{
		MFnTypedAttribute	typedAttr;
		MFnStringData		stringData;

		// This attribute holds the raw material file text
		gMtlContentsAttribute = typedAttr.create( "MaterialFileContents", "MFC", MFnData::kString, stringData.create( ) );
		typedAttr.setInternal( true );
		typedAttr.setKeyable( false );
		addAttribute( gMtlContentsAttribute );

		gProfile.addRenderer( MRenderProfile::kMayaD3D );

		return MStatus::kSuccess;
	}

	tMayaDermlMaterial::tMayaDermlMaterial( )
		: mMayaVtxStructure( "VertexStructure", MVaryingParameter::kStructure )
		, mTextureCacheDirtyStamp( 0 )
	{
		// verify these items exist
		sigassert( 
			tMayaMatEdWindow::fInstance( ) && 
			tMayaMatEdWindow::fInstance( )->fTextureCache( ) &&
			tMayaMatEdWindow::fInstance( )->fDefaultLight( ) );

		fSetPreviewDevice( fGetMayaDevice( ) );
	}

	void tMayaDermlMaterial::postConstructor( )
	{
		MPxHardwareShader::postConstructor( );
	}

	tMayaDermlMaterial::~tMayaDermlMaterial( )
	{
	}

	MStatus tMayaDermlMaterial::populateRequirements( const MPxHardwareShader::ShaderContext& context, MGeometryRequirements& requirements )
	{
		return MPxHardwareShader::populateRequirements( context, requirements );
	}

	MStatus tMayaDermlMaterial::render( MGeometryList& iter )
	{
		if( !mPreviewBundle )
			return MStatus::kFailure;

		Dx9Util::tTextureCache& texCache = *tMayaMatEdWindow::fInstance( )->fTextureCache( );
		if( texCache.fDirtyStamp( ) != mTextureCacheDirtyStamp )
		{
			mTextureCacheDirtyStamp = texCache.fDirtyStamp( );
			mPreviewBundle->fUpdateMaterial( mMaterialFile, texCache );
		}

		const HlslGen::tHlslOutput& hlslGenOutput = mPreviewBundle->fHlslGenOutput( );
		HlslGen::tVertexShaderOutput::tVertexFormatVRamPtr vtxFormatPtr = hlslGenOutput.mStaticVShaders[ HlslGen::cWriteModeColor ].mVtxFormat;
		if( !vtxFormatPtr )
			return MStatus::kFailure;

		const Gfx::tDevicePtr& device = mPreviewBundle->fDevice( );
		const Gfx::tMaterialPtr& material = mPreviewBundle->fMaterial( );
		if( !device || !material )
			return MStatus::kFailure;

		IDirect3DDevice9* d3ddev = device->fGetDevice( );

		// because we're rendering as part of the maya render process, we can't be sure of
		// any previously applied state, so we forcefully invalidate before rendering
		device->fInvalidateLastRenderState( );

		// this is an assumed default state (i.e., our tRenderState type doesn't set it, so we need to here)
		d3ddev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

		// get current material options as specified in exporter toolbox (i.e., the options the artist has specified)
		Sigml::tMaterialRenderOptions mtlRenderOpts;
		tMayaSigmlExporter::fConvertRenderStates( mtlRenderOpts, thisMObject( ) );

		// convert artist's material options to a useable/settable render state object
		Gfx::tRenderState renderState;
		mtlRenderOpts.fToRenderState( renderState );

		// apply vertex format
		vtxFormatPtr->fApply( device );

		for( ; !iter.isDone(); iter.next( ) )
		{
			// fill out renderContext and renderInstance from maya values
			Gfx::tRenderContext renderContext;
			Gfx::tRenderBatchData renderBatch;
			Gfx::tRenderInstance renderInstance;
			Math::tMat3f objectToWorldXform;
			Gfx::tCamera camera;
			fFillContext( iter, renderContext, renderInstance, objectToWorldXform, camera );
			Gfx::tDrawCall drawCall = Gfx::tDrawCall( renderInstance, 0.f );

			// apply the material (shaders, shader constants)
			material->fApplyShared( device, renderContext, renderBatch );
			material->fApplyInstance( device, renderContext, renderBatch, drawCall );

			// apply render state
			renderState.fApply( device, renderContext );

			// render the geometry (geometry is supplied by maya; we tell it our vertex format in fUpdateMayaVtxStructure( ))
			MGeometry& geometry = iter.geometry( MGeometryList::kNone );
			const void* data;
			unsigned int elements, count;
			if( mMayaVtxStructure.numElements( ) && mMayaVtxStructure.getBuffer( geometry, data, elements, count ) == MStatus::kSuccess )
			{
				MGeometryPrimitive primitives = geometry.primitiveArray( 0 );

				// split the geometry using max batch size (just in case the artist has some insanely high poly mesh)
				const u32 maxBatchSize = 40000;
				const u32 numPrimitives = primitives.elementCount( ) / 3;
				const u32 batchSize = numPrimitives / ( numPrimitives / maxBatchSize + 1 ) + 1;
				for( u32 start = 0; start < numPrimitives; )
				{
					u32 end = start + batchSize;
					if( end > numPrimitives)
						end = numPrimitives;
					// just drawing using the slow but oh-so convenient "user pointer" d3d API
					d3ddev->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, count, end - start, ( ( u32* )primitives.data( ) ) + ( start * 3 ), D3DFMT_INDEX32, data, elements );
					start = end;
				}
			}
		}

		// If we don't set these back to NULL, the viewport
		// background gradient is not rendered correctly because
		// Maya starts drawing with our shaders!
		d3ddev->SetPixelShader( NULL );
		d3ddev->SetVertexShader( NULL );

		return MStatus::kSuccess;
	}

	MStatus tMayaDermlMaterial::renderSwatchImage( MImage& image )
	{
		u32 width, height;
		image.setRGBA( true );
		image.getSize( width, height );

		// get the geometry requirements
		ShaderContext context;					// NULL path and shading engine
		MGeometryRequirements requirements;
		populateRequirements( context, requirements );

		MD3D9Renderer* pRenderer = MD3D9Renderer::theRenderer( );
		if( !pRenderer )
			return MStatus::kFailure;

		pRenderer->makeSwatchContextCurrent( width, height );

		// render the back ground
		pRenderer->setBackgroundColor( MColor( 0.1f, 0.1f, 0.1f ) );

		MGeometryList* geomIter = MGeometryManager::referenceDefaultGeometry( MGeometryManager::kDefaultSphere, requirements);

		if( !geomIter )
			return MStatus::kFailure;

		// set up lighting
		// TO DO

		// render the object this the shader and defaulting geometry
		if( render( *geomIter ) != MStatus::kSuccess )
		{
			// revert to a simple fixed color material render
			// TO DO
		}

		// read back the image
		pRenderer->readSwatchContextPixels( image );

		// let go of the geoemtry
		MGeometryManager::dereferenceDefaultGeometry( geomIter );

		return MStatus::kSuccess;
	}

	const MRenderProfile& tMayaDermlMaterial::profile( )
	{
		return gProfile;
	}

	void tMayaDermlMaterial::copyInternalData( MPxNode* pSrc )
	{
		MPxHardwareShader::copyInternalData( pSrc );
		tMayaDermlMaterial& src = *(tMayaDermlMaterial*)pSrc;
		src.mMaterialFile.fClone( mMaterialFile, cScratchFilePath );
		fUpdateAgainstShaderFile( );
	}

	bool tMayaDermlMaterial::setInternalValueInContext( const MPlug& plug, const MDataHandle& handle, MDGContext& context )
	{
		bool o = true;
		if( plug == gMtlContentsAttribute )
		{
			tDynamicBuffer rawStringBuffer;
			rawStringBuffer.fInsert( 0, ( Sig::byte* )handle.asString( ).asChar( ), handle.asString( ).length( ) );
			if( FileSystem::fWriteBufferToFile( rawStringBuffer, cScratchFilePath ) )
			{
				mMaterialFile.fLoadXml( cScratchFilePath );
				fUpdateAgainstShaderFile( );
			}
		}
		else
			o = MPxHardwareShader::setInternalValueInContext( plug, handle, context );
		return o;
	}

	bool tMayaDermlMaterial::getInternalValueInContext( const MPlug& plug, MDataHandle& handle, MDGContext& context )
	{
		bool o = true;
		if( plug == gMtlContentsAttribute )
		{
			mMaterialFile.fSaveXml( cScratchFilePath, false );
			tDynamicBuffer rawStringBuffer;
			if( FileSystem::fReadFileToBuffer( rawStringBuffer, cScratchFilePath, "" ) )
				handle.set( MString( ( const char* )rawStringBuffer.fBegin( ) ) );
		}
		else
			o = MPxHardwareShader::getInternalValueInContext( plug, handle, context );
		return o;
	}

	Gfx::tDevicePtr tMayaDermlMaterial::fGetMayaDevice( )
	{
		MD3D9Renderer* pRenderer = MD3D9Renderer::theRenderer( );
		IDirect3DDevice9* d3ddev = pRenderer ? pRenderer->getD3D9Device( ) : 0;
		return d3ddev ? Gfx::tDevicePtr( new Gfx::tDevice( d3ddev ) ) : Gfx::tDevicePtr( );
	}

	tMayaDermlMaterial* tMayaDermlMaterial::fFromMayaObject( MObject& mobj, MDagPath* path, b32* wasFromTransform )
	{
		if( wasFromTransform ) // default to false
			*wasFromTransform = false;

		if( mobj.apiType( ) == MFn::kPluginHardwareShader )
		{
			MPxHardwareShader* hwShader = MPxHardwareShader::getHardwareShaderPtr( mobj );
			if( hwShader )
			{
				// found a hardware shader, now see if it's a derml
				return dynamic_cast<tMayaDermlMaterial*>( hwShader );
			}
		}
		else if( path && mobj.apiType( ) == MFn::kTransform )
		{
			// additionally, if the user specified a dag path that's a transform,
			// then we'll look for a shader on the mesh
			tMayaDermlMaterial* o = fFromMayaObject( MayaUtil::fFirstShaderFromTransform( *path ) );
			if( o && wasFromTransform )
				*wasFromTransform = true;
			return o;
		}

		return 0;
	}

	void tMayaDermlMaterial::fCloneMaterialFile( Derml::tMtlFile& o )
	{
		mMaterialFile.fClone( o, cScratchFilePath );
	}

	b32 tMayaDermlMaterial::fChangeShader( Derml::tFile& dermlFile, const tFilePathPtr& shaderPath )
	{
		Derml::tNodeList prevNodes = mMaterialFile.mNodes;
		mMaterialFile.fFromShaderFile( dermlFile, shaderPath );
		mMaterialFile.fUpdateAgainstPreviousNodes( prevNodes );
		fGenerateShaders( dermlFile );
		return true;
	}

	void tMayaDermlMaterial::fUpdateAgainstShaderFile( )
	{
		Derml::tFile dermlFile;
		if( mMaterialFile.fUpdateAgainstShaderFile( dermlFile ) )
			fGenerateShaders( dermlFile );
		else if( mPreviewBundle && !mPreviewBundle->fHasShaders( ) )
			fGenerateShadersFromCurrentMtlFile( );
	}

	void tMayaDermlMaterial::fSetPreviewDevice( const Gfx::tDevicePtr& device )
	{
		sigassert( device );
		if( mPreviewBundle && mPreviewBundle->fDevice( ) )
		{
			if( mPreviewBundle->fDevice( )->fGetDevice( ) != device->fGetDevice( ) )
				mPreviewBundle.fReset( new tMaterialPreviewBundle( device ) );
		}
		else
			mPreviewBundle.fReset( new tMaterialPreviewBundle( device ) );
	}

	void tMayaDermlMaterial::fFillContext( MGeometryList& iter, Gfx::tRenderContext& renderContext, Gfx::tRenderInstance& renderInstance, Math::tMat3f& objectToWorld, Gfx::tCamera& camera )
	{
		renderContext.mCamera = &camera;
		renderContext.mGlobalFillMode = Gfx::tRenderState::cGlobalFillSmooth;
		renderContext.mRenderPassMode = Gfx::tRenderState::cRenderPassLighting;
		renderContext.mFogValues.x *= 100.f; renderContext.mFogValues.y *= 100.f; // maya uses cm
		renderContext.mFogValues.z = 0.f; renderContext.mFogValues.w = 0.f; // currently disabling fog entirely using the min/max vog values

		const Dx9Util::tTextureCache& texCache = *tMayaMatEdWindow::fInstance( )->fTextureCache( );
		renderContext.mWhiteTexture.fSetRaw( ( Gfx::tTextureReference::tPlatformHandle )texCache.fWhiteTexture( ).fGetRawPtr( ) );
		renderContext.mBlackTexture.fSetRaw( ( Gfx::tTextureReference::tPlatformHandle )texCache.fBlackTexture( ).fGetRawPtr( ) );

		renderInstance.fRI_SetObjectToWorld( &objectToWorld );

		objectToWorld = fMMatrixToMat3f( iter.objectToWorldMatrix( ) );
		const Math::tMat4f proj = fMMatrixToMat4f( iter.projectionMatrix( ) );
		const Math::tMat3f view = fMMatrixToMat3f( iter.viewMatrix( ) );
		camera.fSetFromMatrices( view, proj );

		// now get lights:
		// iterate through all the nodes in the scene to find our lights
		renderContext.mLightShaderConstants.mLightCount = 0;
		for( MItDependencyNodes itDep(MFn::kInvalid); !itDep.isDone( ); itDep.next( ) ) 
		{
			MObject item = itDep.item( );
			if( !item.hasFn( MFn::kLight ) )
				continue;

			MFnLight fnLight( item );
			if( fnLight.isIntermediateObject( ) ) 
				continue;

			// maya light values
			const MColor color = fnLight.color( );
			const f32 intensity = fnLight.intensity( );
			MFnTransform fnXform( fnLight.parent(0) );

			// convert to our values
			Gfx::tLight light;
			Math::tMat3f objToWorld;
			MayaUtil::fConvertMatrix( fnXform, objToWorld, 1.f );
			objToWorld.fNormalizeBasis( );

			b32 supported = true;
			b32 ambientSet = false;
			switch( item.apiType( ) )
			{
			case MFn::kDirectionalLight:
				{
					objToWorld.fXAxis( -objToWorld.fXAxis( ) );
					objToWorld.fZAxis( -objToWorld.fZAxis( ) );
					light.fColor( Gfx::tLight::cColorTypeFront ) = Math::tVec4f( intensity * color.r, intensity * color.g, intensity * color.b, color.a );
					light.fSetTypeDirection( );
				}
				break;
			case MFn::kPointLight:
				{
					light.fColor( Gfx::tLight::cColorTypeFront ) = Math::tVec4f( color.r, color.g, color.b, color.a );
					light.fSetTypePoint( Math::tVec2f( intensity, intensity + 100.f ) );
				}
				break;
			case MFn::kSpotLight:
			case MFn::kLight:
			case MFn::kAreaLight:
			case MFn::kAmbientLight:
			default:
				supported = false;
				break;
			}

			if( !supported )
				continue; // unsupported light type

			if( !ambientSet )
			{
				ambientSet = true;
				light.fColor( Gfx::tLight::cColorTypeAmbient ) = Math::tVec4f( 0.1f, 0.1f, 0.1f, 0.f );
			}

			Gfx::tLightEntityPtr lightEntity( new Gfx::tLightEntity( objToWorld, light ) );
			lightEntity->fToShaderConstants( renderContext.mLightShaderConstants[ renderContext.mLightShaderConstants.mLightCount ], camera.fGetTripod( ).mEye );

			if( ++renderContext.mLightShaderConstants.mLightCount == Gfx::tMaterial::cMaxLights )
				break; // found as many lights as we can handle
		}

		// in case we didn't find any lights, then add our default light
		if( renderContext.mLightShaderConstants.mLightCount == 0 )
		{
			renderContext.mLightShaderConstants.mLightCount = 1;
			Gfx::tLightShaderConstants& light = renderContext.mLightShaderConstants[ 0 ];
			tMayaMatEdWindow::fInstance( )->fDefaultLight( )->fToShaderConstants( light, camera.fGetTripod( ).mEye );
		}
	}

	void tMayaDermlMaterial::fGenerateShadersFromCurrentMtlFile( )
	{
		const tFilePathPtr absolutePath = ToolsPaths::fMakeResAbsolute( mMaterialFile.mShaderPath );
		Derml::tFile dermlFile;
		if( dermlFile.fLoadXml( absolutePath ) )
			fGenerateShaders( dermlFile );
	}

	void tMayaDermlMaterial::fGenerateShaders( const Derml::tFile& dermlFile )
	{
		if( dermlFile.mGeometryStyle != HlslGen::cVshMeshModel )
		{
			log_warning( 0, "Invalid geometry style on shader [" << mMaterialFile.mShaderPath << "] (Material Name: " << name( ).asChar( ) << "); expected GeometryType: 'MeshModel'" );
		}

		if( mPreviewBundle )
		{
			mPreviewBundle->fGenerateShaders( dermlFile, HlslGen::cToolTypeMaya );
			mPreviewBundle->fUpdateMaterial( mMaterialFile, *tMayaMatEdWindow::fInstance( )->fTextureCache( ) );
		}

		fUpdateMayaVtxStructure( );
	}

	void tMayaDermlMaterial::fUpdateMayaVtxStructure( )
	{
		mMayaVtxStructure.removeElements( );

		if( mPreviewBundle )
		{
			const HlslGen::tHlslOutput& hlslGenOutput = mPreviewBundle->fHlslGenOutput( );
			if( hlslGenOutput.mStaticVShaders[ HlslGen::cWriteModeColor ].mVtxFormat )
			{
				const Gfx::tVertexFormatVRam& vtxFormat = *hlslGenOutput.mStaticVShaders[ HlslGen::cWriteModeColor ].mVtxFormat;

				for( u32 i = 0; i < vtxFormat.fElementCount( ); ++i )
				{
					std::stringstream name;
					MVaryingParameter::MVaryingParameterSemantic semantic;
					u32 numElements;
					switch( vtxFormat[ i ].mSemantic )
					{
					case Gfx::tVertexElement::cSemanticPosition:
						name << "Position";
						semantic = MVaryingParameter::kPosition;
						numElements = 3;
						break;
					case Gfx::tVertexElement::cSemanticNormal:
						name << "Normal";
						semantic = MVaryingParameter::kNormal;
						numElements = 3;
						break;
					case Gfx::tVertexElement::cSemanticTangent:
						name << "Tangent";
						semantic = MVaryingParameter::kTangent;
						numElements = 3;
						break;
					case Gfx::tVertexElement::cSemanticBinormal:
						name << "Binormal";
						semantic = MVaryingParameter::kBinormal;
						numElements = 3;
						break;
					case Gfx::tVertexElement::cSemanticTexCoord:
						name << "Uv" << vtxFormat[ i ].mSemanticIndex;
						semantic = MVaryingParameter::kTexCoord;
						numElements = 2;
						break;
					case Gfx::tVertexElement::cSemanticColor:
						name << "VertexColor";
						semantic = MVaryingParameter::kColor;
						numElements = 4;
						break;
					default:
						name << "Unknown";
						semantic = MVaryingParameter::kNoSemantic;
						numElements = 0;
						break;
					}

					MVaryingParameter varying( MString( name.str( ).c_str( ) ), MVaryingParameter::kFloat, numElements, numElements, semantic, semantic == MVaryingParameter::kTexCoord ? true : false );
					mMayaVtxStructure.addElement( varying );
				}
			}
		}

		MVaryingParameterList list;
		if( mMayaVtxStructure.numElements( ) > 0 )
		{
			// Only add our parameters if we have some - otherwise the empty structure
			// shows up in the UI which is a bit ugly
			list.append( mMayaVtxStructure );
		}

		setVaryingParameters( list );
	}

}
