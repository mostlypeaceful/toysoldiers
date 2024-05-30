#ifndef __tEditablePropertyColor__
#define __tEditablePropertyColor__
#include "tEditablePropertyTypes.hpp"
#include "tWxSlapOnColorPicker.hpp"

namespace Sig
{

	class tools_export tEditablePropertyColor : public tRawDataEditableProperty<tColorPickerData>
	{
		implement_rtti_serializable_base_class( tEditablePropertyColor, 0x81A3C168 );
	private:
		class tools_export tColorPicker : public tWxSlapOnColorPicker
		{
			tEditablePropertyColor* mOwnerProp;
		public:
			tColorPicker( tEditablePropertyColor* ownerProp, wxWindow* parent, const char* label ) 
				: tWxSlapOnColorPicker( parent, label, tColorPickerData( ), ownerProp->fMin( ), ownerProp->fMax( ) ), mOwnerProp( ownerProp ) { }
			virtual void fOnControlUpdated( ) { mOwnerProp->fSetData<tColorPickerData>( fGetValue( ) ); }
		};
		tColorPicker* mColorPicker;
		tColorPickerData mMin, mMax;
		b32 mAllowDelete;
	public:
		tEditablePropertyColor( );
		explicit tEditablePropertyColor( 
			const std::string& name, 
			const tColorPickerData& initState = tColorPickerData( ),
			const tColorPickerData& min = tColorPickerData( 0.f, 0.f, 1.f ),
			const tColorPickerData& max = tColorPickerData( 1.f, 1.f, Math::cInfinity ) );
		virtual void fCreateGui( tCreateGuiData& data );
		virtual void fRefreshGui( );
		virtual void fClearGui( );
		virtual tEditableProperty* fClone( ) const;
		const tColorPickerData& fMin( ) const { return mMin; }
		const tColorPickerData& fMax( ) const { return mMax; }
	protected:
		virtual void fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const;
		virtual void fSetRawData( Rtti::tClassId cid, const void* src, u32 size );

		template<class tSerializer>
		void fSerialize( tSerializer& s )
		{
			tRawDataEditableProperty<tColorPickerData>::fSerializeXml( s );
			s( "Min", mMin );
			s( "Max", mMax );
			s( "AllowDelete", mAllowDelete );
		}

		virtual void fSerializeXml( tXmlSerializer& s ) { fSerialize( s ); }
		virtual void fSerializeXml( tXmlDeserializer& s ) { fSerialize( s ); }
	};

}

#endif//__tEditablePropertyColor__
