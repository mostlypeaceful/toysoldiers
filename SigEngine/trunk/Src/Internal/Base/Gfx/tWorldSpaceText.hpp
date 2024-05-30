//------------------------------------------------------------------------------
// \file tWorldSpaceText.hpp - 03 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tWorldSpaceText__
#define __tWorldSpaceText__

#include "tRenderableEntity.hpp"
#include "tTextGeometry.hpp"

namespace Sig { namespace Gfx
{
	///
	/// \class tWorldSpaceText
	/// \brief 
	class base_export tWorldSpaceText : public tRenderableEntity
	{
		define_dynamic_cast( tWorldSpaceText, tRenderableEntity );

	public:

		//------------------------------------------------------------------------------
		// tEntity
		//------------------------------------------------------------------------------
		virtual b32 fIsHelper( ) const { return true; }
		virtual u32 fSpatialSetIndex( ) const { return cEffectSpatialSetIndex; }


		tTextGeometry & fGeometry( ) { return mGeometry; }
		void fOnGeometryChanged( );
		
	private:

		tTextGeometry mGeometry;
	};

}} // ::Sig::Gfx

#endif//__tWorldSpaceText__
