#ifndef __FUI__
#define __FUI__

#include "Input/tGamepad.hpp"
#include "Input/tKeyboard.hpp"
#include "tFuiFunctionCall.hpp"
#include "tFuiResourceProvider.hpp"

struct Iggy;
struct IggyValuePath;
struct IggyExternalFunctionCallUTF8;

namespace Sig { namespace Gfx
{
	class tRenderableEntity;
	class tLightEntity;
	class tDevice;
} }

namespace Sig { namespace Fui
{

	typedef Iggy* tFuiHandle; 
	typedef IggyExternalFunctionCallUTF8* tFuiFunctionHandle;
	struct tFuiMemoryOptions;

	b32 fGroupRendering( );
	b32 fIgnoreFilteredInput( );

	struct tFuiValuePath : tUncopyable
	{
		mutable tGrowableArray<IggyValuePath> mStorage; //Iggy is fun!

		tFuiValuePath( );
		~tFuiValuePath( );

		IggyValuePath* fValue( ) const { return mStorage.fCount( ) ? &mStorage.fBack( ) : NULL; }
	};

	class tFui : public tRefCounter
	{
		friend class tFuiSystem;
		struct tGamepadTranslator : public tHashTable<Input::tGamepad::tButton, u32> { tGamepadTranslator( ); }; //wrapped in structure so I can insert once it is constructed statically
        struct tKeyboardTranslator : public tHashTable<Input::tKeyboard::tButton, u32> { tKeyboardTranslator( ); };
		static tGamepadTranslator sGamepadTranslator;
        static tKeyboardTranslator sKeyboardTranslator;
		tFuiHandle mHandle; //Iggy*
		b32	mVisible;
		tResourcePtr mRes;
	public:
		tFui( const tResourcePtr& res, tFuiHandle fui );
		~tFui( );

		tFuiHandle fHandle( ) const;

	private:
		b32 fInit( );
	public:
		b32 fIsValid( ) const;
		void fEnd( ); //prevents the fui from being ticked any more
		void fSendGamepadEvent( const Input::tGamepad::tButton& button, b32 isDown );
        void fSendKeyboardEvent( const Input::tKeyboard::tButton& butotn, b32 isDown );

		void fSetVisible( b32 isVisible ) { mVisible = isVisible; }
		b32 fVisible( ) { return mVisible; }

		const tResourcePtr& fResource( ) const { return mRes; }
		
		//accessors for data in fui
		b32 fGetPath( const char* name, tFuiValuePath& pathOut ) const;
		void fSetArray( const tFuiValuePath& path, const tGrowableArray<tStringPtr>& strings );
		void fSetArray( const tFuiValuePath& path, const tGrowableArray<u32>& values );
		void fSetFloat( const tFuiValuePath& path, const f32 val );
		void fSetS32( const tFuiValuePath& path, const s32 val );
		void fSetString( const tFuiValuePath& path, const std::string& val );
		void fSetWString( const tFuiValuePath& path, const std::wstring& val );
		
		void fGetUserData( const char * pathName, void * * userData );
		void fGetUserData( const tFuiValuePath& path, void * * userData );

		void fSetUserData( const char * pathName, void const * userData );
		void fSetUserData( const tFuiValuePath& path, void const * userData );
		
		
		b32 fCallFunction( const tFuiFunctionCall& functionCall );

		tDynamicArray<tFuiValuePath> mFUIDataStorePaths; // Used internally by the tFuiDataStore system
	};
	typedef tRefCounterPtr<tFui> tFuiPtr;

	class tFuiFuncParams
	{
		tFuiPtr mFui;
		tFuiFunctionHandle mParams;
		s32 mIndex;
	public:
		tFuiFuncParams( tFuiPtr fui, tFuiFunctionHandle params );
		b32 fIsValid( ) const;
		const tFuiPtr& fFui( ) const;
		u32  fCount( ) const;
	private:
		b32 fValidate( u32 expectedType );
	public:
		void fGet( u32& val );
		void fGet( s32& val );
		void fGet( f32& val );
		void fGet( bool & val );
		void fGet( const char*& val );
		void fGet( const wchar_t*& val );
		void fGet( std::string& val );
		void fGet( tStringPtr& val );
		void fGet( tFilePathPtr& val );
        void fGetBool( b32& val );
	};

	class tFuiSystem
	{
		declare_singleton_define_own_ctor_dtor( tFuiSystem );
		friend class tFui;
	public:
			
		static const u32 cFontFlag_None;
		static const u32 cFontFlag_Bold;
		static const u32 cFontFlag_Italic;

	public:
		tFuiSystem( );
		~tFuiSystem( );
		
		//basic updating funcs
		void fInit( const Gfx::tDevicePtr& device );
		void fInit( const Gfx::tDevicePtr& device, const tFuiMemoryOptions & memOpts );
		void fTick( );
		void fRender( const tFuiPtr& swf );
		void fOnRenderComplete( );
		void fShutdown( );

		//interface
		void fLoad( const tFilePathPtr& filename );
		tFuiPtr fPlay( const tResourcePtr& res );
		tFuiPtr fLoadAndPlay( const tFilePathPtr& filename );
		
		//function registration
		typedef void(*tFuiCallback)(tFuiFuncParams&);
		void fRegisterFunc( const char* name, tFuiCallback cb );
		void fUnregisterFunc( const char* name );
		void fTranslateAndExecute( const tStringPtr& name, tFuiFuncParams& params );
		tFuiPtr fConvertFromHandle( tFuiHandle fui );

		//fonts
		void fInstallTrueTypeFont( const tStringPtr& name, const void* fontBits, u32 fontFlags );
		void fUninstallFont( const tStringPtr& name, u32 fontFlags );

		//resource stuff
		Gfx::tTextureFile::tPlatformHandle fGetTexture( const char * textureName, u32 * outWidth, u32 * outHeight);
		void fPermaLoadTexture( const tFilePathPtr& filename );
		void fReleaseTexture( const Gfx::tTextureFile::tPlatformHandle& handle );

	private:
		tFuiPtr fLoadInternal( const tFilePathPtr& filename );
		void fRemove( tFuiPtr fui );
		
	private:
		b32 mTicking;
		tRefCounterPtr< Gfx::tDevice > mDevice;
		tGrowableArray<tFuiPtr> mLoadedSwfs;
		tGrowableArray<tFuiPtr> mPlayingSwfs;
		tHashTable<tStringPtr,tFuiCallback> mFuncTranslator;
		tFuiResourceProvider mResourceProvider;
		IDirect3DStateBlock9* mRenderStateBlock;
	};

}}//Sig::Fui

#endif//__FUI__
