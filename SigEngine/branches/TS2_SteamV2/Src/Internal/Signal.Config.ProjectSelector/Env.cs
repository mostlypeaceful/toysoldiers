//------------------------------------------------------------------------------
// \file Env.cs - 1 Mar 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security;
using System.Windows.Forms;

namespace Signal.Config.ProjectSelector
{
	static class Env
	{
		public enum Scope
		{
			System,
			User,
			Perforce,	// ~ p4 set command
			Process,
		}

		static bool WarnedRegistryUAC = false;
		static void WarnRegistryUAC( )
		{
			if( !WarnedRegistryUAC )
				MessageBox.Show( "Couldn't access all registry keys.  You may need to run this as admin.", "UAC Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning );
			WarnedRegistryUAC = true;
		}

		static RegistryKey TryOpenSubKey( RegistryKey parent, string path, bool writeable )
		{
			try
			{
				return parent.OpenSubKey( path, writeable );
			}
			catch( SecurityException ) { return null; } // UAC / non-admin prompt?
		}

		static readonly Dictionary<Scope,RegistryKey> cRegistryKeys = new Dictionary<Scope,RegistryKey>( )
		{
			{ Scope.System,		TryOpenSubKey( Registry.LocalMachine, @"SYSTEM\CurrentControlSet\Control\Session Manager\Environment", true ) },
			{ Scope.User,		TryOpenSubKey( Registry.CurrentUser,  @"Environment", true ) },
			{ Scope.Perforce,	TryOpenSubKey( Registry.CurrentUser,  @"Software\Perforce\Environment", true ) },
		};

		public static string Get( Scope scope, string name )
		{
			return Get( scope, name, RegistryValueOptions.None );
		}

		public static string Get( Scope scope, string name, RegistryValueOptions valueOptions )
		{
			switch( scope )
			{
			case Scope.System:
			case Scope.Perforce:
			case Scope.User:
				{
					var key = cRegistryKeys[ scope ];
					if( key == null )
					{
						WarnRegistryUAC( );
						return null;
					}
					return cRegistryKeys[ scope ].GetValue( name, "", valueOptions ) as string;
				}
			case Scope.Process:
				return Environment.GetEnvironmentVariable( name );
			default:
				throw new ArgumentOutOfRangeException( "scope" );
			}
		}


		public static void Set( Scope scope, string name, string value )
		{
			Set( scope, name, value, RegistryValueKind.Unknown );
		}

		public static void Set( Scope scope, string name, string value, RegistryValueKind valueKind )
		{
			switch( scope )
			{
			case Scope.System:
			case Scope.Perforce:
			case Scope.User:
				cRegistryKeys[ scope ].SetValue( name, value, valueKind );
				break;
			case Scope.Process:
				Environment.SetEnvironmentVariable( name, value, EnvironmentVariableTarget.Process );
				break;
			default:
				throw new ArgumentOutOfRangeException( "scope" );
			}
			Environment.SetEnvironmentVariable( name, value, EnvironmentVariableTarget.Process );
		}

		public unsafe static void BroadcastEnvVarsChanged( )
		{
			IntPtr result;
			var environment = Marshal.StringToHGlobalAuto( "Environment" );
			SendMessageTimeout( new IntPtr( HWND_BROADCAST ), WM_SETTINGCHANGE, IntPtr.Zero, environment, SendMessageTimeoutFlags.SMTO_ABORTIFHUNG, 5000, out result );
			Marshal.FreeHGlobal( environment );
		}

		[Flags]
		enum SendMessageTimeoutFlags : uint
		{
			SMTO_NORMAL				= 0x0,
			SMTO_BLOCK				= 0x1,
			SMTO_ABORTIFHUNG		= 0x2,
			SMTO_NOTIMEOUTIFNOTHUNG	= 0x8
		}

		const int HWND_BROADCAST = 0xFFFF;
		const uint WM_SETTINGCHANGE = 0x001A;

		[DllImport("user32.dll", SetLastError=true, CharSet=CharSet.Auto)]
		static extern IntPtr SendMessageTimeout(
			IntPtr windowHandle, 
			uint Msg, 
			IntPtr wParam, 
			IntPtr lParam, 
			SendMessageTimeoutFlags flags, 
			uint timeout, 
			out IntPtr result);
	}
}
