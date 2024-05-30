#ifndef __tWxSavedLayout__
#define __tWxSavedLayout__

namespace Sig
{
	///
	/// \brief Facilitates loading/saving dialog visual settings/layout
	/// to/from a registry key. You should derive a new type and overload
	/// the fRegistryKeyName for your specific dialog.
	class tools_export tWxSavedLayout : public Win32Util::tRegistrySerializer
	{
		static const char* fVisibleName( )	{ return "vis"; }
		static const char* fMinimizedName( ){ return "min"; }
		static const char* fMaximizedName( ){ return "max"; }
		static const char* fXPosName( )		{ return "xpos"; }
		static const char* fYPosName( )		{ return "ypos"; }
		static const char* fXSizeName( )	{ return "xsize"; }
		static const char* fYSizeName( )	{ return "ysize"; }

	public:

		std::string mRegKeyName;
		b32 mVisible, mMinimized, mMaximized;
		s32 mXPos, mYPos;
		s32 mXSize, mYSize;

		explicit tWxSavedLayout( const std::string& regKeyName = "" );

		virtual std::string fRegistryKeyName( ) const { return mRegKeyName; }

		b32 operator==( const tWxSavedLayout& other ) const;
		inline b32 operator!=( const tWxSavedLayout& other ) const { return !operator==( other ); }

		void fFromWxWindow( wxWindow* window );
		void fToWxWindow( wxWindow* window );
		b32 fIsInBounds( s32 maxWinWidth );

	protected:

		///
		/// \brief Redefine this in your derived type if you want to save more properties. Be sure
		/// to call the base method though to save the standard properties.
		virtual void fSaveInternal( HKEY hKey );

		///
		/// \brief Redefine this in your derived type if you want to load more properties. Be sure
		/// to call the base method though to load the standard properties.
		virtual void fLoadInternal( HKEY hKey );
	};


}


#endif//__tWxSavedLayout__

