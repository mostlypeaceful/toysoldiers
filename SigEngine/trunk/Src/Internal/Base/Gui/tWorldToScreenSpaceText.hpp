//------------------------------------------------------------------------------
// \file tWorldToScreenSpaceText.hpp - 02 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tWorldToScreenSpaceText__
#define __tWorldToScreenSpaceText__

#include "tUser.hpp"
#include "tEntity.hpp"
#include "tText.hpp"

namespace Sig { namespace Gui
{
	///
	/// \class tWorldToScreenSpaceText
	/// \brief 
	class tWorldToScreenSpaceText : public tEntity
	{
		define_dynamic_cast( tWorldToScreenSpaceText, tEntity );
	public:
		struct tEntry { tTextPtr mText; tUserPtr mUser; };
		typedef tDynamicArray<tEntry> tTextArray;

	public:
		tWorldToScreenSpaceText( ) { }
		void fCreate( const tResourcePtr& font, tUser& user, tCanvasFrame& parentCanvas );
		void fCreate( const tResourcePtr& font, const tUserArray& userArray, tCanvasFrame& parentCanvas );
		void fAdd( const tResourcePtr& font, tUser& user, tCanvasFrame& parentCanvas );
		void fClear( );
		const tTextArray& fAccessText( ) const { return mScreenSpaceText; } // use this to do fBake or fBakeBox

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fPreRenderST( f32 dt );

		static void fExportScriptInterface( tScriptVm& vm );
	
	private:
		tTextPtr fCreateText( const tResourcePtr& font, const tUser& user );

	private:
		tTextArray mScreenSpaceText;

	};

	define_smart_ptr( base_export, tRefCounterPtr, tWorldToScreenSpaceText );

}} // ::Sig::Gui

#endif//__tWorldToScreenSpaceText__
