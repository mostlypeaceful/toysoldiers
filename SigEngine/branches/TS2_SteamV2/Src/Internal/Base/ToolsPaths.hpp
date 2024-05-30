#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#ifndef __ToolsPaths__
#define __ToolsPaths__

namespace Sig { namespace ToolsPaths
{

	///
	/// \section Engine-related environment variables.
	///

	base_export void fConfigureEngineEnvironmentVariables( const tFilePathPtr& path );

	///
	/// \section Engine directories.
	///

	base_export tFilePathPtr 	fGetEngineRootFolder( );
	base_export tFilePathPtr 	fGetEngineBinFolder( );
	base_export tFilePathPtr 	fGetEngineTempFolder( );
	base_export tFilePathPtr 	fCreateTempEngineFilePath( const char* extension = 0, const tFilePathPtr& subFolder=tFilePathPtr(), const char* explicitName = 0 );

	///
	/// \section Current Project querying, setting, and misc.
	///

	base_export void			fSetCurrentProjectRootFolder( const std::string& name, const tFilePathPtr& path );
	base_export std::string		fGetCurrentProjectName( );
	base_export tFilePathPtr	fGetCurrentProjectRootFolder( );
	base_export tFilePathPtr	fGetCurrentProjectResFolder( );
	base_export tFilePathPtr	fGetCurrentProjectSrcFolder( );
	base_export tFilePathPtr	fGetCurrentProjectGameFolder( );
	base_export tFilePathPtr	fGetCurrentProjectGamePlatformFolder( tPlatformId pid );
	base_export tFilePathPtr	fGetCurrentProjectFilePath( );
	base_export tFilePathPtr	fGetGameRootPathFromProjectPath( const tFilePathPtr& path );
	base_export std::string		fGetCurrentP4User( );
	base_export void			fSetCurrentP4ClientWorkspaceFromProject( const std::string& project );
	base_export void			fSetCurrentP4ClientWorkspaceName( const std::string& workspaceName );

	///
	/// \brief See if the path is a source folder, or a sub folder of a source folder, or a file within either of the two.
	base_export b32				fIsUnderSourceFolder( const tFilePathPtr& path );

	///
	/// \brief See if the path is a dev folder, or a sub folder of a dev folder, or a file within either of the two.
	base_export b32				fIsUnderDevFolder( const tFilePathPtr& path );

	///
	/// \brief Turn an absolute path into a path relative to the current project's res folder. This is important
	/// for making resource names compatible between different users machines, as well as for making final resource
	/// names as used in-game (as the game will map the equivalent of "res" as its root folder).
	/// \return If 'path' is relative to the res folder, then the returned value with be the portion of the path
	/// following the res folder; if 'res' is not contained in the path, then the returned value will depend on
	/// the argument 'returnNullIfNotRelative'; if true, a null path will be returned, otherwise 'path' will be returned un-modified.
	base_export tFilePathPtr	fMakeResRelative( const tFilePathPtr& path, b32 returnNullIfNotRelative = false, tPlatformId pid = cCurrentPlatform );

	///
	/// \brief Take a path that is specified relative to the current project's res folder and make it absolute.
	/// \note The returned path will only be valid for the current user's machine; i.e., an absolute path on
	/// one user's machine may be different from the "equivalent" path on another user's machine.
	base_export tFilePathPtr	fMakeResAbsolute( const tFilePathPtr& path, tPlatformId pid = cCurrentPlatform );

	///
	/// \brief Validates several aspects of the file path
	/// \return If the path is valid it returns either the og path or the res relative path depending on
	/// the value of 'returnRelativePath', if the path is not valid it returns a null file path
	base_export tFilePathPtr	fValidateResPath ( 
									const tFilePathPtr & path,
									std::string * errorString = NULL,
									b32 returnRelativePath = false,
									tPlatformId pid = cCurrentPlatform );

	///
	/// \brief See fMakeResRelative. Requires you pass the platform id of the game folder to which you want the path to be relative.
	base_export tFilePathPtr	fMakeGameRelative( const tFilePathPtr& path, tPlatformId gamePid, b32 returnNullIfNotRelative = false, tPlatformId pid = cCurrentPlatform );

	///
	/// \brief Take a path that is specified relative to the current project's game\[platform]\ folder and make it absolute.
	/// \note The returned path will only be valid for the current user's machine; i.e., an absolute path on
	/// one user's machine may be different from the "equivalent" path on another user's machine.
	base_export tFilePathPtr	fMakeGameAbsolute( const tFilePathPtr& path, tPlatformId pid = cCurrentPlatform );

	///
	/// \brief Validates several aspects of the file path
	/// \return If the path is valid it returns either the og path or the game relative path depending on
	/// the value of 'returnRelativePath', if the path is not valid it returns a null file path
	base_export tFilePathPtr	fValidateGamePath ( 
									const tFilePathPtr & path,
									tPlatformId gamePid,
									std::string * errorString = NULL,
									b32 returnRelativePath = false,
									tPlatformId pid = cCurrentPlatform );

	///
	/// \brief Loops through all platforms, prepends the appropriate game folder to the path, and attempts to delete the file.
	base_export void			fDeleteGameFileForAllPlatforms( const tFilePathPtr& gameRelativePath );

	///
	/// \brief Get the base registry key name for signal tools apps.
	base_export std::string		fGetSignalRegistryKeyName( );

	///
	/// \brief Launch the game for the specified platform
	base_export void			fLaunchGame( tPlatformId platform, const std::string& gameOptions );

	/// 
	/// \brief Launch SigScript with the specified command line.
	base_export void			fLaunchSigScript( const std::string& commandLine );


	///
	/// \brief Warn that the current file is read-only, and prompt to check out
	base_export b32				fPromptToCheckout( const tFilePathPtr& path );
	base_export b32				fCheckout( const tFilePathPtr& path, b32 msgBoxOnError = true );

	///
	/// \brief Warn that the current file is either checked out from source control
	/// or is not the same revision as the source control server
	base_export b32				fIsOutOfDate( const tFilePathPtr& path );
	base_export b32				fIsCheckedOut ( const tFilePathPtr& path );


	///
	/// \brief Stores the minimum required information for a project setup. Project
	/// setup allows for users to have multiple game projects on one machine, including
	/// different revisions of the engine, and be able to switch back and forth
	/// between them easily. Configures environment variables, etc.
	class base_export tProjectProfile
	{
	public:

		b32				mActiveProfile;
		std::string		mProfileName;
		std::string		mWorkSpaceOverride;
		tFilePathPtr	mEnginePath;
		tFilePathPtr	mProjectPath;

		static void fLoadProfilesFromRegistry( tGrowableArray<tProjectProfile>& profiles );
		static void fSaveProfilesToRegistry( const tGrowableArray<tProjectProfile>& profiles );

		tProjectProfile( );
		~tProjectProfile( );
		std::string fToString( ) const;
		void fLoadFromRegistry( );
		void fSaveToRegistry( ) const;
		void fConfigureEnvironment( );

		b32 operator==( const tProjectProfile& other ) const { return mProfileName == other.mProfileName && mEnginePath == other.mEnginePath && mProjectPath == other.mProjectPath; }
	};

}}

#endif//__ToolsPaths__
#endif//defined( platform_pcdx9 ) || defined( platform_pcdx10 )
