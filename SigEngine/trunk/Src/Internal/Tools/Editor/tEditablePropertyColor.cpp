#include "ToolsPch.hpp"
#include "tEditablePropertyColor.hpp"

namespace Sig
{
	///
	/// \section tEditablePropertyColor
	///

	register_rtti_factory( tEditablePropertyColor, false )

	tEditablePropertyColor::tEditablePropertyColor( ) 
		: mMin( tColorPickerData( 0.f, 0.f, 1.f ) )
		, mMax( tColorPickerData( 1.f, 1.f, Math::cInfinity ) )
		, mAllowDelete( false )
		, mColorPicker( 0 )
	{
		mRawData = tColorPickerData( );
	}

	tEditablePropertyColor::tEditablePropertyColor( 
		const std::string& name, 
		const tColorPickerData& initState,
		const tColorPickerData& min,
		const tColorPickerData& max )
		: tRawDataEditableProperty<tColorPickerData>( name, initState )
		, mMin( min )
		, mMax( max )
		, mAllowDelete( false )
		, mColorPicker( 0 )
	{
	}

	void tEditablePropertyColor::fCreateGui( tCreateGuiData& data )
	{
		mColorPicker = new tColorPicker( this, data.mParent, data.mLabel.c_str( ) );

		fRefreshGui( );

		if( mAllowDelete )
			fAddRemoveButton( data.mParent, *mColorPicker );
	}
	void tEditablePropertyColor::fRefreshGui( )
	{
		if( fHasConflict( ) )
			mColorPicker->fSetValue( tColorPickerData( ) );
		else
			mColorPicker->fSetValue( mRawData );
	}
	void tEditablePropertyColor::fClearGui( )
	{
		mColorPicker = 0;
	}
	tEditableProperty* tEditablePropertyColor::fClone( ) const
	{
		tEditablePropertyColor* o = new tEditablePropertyColor( fGetName( ), mRawData, mMin, mMax );
		o->mAllowDelete = mAllowDelete;
		return o;
	}
	void tEditablePropertyColor::fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const
	{
		sigassert( cid == Rtti::fGetClassId<tColorPickerData>( ) );
		sigassert( size == sizeof( mRawData ) );
		*( tColorPickerData* )dst = mRawData;
	}
	void tEditablePropertyColor::fSetRawData( Rtti::tClassId cid, const void* src, u32 size )
	{
		sigassert( cid == Rtti::fGetClassId<tColorPickerData>( ) );
		sigassert( size == sizeof( mRawData ) );
		mRawData = *( tColorPickerData* )src;
	}

}
