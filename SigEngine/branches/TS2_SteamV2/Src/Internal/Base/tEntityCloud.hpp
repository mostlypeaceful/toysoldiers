//------------------------------------------------------------------------------
// \file tEntityCloud.hpp - 13 Sep 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tEntityCloud__
#define __tEntityCloud__
#include "Math/Math.hpp"
#include "Gfx/tCamera.hpp"
#include "tEntity.hpp"

namespace Sig
{
	define_smart_ptr( base_export, tRefCounterPtr, tEntityCloud );	

	///
	/// \class tEntityCloud
	/// \brief 
	class base_export tEntityCloud : public tEntity
	{
		define_dynamic_cast( tEntityCloud, tEntity );

	public:

		typedef tDelegate<void ( tEntityBVH::tObjectPtr* ents, u32 count )> tGatherCb;

		tEntityCloud( ) { }

		virtual void fOnSpawn( );
		virtual void fOnDelete( );

	public:

		virtual void fPrepareRenderables( const Gfx::tCamera & camera ) { }

		// NOTE: Implementations of this function must ONLY pass objects convertible
		// to tRenderableEntity to the callback function
		virtual void fGatherRenderables( tGatherCb * cb, b32 forShadows ) { };

		// Gives the underlying systems a chance to clean up
		virtual void fCleanRenderables( ) { }

	private:

		friend class tEntity;
		friend class tSceneGraph;

	};
}

#endif//__tEntityCloud__
