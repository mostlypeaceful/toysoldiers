#ifndef __tLenseFlareCanvas__
#define __tLenseFlareCanvas__

#include "tCanvas.hpp"
#include "Gfx\tLenseFlareData.hpp"

namespace Sig { 

	class tDataTable;	

	namespace Gfx { class tViewport; }
	
namespace Gui
{

	class tTexturedQuad;

	class base_export tLenseFlareCanvas : public tCanvasFrame
	{
		define_dynamic_cast( tLenseFlareCanvas, tCanvasFrame );
	public:
		tLenseFlareCanvas( );

		virtual void fOnTickCanvas( f32 dt );

		void fSetLenseFlare( u32 key );
		void fSetViewport( const Gfx::tViewport& vp );
		void fSetTrackingEntity( tEntity& ent ) { mTrackingEnt.fReset( &ent ); }

	private:
		struct tFlare
		{
			tTexturedQuad*			mTexture;
			tCanvasPtr				mTexturePtr;
			Gfx::tLenseFlareData::tFlare mFlare;

			tFlare( );
			tFlare( const Gfx::tLenseFlareData::tFlare& flare );
		};

		tGrowableArray< tFlare > mFlares;
		tRefCounterPtr< const Gfx::tViewport > mViewport;
		tEntityPtr mTrackingEnt;

		f32 mCurrentAlpha;

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

}}

#endif//__tLenseFlareCanvas__
