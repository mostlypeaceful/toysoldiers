#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tProjectFile.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"
#include "FileSystem.hpp"
#include "Win32Util.hpp"
#include <fstream>

namespace Sig
{
	///
	/// \section tProjectFile::tAssetGenConfig
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tAssetGenConfig& o )
	{
		s( "UseEnginePlugins", o.mUseEnginePlugins );
		s( "AdditionalPluginPaths", o.mAdditionalPluginPaths );
	}

	tProjectFile::tAssetGenConfig::tAssetGenConfig( )
		: mUseEnginePlugins( true )
	{
		mAdditionalPluginPaths.fPushBack( tFilePathPtr( "Bin" ) );
	}


	///
	/// \section tProjectFile::tGameTag
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tGameTag& o )
	{
		s( "Name", o.mName );
		s( "Key", o.mKey );
	}


	///
	/// \section tProjectFile::tEvent
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tEvent& o )
	{
		s( "Name", o.mName );
		s( "Key", o.mKey );
	}

	///
	/// \section tProjectFile::tGameEnumeratedValue
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tGameEnumeratedValue& o )
	{
		s( "Name", o.mName );
		s( "Key", o.mKey );
	}

	///
	/// \section tProjectFile::tGameEnumeratedTypeAlias
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tGameEnumeratedTypeAlias& o )
	{
		s( "Name", o.mName );
		s( "Key", o.mKey );
		s( "Hide", o.mHide );
	}

	///
	/// \section tProjectFile::tGameEnumeratedType
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tGameEnumeratedType& o )
	{
		s( "Name", o.mName );
		s( "Key", o.mKey );
		s( "Hide", o.mHide );
		s( "Aliases", o.mAliases );
		s( "Values", o.mValues );
	}

	u32 tProjectFile::tGameEnumeratedType::fFindValueIndexByKey( u32 key ) const
	{
		for( u32 i = 0; i < mValues.fCount( ); ++i )
			if( mValues[ i ].mKey == key )
				return i;

		return ~0;
	}

	u32 tProjectFile::tGameEnumeratedType::fFindValueIndexByName( const std::string& name ) const
	{
		for( u32 i = 0; i < mValues.fCount( ); ++i )
			if( _stricmp( mValues[ i ].mName.c_str( ), name.c_str( ) ) == 0 )
				return i;
		return ~0;
	}

	void tProjectFile::tGameEnumeratedType::fInsertValue( const std::string& name, u32 index )
	{
		mValues.fInsertSafe( index, tGameEnumeratedValue( name, fNextKey( mValues ) ) );
	}

	std::string tProjectFile::tEvent::fCppName( const std::string& name )
	{
		return "GAME_EVENT_" + fToCppName( name );
	}

	///
	/// \section tProjectFile
	///


	namespace
	{
		static tProjectFile gCachedProjectFile;
		static Time::tStamp gCachedProjectFileStamp = ~0;
	}

	const tProjectFile& tProjectFile::fGetCurrentProjectFileCached( f32 maxTimeBetweenCaches )
	{
		const Time::tStamp now = Time::fGetStamp( );
		if( gCachedProjectFileStamp != ~0 && Time::fGetElapsedS( gCachedProjectFileStamp, now ) < maxTimeBetweenCaches )
			return gCachedProjectFile;

		gCachedProjectFileStamp = now;

		if( FileSystem::fFileExists( ToolsPaths::fGetCurrentProjectFilePath( ) ) )
			gCachedProjectFile.fLoadXml( ToolsPaths::fGetCurrentProjectFilePath( ) );

		return gCachedProjectFile;
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile& o )
	{
		s( "ShadowMapLayerCount", o.mShadowMapLayerCount );
		s( "AssetGenConfig", o.mAssetGenConfig );
		s( "GameFlags", o.mGameTags );
		s( "GameEnumeratedValues", o.mGameEnumeratedTypes );
		s( "GameEvents", o.mGameEvents );
		s( "KeyframeEvents", o.mKeyFrameEvents );
	}

	tProjectFile::tProjectFile( )
		: mShadowMapLayerCount( 1 )
	{
	}

	tProjectFile::~tProjectFile( )
	{
	}

	b32 tProjectFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		return ser.fSave( path, "SigProj", *this, promptToCheckout );
	}

	b32 tProjectFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		b32 result = des.fLoad( path, "SigProj", *this );

		if( !result ) return false;

		//validate keys are unique
		std::stringstream errors;

		tGrowableArray<u32> keys;
		for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
		{
			tGameEnumeratedType& e = mGameEnumeratedTypes[ i ];
			if( keys.fFind( e.mKey ) )
			{
				errors << "Enum key " << e.mKey << " used more than once. ( " + e.mName + " )" << std::endl;
			}
			else 
				keys.fPushBack( e.mKey );

			for( u32 a = 0; a < e.mAliases.fCount( ); ++a )
			{
				tGameEnumeratedTypeAlias& al = e.mAliases[ a ];
				if( keys.fFind( al.mKey ) )
				{
					errors << "Enum key " << al.mKey << " used more than once. ( " + al.mName + " )" << std::endl;
				}
				else 
					keys.fPushBack( al.mKey );
			}

			tGrowableArray<u32> valKeys;
			for( u32 v = 0; v < e.mValues.fCount( ); ++v )
			{
				tGameEnumeratedValue& val = e.mValues[ v ];
				if( valKeys.fFind( val.mKey ) )
				{
					errors << "Enum value key " << val.mKey << " used more than once. ( " + e.mName + ":" + val.mName + " )" << std::endl;
				}
				else 
					valKeys.fPushBack( val.mKey );
			}
		}

		keys.fSetCount( 0 );
		for( u32 i = 0; i < mGameTags.fCount( ); ++i )
		{
			tGameTag& t = mGameTags[ i ];
			if( keys.fFind( t.mKey ) )
			{
				errors << "Tag key " << t.mKey << " used more than once. ( " + t.mName + " )" << std::endl;
			}
			else 
				keys.fPushBack( t.mKey );
		}

		keys.fSetCount( 0 );
		for( u32 i = 0; i < mGameEvents.fCount( ); ++i )
		{
			tEvent& e = mGameEvents[ i ];
			if( keys.fFind( e.mKey ) )
			{
				errors << "Event key " << e.mKey << " used more than once. ( " + e.mName + " )" << std::endl;
			}
			else 
				keys.fPushBack( e.mKey );
		}

		keys.fSetCount( 0 );
		for( u32 i = 0; i < mKeyFrameEvents.fCount( ); ++i )
		{
			tEvent& e = mKeyFrameEvents[ i ];
			if( keys.fFind( e.mKey ) )
			{
				errors << "Key Frame Event key " << e.mKey << " used more than once. ( " + e.mName + " )" << std::endl;
			}
			else 
				keys.fPushBack( e.mKey );
		}

		log_assert( errors.str( ).length( ) == 0, "Project XML file has a key conflict." << std::endl << errors.str( ) );
		return true;
	}

	std::string tProjectFile::fToCppName( const std::string& in )
	{
		std::string out = in;
		for( u32 i = 0; i < out.length( ); ++i )
		{
			if( isalpha( out[ i ] ) )	out[ i ] = toupper( out[ i ] );
			else if( !isdigit( out[ i ] ) ) out[ i ] = '_';
		}
		return out;
	}

	void tProjectFile::fSetupProjectDefaults( )
	{
		// these are just some very basic examples that make it easier to start editing the file

		mGameTags.fPushBack( tGameTag( "Edible", 0 ) );
		mGameTags.fPushBack( tGameTag( "Visible", 1 ) );
		mGameTags.fPushBack( tGameTag( "Collidable", 2 ) );

		tGameEnumeratedType damageType;
		damageType.mName = "Damage Type";
		damageType.mKey = 0;
		damageType.mValues.fPushBack( tGameEnumeratedValue( "Bludgeoning", 0 ) );
		damageType.mValues.fPushBack( tGameEnumeratedValue( "Slashing", 1 ) );
		damageType.mValues.fPushBack( tGameEnumeratedValue( "Piercing", 2 ) );

		tGameEnumeratedType iceCreamFlavor;
		iceCreamFlavor.mName = "Ice Cream Flavor";
		damageType.mKey = 1;
		iceCreamFlavor.mValues.fPushBack( tGameEnumeratedValue( "Vanilla", 0 ) );
		iceCreamFlavor.mValues.fPushBack( tGameEnumeratedValue( "Chocolate", 1 ) );
		iceCreamFlavor.mValues.fPushBack( tGameEnumeratedValue( "Pralines 'N Cream", 2 ) );

		mGameEnumeratedTypes.fPushBack( damageType );
		mGameEnumeratedTypes.fPushBack( iceCreamFlavor );

		mGameEvents.fPushBack( tEvent( "Invalid", 0 ) );
		mGameEvents.fPushBack( tEvent( "Fall", 1 ) );
		mGameEvents.fPushBack( tEvent( "Land", 2 ) );
	}

	void tProjectFile::fCompileGameSettings( b32 autoBuildRes ) const
	{
		fWriteGameSettingsHpp( );
		fWriteGameSettingsCpp( );

		if( autoBuildRes )
			system( "BuildRes.cmd" );
	}
	u32 tProjectFile::fToTagBitIndexFromKey( u32 key ) const
	{
		for( u32 i = 0; i < mGameTags.fCount( ); ++i )
			if( mGameTags[ i ].mKey == key ) return i;
		return ~0;
	}
	const tProjectFile::tGameTag* tProjectFile::fFindTagByName( const std::string& tagName ) const
	{
		for( u32 i = 0; i < mGameTags.fCount( ); ++i )
		{
			if( mGameTags[ i ].mName == tagName )
				return &mGameTags[ i ];
		}

		return NULL;
	}
	const tProjectFile::tGameTag* tProjectFile::fFindTagByKey( u32 key ) const
	{
		for( u32 i = 0; i < mGameTags.fCount( ); ++i )
			if( mGameTags[ i ].mKey == key ) return &mGameTags[ i ];

		return NULL;
	}
	const tProjectFile::tGameEnumeratedType* tProjectFile::fFindEnumeratedTypeByName( const std::string& enumName, u32* keyOut ) const
	{
		for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
		{
			if( mGameEnumeratedTypes[ i ].mName == enumName )
			{
				if( keyOut )
					*keyOut = mGameEnumeratedTypes[ i ].mKey;
				return &mGameEnumeratedTypes[ i ];
			}
			else
			{
				const tGameEnumeratedTypeAlias* alias = mGameEnumeratedTypes[ i ].mAliases.fFind( enumName );
				if( alias )
				{
					if( keyOut )
						*keyOut = alias->mKey;
					return &mGameEnumeratedTypes[ i ];
				}
			}
		}
		return 0;
	}
	const tProjectFile::tGameEnumeratedType* tProjectFile::fFindEnumeratedTypeByKey( u32 key ) const
	{
		for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
		{
			if( mGameEnumeratedTypes[ i ].mKey == key )
				return &mGameEnumeratedTypes[ i ];

			for( u32 a = 0; a < mGameEnumeratedTypes[ i ].mAliases.fCount( ); ++a )
				if( mGameEnumeratedTypes[ i ].mAliases[ a ].mKey == key )
					return &mGameEnumeratedTypes[ i ];
		}

		return NULL;
	}

	u32 tProjectFile::fFindKeyframeEventIndexByKey( u32 key ) const
	{
		for( u32 i = 0; i < mKeyFrameEvents.fCount( ); ++i )
		{
			if( mKeyFrameEvents[ i ].mKey == key )
				return i;
		}

		return ~0;
	}
	const tProjectFile::tEvent* tProjectFile::fFindEventByName( const std::string& name, const tGrowableArray< tEvent >& list )
	{
		for( u32 i = 0; i < list.fCount( ); ++i )
		{
			if( list[ i ].mName == name )
				return &list[ i ];
		}

		return NULL;
	}
	tProjectFile::tEvent* tProjectFile::fFindEventByKey( u32 key, tGrowableArray< tEvent >& list )
	{
		for( u32 i = 0; i < list.fCount( ); ++i )
		{
			if( list[ i ].mKey == key )
				return &list[ i ];
		}

		return NULL;
	}
	const tProjectFile::tEvent* tProjectFile::fFindEventByKey( u32 key, const tGrowableArray< tEvent >& list )
	{
		for( u32 i = 0; i < list.fCount( ); ++i )
		{
			if( list[ i ].mKey == key )
				return &list[ i ];
		}

		return NULL;
	}
	void tProjectFile::fWriteGameSettingsHpp( ) const
	{
		const tFilePathPtr filePath = tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectSrcFolder( ), tFilePathPtr( "GameFlags.hpp" ) );

		if( Win32Util::fIsFileReadOnly( filePath ) )
			ToolsPaths::fCheckout( filePath );

		std::ofstream file( filePath.fCStr( ) );

		file << "#ifndef __GameFlags__" << std::endl;
		file << "#define __GameFlags__" << std::endl;
		file << std::endl;

		file << "// Auto-generated file containing the project's list of game flags and game enumerations." << std::endl;
		file << "// These values are exposed to the editor, script, and code." << std::endl;
		file << "// DO NOT EDIT THIS FILE MANUALLY" << std::endl;
		file << std::endl;
		file << "namespace Sig { namespace GameFlags" << std::endl;
		file << "{" << std::endl;

		// Flags
		{
			file << std::endl;
			file << "\t" << "// Flags..." << std::endl;
			file << std::endl;

			for( u32 i = 0; i < mGameTags.fCount( ); ++i )
				file << "\t" << "static const u32 cFLAG_" << fToCppName( mGameTags[ i ].mName ) << " = ( 1u << " << i << "u );" << std::endl;
			file << "\t" << "static const u32 cFLAG_COUNT = " << mGameTags.fCount( ) << ";" << std::endl;
		}

		// Game Events
		{
			file << std::endl;
			file << "\t" << "// Game Events..." << std::endl;
			file << std::endl;

			for( u32 i = 0; i < mGameEvents.fCount( ); ++i )
				file << "\t" << "static const u32 cEVENT_" << fToCppName( mGameEvents[ i ].mName ) << " = " << i << "u;" << std::endl;
			file << "\t" << "static const u32 cEVENT_COUNT = " << mGameEvents.fCount( ) << ";" << std::endl;

			file << std::endl;
			file << "\t" << "const tStringPtr& fGameEventToString( u32 eventValue );      // returns \"GAME_EVENT_VALUE_NAME\"" << std::endl; 
			file << "\t" << "u32 fGameEventToValue( const tStringPtr& name );  " << std::endl; 
		}

		// KeyFrame Events
		{
			file << std::endl;
			file << "\t" << "// KeyFrame Events..." << std::endl;
			file << std::endl;

			for( u32 i = 0; i < mKeyFrameEvents.fCount( ); ++i )
				file << "\t" << "static const u32 cKEYFRAME_EVENT_" << fToCppName( mKeyFrameEvents[ i ].mName ) << " = " << i << "u;" << std::endl;
			file << "\t" << "static const u32 cKEYFRAME_EVENT_COUNT = " << mKeyFrameEvents.fCount( ) << ";" << std::endl;
		}

		// Enums
		{
			file << std::endl;
			file << "\t" << "// Enumerated Types..." << std::endl;
			file << std::endl;

			u32 numEnumTypes = mGameEnumeratedTypes.fCount( );
			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
			{
				file << "\t" << "static const u32 cENUM_" << fToCppName( mGameEnumeratedTypes[ i ].mName ) << " = " << mGameEnumeratedTypes[ i ].mKey << "u;" << std::endl;
				for( u32 j = 0; j < mGameEnumeratedTypes[ i ].mAliases.fCount( ); ++j )
					file << "\t" << "static const u32 cENUM_" << fToCppName( mGameEnumeratedTypes[ i ].mAliases[ j ].mName ) << " = " << mGameEnumeratedTypes[ i ].mAliases[ j ].mKey << "u;" << std::endl;
				numEnumTypes += mGameEnumeratedTypes[ i ].mAliases.fCount( );
			}
			file << "\t" << "static const u32 cENUM_COUNT = " << numEnumTypes << ";" << std::endl;

			file << std::endl;
			file << "\t" << "// Enumerated Type Values..." << std::endl;

			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
			{
				// write the enum
				const std::string cppEnumName = fToCppName( mGameEnumeratedTypes[ i ].mName );
				file << std::endl;
				file << "\t" << "enum t" << cppEnumName << std::endl;
				file << "\t" << "{" << std::endl;
				for( u32 j = 0; j < mGameEnumeratedTypes[ i ].mValues.fCount( ); ++j )
					file << "\t\t" << "c" << cppEnumName << "_" << fToCppName( mGameEnumeratedTypes[ i ].mValues[ j ].mName ) << " = " << j << "u," << std::endl;
				file << "\t\t" << "c" << cppEnumName << "_COUNT = " << mGameEnumeratedTypes[ i ].mValues.fCount( ) << "," << std::endl;
				file << "\t" << "};" << std::endl;

				// declare conversion funcs
				file << "\t" << "const tStringPtr& f" << cppEnumName << "EnumToString( u32 enumValue );      // returns \"" << cppEnumName << "_VALUE_NAME\"" << std::endl;  
				file << "\t" << "const tStringPtr& f" << cppEnumName << "EnumToValueString( u32 enumValue ); // returns \"VALUE_NAME\"" << std::endl;
				file << "\t" << "u32 f" << cppEnumName << "StringToEnum( const tStringPtr& enumString );" << std::endl;
				file << "\t" << "u32 f" << cppEnumName << "ValueStringToEnum( const tStringPtr& enumValueString );" << std::endl;
			}
		}

		file << std::endl;
		file << "}}" << std::endl;
		file << std::endl;

		file << std::endl;

		file << "namespace Sig { namespace GameFlags { namespace Detail" << std::endl;
		file << "{" << std::endl;
		file << std::endl;
		file << "\textern const tStringPtr      cGameFlagNames[];" << std::endl;
		file << "\textern const u32             cGameFlagValues[];" << std::endl;
		file << "\textern const tStringPtr      cGameEnumTypeNames[];" << std::endl;
		file << "\textern const u32             cGameEnumTypeKeys[];" << std::endl;
		file << "\textern const tStringPtr*const cGameEnumValueNames[];" << std::endl;
		file << "\textern const u32*const       cGameEnumValues[];" << std::endl;
		file << "\textern const tStringPtr      cGameEventNames[];" << std::endl;
		file << "\textern const u32             cGameEventValues[];" << std::endl;
		file << "\textern const tStringPtr      cKeyFrameEventNames[];" << std::endl;
		file << "\textern const u32             cKeyFrameEventValues[];" << std::endl;
		file << std::endl;
		file << "}}}" << std::endl;
		file << std::endl;

		file << "#endif//__GameFlags__" << std::endl;
		file << std::endl;
	}
	void tProjectFile::fWriteGameSettingsCpp( ) const
	{
		const tFilePathPtr cppPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectSrcFolder( ), tFilePathPtr( "GameFlags.cpp" ) );
		const tFilePathPtr nutPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectResFolder( ), tFilePathPtr( "GameFlags.nut" ) );

		if( Win32Util::fIsFileReadOnly( cppPath ) )
			ToolsPaths::fCheckout( cppPath );
		if( Win32Util::fIsFileReadOnly( nutPath ) )
			ToolsPaths::fCheckout( nutPath );

		std::ofstream fcpp( cppPath.fCStr( ) );
		std::ofstream fnut( nutPath.fCStr( ) );

		fcpp << std::endl;
		fcpp << "#include \"GameAppPch.hpp\"" << std::endl;
		fcpp << "#include \"gameflags.hpp\"" << std::endl;
		fcpp << std::endl;
		fcpp << "// Auto-generated file containing the project's list of game flags and game enumerations." << std::endl;
		fcpp << "// These values are exposed to the editor, script, and code." << std::endl;
		fcpp << "// DO NOT EDIT THIS FILE MANUALLY" << std::endl;
		fcpp << std::endl;
		fcpp << "namespace Sig { namespace GameFlags { namespace Detail" << std::endl;
		fcpp << "{" << std::endl;

		fnut << "// Auto-generated file containing the project's list of game flags and game enumerations." << std::endl;
		fnut << "// These values are exposed to the editor, script, and code." << std::endl;
		fnut << "// DO NOT EDIT THIS FILE MANUALLY" << std::endl;
		fnut << std::endl;

		// Flags
		{
			fcpp << std::endl;
			fcpp << "\tconst tStringPtr cGameFlagNames[]={" << std::endl;
			for( u32 i = 0; i < mGameTags.fCount( ); ++i )
			{
				const std::string name = "FLAG_" + fToCppName( mGameTags[ i ].mName );
				fcpp << "\t\t" << "tStringPtr( \"" << name << "\" )," << std::endl;
				fnut << "const " << name << " = " << ( 1u << i ) << std::endl;
			}
			fcpp << "\t\t" << "tStringPtr::cNullPtr" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
			fnut << std::endl;

			fcpp << std::endl;
			fcpp << "\tconst u32 cGameFlagValues[]={" << std::endl;
			for( u32 i = 0; i < mGameTags.fCount( ); ++i )
				fcpp << "\t\t" << "( 1u << " << i << "u )," << std::endl;
			fcpp << "\t\t" << "0" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
		}

		// Game Events
		{
			fcpp << std::endl;
			fcpp << "\tconst tStringPtr cGameEventNames[]={" << std::endl;
			for( u32 i = 0; i < mGameEvents.fCount( ); ++i )
			{
				const std::string name = tEvent::fCppName( mGameEvents[ i ].mName );
				fcpp << "\t\t" << "tStringPtr( \"" << name << "\" )," << std::endl;
				fnut << "const " << name << " = " << i << std::endl;
			}
			fcpp << "\t\t" << "tStringPtr::cNullPtr" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
			fnut << std::endl;

			fcpp << std::endl;
			fcpp << "\tconst u32 cGameEventValues[]={" << std::endl;
			for( u32 i = 0; i < mGameEvents.fCount( ); ++i )
				fcpp << "\t\t" << "( " << i << "u )," << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
		}

		// KeyFrame Events
		{
			fcpp << std::endl;
			fcpp << "\tconst tStringPtr cKeyFrameEventNames[]={" << std::endl;
			for( u32 i = 0; i < mKeyFrameEvents.fCount( ); ++i )
			{
				const std::string name = "KEYFRAME_EVENT_" + fToCppName( mKeyFrameEvents[ i ].mName );
				fcpp << "\t\t" << "tStringPtr( \"" << name << "\" )," << std::endl;
				fnut << "const " << name << " = " << i << std::endl;
			}
			fcpp << "\t\t" << "tStringPtr::cNullPtr" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
			fnut << std::endl;

			fcpp << std::endl;
			fcpp << "\tconst u32 cKeyFrameEventValues[]={" << std::endl;
			for( u32 i = 0; i < mKeyFrameEvents.fCount( ); ++i )
				fcpp << "\t\t" << "( " << i << "u )," << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
		}

		// Enums
		{
			fcpp << std::endl;
			fcpp << "\tconst tStringPtr cGameEnumTypeNames[]={" << std::endl;
			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
			{
				{
					const std::string name = "ENUM_" + fToCppName( mGameEnumeratedTypes[ i ].mName );
					fcpp << "\t\t" << "tStringPtr( \"" << name << "\" )," << std::endl;
					fnut << "const " << name << " = " << mGameEnumeratedTypes[ i ].mKey << std::endl;
				}
				for( u32 j = 0; j < mGameEnumeratedTypes[ i ].mAliases.fCount( ); ++j )
				{
					const std::string name = "ENUM_" + fToCppName( mGameEnumeratedTypes[ i ].mAliases[ j ].mName );
					fcpp << "\t\t" << "tStringPtr( \"" << name << "\" )," << std::endl;
					fnut << "const " << name << " = " << mGameEnumeratedTypes[ i ].mAliases[ j ].mKey << std::endl;
				}
			}
			fcpp << "\t\t" << "tStringPtr::cNullPtr" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
			fnut << std::endl;

			u32 numEnumTypes = mGameEnumeratedTypes.fCount( );
			fcpp << std::endl;
			fcpp << "\tconst u32 cGameEnumTypeKeys[]={" << std::endl;
			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
			{
				fcpp << "\t\t" << mGameEnumeratedTypes[ i ].mKey << "," << std::endl;
				for( u32 j = 0; j < mGameEnumeratedTypes[ i ].mAliases.fCount( ); ++j )
					fcpp << "\t\t" << j << "," << std::endl;
				numEnumTypes += mGameEnumeratedTypes[ i ].mAliases.fCount( );
			}
			fcpp << "\t\t" << "~0" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;



			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
			{
				fcpp << std::endl;
				fcpp << "\tstatic const tStringPtr cGameEnumValueNames" << i << "[]={" << std::endl;
				const std::string cppEnumName = fToCppName( mGameEnumeratedTypes[ i ].mName );
				for( u32 j = 0; j < mGameEnumeratedTypes[ i ].mValues.fCount( ); ++j )
				{
					const std::string name = cppEnumName + "_" + fToCppName( mGameEnumeratedTypes[ i ].mValues[ j ].mName );
					fcpp << "\t\ttStringPtr( \"" << name << "\" )," << std::endl;
					fnut << "const " << name << " = " << j << std::endl;
				}
				fcpp << "\t\t" << "tStringPtr::cNullPtr" << std::endl;
				fcpp << "\t};" << std::endl;
				fcpp << std::endl;

				fnut << "const " << cppEnumName + "_COUNT = " << mGameEnumeratedTypes[ i ].mValues.fCount( ) << std::endl;
				fnut << std::endl;

				fcpp << "\tstatic const tStringPtr cGameEnumValueNamesRaw" << i << "[]={" << std::endl;
				for( u32 j = 0; j < mGameEnumeratedTypes[ i ].mValues.fCount( ); ++j )
					fcpp << "\t\ttStringPtr( \"" << fToCppName( mGameEnumeratedTypes[ i ].mValues[ j ].mName ) << "\" )," << std::endl;
				fcpp << "\t\t" << "tStringPtr::cNullPtr" << std::endl;
				fcpp << "\t};" << std::endl;
			}
			fcpp << std::endl;
			fcpp << "\tconst tStringPtr*const cGameEnumValueNames[]={" << std::endl;
			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
				fcpp << "\t\tcGameEnumValueNames" << i << "," << std::endl;
			fcpp << "\t\t0" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
			fcpp << "\tconst tStringPtr*const cGameEnumValueNamesRaw[]={" << std::endl;
			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
				fcpp << "\t\tcGameEnumValueNamesRaw" << i << "," << std::endl;
			fcpp << "\t\t0" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;


			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
			{
				fcpp << std::endl;
				fcpp << "\tstatic const u32 cGameEnumValues" << i << "[]={" << std::endl;
				for( u32 j = 0; j < mGameEnumeratedTypes[ i ].mValues.fCount( ); ++j )
					fcpp << "\t\t" << j << "u," << std::endl;
				fcpp << "\t\t" << "~0" << std::endl;
				fcpp << "\t};" << std::endl;
			}
			fcpp << std::endl;
			fcpp << "\tconst u32*const cGameEnumValues[]={" << std::endl;
			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
				fcpp << "\t\tcGameEnumValues" << i << "," << std::endl;
			fcpp << "\t\t0" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;

			fcpp << std::endl;
			fcpp << "}}}" << std::endl;
			fcpp << std::endl;

			fcpp << std::endl;
			fcpp << "namespace Sig { namespace GameFlags" << std::endl;
			fcpp << "{" << std::endl;

			// define conversion funcs
			fcpp << "\tconst tStringPtr& fGameEventToString( u32 eventValue )"  << std::endl; 
			fcpp << "\t{" << std::endl;
			fcpp << "\t\t" << "for( u32 i = 0; i < array_length( Detail::cGameEventValues ); ++i )" << std::endl;
			fcpp << "\t\t\t" << "if( Detail::cGameEventValues[ i ] == eventValue )" << std::endl;
			fcpp << "\t\t\t\t" << "return Detail::cGameEventNames[ i ];" << std::endl;
			fcpp << "\t\t" << "return tStringPtr::cNullPtr;" << std::endl;
			fcpp << "\t}" << std::endl;

			fcpp << "\t" << "u32 fGameEventToValue( const tStringPtr& name )" << std::endl;
			fcpp << "\t" << "{" << std::endl;
			fcpp << "\t\t" << "for( u32 i = 0; i < array_length( Detail::cGameEventNames ); ++i )" << std::endl;
			fcpp << "\t\t\t" << "if( Detail::cGameEventNames[ i ] == name )" << std::endl;
			fcpp << "\t\t\t\t" << "return Detail::cGameEventValues[ i ];" << std::endl;
			fcpp << "\t\t" << "return cEVENT_COUNT;" << std::endl;
			fcpp << "\t" << "}" << std::endl;

			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
			{
				const std::string cppEnumName = fToCppName( mGameEnumeratedTypes[ i ].mName );

				fcpp << "\t" << "const tStringPtr& f" << cppEnumName << "EnumToString( u32 enumValue )" << std::endl;
				fcpp << "\t" << "{" << std::endl;
				fcpp << "\t\t" << "for( u32 i = 0; i < array_length( Detail::cGameEnumValues" << i << " ); ++i )" << std::endl;
				fcpp << "\t\t\t" << "if( Detail::cGameEnumValues" << i << "[ i ] == enumValue )" << std::endl;
				fcpp << "\t\t\t\t" << "return Detail::cGameEnumValueNames" << i << "[ i ];" << std::endl;
				fcpp << "\t\t" << "return tStringPtr::cNullPtr;" << std::endl;
				fcpp << "\t" << "}" << std::endl;

				fcpp << "\t" << "const tStringPtr& f" << cppEnumName << "EnumToValueString( u32 enumValue )" << std::endl;
				fcpp << "\t" << "{" << std::endl;
				fcpp << "\t\t" << "for( u32 i = 0; i < array_length( Detail::cGameEnumValues" << i << " ); ++i )" << std::endl;
				fcpp << "\t\t\t" << "if( Detail::cGameEnumValues" << i << "[ i ] == enumValue )" << std::endl;
				fcpp << "\t\t\t\t" << "return Detail::cGameEnumValueNamesRaw" << i << "[ i ];" << std::endl;
				fcpp << "\t\t" << "return tStringPtr::cNullPtr;" << std::endl;
				fcpp << "\t" << "}" << std::endl;

				fcpp << "\t" << "u32 f" << cppEnumName << "StringToEnum( const tStringPtr& enumString )" << std::endl;
				fcpp << "\t" << "{" << std::endl;
				fcpp << "\t\t" << "for( u32 i = 0; i < array_length( Detail::cGameEnumValueNames" << i << " ); ++i )" << std::endl;
				fcpp << "\t\t\t" << "if( Detail::cGameEnumValueNames" << i << "[ i ] == enumString )" << std::endl;
				fcpp << "\t\t\t\t" << "return Detail::cGameEnumValues" << i << "[ i ];" << std::endl;
				fcpp << "\t\t" << "return c" << cppEnumName << "_COUNT;" << std::endl;
				fcpp << "\t" << "}" << std::endl;

				fcpp << "\t" << "u32 f" << cppEnumName << "ValueStringToEnum( const tStringPtr& enumValueString )" << std::endl;
				fcpp << "\t" << "{" << std::endl;
				fcpp << "\t\t" << "for( u32 i = 0; i < array_length( Detail::cGameEnumValueNamesRaw" << i << " ); ++i )" << std::endl;
				fcpp << "\t\t\t" << "if( Detail::cGameEnumValueNamesRaw" << i << "[ i ] == enumValueString )" << std::endl;
				fcpp << "\t\t\t\t" << "return Detail::cGameEnumValues" << i << "[ i ];" << std::endl;
				fcpp << "\t\t" << "return c" << cppEnumName << "_COUNT;" << std::endl;
				fcpp << "\t" << "}" << std::endl;
			}
			fcpp << std::endl;
		}

		fcpp << "}}" << std::endl;
		fcpp << std::endl;
	}
	
}

#else
void dkjhdlsajfhldkhfasf134240( ){ }
#endif//defined( platform_pcdx9 ) || defined( platform_pcdx10 )
