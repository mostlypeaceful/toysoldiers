#ifndef __tWorldSpaceScriptedControl__
#define __tWorldSpaceScriptedControl__
#include "tUser.hpp"
#include "tEntity.hpp"
#include "tScriptedControl.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCreateScriptedControl( "CanvasCreateControl" ); }

	class base_export tWorldSpaceScriptedControl : public tEntity
	{
	public:
		static b32 cUseFadeSettings;
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		struct tEntry { tScriptedControlPtr mControl; tUserPtr mUser; };
		typedef tDynamicArray<tEntry> tControlArray;
	public:
		explicit tWorldSpaceScriptedControl( const tStringPtr& createControlFunc = cCreateScriptedControl );
		virtual ~tWorldSpaceScriptedControl( ) { }
		void fCreate( const tResourcePtr& guiScript, tUser& user, tCanvasFrame& parentCanvas );
		void fCreate( const tResourcePtr& guiScript, const tUserArray& userArray, tCanvasFrame& parentCanvas );
		const tControlArray& fAccessControls( ) const { return mScreenSpaceControls; } // use this to do fBake or fBakeBox
		const Math::tVec3f& fObjectSpaceOffset( ) const { return mObjectSpaceOffset; }
		void fSetObjectSpaceOffset( const Math::tVec3f& offset ) { mObjectSpaceOffset = offset; }
		f32 fGetAlpha( ) const  { return mAlpha; }
		void fSetAlpha( f32 a );
	public:
		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fOnDelete( );
		virtual void fPreRenderST( f32 dt );
	private:
		tScriptedControlPtr fCreateControl( const tResourcePtr& guiScript, const tUser& user );
	private:
		tStringPtr mCreateControlFunc;
		Math::tVec3f mObjectSpaceOffset;
		tControlArray mScreenSpaceControls;
		f32 mAlpha;
	};
}}

#endif//__tWorldSpaceScriptedControl__
