#include "SigEdPch.hpp"
#include "tEditableMiniMapProperties.hpp"
#include "tEditorAppWindow.hpp"
#include "tWxSlapOnGroup.hpp"
#include "tWxToolsPanelSlider.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tWxSlapOnButton.hpp"
#include "tEditableObjectContainer.hpp"
#include "FileSystem.hpp"
#include "tEditorDialog.hpp"
#include "tPluginMinimapData.hpp"

namespace Sig
{
	namespace
	{

		s32 fAlignValue( s32 v, s32 alignment )
		{
			//align to the closest boundry
			const s32 h = fAlignHigh( v, alignment );
			const s32 l = fAlignLow( v, alignment );
			if( h - v > v - l )
				return l;
			else
				return h;
		}

		struct tMiniMapPaths
		{
			//INPUT
			const std::string mINPUT_AbsSigmlPath;
			const std::string mINPUT_TextureDir;
			const std::string mINPUT_NutDir;
			//OUTPUTS
			std::string mAbsToRes; //absolute path to res directory (including res\)
			std::string mRelativeDirectory; //ex: levels/maps/
			std::string mSimpleFileName; //ex: hl_mount_01
			std::string mRelTexturePath;
			std::string mAbsTexturePath;
			std::string mAbsNutPath;

			static void fSanitizeInPlace( std::string& s )
			{
				for( u32 i = 0; i < s.size( ); ++i )
				{
					if( s[i] == '\\' )
						s[i] = '/';
				}
			}

			tMiniMapPaths( const std::string& absFilePath, const std::string& textureDir, const std::string& nutDir )
				: mINPUT_AbsSigmlPath( absFilePath )
				, mINPUT_TextureDir( textureDir )
				, mINPUT_NutDir( nutDir )
			{
				sigassert( mINPUT_AbsSigmlPath.size( ) > 0 );

				//various indicies into path
				const u32 resPos = mINPUT_AbsSigmlPath.find( "res" );
				sigassert( resPos != std::string::npos );
				const u32 endOfRes = resPos + 4; //get past "res\"
				sigassert( endOfRes < mINPUT_AbsSigmlPath.size( ) );
				const u32 lastSlash = mINPUT_AbsSigmlPath.find_last_of( "\\/" );
				sigassert( lastSlash != std::string::npos );
				const u32 beforeFile = lastSlash + 1;
				sigassert( beforeFile < mINPUT_AbsSigmlPath.size( ) );
				sigassert( beforeFile >= endOfRes );

				//construct AbsToRes
				mAbsToRes = mINPUT_AbsSigmlPath.substr( 0, endOfRes );

				//construct relative directory
				const u32 relPathCount = beforeFile - endOfRes;
				mRelativeDirectory = mINPUT_AbsSigmlPath.substr( endOfRes, relPathCount ); //get all the data between ..\res\ and \filename.sigml

				//construct simple file name
				sigassert( mINPUT_AbsSigmlPath.find( ".sigml" ) == mINPUT_AbsSigmlPath.size( ) - 6 );
				const u32 simpleFileCount = mINPUT_AbsSigmlPath.size( ) - 6 - beforeFile;
				mSimpleFileName = mINPUT_AbsSigmlPath.substr( beforeFile, simpleFileCount );

				//construct final paths
				mRelTexturePath = mRelativeDirectory + mINPUT_TextureDir + mSimpleFileName + "_g.png";
				mAbsTexturePath = mAbsToRes + mRelTexturePath;
				mAbsNutPath = mAbsToRes + mRelativeDirectory + mINPUT_NutDir + mSimpleFileName + "_minimap.nut";


				//final: swap all '\' with '/'. script can only work with '/' and windows can handle both
				fSanitizeInPlace( mRelTexturePath );
				fSanitizeInPlace( mAbsTexturePath );
				fSanitizeInPlace( mAbsNutPath );
			}
		};

		class tWxIntegerBox : public tWxSlapOnTextBox
		{
			const s32 mAlignBoundry;
			s32 mVal;
		public:
			tWxIntegerBox( wxWindow* parent, const char* label, s32 alignBoundry, s32 startVal )
				: tWxSlapOnTextBox( parent, label )
				, mAlignBoundry( alignBoundry )
				, mVal( startVal )
			{
				fSetToOurValue( );
			}
			virtual void fOnControlUpdated( )
			{
				u32 val = atoi( fGetValue( ).c_str( ) );
				if( val <= 0 )
				{
					//invalid!
					val = mVal;
				}
				val = fAlignValue( val, mAlignBoundry );
				mVal = val;
				fSetToOurValue( );
			}
			s32 fGetOurValue( )
			{
				return mVal;
			}
			void fSetOurValue( s32 v )
			{
				sigassert( v > 0 );
				sigassert( fIsAligned<s32>( v, mAlignBoundry ) );
				mVal = v;
				fSetToOurValue( );
			}
		private:
			void fSetToOurValue( )
			{
				char buf[32];
				_itoa( mVal, buf, 10 );
				fSetValue( buf );
			}
		};
	}

	class tEditableMiniMapProperties : public tEditorDialog
	{
	public:
		tEditableMiniMapProperties( tEditorAppWindow* editorWindow );

		u32 fGetMaxMiniMapDimension( ) const;

	protected:

		///
		/// \brief Redefine this in your derived type if you want to save more properties. Be sure
		/// to call the base method though to save the standard properties.
		virtual void fSaveInternal( HKEY hKey );

		///
		/// \brief Redefine this in your derived type if you want to load more properties. Be sure
		/// to call the base method though to load the standard properties.
		virtual void fLoadInternal( HKEY hKey );


	private:
		tWxIntegerBox* mDim;
		tEditorAppWindow* mEditor;

		static const char* fMaxMiniMapDimension( ) { return "max_minimap_dim"; }

		void fSetAppearance( );
		void fGenerateMiniMap( wxCommandEvent& );
		void fWriteData( const std::string& texturePath, f32 unitSize, const Math::tVec2i& pixelCenter );
	};



	tEditableMiniMapProperties::tEditableMiniMapProperties( tEditorAppWindow* editorWindow )
		: tEditorDialog( editorWindow, "EditableMiniMapProperties" )
		, mDim( NULL )
		, mEditor( editorWindow )
	{
		// force window to specific size
		const int width = 376;
		const int height = 156;
		SetMinSize( wxSize( width, height ) );
		SetMaxSize( wxSize( width, height ) );
		SetSize( wxSize( width, height ) );

		SetTitle( "Mini Map Creator" );


		tWxSlapOnGroup *group = new tWxSlapOnGroup( this, "Max Side Dimension", false );

		mDim = new tWxIntegerBox( group->fGetMainPanel( ), "Length", 4, 512 );
		group->fGetMainPanel( )->GetSizer( )->AddSpacer( 4 );

		tWxSlapOnButton* genButton = new tWxSlapOnButton( group->fGetMainPanel( ), "Generate", " " );
		genButton->fButton( )->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEditableMiniMapProperties::fGenerateMiniMap ), NULL, this );

		fLoad( );
		Layout( );
	}
	u32 tEditableMiniMapProperties::fGetMaxMiniMapDimension( ) const
	{
		return mDim->fGetOurValue( );
	}
	void tEditableMiniMapProperties::fSaveInternal( HKEY hKey )
	{
		tEditorDialog::fSaveInternal( hKey );
		Win32Util::fSetRegistryKeyValue( hKey, mDim->fGetOurValue( ), fMaxMiniMapDimension( ) );
	}
	void tEditableMiniMapProperties::fLoadInternal( HKEY hKey )
	{
		tEditorDialog::fLoadInternal( hKey );
		int d;
		if( Win32Util::fGetRegistryKeyValue( hKey, d, fMaxMiniMapDimension( ) ) )
			mDim->fSetOurValue( d );
	}

	void tEditableMiniMapProperties::fGenerateMiniMap( wxCommandEvent& )
	{
		if( mEditor->fGuiApp( ).fRecentFiles( ).fCount( ) == 0 )
			return;
		if( mEditor->fGuiApp( ).fRecentFiles( )[0].fCStr( ) != mEditor->fGuiApp( ).fDocName( ) ) //make sure the current doc name is not "(untitled)"
			return;

		const u32 TEXTURE_ALIGNMENT = 4;

		//get screen
		const Gfx::tScreenPtr& screen = mEditor->fRenderPanelContainer( )->fGetFocusRenderPanel( )->fGetScreen( );
		sigassert( screen );

		//get bounding volume
		const Math::tAabbf box = mEditor->fGuiApp( ).fEditableObjects( ).fComputeBounding( );
		const f32 boxWidth = box.fWidth( );
		const f32 boxDepth = box.fDepth( ); //use depth because we are looking straight down at this level

		//choose texture dimension using max dim
		const u32 maxDim = fGetMaxMiniMapDimension( );
		sigassert( fIsAligned( maxDim, TEXTURE_ALIGNMENT ) );
		u32 texWidth = ~0;
		u32 texHeight = ~0;
		if( boxWidth > boxDepth )
		{
			texWidth = maxDim;
			texHeight = maxDim * boxDepth / boxWidth;
			texHeight = fAlignHigh<u32>( texHeight, TEXTURE_ALIGNMENT );
		}
		else
		{
			texWidth = maxDim * boxWidth / boxDepth;
			texHeight = maxDim;
			texWidth = fAlignHigh<u32>( texWidth, TEXTURE_ALIGNMENT );
		}
		sigassert( texWidth <= maxDim );
		sigassert( texHeight <= maxDim );
		sigassert( fIsAligned( texWidth, TEXTURE_ALIGNMENT ) );
		sigassert( fIsAligned( texHeight, TEXTURE_ALIGNMENT ) );

		//create texture to render to
		Gfx::tRenderToTexturePtr tex( NEW Gfx::tRenderToTexture( Gfx::tDevice::fGetDefaultDevice( ), texWidth, texHeight, Gfx::tRenderTarget::cFormatRGBA8, Gfx::tRenderTarget::cFormatD24S8, 0 ) );
		D3DSURFACE_DESC desc;
		tex->fRenderTarget( )->fGetSurface( )->GetDesc( &desc );
		sigassert( desc.Format == D3DFMT_A8R8G8B8 );

		//render from above
		screen->fRenderMiniMap( *tex, box );


		//construct all the paths we need
		tMiniMapPaths paths( mEditor->fGuiApp( ).fDocName( ), "minimaps\\", "scripts\\" );

		//create directories if they aren't there
		FileSystem::fCreateDirectory( tFilePathPtr( paths.mAbsToRes + paths.mRelativeDirectory + paths.mINPUT_TextureDir ) );
		FileSystem::fCreateDirectory( tFilePathPtr( paths.mAbsToRes + paths.mRelativeDirectory + paths.mINPUT_NutDir ) );

		//save texture file
		ToolsPaths::fCheckoutAdd( tFilePathPtr( paths.mAbsTexturePath ) );
		tex->fRenderTarget( )->fSaveToDisk( tFilePathPtr( paths.mAbsTexturePath ) );
		log_line( 0, "Mini Map texture saved to: " << paths.mAbsTexturePath );


		//gen nut
		const f32 unitSize = boxWidth > boxDepth
			? texWidth / boxWidth
			: texHeight / boxDepth;
		const Math::tVec3f boxCenter = box.fComputeCenter( );
		const Math::tVec2i pixelCenter( texWidth / 2.0f - boxCenter.x * unitSize, texHeight / 2.0f - boxCenter.z * unitSize );
	
		fWriteData( paths.mRelTexturePath, unitSize, pixelCenter );

		log_line( 0, "Mini Map data added to sigml. Path: " << paths.mRelTexturePath << " Center: " << pixelCenter << " Scale: " << unitSize );	
	}


	/*
		Plugin example. The code above is merely implementation details.
	*/

	/* 
		Editor-only information example.

		RTTI:
			Use null reflector because we dont need binary RTTI support.
			No need to run RTTIGen
			We only need the factory creation RTTI functionality and XML serialization, no binary support needed on this object.
	*/
	class tEditorPluginMiniMapData : public tEditorPluginData
	{
		declare_null_reflector( );
		implement_rtti_serializable_base_class( tEditorPluginMiniMapData, 0x997189BD );
	public:
		std::string		mTexturePath;
		Math::tVec2f	mWorldOrigin;
		f32				mUnitScale;

		// You must pass your plugin's unique id so assetgen knows were the data came from.
		tEditorPluginMiniMapData( )
			: tEditorPluginData( tMiniMapEditorPlugin::cUniqueID )
		{ }

		virtual tEditorPluginData* fClone( ) const
		{
			tEditorPluginMiniMapData* data = new tEditorPluginMiniMapData( );
			*data = *this;
			return data;
		}

		virtual void fSerialize( tXmlSerializer& s ) { fSerializeDerived( s ); }
		virtual void fSerialize( tXmlDeserializer& s ) { fSerializeDerived( s ); }

		template< typename t >
		void fSerializeDerived( t& s )
		{
			s( "TexPath", mTexturePath );
			s( "Origin", mWorldOrigin );
			s( "UnitScale", mUnitScale );
		}
	};

	// Factory must be registered in a cpp file.
	register_rtti_factory( tEditorPluginMiniMapData, false );


	/* 
		This plugin simply creates a global dialog that operates on the current scene.

		editor may be null in the case of the asset gen plugin who is trying to only use the factory.
	*/
	void tMiniMapEditorPlugin::fConstruct( tEditorAppWindow* editor )
	{
		if( editor )
			mDialog = new tEditableMiniMapProperties( editor );
	}

	/* 
		That when invoked, will write some data to the scene.
	*/
	void tEditableMiniMapProperties::fWriteData( const std::string& texturePath, f32 unitSize, const Math::tVec2i& pixelCenter )
	{
		tEditorPluginMiniMapData* data = mEditor->fFirstPluginData< tEditorPluginMiniMapData >( );
		if( !data )
		{
			data = new tEditorPluginMiniMapData( );
			mEditor->fPluginData( ).fPushBack( tEditorPluginDataPtr( data ) );
		}

		data->mUnitScale = unitSize;
		data->mWorldOrigin.x = pixelCenter.x;
		data->mWorldOrigin.y = pixelCenter.y;
		data->mTexturePath = texturePath;

		// TODO, support undo. :/
		mEditor->fGuiApp( ).fActionStack( ).fForceSetDirty( );
	}

	/* 
		This is called when the menu option is chosen for this plugin.
		Toggle the visibility. 
	*/
	void tMiniMapEditorPlugin::fToggle( )
	{
		mDialog->fToggle( );
	}

	/* 
		This is called the scene is cleared or a new document has been loaded.
		Reinitialize your GUI if necessary.
	*/
	void tMiniMapEditorPlugin::fFileOpened( ) 
	{ 
		//tEditorPluginMiniMapData* data = mEditor->fFirstPluginData< tEditorPluginMiniMapData >( );

		//std::string texturePath = data ? data->mTexturePath : "";
		//mDialog->fSetData( texturePath );
	}

	/* 
		This function is called by asset gen to convert the tEditorPluginData to tEntityData, ingame binary format
	*/
	tEntityData* tMiniMapEditorPlugin::fSerializeData( tLoadInPlaceFileBase* fileOut, tEditorPluginData* baseDataPtr )
	{
		// Original data stored on scene file.
		tEditorPluginMiniMapData* data = dynamic_cast<tEditorPluginMiniMapData*>( baseDataPtr );
		sigassert( data );

		// Data to be binary serialized for loading ingame.
		tPluginMiniMapData* dataOut = new tPluginMiniMapData( );

		dataOut->mTexture = fileOut->fAddLoadInPlaceResourcePtr( tResourceId::fMake<Gfx::tTextureFile>( tFilePathPtr( data->mTexturePath ) ) );
		dataOut->mWorldOrigin = data->mWorldOrigin;
		dataOut->mUnitScale = data->mUnitScale;

		return dataOut;
	}


}

