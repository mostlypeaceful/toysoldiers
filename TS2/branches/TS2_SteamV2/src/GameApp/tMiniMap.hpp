#ifndef __tMiniMap__
#define __tMiniMap__
#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"

namespace Sig { 

	class tUnitLogic;
	class tPlayer;
	
namespace Gui
{

	class tMiniMapBase : public tScriptedControl
	{
	public:
		explicit tMiniMapBase( const tResourcePtr& scriptResource, const tUserPtr& user );
		virtual ~tMiniMapBase( );

		f32 fMiniMapAngle( tUnitLogic* unit ) const;
		tUser* fUser( ) const { return mUser.fGetRawPtr( ); }
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	protected:
		tUserPtr mUser;
	};

	class tMiniMapUnitIcon : public tMiniMapBase
	{
	public:
		explicit tMiniMapUnitIcon( const tResourcePtr& scriptResource, const tUserPtr& user );
		virtual ~tMiniMapUnitIcon( );

		void fSetUnit( tUnitLogic* unit );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class tMiniMap : public tMiniMapBase
	{
	public:
		explicit tMiniMap( const tResourcePtr& scriptResource, const tUserPtr& user );
		void fInitialize( tPlayer* player );
		void fFadeIn( );
		void fFadeOut( );
		void fUpdate( );

	public: // Add icons to the mini map
		void fAddUnit( const Sqrat::Object& unit ) const;
		
	private:
		void fAddToyBoxes( tPlayer* player ) const;
		void fAddDrivableUnits( tPlayer* player ) const;
	public:
		static void fExportScriptInterface( tScriptVm& vm );	
	};

	typedef tRefCounterPtr< tMiniMap > tMiniMapPtr;

}}

#endif//__tMiniMap__
