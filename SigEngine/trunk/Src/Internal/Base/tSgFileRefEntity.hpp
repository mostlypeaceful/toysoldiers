#ifndef __tSgFileRefEntity__
#define __tSgFileRefEntity__
#include "tSceneRefEntity.hpp"

namespace Sig
{
	namespace Gfx { class tScreen; class tLightEntity; }

	///
	/// \brief Extends tSceneRefEntity: adds ownership and load semantics on top of the sg resource.
	class base_export tSgFileRefEntity : public tSceneRefEntity
	{
		debug_watch( tSgFileRefEntity );
		declare_uncopyable( tSgFileRefEntity );
		define_dynamic_cast( tSgFileRefEntity, tSceneRefEntity );
	private:
		tResource::tOnLoadComplete::tObserver	mSgResourceLoaded;
	public:
		explicit tSgFileRefEntity( const tResourcePtr& sgResource, const tEntity* proxy = 0 ); // adds load call
		virtual ~tSgFileRefEntity( );
		Gfx::tLightEntity* fSpawnDefaultLight( tEntity& lightParent, Gfx::tScreen& screen );
	private:
		void fOnSgResourceLoaded( tResource& theResource, b32 success );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tSgFileRefEntity );

}


#endif//__tSgFileRefEntity__
