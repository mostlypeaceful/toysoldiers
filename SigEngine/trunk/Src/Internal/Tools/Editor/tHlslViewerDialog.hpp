#ifndef __tHlslViewerDialog__
#define __tHlslViewerDialog__
#include <wx/aui/auibook.h>
#include "hlslgen/tHlslOutput.hpp"

namespace Sig
{
	class tools_export tHlslViewerDialog : public wxDialog
	{
	public:
		tHlslViewerDialog( wxWindow* parent, const HlslGen::tHlslOutput& hlsl );
	private:
		void fAddPage( const HlslGen::tShaderOutputBase& shader );
	private:
		wxAuiNotebook* mNotebook;
	};
}

#endif//__tHlslViewerDialog__
