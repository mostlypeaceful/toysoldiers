#ifndef __tLightCombo__
#define __tLightCombo__
#include "tLightEntity.hpp"
#include "tDisplayList.hpp"
#include "Threads/tMutex.hpp"

namespace Sig
{
	class tSceneGraph;
}

namespace Sig { namespace Gfx
{
	class tViewport;
	class tRenderableEntity;
	class tWorldSpaceDisplayList;

	///
	/// \brief Represents a unique combination of lights in the scene affecting
	/// a set of objects; put another way, all objects in this set share the
	/// same lights; these objects can then all be rendered in batches together, without
	/// having to change or set indvidual light states.
	class tLightCombo
	{
	public:
		define_class_pool_new_delete_mt( tLightCombo, 64 );
		
		tLightCombo() {} // stupid no-op default ctor for GCC's sake
	public:
		tLightEntityList mLights;
	};

	class base_export tLightComboList : public tGrowableArray< tLightCombo* >
	{
	public:
		tWorldSpaceDisplayList mDisplayList;

	public:
		tLightComboList( );
		~tLightComboList( );

		void fBuildLightCombos( tScreen& screen, tSceneGraph& sg, const tViewport& vp, b32 deferred, const tLightEntityList& lightList, tWorldSpaceDisplayList& depthDisplayList, b32 buildDepthList, tLightEntityList* shadows );

		void fInvalidate( );
		void fClearCombos( );
	};

}}


#endif//__tLightCombo__
