#ifndef __tDevVarTypes__
#define __tDevVarTypes__
#ifndef __tDevMenu__
#error Must not be included directly! Include tDevMenu.hpp instead.
#endif//__tDevMenu__
#ifdef sig_devmenu
#include "tDelegate.hpp"
#include <iomanip>

namespace Sig
{
	///
	/// \brief Generic template type for single-valued dev variables, including all int/float types.
	/// \see devvar macro.
	template<class t>
	class tDevVar : public tDevVarBase
	{
		t mValue, mMin, mMax;
		u32 mPrecision;
	public:
		tDevVar( const char* path, const t& startVal )
			: tDevVarBase( path ), mValue( startVal ), mMin( t(0) ), mMax( std::numeric_limits<t>::max( ) ), mPrecision( (t(0.5f)==t(0))?0u:3u )
		{
			fFindAndApplyIniOverride( );
		}
		tDevVar( const char* path, const t& startVal, const t& min, const t& max, u32 precision )
			: tDevVarBase( path ), mValue( startVal ), mMin( min ), mMax( max ), mPrecision( precision )
		{
			fFindAndApplyIniOverride( );
		}
		operator const t&() const { return mValue; }

		virtual std::string fIthItemValueString( u32 i ) const
		{
			std::stringstream ss;
			ss << std::fixed << std::setprecision( mPrecision ) << mValue;
			return ss.str( );
		}

		virtual void fOnHighlighted( u32 ithItem, tUser& user, f32 absTime, f32 dt )
		{
			fUpdateValue( user, absTime, dt, mValue, mPrecision, mMin, mMax );
		}

		virtual void fSetFromVector( const Math::tVec4f& v )
		{
			fSetSingleValueFromFloat( v.x, mValue, mPrecision, mMin, mMax );
		}
	};

	///
	/// \brief For pointers to basic types.
	template<class t>
	class tDevVar<t*> : public tDevVarBase
	{
		t* mValue;
		t mMin, mMax;
		u32 mPrecision;
	public:
		tDevVar( const char* path, t& startVal )
			: tDevVarBase( path ), mValue( &startVal ), mMin( t(0) ), mMax( std::numeric_limits<t>::max( ) ), mPrecision( (t(0.5f)==t(0))?0u:3u )
		{
			fFindAndApplyIniOverride( );
		}
		tDevVar( const char* path, t& startVal, const t& min, const t& max, u32 precision )
			: tDevVarBase( path ), mValue( &startVal ), mMin( min ), mMax( max ), mPrecision( precision )
		{
			fFindAndApplyIniOverride( );
		}
		operator const t&() const { return *mValue; }

		virtual std::string fIthItemValueString( u32 i ) const
		{
			std::stringstream ss;
			ss << std::fixed << std::setprecision( mPrecision ) << *mValue;
			return ss.str( );
		}

		virtual void fOnHighlighted( u32 ithItem, tUser& user, f32 absTime, f32 dt )
		{
			fUpdateValue( user, absTime, dt, *mValue, mPrecision, mMin, mMax );
		}

		virtual void fSetFromVector( const Math::tVec4f& v )
		{
			fSetSingleValueFromFloat( v.x, *mValue, mPrecision, mMin, mMax );
		}
	};

	///
	/// \brief Specialization for bool dev variables, displays as "true" or "false" in dev menu.
	/// \note use devvar( bool, your_dev_var_name_here, <bool value true of false> );
	template<>
	class base_export tDevVar<bool> : public tDevVarBase
	{
		b32 mValue;
	public:
		tDevVar( const char* path, const b32& startVal )
			: tDevVarBase( path ), mValue( startVal )
		{
			fFindAndApplyIniOverride( );
		}
		operator const b32&() const { return mValue; }

		virtual std::string fIthItemValueString( u32 i ) const;
		virtual void fOnHighlighted( u32 ithItem, tUser& user, f32 absTime, f32 dt );
		virtual void fSetFromString( const std::string& v );
		virtual void fSetFromVector( const Math::tVec4f& v );
	};

	template<>
	class base_export tDevVar<tStringPtr> : public tDevVarBase
	{
		tStringPtr mValue;
	public:
		tDevVar( const char * path, const tStringPtr & startVal )
			: tDevVarBase( path ), mValue( startVal )
		{
			fFindAndApplyIniOverride( );
		}

		operator const tStringPtr&() const { return mValue; }

		virtual std::string fIthItemValueString( u32 i ) const;
		virtual void fOnHighlighted( u32 ithItem, tUser & user, f32 absTime, f32 dt );
		virtual void fSetFromString( const std::string& v );
	};

	///
	/// \brief Base type for dev variable vectors of 2,3, and 4 dimensions.
	/// \see devvar, devrgb, and devrgba macros.
	class base_export tDevVarVectorBase : public tDevVarBase
	{
		b32 mRgbaStyleName;
	public:
		tDevVarVectorBase( const char* path, u32 dimension, b32 rgbaStyleName ) 
			: tDevVarBase( path, dimension ), mRgbaStyleName( rgbaStyleName ) { }
		virtual std::string fIthItemName( u32 i ) const;
	};

#define define_devvar_vector_template( dim ) \
	template<class t> \
	class tDevVar< Math::tVector##dim<t> > : public tDevVarVectorBase \
	{ \
		Math::tVector##dim<t> mValue; \
		t mMin, mMax; \
		u32 mPrecision; \
	public: \
		tDevVar( const char* path, const Math::tVector##dim<t>& startVal, b32 rgbaStyleName = false ) \
			: tDevVarVectorBase( path, mValue.cDimension, rgbaStyleName ), mValue( startVal ), mMin( t(0) ), mMax( std::numeric_limits<t>::max( ) ), mPrecision( (t(0.5f)==t(0))?0u:3u ) \
		{ \
			fFindAndApplyIniOverride( ); \
		} \
		tDevVar( const char* path, const Math::tVector##dim<t>& startVal, const t& min, const t& max, u32 precision, b32 rgbaStyleName = false ) \
			: tDevVarVectorBase( path, mValue.cDimension, rgbaStyleName ), mValue( startVal ), mMin( min ), mMax( max ), mPrecision( precision ) \
		{ \
			fFindAndApplyIniOverride( ); \
		} \
		operator const Math::tVector##dim<t>&() const { return mValue; } \
		virtual std::string fIthItemValueString( u32 i ) const \
		{ \
			std::stringstream ss; \
			ss << std::fixed << std::setprecision( mPrecision ) << mValue.fAxis( i ); \
			return ss.str( ); \
		} \
		virtual void fOnHighlighted( u32 ithItem, tUser& user, f32 absTime, f32 dt ) \
		{ \
			fUpdateValue( user, absTime, dt, mValue.fAxis( ithItem ), mPrecision, mMin, mMax ); \
		} \
		virtual void fSetFromVector( const Math::tVec4f& v ) \
		{ \
			for( u32 i = 0; i < fMin<u32>( mValue.cDimension, v.cDimension ); ++i ) \
				fSetSingleValueFromFloat( v.fAxis( i ), mValue.fAxis( i ), mPrecision, mMin, mMax ); \
		} \
	}; \
	template<class t> \
	class tDevVar< Math::tVector##dim<t>* > : public tDevVarVectorBase \
	{ \
		Math::tVector##dim<t>* mValue; \
		t mMin, mMax; \
		u32 mPrecision; \
	public: \
		tDevVar( const char* path, Math::tVector##dim<t>& startVal, b32 rgbaStyleName = false ) \
			: tDevVarVectorBase( path, mValue->cDimension, rgbaStyleName ), mValue( &startVal ), mMin( t(0) ), mMax( std::numeric_limits<t>::max( ) ), mPrecision( (t(0.5f)==t(0))?0u:3u ) \
		{ \
			fFindAndApplyIniOverride( ); \
		} \
		tDevVar( const char* path, Math::tVector##dim<t>& startVal, const t& min, const t& max, u32 precision, b32 rgbaStyleName = false ) \
			: tDevVarVectorBase( path, mValue->cDimension, rgbaStyleName ), mValue( &startVal ), mMin( min ), mMax( max ), mPrecision( precision ) \
		{ \
			fFindAndApplyIniOverride( ); \
		} \
		operator const Math::tVector##dim<t>&() const { return *mValue; } \
		virtual std::string fIthItemValueString( u32 i ) const \
		{ \
			std::stringstream ss; \
			ss << std::fixed << std::setprecision( mPrecision ) << mValue->fAxis( i ); \
			return ss.str( ); \
		} \
		virtual void fOnHighlighted( u32 ithItem, tUser& user, f32 absTime, f32 dt ) \
		{ \
			fUpdateValue( user, absTime, dt, mValue->fAxis( ithItem ), mPrecision, mMin, mMax ); \
		} \
		virtual void fSetFromVector( const Math::tVec4f& v ) \
		{ \
			for( u32 i = 0; i < fMin<u32>( mValue->cDimension, v.cDimension ); ++i ) \
				fSetSingleValueFromFloat( v.fAxis( i ), mValue->fAxis( i ), mPrecision, mMin, mMax ); \
		} \
	};

	define_devvar_vector_template( 2 )
	define_devvar_vector_template( 3 )
	define_devvar_vector_template( 4 )
#undef define_devvar_vector_template

	///
	/// \brief Dev variable type for calling custom user callbacks from the dev menu.
	/// \see devcb macro.
	class base_export tDevCallback : public tDevVarBase
	{
	public:
		enum tEventType { cEventTypeSetValue, cEventTypeIncrement, cEventTypeDecrement, cEventTypeNA };

		struct tArgs
		{
			tUser* mUser;
			std::string mValueText; ///< Yours to modify in the callback; this will be used as the new value text in the dev menu.
			tEventType mEvent;

			tArgs( tUser* user, tEventType event = cEventTypeNA ) : mUser( user ), mEvent( event ) { }
			s32 fGetIncrementValue( ) const { return mEvent == cEventTypeIncrement ? 1 : mEvent == cEventTypeDecrement ? -1 : 0; }
		};

		typedef tDelegate<void ( tArgs& args )> tFunction;

	private:
		std::string mValueText;
		tFunction	mFunction;

	public:
		tDevCallback( const char* path, const std::string& initialValueText, const tFunction& function );
		virtual std::string fIthItemValueString( u32 i ) const;
		virtual void fOnHighlighted( u32 ithItem, tUser& user, f32 absTime, f32 dt );
		virtual void fSetFromString( const std::string& v );
		void fSetValueText( const char* text ) { mValueText = text; }
	};

}


#	define devvar( type, name, value )								::Sig::tDevVar<type> name( #name, value )
#	define devvar_clamp( type, name, value, min, max, precision )	::Sig::tDevVar<type> name( #name, value, min, max, precision )
#	define devrgb( name, value )									::Sig::tDevVar< ::Sig::Math::tVec3f > name( #name, value, true )
#	define devrgb_clamp( name, value, min, max, precision )			::Sig::tDevVar< ::Sig::Math::tVec3f > name( #name, value, min, max, precision, true )
#	define devrgba( name, value )									::Sig::tDevVar< ::Sig::Math::tVec4f > name( #name, value, true )
#	define devrgba_clamp( name, value, min, max, precision )		::Sig::tDevVar< ::Sig::Math::tVec4f > name( #name, value, min, max, precision, true )
#	define devvarptr( type, name, value )							::Sig::tDevVar<type*> name( #name, value )
#	define devvarptr_clamp( type, name, value, min, max, precision ) ::Sig::tDevVar<type*> name( #name, value, min, max, precision )
#	define devrgbptr( name, value )									::Sig::tDevVar< ::Sig::Math::tVec3f* > name( #name, value, true )
#	define devrgbptr_clamp( name, value, min, max, precision )		::Sig::tDevVar< ::Sig::Math::tVec3f* > name( #name, value, min, max, precision, true )
#	define devrgbaptr( name, value )								::Sig::tDevVar< ::Sig::Math::tVec4f* > name( #name, value, true )
#	define devrgbaptr_clamp( name, value, min, max, precision )		::Sig::tDevVar< ::Sig::Math::tVec4f* > name( #name, value, min, max, precision, true )
#	define devcb( name, valueText, cb )								::Sig::tDevCallback name( #name, valueText, cb )
#	define devvar_update_value( name, value )						name.fSetFromVector( value )
#	define devcb_update_value( name, valueText )					name.fSetValueText( valueText )

#else//sig_devmenu

#	define devvar( type, name, value )								const type name = type( value )
#	define devvar_clamp( type, name, value, min, max, precision )	const type name = type( value )
#	define devrgb( name, value )									const ::Sig::Math::tVec3f name = value
#	define devrgb_clamp( name, value, min, max, precision )			const ::Sig::Math::tVec3f name = value
#	define devrgba( name, value )									const ::Sig::Math::tVec4f name = value
#	define devrgba_clamp( name, value, min, max, precision )		const ::Sig::Math::tVec4f name = value
#	define devvarptr( type, name, value )							const type& name = value
#	define devvarptr_clamp( type, name, value, min, max, precision ) const type& name = value
#	define devrgbptr( name, value )									const ::Sig::Math::tVec3f& name = value
#	define devrgbptr_clamp( name, value, min, max, precision )		const ::Sig::Math::tVec3f& name = value
#	define devrgbaptr( name, value )								const ::Sig::Math::tVec4f& name = value
#	define devrgbaptr_clamp( name, value, min, max, precision )		const ::Sig::Math::tVec4f& name = value
#	define devcb( name, valueText, cb )
#	define devvar_update_value( name, value )
#	define devcb_update_value( name, valueText )

#endif//sig_devmenu

#endif//__tDevVarTypes__
