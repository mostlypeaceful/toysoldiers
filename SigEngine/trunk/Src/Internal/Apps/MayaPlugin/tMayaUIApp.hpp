#ifndef __tMayaUIApp__
#define __tMayaUIApp__
#include "MayaPluginPch.hpp"
#include "tMayaExporterToolbox.hpp"
#include "tMayaMatEdWindow.hpp"

namespace Sig
{
	///
	/// \brief TODO document
	class tMayaUIApp : public wxApp
	{
		tStrongPtr<tMayaExporterToolbox>	mExporterDialog;
		tStrongPtr<tMayaMatEdWindow>		mMatEd;
		tMayaEventPtr						mOnMayaQuit;

	public:

		tMayaUIApp( );
		~tMayaUIApp( );

		void fToggleShowExporterDialog( );
		void fToggleShowMatEd( );
		void fShowMatEd( const MString& mtlNodeName );

		void fOnMayaStartup( );
		void fOnMayaQuit( );

	private:

		virtual bool	OnInit( );
		virtual int		OnExit( );
	};
}


#endif//__tMayaUIApp__
