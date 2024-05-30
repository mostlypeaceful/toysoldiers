//------------------------------------------------------------------------------
// \file Profiles.cs - 30 Apr 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Signal.Config.ProjectSelector
{
	/// <summary>
	/// Manage all the profiles saved on this computer.
	/// </summary>
	static class Profiles
	{
		static readonly	RegistryKey	ProjectProfileList = Registry.CurrentUser.CreateSubKey( @"Software\SignalStudios\ProjectProfiles" );
		const			string		cActiveProfile = "ActiveProfile";

		public static IEnumerable<string> Names { get
		{
            string value = ( string ) ProjectProfileList.GetValue( null );
            return String.IsNullOrEmpty( value ) ? new String[0] : value.Split( ";".ToCharArray(), StringSplitOptions.RemoveEmptyEntries );
		}}

		public static string ActiveProfileName
		{
			get
			{
				return Names.FirstOrDefault( name =>
				{
					using( var subkey = ProjectProfileList.CreateSubKey( name ) )
						return subkey.GetValue( cActiveProfile ) as string == "1";
				});
			}
			set
			{
				foreach( var name in Names )
				{
					using( var subkey = ProjectProfileList.CreateSubKey( name ) )
					{
						var active = name.ToLower( ) == value.ToLower( );
						subkey.SetValue( cActiveProfile, active ? "1" : "0" );
						var pd = ProfileData.LoadFrom( subkey );
						if( active )
							pd.SetUserEnvironmentVariables( name );
					}
				}
			}
		}

		public static Dictionary<string,ProfileData> LoadAll( )
		{
			return Names.ToDictionary( name => name, name =>
			{
				using( var subkey = ProjectProfileList.CreateSubKey( name ) )
				{
					return ProfileData.LoadFrom( subkey );
				}
			});
		}

		public static void Save( string name, ProfileData data )
		{
			using( var subkey = ProjectProfileList.CreateSubKey( name ) )
				data.SaveTo( subkey );
			ProjectProfileList.SetValue( null, string.Join( ";", Names.Concat(new[]{name}).Distinct().ToArray() ) );
		}

		public static void Delete( string name )
		{
			ProjectProfileList.SetValue( null, string.Join( ";", Names.Except(new[]{name}).Distinct().ToArray() ) );
		}
	}
}
