#include "ToolsGuiPch.hpp"
#include "tEditorHotKey.hpp"

namespace Sig
{


	tEditorHotKey::tEditorHotKey( tEditorHotKeyTable& table, Input::tKeyboard::tButton kbButton, u32 optionFlags )
		: tEditorHotKeyEntry( kbButton, optionFlags )
		, mTable( &table )
	{
		table.fAddHotKey( this );
	}
	tEditorHotKey::~tEditorHotKey( )
	{
		if( mTable )
			mTable->fRemove( *this );
	}

	tEditorHotKeyTable::tEditorHotKeyTable( )
		: tEditorHotKeyTableBase( 32 )
	{
	}

	tEditorHotKeyTable::~tEditorHotKeyTable( )
	{
		for( tIterator i = fBegin( ); i != fEnd( ); ++i )
		{
			if( i->fNullOrRemoved( ) ) continue;
			i->mValue->mTable = 0;
		}
	}

	void tEditorHotKeyTable::fAddHotKey( tEditorHotKey* hotKey )
	{
		const b32 alreadyExists = fFind( *hotKey ) != 0;
		if( alreadyExists )
		{
			hotKey->mTable = 0;
			log_warning( 0, "Trying to add duplicate hot key." );
		}
		else
			(*this)[ *hotKey ] = hotKey;
	}

	b32 tEditorHotKeyTable::fUpdateHotKeys( Input::tKeyboard& kb ) const
	{
		for( tConstIterator i = fBegin( ); i != fEnd( ); ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			if( i->mValue->fRequirementsMet( kb ) )
			{
				i->mValue->fFire( );
				return true; // return after processing the first hot key
			}
		}

		return false;
	}

}

