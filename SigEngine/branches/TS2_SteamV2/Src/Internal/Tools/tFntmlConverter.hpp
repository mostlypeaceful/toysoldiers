#ifndef __tFntmlConverter__
#define __tFntmlConverter__
#include "tStrongPtr.hpp"
#include "Gui/tFont.hpp"

namespace Sig
{

	namespace Fntml
	{
		class tFile;

		const char* fGetFileExtension( );
	}

	class tFntmlConverter : tUncopyable
	{
		tStrongPtr<Fntml::tFile>	mInputFont;
		tStrongPtr<Gfx::tFontMaterial>	mAutoDeleteMaterial;

		Gui::tFont mOutputFont;

	public:

		tFntmlConverter( );
		~tFntmlConverter( );

		b32 fLoad( const tFilePathPtr& path );
		void fSaveGameBinary( const tFilePathPtr& path, tPlatformId pid );
	};

}


#endif//__tFntmlConverter__
