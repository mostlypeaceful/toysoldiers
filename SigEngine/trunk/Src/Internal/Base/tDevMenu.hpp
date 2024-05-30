#ifndef __tDevMenu__
#define __tDevMenu__

namespace Sig { namespace Gfx
{
	class tGeometryBufferVRamAllocatorPtr;
	class tIndexBufferVRamAllocatorPtr;
	class tMaterialPtr;
	class tScreenPtr;
}}

namespace Sig
{
	class tUser;
	class tResourcePtr;

	namespace Gui { class tColorPicker; }

#ifdef sig_devmenu

	class tDevMenuIni;
	class tDevMenuImpl;

	class tDevMenuBase
	{
	public:
		virtual ~tDevMenuBase( ) { }

		virtual Gui::tColorPicker* fColorPicker( ) { return NULL; }
	};


	/// \class	tDevMenuItem
	///
	/// \brief	Base class for dev menu variables and callbacks.  These are variables specified in code which
	///			can be manipulated at runtime from the dev menu, as well as via tDevMenuIni objects.
	///			Variables can represent anything from a callback to a vector to a single value.
	///
	/// \note	You should generally not instantiate objects of one of the derived dev variable types
	///			directly, instead you should use a macro from the "devvar" family of macros, such
	///			as devvar, devvar_clamp, devrgb, devcb, etc.
	///
	class base_export tDevMenuItem : public tRefCounter
	{
	public:
		struct tClientData
		{
			f32 mLastTime;
			f32 mHeldTime;
			tClientData( ) : mLastTime( 0.f ), mHeldTime( 0.f ) { }
		};
		typedef tHashTable<void*,tClientData,tHashTableExpandOnlyResizePolicy> tClientDataTable;

	private:
		u32					mNumItems;
		u32					mNumDisplayItems;
		std::string			mDirectory;
		std::string			mShortName;
		tStringPtr			mFullPath;
		tClientDataTable	mInputTiming;

	public:

		static void fSplitPath( const char* path, std::string& shortName, std::string& folder );

		tDevMenuItem( const char* path, u32 numItems = 1 );
		virtual ~tDevMenuItem( );
		const std::string& fDirectory( ) const { return mDirectory; }
		const std::string& fShortName( ) const { return mShortName; }
		const tStringPtr&  fFullPath( ) const { return mFullPath; }

		u32 fNumItems( ) const { return mNumItems; }
		u32 fNumDisplayItems( ) const { return mNumDisplayItems; }

		virtual std::string fIthItemName( u32 i ) const { return mShortName; }
		virtual std::string fIthItemValueString( u32 i ) const { return " "; }

		virtual void fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt ) { }
		virtual void fSetFromVector( const Math::tVec4f& v ) { }
		virtual void fSetFromString( const std::string& v ) { }
		virtual void fSetItemValue( u32 index, const std::string & value ) { }
		virtual u32 fFindItemIndex( const std::string & itemName );

		// This may not be implemented for certain types. feel free to define those.
		virtual Math::tVec4f fGetVector( ) { return Math::tVec4f::cZeroVector; }

	protected:

		void fSetNumDisplayItems( u32 num );

		void fFindAndApplyIniOverride( );

		template<class t>
		void fUpdateValue( tUser& user, f32 absTime, f32 dt, t& value, u32 precision, const t& min, const t& max )
		{
			const f32 delta		= fComputeNumericDelta( ( f32 )value, 1.f / Math::fPow( 10.f, ( f32 )precision ), user, absTime, dt );
			const f32 newval	= fClamp( ( f32 )value + delta, ( f32 )min, ( f32 )max );
			value				= ( t )newval;
		}

		template<class t>
		void fSetSingleValueFromFloat( f32 flt, t& value, u32 precision, const t& min, const t& max )
		{
			std::stringstream ss;
			ss << std::fixed << std::setprecision( precision ) << flt;
			ss >> value;
			value = fClamp( value, min, max );
		}


		f32 fComputeNumericDelta( f32 value, f32 inc, tUser& user, f32 absTime, f32 dt );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tDevMenuItem );
	typedef tDevMenuItemPtr tDevVarPtr;

	/// \brief	Global repository for all dev menu variables. Shouldn't be accessed by anything
	///			other than dev menu specific code.
	class base_export tDevVarDepot
	{
		friend class tDevMenuItem;
		friend class tDevMenuIni;
		declare_singleton_define_own_ctor_dtor( tDevVarDepot );
	public:
		struct tPage;
		typedef tRefCounterPtr<tPage> tPagePtr;
		struct tPage : public tRefCounter
		{
			typedef tHashTable<void*,u32,tHashTableExpandOnlyResizePolicy> tClientDataTable;

			tPage*							mParent;
			std::string						mName;
			u32								mNumVarItems; // might be more than mPages.fCount( ) + mVars.fCount( )
			tGrowableArray< tPagePtr >		mPages;
			tGrowableArray< tDevMenuItem* >	mVars;
			tClientDataTable				mLastSelectedItem;

			tPage( tPage* parent, const std::string& name ) : mParent( parent ), mName( name ), mNumVarItems( 0 ), mLastSelectedItem( 10 ) { }
			void fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt );
			u32 fNumTotalItems( ) const { return mPages.fCount( ) + mNumVarItems; }
			b32 fIsOrphaned( ) const { return !mParent && mPages.fCount( ) == 0; }
			std::string fFullPathString( ) const;
			std::string fSubPagesString( u32 scrollCount ) const;
			std::string fVarItemsString( u32 scrollCount ) const;
			std::string fVarValuesString( u32 scrollCount ) const;
		};
	private:
		tPagePtr mRoot;
		tGrowableArray<tDevMenuIni*> mInis;
	public:
		const tPagePtr& fRoot( ) const { return mRoot; }
		tDevMenuItem* fFindVar( const char* path );
		b32 fFindVarPos( const char* path, tDevVarDepot::tPage*& outPage, u32& outPos );
		tPage * fFindPage( const char * folder );
	private:
		tDevVarDepot( );
		~tDevVarDepot( );
		void fInsert( tDevMenuItem& var );
		void fRemove( tDevMenuItem& var );
		tPagePtr fFindPage( const std::string& path, b32 createIfNotFound );
	};


	/// \brief	tDevMenu objects represent individual instances of a graphical
	///			menu display that allows the user to interact with and manipulate dev variables.
	///			The menu acts as a view onto the global tDevVarDepot, so while you can
	///			have multiple dev menu instances (perhaps one for each user/viewport),
	///			they will all be manipulating and sharing the same global list of dev variables.
	class base_export tDevMenu : public tUncopyable
	{
		tDevMenuImpl* mImpl;
	public:
		tDevMenu( );
		~tDevMenu( );
		void fInitDevMenu( const tResourcePtr& smallFont );
		void fSetDevMenuStart( const std::string& start );
		b32  fIsActive( ) const;
		void fActivate( tUser& user, const Math::tVec2f& origin, const f32 height );
		void fDeactivate( tUser& user );
		void fOnTick( tUser& user, f32 dt );

		// Captures all the variables on the page and below
		void fCaptureVarInfo( const char * folder, std::stringstream & ss, b32 recursive = false );
		void fLoadVarInfo( std::stringstream & ss );

		void fLogValuesToFile( );

	private:
		u32 mGamePadFilter;
	};

#define if_devmenu( x ) x

#else


	/// \brief	The dev menu is disabled in this build, and so compiles out to do-nothing methods.
	class base_export tDevMenu : public tUncopyable
	{
	public:
		inline tDevMenu( ) { }
		inline ~tDevMenu( ) { }
		inline void fInitDevMenu( const tResourcePtr& smallFont ) { }
		inline void fSetDevMenuStart( const std::string& start ) { }
		inline b32  fIsActive( ) const { return false; }
		inline void fActivate( const Math::tVec2f& origin, const f32 height ) { }
		inline void fDeactivate( ) { }
		inline void fOnTick( tUser& user, f32 dt ) { }
	};

#define if_devmenu( x )

#endif//sig_devmenu

}

#include "tDevVarTypes.hpp"
#include "tDevMenuIni.hpp"

#endif//__tDevMenu__
