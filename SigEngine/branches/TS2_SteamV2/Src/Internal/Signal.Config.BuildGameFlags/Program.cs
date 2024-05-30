//------------------------------------------------------------------------------
// \file Program.cs - 1 May 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Signal.Config.BuildGameFlags
{
	class Program
	{
		static void Checkout( string path )
		{
			var psi = new ProcessStartInfo( Environment.ExpandEnvironmentVariables( @"%SigEngine%\bin\OpenForEdit.cmd" ), string.Format( "\"{0}\"", path ) )
			{
				WorkingDirectory	= Path.GetDirectoryName( path ),
				WindowStyle			= ProcessWindowStyle.Hidden,
			};
			Process.Start( psi ).WaitForExit( );
		}

		static StreamWriter PrepareToWrite( string path, bool ignoreFailures )
		{
			path = Environment.ExpandEnvironmentVariables( path );

			Checkout( path );

			Action<string,string> onErr = (title,message) =>
			{
				if( !ignoreFailures )
					MessageBox.Show( "File not found for writing:\n"+path, "File not found", MessageBoxButtons.OK, MessageBoxIcon.Error );
			};

			try
			{
				return new StreamWriter( path );
			}
			catch( FileNotFoundException )		{ onErr( "File not found", "File not found for writing:\n"+path ); }
			catch( DirectoryNotFoundException ) { onErr( "Directory not found", "Directory of the file not found for writing:\n"+path ); }

			return new StreamWriter( new MemoryStream( ) ); // Create a dummy StreamWriter and discard the results
		}

		static void Main( string[] args )
		{
			var projectXmlPath
				= args.Length > 0 ? args[0]
				: Environment.ExpandEnvironmentVariables( @"%SigCurrentProject%\res\project.xml" )
				;

			var projectXml = ProjectXml.LoadFrom( projectXmlPath );

			using( var hpp = PrepareToWrite( @"%SigCurrentProject%\Src\GameFlags.hpp", false ) )
			using( var cpp = PrepareToWrite( @"%SigCurrentProject%\Src\GameFlags.cpp", false ) )
			using( var nut = PrepareToWrite( @"%SigCurrentProject%\Res\GameFlags.nut", false ) )
			using( var act = PrepareToWrite( @"%SigCurrentProject%\Res\Gui\Flash\~src\GameFlags.as", true ) )
			{
				WritePreamble( hpp, cHppTemplate );
				WritePreamble( cpp, cCppTemplate );
				WritePreamble( nut, cNutTemplate );
				WritePreamble( act, cActTemplate );

				WriteGameFlags		( hpp, cpp, nut, act, projectXml.GameFlags );
				WriteGameEvents		( hpp, cpp, nut, act, projectXml.GameEvents );
				WriteAiFlags		( hpp, cpp, nut, act, projectXml.AIFlags );
				WriteKeyframeEvents	( hpp, cpp, nut, act, projectXml.KeyframeEvents );
				WriteGameEnums		( hpp, cpp, nut, act, projectXml.GameEnums );

				WriteMiddle( hpp, cHppTemplate );
				WriteMiddle( cpp, cCppTemplate );
				WriteMiddle( nut, cNutTemplate );
				WriteMiddle( act, cActTemplate );

				WriteConversionFuncs( hpp, cpp, nut, act, projectXml.GameEnums );

				WritePostamble( hpp, cHppTemplate );
				WritePostamble( cpp, cCppTemplate );
				WritePostamble( nut, cNutTemplate );
				WritePostamble( act, cActTemplate );
			}
		}

		static string ToCodeName( string name )
		{
			name = name.ToUpperInvariant( );
			var sb = new StringBuilder( name.Length );
			foreach( char ch in name )
				sb.Append( char.IsLetterOrDigit(ch) ? ch : '_' );
			return sb.ToString( );
		}

		static void WriteGameFlags( StreamWriter hpp, StreamWriter cpp, StreamWriter nut, StreamWriter act, IEnumerable<NameKeyPair> gameFlags_ )
		{
			var gameFlags = gameFlags_.Select( (gf,i) => new { CodeName = ToCodeName( gf.Name ), Index = i } ).ToArray( );


			hpp.WriteLine( );
			hpp.WriteLine( @"	// Flags..." );
			hpp.WriteLine( );



			cpp.WriteLine( );
			cpp.WriteLine( @"	const tStringPtr cGameFlagNames[]={" );
			foreach( var gf in gameFlags )
				cpp.WriteLine( @"		tStringPtr( ""FLAG_{0}"" ),", gf.CodeName );
			cpp.WriteLine( @"		tStringPtr::cNullPtr" );
			cpp.WriteLine( @"	};" );
			cpp.WriteLine( );


			cpp.WriteLine( );
			cpp.WriteLine( @"	const u32 cGameFlagValues[]={" );
			foreach( var gf in gameFlags )
			{
				hpp.WriteLine( @"	static const u32 cFLAG_{0} = ( 1u << {1}u );", gf.CodeName, gf.Index );
				cpp.WriteLine( @"		( 1u << {0}u ),", gf.Index );
				act.WriteLine( @"		public static const FLAG_{0}:uint = {1}", gf.CodeName, 1u << gf.Index );
				nut.WriteLine( @"const FLAG_{0} = {1}", gf.CodeName, 1u << gf.Index );
			}
			cpp.WriteLine( @"		0" );
			cpp.WriteLine( @"	};" );
			cpp.WriteLine( );
			act.WriteLine( );
			nut.WriteLine( );



			hpp.WriteLine( @"	static const u32 cFLAG_COUNT = {0};", gameFlags.Length );
		}

		static void WriteGameEvents( StreamWriter hpp, StreamWriter cpp, StreamWriter nut, StreamWriter act, IEnumerable<NameKeyPair> gameEvents_ )
		{
			var gameEvents = gameEvents_.Select( (ge,i) => new { CodeName = ToCodeName( ge.Name ), Index = i } ).ToArray( );

			hpp.WriteLine( );
			hpp.WriteLine( @"	// Game Events..." );
			hpp.WriteLine( );



			cpp.WriteLine( );
			cpp.WriteLine( @"	const tStringPtr cGameEventNames[]={" );
			foreach( var ge in gameEvents )
			{
				cpp.WriteLine( @"		tStringPtr( ""GAME_EVENT_{0}"" ),", ge.CodeName );
			}
			cpp.WriteLine( @"		tStringPtr::cNullPtr" );
			cpp.WriteLine( @"	};" );
			cpp.WriteLine( );



			cpp.WriteLine( @"	const tStringPtr cGameEventValueNames[]={" );
			foreach( var ge in gameEvents )
				cpp.WriteLine( @"		tStringPtr( ""{0}"" ),", ge.CodeName );
			cpp.WriteLine( @"		tStringPtr::cNullPtr" );
			cpp.WriteLine( @"	};" );
			cpp.WriteLine( );


			cpp.WriteLine( );
			cpp.WriteLine( @"	const u32 cGameEventValues[]={" );
			foreach( var ge in gameEvents )
			{
				hpp.WriteLine( @"	static const u32 cEVENT_{0} = {1}u;", ge.CodeName, ge.Index );
				cpp.WriteLine( @"		( {0}u ),", ge.Index );
				nut.WriteLine( @"const GAME_EVENT_{0} = {1}", ge.CodeName, ge.Index );
				act.WriteLine( @"		public static const GAME_EVENT_{0}:uint = {1}", ge.CodeName, ge.Index );
			}
			cpp.WriteLine( @"		( 0u ), // placeholder to make name and value arrays the same length" );
			cpp.WriteLine( @"	};" );
			cpp.WriteLine( );

			hpp.WriteLine( @"	static const u32 cEVENT_COUNT = {0};", gameEvents.Length );

			hpp.WriteLine( );
			hpp.WriteLine( @"	const tStringPtr& fGameEventToString( u32 eventValue );			// returns ""GAME_EVENT_VALUE_NAME""" );
			hpp.WriteLine( @"	u32 fGameEventToValue( const tStringPtr& name );				// takes input like GAME_EVENT_VALUE_NAME" );
			hpp.WriteLine( @"	u32 fGameEventValueStringToEnum( const tStringPtr& name );		// takes input like VALUE_NAME" );
			hpp.WriteLine( );
			nut.WriteLine( );
			act.WriteLine( );
		}

		static void WriteKeyframeEvents( StreamWriter hpp, StreamWriter cpp, StreamWriter nut, StreamWriter act, IEnumerable<NameKeyPair> keyframeEvents_ )
		{
			var keyframeEvents = keyframeEvents_.Select( (ke,i) => new { CodeName = ToCodeName( ke.Name ), Index = i } ).ToArray( );

			hpp.WriteLine( @"	// KeyFrame Events..." );
			hpp.WriteLine( );

			cpp.WriteLine( );
			cpp.WriteLine( @"	const tStringPtr cKeyFrameEventNames[]={" );
			foreach( var ke in keyframeEvents )
				cpp.WriteLine( @"		tStringPtr( ""KEYFRAME_EVENT_{0}"" ),", ke.CodeName );
			cpp.WriteLine( @"		tStringPtr::cNullPtr" );
			cpp.WriteLine( @"	};" );
			cpp.WriteLine( );

			cpp.WriteLine( );
			cpp.WriteLine( @"	const u32 cKeyFrameEventValues[]={" );
			foreach( var ke in keyframeEvents )
				cpp.WriteLine( @"		( {0}u ),", ke.Index );
			cpp.WriteLine( @"	};" );
			cpp.WriteLine( );
			nut.WriteLine( );
			act.WriteLine( );


			foreach( var ke in keyframeEvents )
			{
				hpp.WriteLine( @"	static const u32 cKEYFRAME_EVENT_{0} = {1}u;", ke.CodeName, ke.Index );
				nut.WriteLine( @"const KEYFRAME_EVENT_{0} = {1}", ke.CodeName, ke.Index );
				act.WriteLine( @"		public static const KEYFRAME_EVENT_{0}:uint = {1}", ke.CodeName, ke.Index );
			}

			hpp.WriteLine( @"	static const u32 cKEYFRAME_EVENT_COUNT = {0};", keyframeEvents.Length );

			hpp.WriteLine( );
			nut.WriteLine( );
			act.WriteLine( );
		}

		static void WriteAiFlags( StreamWriter hpp, StreamWriter cpp, StreamWriter nut, StreamWriter act, IEnumerable<NameKeyPair> aiFlags_ )
		{
			var aiFlags = aiFlags_.Select( (af,i) => new { CodeName = ToCodeName( af.Name ), Value = i } ).ToArray( );



			hpp.WriteLine( @"	// AI Flags..." );
			hpp.WriteLine( );



			cpp.WriteLine( );
			cpp.WriteLine( @"	const tStringPtr cAIFlagNames[]={" );
			foreach( var af in aiFlags )
				cpp.WriteLine( @"		tStringPtr( ""AIFLAG_{0}"" ),", af.CodeName );
			cpp.WriteLine( @"		tStringPtr::cNullPtr" );
			cpp.WriteLine( @"	};" );
			cpp.WriteLine( );



			cpp.WriteLine( );
			cpp.WriteLine( @"	const tStringPtr cAIFlagValueNames[]={" );
			foreach( var af in aiFlags )
				cpp.WriteLine( @"		tStringPtr( ""{0}"" ),", af.CodeName );
			cpp.WriteLine( @"		tStringPtr::cNullPtr" );
			cpp.WriteLine( @"	};" );
			cpp.WriteLine( );



			cpp.WriteLine( );
			cpp.WriteLine( @"	const u32 cAIFlagValues[]={" );
			foreach( var af in aiFlags )
			{
				hpp.WriteLine( @"	static const u32 cAIFLAG_{0} = {1}u;", af.CodeName, af.Value );
				cpp.WriteLine( @"		( {0}u ),", af.Value );
				act.WriteLine( @"		public static const AIFLAG_{0}:uint = {1}", af.CodeName, af.Value );
				nut.WriteLine( @"const AIFLAG_{0} = {1}", af.CodeName, af.Value );
			}
			cpp.WriteLine( @"		( 0u ), // placeholder to make name and value arrays the same length" );
			cpp.WriteLine( @"	};" );
			cpp.WriteLine( );
			act.WriteLine( );
			nut.WriteLine( );


			hpp.WriteLine( @"	static const u32 cAIFLAG_COUNT = {0};", aiFlags.Length );
			hpp.WriteLine( @"	const tStringPtr& fAIFlagToString( u32 eventValue );      // returns ""AIFLAG_VALUE_NAME""" );
			hpp.WriteLine( @"	const tStringPtr& fAIFlagToValueString( u32 eventValue );      // returns ""VALUE_NAME""" );
			hpp.WriteLine( @"	u32 fAIFlagToValue( const tStringPtr& name );" );
			hpp.WriteLine( @"	u32 fAIFlagValueStringToValue( const tStringPtr& name );");
			hpp.WriteLine( );
		}

		static void WriteGameEnums( StreamWriter hpp, StreamWriter cpp, StreamWriter nut, StreamWriter act, IEnumerable<GameEnum> gameEnums_ )
		{
			var gameEnums = gameEnums_.Select( (ge,i) => new { CodeName = ToCodeName( ge.Name ), Key = ge.Key, Index = i, Values = ge.Values.Select( (v,vi) => new { CodeName = ToCodeName( v.Name ), Index = vi } ).ToArray( ) } ).ToArray( );

			hpp.WriteLine( @"	// Enumerated Types..." );
			hpp.WriteLine( );

			foreach( var ge in gameEnums )
			{
				hpp.WriteLine( "	static const u32 cENUM_{0} = {1}u;", ge.CodeName, ge.Key );
				nut.WriteLine( "const ENUM_{0} = {1}", ge.CodeName, ge.Key );
				act.WriteLine( "		public static const ENUM_{0}:uint = {1}", ge.CodeName, ge.Key );
			}
			hpp.WriteLine( "	static const u32 cENUM_COUNT = {0};", gameEnums.Length );
			hpp.WriteLine( );
			nut.WriteLine( );
			act.WriteLine( );


			cpp.WriteLine( );
			cpp.WriteLine( @"	const tStringPtr cGameEnumTypeNames[]={" );
			foreach( var ge in gameEnums )
				cpp.WriteLine( @"		tStringPtr( ""ENUM_{0}"" ),", ge.CodeName );
			cpp.WriteLine( @"		tStringPtr::cNullPtr" );
			cpp.WriteLine( @"	};" );
			cpp.WriteLine( );


			cpp.WriteLine( );
			cpp.WriteLine( @"	const u32 cGameEnumTypeKeys[]={" );
			foreach( var ge in gameEnums )
				cpp.WriteLine( @"		{0},", ge.Key );
			cpp.WriteLine( @"		~0u" );
			cpp.WriteLine( @"	};" );
			cpp.WriteLine( );


			hpp.WriteLine( @"	// Enumerated Type Values..." );
			hpp.WriteLine( );

			foreach( var ge in gameEnums )
			{
				hpp.WriteLine( @"	enum t{0}", ge.CodeName );
				hpp.WriteLine( @"	{" );

				cpp.WriteLine( );
				cpp.WriteLine( @"	static const tStringPtr cGameEnumValueNames{0}[]={{", ge.Index );

				foreach( var value in ge.Values )
				{
					hpp.WriteLine( @"		c{0}_{1} = {2}u,", ge.CodeName, value.CodeName, value.Index );
					cpp.WriteLine( @"		tStringPtr( ""{0}_{1}"" ),", ge.CodeName, value.CodeName );
					nut.WriteLine( @"const {0}_{1} = {2}", ge.CodeName, value.CodeName, value.Index );
					act.WriteLine( @"		public static const {0}_{1}:uint = {2}", ge.CodeName, value.CodeName, value.Index );
				}

				cpp.WriteLine( @"		tStringPtr::cNullPtr" );
				cpp.WriteLine( @"	};" );
				cpp.WriteLine( );
				nut.WriteLine( @"const {0}_COUNT = {1}", ge.CodeName, ge.Values.Length );
				nut.WriteLine( );
				act.WriteLine( @"		public static const {0}_COUNT:uint = {1}", ge.CodeName, ge.Values.Length );
				act.WriteLine( );


				cpp.WriteLine( @"	static const tStringPtr cGameEnumValueNamesRaw{0}[]={{", ge.Index );
				foreach( var value in ge.Values )
					cpp.WriteLine( @"		tStringPtr( ""{0}"" ),", value.CodeName );
				cpp.WriteLine( @"		tStringPtr::cNullPtr" );
				cpp.WriteLine( @"	};" );

				hpp.WriteLine( @"		c{0}_COUNT = {1},", ge.CodeName, ge.Values.Length );
				hpp.WriteLine( @"	};" );
				hpp.WriteLine( @"	const tStringPtr& f{0}EnumToString( u32 enumValue );      // returns ""{0}_VALUE_NAME""", ge.CodeName );
				hpp.WriteLine( @"	const tStringPtr& f{0}EnumToValueString( u32 enumValue ); // returns ""VALUE_NAME""", ge.CodeName );
				hpp.WriteLine( @"	u32 f{0}StringToEnum( const tStringPtr& enumString );", ge.CodeName );
				hpp.WriteLine( @"	u32 f{0}StringToRequiredEnum( const tStringPtr& enumString );", ge.CodeName );
				hpp.WriteLine( @"	u32 f{0}ValueStringToEnum( const tStringPtr& enumValueString );", ge.CodeName );
				hpp.WriteLine( @"	u32 f{0}ValueStringToRequiredEnum( const tStringPtr& enumValueString );", ge.CodeName );
				hpp.WriteLine( );

			}


			cpp.WriteLine( );
			cpp.WriteLine( @"	const tStringPtr*const cGameEnumValueNames[]={" );
			foreach( var ge in gameEnums )
				cpp.WriteLine( @"		cGameEnumValueNames{0},", ge.Index );
			cpp.WriteLine( @"		0" );
			cpp.WriteLine( @"	};" );


			cpp.WriteLine( );
			cpp.WriteLine( @"	const tStringPtr*const cGameEnumValueNamesRaw[]={" );
			foreach( var ge in gameEnums )
				cpp.WriteLine( @"		cGameEnumValueNamesRaw{0},", ge.Index );
			cpp.WriteLine( @"		0" );
			cpp.WriteLine( @"	};" );
			cpp.WriteLine( );

			foreach( var ge in gameEnums )
			{
				cpp.WriteLine( );
				cpp.WriteLine( @"	static const u32 cGameEnumValues{0}[]={{", ge.Index );

				foreach( var value in ge.Values )
				{
					cpp.WriteLine( @"		{0}u,", value.Index );
				}

				cpp.WriteLine( @"		~0u" );
				cpp.WriteLine( @"	};" );
			}

			cpp.WriteLine( );
			cpp.WriteLine( @"	const u32*const cGameEnumValues[]={" );
			foreach( var ge in gameEnums )
				cpp.WriteLine( @"		cGameEnumValues{0},", ge.Index );
			cpp.WriteLine( @"		0" );
			cpp.WriteLine( @"	};" );
			cpp.WriteLine( );
			cpp.WriteLine( );
		}

		static void WritePreamble( StreamWriter writer, string template )
		{
			writer.Write( template.Substring( 0, template.IndexOf( cGeneratedMarker ) ).TrimStart("\r\n".ToCharArray()) );
		}

		static void WriteMiddle( StreamWriter writer, string template )
		{
			var startGenerated = template.IndexOf( cGeneratedMarker );
			var endGenerated = startGenerated + cGeneratedMarker.Length;
			if( startGenerated == -1 )
				startGenerated = endGenerated = template.Length;

			var startGeneratedFuncs = template.IndexOf( cGeneratedFuncsMarker );
			var endGeneratedFuncs = startGeneratedFuncs + cGeneratedFuncsMarker.Length;
			if( startGeneratedFuncs == -1 )
				startGeneratedFuncs = endGeneratedFuncs = template.Length;

			writer.Write( template.Substring( endGenerated, startGeneratedFuncs-endGenerated ) );
		}

		static void WritePostamble( StreamWriter writer, string template )
		{
			var startGeneratedFuncs = template.IndexOf( cGeneratedFuncsMarker );
			var endGeneratedFuncs = startGeneratedFuncs + cGeneratedFuncsMarker.Length;
			if( startGeneratedFuncs == -1 )
				startGeneratedFuncs = endGeneratedFuncs = template.Length;

			writer.Write( template.Substring( endGeneratedFuncs ) );
		}

		static void WriteConversionFuncs( StreamWriter hpp, StreamWriter cpp, StreamWriter nut, StreamWriter act, IEnumerable<GameEnum> gameEnums_ )
		{
			var gameEnums = gameEnums_.Select( (ge,i) => new { CodeName = ToCodeName( ge.Name ), Key = ge.Key, Index = i, Values = ge.Values.Select( (v,vi) => new { CodeName = ToCodeName( v.Name ), Index = vi } ).ToArray( ) } ).ToArray( );

			foreach( var ge in gameEnums )
			{
				cpp.WriteLine( @"	const tStringPtr& f{0}EnumToString( u32 enumValue )", ge.CodeName );
				cpp.WriteLine( @"	{" );
				cpp.WriteLine( @"		if( enumValue < array_length( Detail::cGameEnumValueNames{0} ) )", ge.Index );
				cpp.WriteLine( @"			return Detail::cGameEnumValueNames{0}[ enumValue ];", ge.Index );
				cpp.WriteLine( @"		return tStringPtr::cNullPtr;" );
				cpp.WriteLine( @"	}" );
				cpp.WriteLine( @"	const tStringPtr& f{0}EnumToValueString( u32 enumValue )", ge.CodeName );
				cpp.WriteLine( @"	{" );
				cpp.WriteLine( @"		if( enumValue < array_length( Detail::cGameEnumValueNamesRaw{0} ) )", ge.Index );
				cpp.WriteLine( @"			return Detail::cGameEnumValueNamesRaw{0}[ enumValue ];", ge.Index );
				cpp.WriteLine( @"		return tStringPtr::cNullPtr;" );
				cpp.WriteLine( @"	}" );
				cpp.WriteLine( @"	u32 f{0}StringToEnum( const tStringPtr& enumString )", ge.CodeName );
				cpp.WriteLine( @"	{" );
				cpp.WriteLine( @"		for( u32 i = 0; i < array_length( Detail::cGameEnumValueNames{0} ); ++i )", ge.Index );
				cpp.WriteLine( @"		{" );
				cpp.WriteLine( @"			if( Detail::cGameEnumValueNames{0}[ i ] == enumString )", ge.Index );
				cpp.WriteLine( @"				return i;" );
				cpp.WriteLine( @"		}" );
				cpp.WriteLine( @"		return ~0u;" );
				cpp.WriteLine( @"	}" );
				cpp.WriteLine( @"	u32 f{0}StringToRequiredEnum( const tStringPtr& enumString )", ge.CodeName );
				cpp.WriteLine( @"	{" );
				cpp.WriteLine( @"		const u32 value = f{0}StringToEnum( enumString );", ge.CodeName );
				cpp.WriteLine( @"		log_assert( value != ~0u, ""Could not convert enum string '"" << enumString << ""' to t{0}"" );", ge.CodeName );
				cpp.WriteLine( @"		return value;" );
				cpp.WriteLine( @"	}" );
				cpp.WriteLine( @"	u32 f{0}ValueStringToEnum( const tStringPtr& enumValueString )", ge.CodeName );
				cpp.WriteLine( @"	{" );
				cpp.WriteLine( @"		for( u32 i = 0; i < array_length( Detail::cGameEnumValueNamesRaw{0} ); ++i )", ge.Index );
				cpp.WriteLine( @"		{" );
				cpp.WriteLine( @"			if( Detail::cGameEnumValueNamesRaw{0}[ i ] == enumValueString )", ge.Index );
				cpp.WriteLine( @"				return i;" );
				cpp.WriteLine( @"		}" );
				cpp.WriteLine( @"		return ~0u;" );
				cpp.WriteLine( @"	}" );
				cpp.WriteLine( @"	u32 f{0}ValueStringToRequiredEnum( const tStringPtr& enumValueString )", ge.CodeName );
				cpp.WriteLine( @"	{" );
				cpp.WriteLine( @"		const u32 value = f{0}ValueStringToEnum( enumValueString );", ge.CodeName );
				cpp.WriteLine( @"		log_assert( value != ~0u, ""Could not convert enum value string '"" << enumValueString << ""' to t{0}"" );", ge.CodeName );
				cpp.WriteLine( @"		return value;" );
				cpp.WriteLine( @"	}" );
			}
		}


		const string
			cGeneratedMarker = "{GENERATED}",
			cGeneratedFuncsMarker = "{GENERATED_FUNCS}";

		const string cActTemplate = @"

// Auto-generated file containing the project's list of game flags and game enumerations.
// These values are exposed to the editor, script, and code.
// DO NOT EDIT THIS FILE MANUALLY

package
{
	public class gameflags
	{
{GENERATED}
	}
}
";

		const string cNutTemplate = @"

// Auto-generated file containing the project's list of game flags and game enumerations.
// These values are exposed to the editor, script, and code.
// DO NOT EDIT THIS FILE MANUALLY

{GENERATED}
";

		const string cHppTemplate = @"
#ifndef __GameFlags__
#define __GameFlags__

// Auto-generated file containing the project's list of game flags and game enumerations.
// These values are exposed to the editor, script, and code.
// DO NOT EDIT THIS FILE MANUALLY

namespace Sig { namespace GameFlags
{
{GENERATED}
}}


namespace Sig { namespace GameFlags { namespace Detail
{

	extern const tStringPtr      cGameFlagNames[];
	extern const u32             cGameFlagValues[];
	extern const tStringPtr      cGameEnumTypeNames[];
	extern const u32             cGameEnumTypeKeys[];
	extern const tStringPtr*const cGameEnumValueNames[];
	extern const u32*const       cGameEnumValues[];
	extern const tStringPtr      cGameEventNames[];
	extern const u32             cGameEventValues[];
	extern const tStringPtr      cKeyFrameEventNames[];
	extern const u32             cKeyFrameEventValues[];

}}}

#endif//__GameFlags__

";

		const string cCppTemplate = @"
#include ""GameAppPch.hpp""
#include ""gameflags.hpp""

// Auto-generated file containing the project's list of game flags and game enumerations.
// These values are exposed to the editor, script, and code.
// DO NOT EDIT THIS FILE MANUALLY

namespace Sig { namespace GameFlags { namespace Detail
{
{GENERATED}
}}}


namespace Sig { namespace GameFlags
{
	const tStringPtr& fGameEventToString( u32 eventValue )
	{
		for( u32 i = 0; i < array_length( Detail::cGameEventValues ); ++i )
			if( Detail::cGameEventValues[ i ] == eventValue )
				return Detail::cGameEventNames[ i ];
		return tStringPtr::cNullPtr;
	}
	u32 fGameEventToValue( const tStringPtr& name )
	{
		for( u32 i = 0; i < array_length( Detail::cGameEventNames ); ++i )
			if( Detail::cGameEventNames[ i ] == name )
				return Detail::cGameEventValues[ i ];
		return ~0u;
	}
	u32 fGameEventValueStringToEnum( const tStringPtr& name )
	{
		for( u32 i = 0; i < array_length( Detail::cGameEventValueNames ); ++i )
			if( Detail::cGameEventValueNames[ i ] == name )
				return Detail::cGameEventValues[ i ];
		return ~0u;
	}
	const tStringPtr& fAIFlagToString( u32 eventValue )
	{
		for( u32 i = 0; i < array_length( Detail::cAIFlagValues ); ++i )
			if( Detail::cAIFlagValues[ i ] == eventValue )
				return Detail::cAIFlagNames[ i ];
		return tStringPtr::cNullPtr;
	}
	const tStringPtr& fAIFlagToValueString( u32 eventValue )
	{
		for( u32 i = 0; i < array_length( Detail::cAIFlagValueNames ); ++i )
			if( Detail::cAIFlagValues[ i ] == eventValue )
				return Detail::cAIFlagValueNames[ i ];
		return tStringPtr::cNullPtr;
	}
	u32 fAIFlagToValue( const tStringPtr& name )
	{
		for( u32 i = 0; i < array_length( Detail::cAIFlagNames ); ++i )
			if( Detail::cAIFlagNames[ i ] == name )
				return Detail::cAIFlagValues[ i ];
		return ~0u;
	}
	u32 fAIFlagValueStringToValue( const tStringPtr& name )
	{
		for( u32 i = 0; i < array_length( Detail::cAIFlagValueNames ); ++i )
			if( Detail::cAIFlagValueNames[ i ] == name )
				return Detail::cAIFlagValues[ i ];
		return ~0u;
	}
{GENERATED_FUNCS}
}}
";
	}
}
