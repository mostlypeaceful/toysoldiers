#include "ToolsPch.hpp"
#include "tFxmlConverter.hpp"
#include "tLoadInPlaceSerializer.hpp"
#include "tExporterToolbox.hpp"
#include "iAssetPlugin.hpp"
#include "FileSystem.hpp"
#include "tMesh.hpp"
#include <limits>

// from graphics
#include "Gfx/tDevice.hpp"
#include "Gfx/tGeometryFile.hpp"
#include "Gfx/tMaterialFile.hpp"
#include "Gfx/tDefaultMaterial.hpp"

namespace Sig
{
	tFxmlConverter::tFxmlConverter( )
	{
	}

	tFxmlConverter::~tFxmlConverter( )
	{
	}

	b32 tFxmlConverter::fLoadFxmlFile( const tFilePathPtr& fxmlFilePath, const tFilePathPtr& outputResourcePath )
	{
		if( mFxmlFile.fLoadXml( fxmlFilePath ) )
		{
			mResourcePath = outputResourcePath;
			return true;
		}

		return false;
	}

	b32 tFxmlConverter::fConvertPlatformCommon( )
	{
		// convert all objects
		mObjects.fNewArray( fFxmlFile( ).mObjects.fCount( ) );
		mObjects.fFill( 0 );
		u32 numConverted = 0;
		u32 numObjects = fFxmlFile( ).mObjects.fCount( );
		for( u32 i = 0; i < numObjects; ++i )
		{
			// convert object (virtually)
			if( fFxmlFile( ).mObjects[ i ] )
			{
				Fxml::tFxParticleSystemObject* fxParticleSystem = dynamic_cast< Fxml::tFxParticleSystemObject* >( fFxmlFile( ).mObjects[ i ].fGetRawPtr( ) );
				Fxml::tFxAttractorObject* fxAttractor = dynamic_cast< Fxml::tFxAttractorObject* >( fFxmlFile( ).mObjects[ i ].fGetRawPtr( ) );
				Fxml::tFxMeshSystemObject* fxMeshSystem = dynamic_cast< Fxml::tFxMeshSystemObject* >( fFxmlFile( ).mObjects[ i ].fGetRawPtr( ) );
				Fxml::tFxLightObject* fxLight = dynamic_cast< Fxml::tFxLightObject* >( fFxmlFile( ).mObjects[ i ].fGetRawPtr( ) );

				if( !fxParticleSystem && !fxAttractor && !fxMeshSystem && !fxLight )
					continue;

				mObjects[ numConverted ] = fFxmlFile( ).mObjects[ i ]->fCreateEntityDef( *this );
			}

			// check for conversion failure
			if( mObjects[ numConverted ] )
				++numConverted;
		}

		// FxScene properties/info
		mLifetime = fFxmlFile( ).mLifetime;
		mFlags = fFxmlFile( ).mFlags;

		if( numConverted == 0 )
		{
			log_warning( "Failed to correctly build any fx objects. Check your materials!" );
			return false;
		}
		// cull objects that failed to convert
		if( numConverted != mObjects.fCount( ) )
		{
			if( numConverted == 0 )
			{
				mObjects.fDeleteArray( );
			}
			else
			{
				tSceneGraphFile::tObjectArray resizedObjects( numConverted );
				fMemCpy( resizedObjects.fBegin( ), mObjects.fBegin( ), resizedObjects.fCount( ) * sizeof( resizedObjects[ 0 ] ) );
				mObjects.fSwap( resizedObjects );
			}
		}
		
		return true;
	}

	b32 tFxmlConverter::fConvertPlatformSpecific( tPlatformId pid )
	{
		return true;
	}

	b32 tFxmlConverter::fOutput( tFileWriter& ofile, tPlatformId pid )
	{
		sigassert( ofile.fIsOpen( ) );

		// set the binary file signature
		this->fSetSignature< tFxFile >( pid );

		// output
		tLoadInPlaceSerializer ser;
		ser.fSave( static_cast< tFxFile& >( *this ), ofile, pid );

		return true;
	}

	
}
