#ifndef __tSigFxMatEd__
#define __tSigFxMatEd__
#include "tMatEdMainWindow.hpp"

namespace Sig
{
	class tSigFxMainWindow;

	class tSigFxMatEd : public tMatEdMainWindow
	{
		tSigFxMainWindow* mSigFx;
		tEntityPtr mSelected;
		b32 mDefaultTextureBrowserPathSet;
	public:
		tSigFxMatEd( tSigFxMainWindow* parent, tEditorActionStack& actionStack, const std::string& regKeyName );
		virtual b32 fOnTick( );
		virtual void fOnShaderSelected( const tFilePathPtr& shaderPath );
		void fSetDefaultShader( const tEntityPtr& sel );
	private:
		void fSetSelected( const tEntityPtr& sel );
	};

}


#endif//__tSigFxMatEd__

