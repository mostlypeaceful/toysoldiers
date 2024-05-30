#ifndef __tWorldSpaceText__
#define __tWorldSpaceText__
#include "tUser.hpp"
#include "tEntity.hpp"
#include "tText.hpp"

namespace Sig { namespace Gui
{
	class tWorldSpaceText : public tEntity
	{
	public:
		struct tEntry { tTextPtr mText; tUserPtr mUser; };
		typedef tDynamicArray<tEntry> tTextArray;
	public:
		tWorldSpaceText( ) { }
		void fCreate( const tResourcePtr& font, tUser& user, tCanvasFrame& parentCanvas );
		void fCreate( const tResourcePtr& font, const tUserArray& userArray, tCanvasFrame& parentCanvas );
		void fAdd( const tResourcePtr& font, tUser& user, tCanvasFrame& parentCanvas );
		void fClear( );
		const tTextArray& fAccessText( ) const { return mScreenSpaceText; } // use this to do fBake or fBakeBox
	public:
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fWorldSpaceUIST( f32 dt );
	private:
		tTextPtr fCreateText( const tResourcePtr& font, const tUser& user );
	private:
		tTextArray mScreenSpaceText;
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	typedef tRefCounterPtr<tWorldSpaceText> tWorldSpaceTextPtr;
}}

#endif//__tWorldSpaceText__
