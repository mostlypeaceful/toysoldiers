//------------------------------------------------------------------------------
// \file ProfileData.cs - 30 Apr 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

using Microsoft.Win32;
using System;
using System.IO;
using System.Windows.Forms;

namespace Signal.Config.ProjectSelector
{
	/// <summary>
	/// Represents all the settings associated with a profile.
	/// This does *not* include the profile name, which I'm treating more like a filename.
	/// See Profiles.cs for profile enumeration etc.
	/// </summary>
	class ProfileData
	{
		public string	WorkSpaceOverride	{ get; set; }
		public string	EnginePath			{ get; set; }
		public string	ProjectPath			{ get; set; }
		public string	DeployToDir			{ get; set; }
		public string	PullFromPath		{ get; set; }
		public string	PerforcePort		{ get; set; }
		public bool		SyncChangelist		{ get; set; }

		const string
			cWorkSpaceOverrideName	= "WorkSpaceOverride",
			cEnginePath				= "EnginePath",
			cProjectPath			= "ProjectPath",
			cDeployTo				= "DeployTo",
			cPullFrom				= "PullFrom",
			cPerforcePort			= "PerforcePort",
			cActiveProfile			= "ActiveProfile",
			cSyncChangelist			= "SyncChangelist";

		public ProfileData Clone( )
		{
			return (ProfileData)MemberwiseClone( );
		}

		void FillDefaults( RegistryKey key )
		{
			var i = key.Name.LastIndexOf( '\\' );
			FillDefaults( key.Name.Substring( i == -1 ? 0 : i+1 ) );
		}

		void FillDefaults( string profileName )
		{
			if( string.IsNullOrEmpty( WorkSpaceOverride ) )	WorkSpaceOverride	= "";
			if( string.IsNullOrEmpty( EnginePath ) )		EnginePath			= string.Format( @"C:\SignalStudios\{0}\SigEngine", profileName );
			if( string.IsNullOrEmpty( ProjectPath ) )		ProjectPath			= string.Format( @"C:\SignalStudios\{0}\Project", profileName );
			if( string.IsNullOrEmpty( DeployToDir ) )		DeployToDir			= profileName;
			if( string.IsNullOrEmpty( PullFromPath ) )		PullFromPath		= string.Format( @"\\build-{0}\{0}\Project\Builds\", profileName );
			if( string.IsNullOrEmpty( PerforcePort ) )		PerforcePort		= "";
		}

		public static ProfileData LoadFrom( RegistryKey key )
		{
			var pd = new ProfileData( )
			{
				WorkSpaceOverride	= key.GetValue( cWorkSpaceOverrideName ) as string,
				EnginePath			= key.GetValue( cEnginePath ) as string,
				ProjectPath			= key.GetValue( cProjectPath ) as string,
				DeployToDir			= key.GetValue( cDeployTo ) as string,
				PullFromPath		= key.GetValue( cPullFrom ) as string,
				PerforcePort		= key.GetValue( cPerforcePort ) as string,
				SyncChangelist		= key.GetValue( cSyncChangelist ) as string == "1",
			};
			pd.FillDefaults( key );
			return pd;
		}

		public void SaveTo( RegistryKey key )
		{
			FillDefaults( key );
			key.SetValue( cWorkSpaceOverrideName,	WorkSpaceOverride );
			key.SetValue( cEnginePath,				EnginePath );
			key.SetValue( cProjectPath,				ProjectPath );
			key.SetValue( cDeployTo,				DeployToDir );
			key.SetValue( cPullFrom,				PullFromPath );
			key.SetValue( cPerforcePort,			PerforcePort );
			key.SetValue( cSyncChangelist,			SyncChangelist ? "1" : "0" );
		}

		string EffectiveP4Client( string profileName )
		{
			if( !string.IsNullOrEmpty( WorkSpaceOverride ) )
				return WorkSpaceOverride;

			var p4user = (Env.Get( Env.Scope.Perforce, "P4USER" ) ?? Environment.UserName).ToLowerInvariant( );
			var p4profile = profileName.Replace( ' ', '_' ).ToLowerInvariant( );
			return string.Format( "{0}_{1}", p4user, p4profile );
		}

		string EffectiveP4Port
		{
			get { return string.IsNullOrEmpty( PerforcePort ) ? "ssl:sig-perforce:1666" : PerforcePort; }
		}

		public void SetUserEnvironmentVariables( string profileName )
		{
			// WorkSpaceOverride
			Env.Set( Env.Scope.User,			"MAYA_PLUG_IN_PATH",		Path.Combine( EnginePath, @"bin" ) );
			Env.Set( Env.Scope.User,			"MAYA_SCRIPT_PATH",			Path.Combine( EnginePath, @"bin\mayascripts" ) );
			Env.Set( Env.Scope.User,			"PYTHONPATH",				Path.Combine( EnginePath, @"bin\mayascripts" ) );
			Env.Set( Env.Scope.User,			"XBMLANGPATH",				Path.Combine( EnginePath, @"bin\mayascripts\icons" ) );
			Env.Set( Env.Scope.User,			"XEDK",						Path.Combine( EnginePath, @"Src\External\Microsoft Xbox 360 SDK" ) );
			Env.Set( Env.Scope.User,			"SigPath",					Path.Combine( EnginePath, @"bin" ) + ";" + Path.Combine( EnginePath, @"bin\external\Maya" ) );
			Env.Set( Env.Scope.User,			"SigEngine",				EnginePath );
			Env.Set( Env.Scope.User,			"SigCurrentProject",		ProjectPath );
			Env.Set( Env.Scope.User,			"SigCurrentProjectName",	profileName );
			Env.Set( Env.Scope.User,			"SigDeployTo",				DeployToDir );
			Env.Set( Env.Scope.User,			"SigPullFrom",				PullFromPath );
			Env.Set( Env.Scope.Perforce,		"P4CLIENT",					EffectiveP4Client( profileName ) );
			Env.Set( Env.Scope.Perforce,		"P4PORT",					EffectiveP4Port );
			
			if( string.IsNullOrEmpty( Env.Get( Env.Scope.Perforce, "P4USER" ) ) )
				Env.Set( Env.Scope.Perforce,	"P4USER",					Environment.UserName );

			var path = Env.Get( Env.Scope.User,	"PATH",						RegistryValueOptions.DoNotExpandEnvironmentNames ) ?? "";
			if( !path.Contains( "%SigPath%" ) )
				Env.Set( Env.Scope.User,		"PATH", "%SigPath%;"+path,	RegistryValueKind.ExpandString );

			Env.BroadcastEnvVarsChanged( );
		}
	}
}
