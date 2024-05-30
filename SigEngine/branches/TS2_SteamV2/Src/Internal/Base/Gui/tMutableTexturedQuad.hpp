/// \file   tMutableTexturedQuad.hpp
/// \author Randall Knapp
/// \par    Email:\n rknapp\@signalstudios.net
/// \date   December 2, 2010 - 10:32
/// \par    Copyright:\n &copy; Signal Studios 2010-2011
/// \brief  More modifiable textured quad
#ifndef __tMutableTexturedQuad__
#define __tMutableTexturedQuad__

#include "tTexturedQuad.hpp"

namespace Sig{ 
	
	enum tRadialDirection
	{
		cCLOCKWISE = 0,
		cCOUNTERCLOCKWISE = 1
	};

namespace Gui
{
	class tMutableTexturedQuad : public tTexturedQuad
	{
		define_dynamic_cast( tMutableTexturedQuad, tTexturedQuad );
	public:
		tMutableTexturedQuad( );
		explicit tMutableTexturedQuad( Gfx::tDefaultAllocators& allocators );
		~tMutableTexturedQuad( );

		void fSetVertexPos( u32 index, const Math::tVec2f& pos );
		void fSetVertexUV( u32 index, const Math::tVec2f& uv );
		void fSetAngleLayout( u32 pivotIndex, f32 angle, const tRect& uvCoords, const tRect& size, u32 radialDirection );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};
}}

#endif //__tMutableTexturedQuad__