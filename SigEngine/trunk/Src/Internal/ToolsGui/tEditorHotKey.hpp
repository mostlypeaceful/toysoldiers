#ifndef __tEditorHotKey__
#define __tEditorHotKey__
#include "Input/tKeyboard.hpp"

namespace Sig
{

	class tEditorHotKeyTable;

	///
	/// \brief Simple POD type that describes the minimal
	/// data/signature of a hot key, notably which keys/options
	/// cause the hot key action to fire.
	class toolsgui_export tEditorHotKeyEntry
	{
	public:
		enum tOption
		{
			cOptionShift	= (1<<0),
			cOptionCtrl		= (1<<1),
			cOptionAlt		= (1<<2),
		};
	protected:
		Input::tKeyboard::tButton	mKbButton;
		u32							mOptions;
	public:
		inline tEditorHotKeyEntry( )
			: mKbButton( Input::tKeyboard::cButtonScrollLock ) // a pretty harmless button
			, mOptions( 0 )
		{
		}

		explicit tEditorHotKeyEntry( Input::tKeyboard::tButton kbButton, u32 optionFlags = 0 )
			: mKbButton( kbButton )
			, mOptions( optionFlags )
		{
		}

		inline b32 operator==( const tEditorHotKeyEntry& e ) const { return mKbButton == e.mKbButton && mOptions == e.mOptions; }
		inline b32 operator!=( const tEditorHotKeyEntry& e ) const { return !operator==( e ); }

		inline b32 fRequirementsMet( const Input::tKeyboard& kb ) const
		{
			if( !kb.fButtonDown( mKbButton ) ) return false;

			const bool shift = kb.fButtonHeld( Input::tKeyboard::cButtonLShift ) || kb.fButtonHeld( Input::tKeyboard::cButtonRShift );
			const bool ctrl = kb.fButtonHeld( Input::tKeyboard::cButtonLCtrl ) || kb.fButtonHeld( Input::tKeyboard::cButtonRCtrl );
			const bool alt = kb.fButtonHeld( Input::tKeyboard::cButtonLAlt ) || kb.fButtonHeld( Input::tKeyboard::cButtonRAlt );

			if( bool( ( mOptions & cOptionShift ) != 0 )	!= shift )	return false;
			if( bool( ( mOptions & cOptionCtrl ) != 0 )		!= ctrl )	return false;
			if( bool( ( mOptions & cOptionAlt ) != 0 )		!= alt )	return false;

			return true;
		}
	};

	///
	/// \brief Shared,ref-counted type representing an actual instance of a hot key
	/// action in the editor. These objects are tracked in hot key tables, updated
	/// together, and fired from a central location. You should derive your hot key
	/// type from this class and implement the fFire method.
	class toolsgui_export tEditorHotKey : public tEditorHotKeyEntry, public tRefCounter
	{
		friend class tEditorHotKeyTable;
	private:
		tEditorHotKeyTable* mTable;
	public:
		tEditorHotKey( tEditorHotKeyTable& table, Input::tKeyboard::tButton kbButton, u32 optionFlags );
		virtual ~tEditorHotKey( );
		virtual void fFire( ) const = 0;
	};
	typedef tRefCounterPtr< tEditorHotKey > tEditorHotKeyPtr;


	typedef tHashTable<tEditorHotKeyEntry, tEditorHotKey*> tEditorHotKeyTableBase;

	///
	/// \brief Acts as a repository for a group of hot keys. Allows centralized hot key
	/// processing. You can derive from this type if you want to provide custom updating/adding.
	class toolsgui_export tEditorHotKeyTable : public tEditorHotKeyTableBase
	{
	public:
		tEditorHotKeyTable( );
		virtual ~tEditorHotKeyTable( );
		virtual void fAddHotKey( tEditorHotKey* hotKey );
		///
		/// \brief Returns true if a hot key was processed.
		virtual b32 fUpdateHotKeys( Input::tKeyboard& kb ) const;
	};

}

#endif//__tEditorHotKey__
