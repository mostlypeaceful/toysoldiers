#ifndef __iMaterialGenPlugin__
#define __iMaterialGenPlugin__

namespace Sig
{
	///
	/// \brief Interface for generating and writing to file any required material resources (i.e., shader binaries).
	/// Material resource plugins will be invoked from within the MaterialGen application.
	class tools_export iMaterialGenPlugin
	{
	public:

		virtual ~iMaterialGenPlugin( ) { }

		///
		/// \brief Return a logical name describing the material type.
		virtual const char* fGetMaterialName( ) const = 0;

		///
		/// \brief Return the actual physical file path of the material file.
		virtual tFilePathPtr fMaterialFilePath( ) const = 0;

		///
		/// \brief Called once for the plugin to generate all required binaries for all platforms. The
		/// plugin is free to create the file(s) in a subfolder of the supplied root, but should make
		/// sure the output files are actually within the specified folder.
		virtual void fGenerateMaterialFile( const tFilePathPtr& exePath, b32 force ) = 0;
	};
}

#endif // __iMaterialGenPlugin__