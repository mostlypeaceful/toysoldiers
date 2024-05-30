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
	class tDevVar : public tDevMenuItem
	{
	protected:
		t mValue, mMin, mMax;
		u32 mPrecision;
	public:
		tDevVar( const char* path, const t& startVal )
			: tDevMenuItem( path ), mValue( startVal ), mMin( t(0) ), mMax( std::numeric_limits<t>::max( ) ), mPrecision( (t(0.5f)==t(0))?0u:3u )
		{
			fFindAndApplyIniOverride( );
		}
		tDevVar( const char* path, const t& startVal, u32 precision )
			: tDevMenuItem( path ), mValue( startVal ), mMin( t(0) ), mMax( std::numeric_limits<t>::max( ) ), mPrecision( precision )
		{
			fFindAndApplyIniOverride( );
		}
		tDevVar( const char* path, const t& startVal, const t& min, const t& max, u32 precision )
			: tDevMenuItem( path ), mValue( startVal ), mMin( min ), mMax( max ), mPrecision( precision )
		{
			fFindAndApplyIniOverride( );
		}
		operator const t&() const { return mValue; }

		virtual std::string fIthItemValueString( u32 i ) const OVERRIDE
		{
			std::stringstream ss;
			ss << std::fixed << std::setprecision( mPrecision ) << mValue;
			return ss.str( );
		}

		virtual void fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt ) OVERRIDE
		{
			fUpdateValue( user, absTime, dt, mValue, mPrecision, mMin, mMax );
		}

		virtual void fSetFromVector( const Math::tVec4f& v ) OVERRIDE
		{
			fSetSingleValueFromFloat( v.x, mValue, mPrecision, mMin, mMax );
		}

		virtual void fSetItemValue( u32 itemIndex, const std::string & value ) OVERRIDE
		{
			std::stringstream ss; ss << value; f32 floatValue; ss >> floatValue;
			fSetSingleValueFromFloat( floatValue, mValue, mPrecision, mMin, mMax );
		}
	};

	///
	/// \brief For pointers to basic types.
	template<class t>
	class tDevVar<t*> : public tDevMenuItem
	{
		t* mValue;
	protected:
		t mMin, mMax;
		u32 mPrecision;
	public:
		tDevVar( const char* path, t& startVal )
			: tDevMenuItem( path ), mValue( &startVal ), mMin( t(0) ), mMax( std::numeric_limits<t>::max( ) ), mPrecision( (t(0.5f)==t(0))?0u:3u )
		{
			fFindAndApplyIniOverride( );
		}
		tDevVar( const char* path, t& startVal, u32 precision )
			: tDevMenuItem( path ), mValue( &startVal ), mMin( t(0) ), mMax( std::numeric_limits<t>::max( ) ), mPrecision( precision )
		{
			fFindAndApplyIniOverride( );
		}
		tDevVar( const char* path, t& startVal, const t& min, const t& max, u32 precision )
			: tDevMenuItem( path ), mValue( &startVal ), mMin( min ), mMax( max ), mPrecision( precision )
		{
			fFindAndApplyIniOverride( );
		}
		operator const t&() const { return *mValue; }

		virtual std::string fIthItemValueString( u32 i ) const OVERRIDE
		{
			std::stringstream ss;
			ss << std::fixed << std::setprecision( mPrecision ) << *mValue;
			return ss.str( );
		}

		virtual void fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt ) OVERRIDE
		{
			fUpdateValue( user, absTime, dt, *mValue, mPrecision, mMin, mMax );
		}

		virtual void fSetFromVector( const Math::tVec4f& v ) OVERRIDE
		{
			fSetSingleValueFromFloat( v.x, *mValue, mPrecision, mMin, mMax );
		}
		
		virtual void fSetItemValue( u32 itemIndex, const std::string & value ) OVERRIDE
		{
			std::stringstream ss; ss << value; f32 floatValue; ss >> floatValue;
			fSetSingleValueFromFloat( floatValue, *mValue, mPrecision, mMin, mMax );
		}

	};

	///
	/// \brief Specialization for bool dev variables, displays as "true" or "false" in dev menu.
	/// \note use devvar( bool, your_dev_var_name_here, <bool value true of false> );
	template<>
	class base_export tDevVar<bool> : public tDevMenuItem
	{
		b32 mValue;
	public:
		tDevVar( const char* path, const b32& startVal )
			: tDevMenuItem( path ), mValue( startVal )
		{
			fFindAndApplyIniOverride( );
		}
		operator const b32&() const { return mValue; }

		virtual std::string fIthItemValueString( u32 i ) const OVERRIDE;
		virtual void fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt ) OVERRIDE;
		virtual void fSetFromVector( const Math::tVec4f& v ) OVERRIDE;
		virtual void fSetItemValue( u32 index, const std::string & value ) OVERRIDE;
	};

	template<>
	class base_export tDevVar<tStringPtr> : public tDevMenuItem
	{
		tStringPtr mValue;
	public:
		tDevVar( const char * path, const tStringPtr & startVal )
			: tDevMenuItem( path ), mValue( startVal )
		{
			fFindAndApplyIniOverride( );
		}

		operator const tStringPtr&() const { return mValue; }
		b32 fExists( ) const { return mValue.fExists( ); }

		virtual std::string fIthItemValueString( u32 i ) const OVERRIDE;
		virtual void fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser & user, f32 absTime, f32 dt ) OVERRIDE;
		virtual void fSetFromString( const std::string& v ) OVERRIDE;
		virtual void fSetItemValue( u32 index, const std::string & value ) OVERRIDE;
	};

	///
	/// \brief Base type for dev variable vectors of 2,3, and 4 dimensions.
	/// \see devvar, devrgb, and devrgba macros.
	class base_export tDevVarVectorBase : public tDevMenuItem
	{
		const char** mNameStrings; //ptr to 4 const char* name strings, like the xyz and rgb array below. User can pass in their own.
	public:
		static const char* gRGBAStrings[4];
		static const char* gXYZWStrings[4];

		tDevVarVectorBase( const char* path, u32 dimension, const char** nameStrings = gXYZWStrings ) 
			: tDevMenuItem( path, dimension ), mNameStrings( nameStrings ) { }
		virtual std::string fIthItemName( u32 i ) const OVERRIDE;
	};

#define define_devvar_vector_template( dim ) \
	template<class t> \
	class tDevVar< Math::tVector##dim<t> > : public tDevVarVectorBase \
	{ \
	protected: \
		Math::tVector##dim<t> mValue; \
		Math::tVector##dim<t> mMin, mMax; \
		u32 mPrecision; \
	public: \
		tDevVar( const char* path, const Math::tVector##dim<t>& startVal, const char** names = gXYZWStrings, u32 dims = dim ) \
			: tDevVarVectorBase( path, dims, names ), mValue( startVal ), mMin( -std::numeric_limits<t>::max( ) ), mMax( std::numeric_limits<t>::max( ) ), mPrecision( (t(0.5f)==t(0))?0u:3u ) \
		{ \
			fFindAndApplyIniOverride( ); \
		} \
		tDevVar( const char* path, const Math::tVector##dim<t>& startVal, const Math::tVector##dim<t>& min, const Math::tVector##dim<t>& max, u32 precision, const char** names = gXYZWStrings, u32 dims = dim ) \
			: tDevVarVectorBase( path, dims, names ), mValue( startVal ), mMin( min ), mMax( max ), mPrecision( precision ) \
		{ \
			fFindAndApplyIniOverride( ); \
		} \
		operator const Math::tVector##dim<t>&() const { return mValue; } \
		virtual std::string fIthItemValueString( u32 i ) const OVERRIDE \
		{ \
			std::stringstream ss; \
			ss << std::fixed << std::setprecision( mPrecision ) << mValue.fAxis( i ); \
			return ss.str( ); \
		} \
		virtual void fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt ) OVERRIDE \
		{ \
			fUpdateValue( user, absTime, dt, mValue.fAxis( ithItem ), mPrecision, mMin.fAxis( ithItem ), mMax.fAxis( ithItem ) ); \
		} \
		virtual void fSetFromVector( const Math::tVec4f& v ) OVERRIDE \
		{ \
			for( u32 i = 0; i < fMin<u32>( mValue.cDimension, v.cDimension ); ++i ) \
				fSetSingleValueFromFloat( v.fAxis( i ), mValue.fAxis( i ), mPrecision, mMin.fAxis( i ), mMax.fAxis( i ) ); \
		} \
		virtual void fSetItemValue( u32 index, const std::string & value  ) OVERRIDE \
		{ \
			if( index < mValue.cDimension ) \
			{ \
				std::stringstream ss; ss << value; f32 floatValue; ss >> floatValue; \
				fSetSingleValueFromFloat( floatValue, mValue.fAxis( index ), mPrecision, mMin.fAxis( index ), mMax.fAxis( index ) ); \
			} \
		} \
		virtual Math::tVec4f fGetVector( ) OVERRIDE {  Math::tVec4f result = Math::tVec4f::cZeroVector; for( u32 i = 0; i < dim; ++i ) result[ i ] = mValue[ i ]; return result; } \
	}; \
	template<class t> \
	class tDevVar< Math::tVector##dim<t>* > : public tDevVarVectorBase \
	{ \
		Math::tVector##dim<t>* mValue; \
	protected: \
		Math::tVector##dim<t> mMin, mMax; \
		u32 mPrecision; \
	public: \
		tDevVar( const char* path, Math::tVector##dim<t>& startVal, const char** names = gXYZWStrings, u32 dims = dim ) \
			: tDevVarVectorBase( path, dims, names ), mValue( &startVal ), mMin( -std::numeric_limits<t>::max( ) ), mMax( std::numeric_limits<t>::max( ) ), mPrecision( (t(0.5f)==t(0))?0u:3u ) \
		{ \
			fFindAndApplyIniOverride( ); \
		} \
		tDevVar( const char* path, Math::tVector##dim<t>& startVal, const Math::tVector##dim<t>& min, const Math::tVector##dim<t>& max, u32 precision, const char** names = gXYZWStrings, u32 dims = dim ) \
			: tDevVarVectorBase( path, dims, names ), mValue( &startVal ), mMin( min ), mMax( max ), mPrecision( precision ) \
		{ \
			fFindAndApplyIniOverride( ); \
		} \
		operator const Math::tVector##dim<t>&() const { return *mValue; } \
		virtual std::string fIthItemValueString( u32 i ) const OVERRIDE \
		{ \
			std::stringstream ss; \
			ss << std::fixed << std::setprecision( mPrecision ) << mValue->fAxis( i ); \
			return ss.str( ); \
		} \
		virtual void fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt ) OVERRIDE \
		{ \
			fUpdateValue( user, absTime, dt, mValue->fAxis( ithItem ), mPrecision, mMin.fAxis( ithItem ), mMax.fAxis( ithItem ) ); \
		} \
		virtual void fSetFromVector( const Math::tVec4f& v ) OVERRIDE \
		{ \
			for( u32 i = 0; i < fMin<u32>( mValue->cDimension, v.cDimension ); ++i ) \
				fSetSingleValueFromFloat( v.fAxis( i ), mValue->fAxis( i ), mPrecision, mMin.fAxis( i ), mMax.fAxis( i ) ); \
		} \
		virtual void fSetItemValue( u32 index, const std::string & value ) OVERRIDE \
		{ \
			if( index < mValue->cDimension ) \
			{ \
				std::stringstream ss; ss << value; f32 floatValue; ss >> floatValue; \
				fSetSingleValueFromFloat( floatValue, mValue->fAxis( index ), mPrecision, mMin.fAxis( index ), mMax.fAxis( index ) ); \
			} \
		} \
	};

	define_devvar_vector_template( 2 )
	define_devvar_vector_template( 3 )
	define_devvar_vector_template( 4 )
#undef define_devvar_vector_template

	///
	/// \brief Dev variable type for calling custom user callbacks from the dev menu.
	/// \see devcb macro.
	class base_export tDevCallback : public tDevMenuItem
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
		virtual void fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt );
		virtual void fSetFromString( const std::string& v );
		void fSetValueText( const char* text ) { mValueText = text; }
	};

	// This class works for both vec3 and vec4s
	// Does not handle ptrs.
	class base_export tColorDevVar : public tDevVar< ::Sig::Math::tVec4f >
	{
		Math::tVec3f mV3Cache; // since this class is dual purpose, need to cache off the vec3, so we can safely return a reference to it.
		b32 mShowAlpha;

	public:
		tColorDevVar( const char* path, Math::tVec4f& startVal, const f32& min, const f32& max, b32 showAlpha )
			: tDevVar( path, startVal, min, max, 3 )
			, mShowAlpha( showAlpha )
			, mV3Cache( startVal.fXYZ( ) )
		{
			fCommonInit( );
		}

		tColorDevVar( const char* path, Math::tVec3f& startVal, const f32& min, const f32& max, b32 showAlpha )
			: tDevVar( path, Math::tVec4f( startVal, 0 ), min, max, 3 )
			, mShowAlpha( showAlpha )
			, mV3Cache( startVal )
		{
			fCommonInit( );
		}

		void fCommonInit( )
		{ 
			sigassert( mMin.fMin( ) >= 0 && "Are you drunk? (Negative min color)" );
			fFindAndApplyIniOverride( );
			fSetNumDisplayItems( 1 );
		}

		operator const Math::tVec3f&( ) const { return mV3Cache; }

		virtual void fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt ) OVERRIDE;

		virtual std::string fIthItemName( u32 i ) const OVERRIDE { return fShortName( ); }
		virtual std::string fIthItemValueString( u32 i ) const OVERRIDE { return "Color"; }

		virtual void fSetFromVector( const Math::tVec4f& v ) OVERRIDE
		{
			tDevVar< ::Sig::Math::tVec4f >::fSetFromVector( v );
			mV3Cache = mValue.fXYZ( );
		}
		virtual void fSetItemValue( u32 index, const std::string & value ) OVERRIDE
		{
			tDevVar< ::Sig::Math::tVec4f >::fSetItemValue( index, value );
			mV3Cache = mValue.fXYZ( );
		}
	};

	// This one handles the ptrs, kind of hacky that they're multiple separate classes, but the amount of duplication is very small
	// Could use a macro or something to reduce.
	class base_export tColorDevVarPtr3 : public tDevVar< ::Sig::Math::tVec3f* >
	{
		b32 mShowAlpha;

	public:
		tColorDevVarPtr3( const char* path, Math::tVec3f& startVal, const f32& min, const f32& max  )
			: tDevVar( path, startVal, min, max, 3 )
			, mShowAlpha( false )
		{
			fCommonInit( );
		}

		void fCommonInit( )
		{ 
			sigassert( mMin.fMin( ) >= 0 && "Are you drunk? (Negative min color)" );
			fFindAndApplyIniOverride( );
			fSetNumDisplayItems( 1 );
		}

		virtual void fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt ) OVERRIDE;
		virtual std::string fIthItemName( u32 i ) const OVERRIDE { return fShortName( ); }
		virtual std::string fIthItemValueString( u32 i ) const OVERRIDE { return "Color"; }
	};

	class base_export tColorDevVarPtr4 : public tDevVar< ::Sig::Math::tVec4f* >
	{
		b32 mShowAlpha;

	public:
		tColorDevVarPtr4( const char* path, Math::tVec4f& startVal, const f32& min, const f32& max, b32 showAlpha = true )
			: tDevVar( path, startVal, min, max, 3 )
			, mShowAlpha( showAlpha )
		{
			fCommonInit( );
		}

		void fCommonInit( )
		{ 
			sigassert( mMin.fMin( ) >= 0 && "Are you drunk? (Negative min color)" );
			fFindAndApplyIniOverride( );
			fSetNumDisplayItems( 1 );
		}

		virtual void fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt ) OVERRIDE;
		virtual std::string fIthItemName( u32 i ) const OVERRIDE { return fShortName( ); }
		virtual std::string fIthItemValueString( u32 i ) const OVERRIDE { return "Color"; }
	};

}


#	define devvar( type, name, value )								::Sig::tDevVar<type> name( #name, value )
#   define devvar_p( type, name, value, precision )					::Sig::tDevVar<type> name( #name, value, precision )
#	define devvar_clamp( type, name, value, min, max, precision )	::Sig::tDevVar<type> name( #name, value, min, max, precision )
#	define devrgb( name, value )									::Sig::tColorDevVar name( #name, value, 0.f, 1.f, false )
#	define devrgb_clamp( name, value, min, max )					::Sig::tColorDevVar name( #name, value, min, max, false )
#	define devrgba( name, value )									::Sig::tColorDevVar name( #name, value, 0.f, 1.f, true )
#	define devrgba_clamp( name, value, min, max )					::Sig::tColorDevVar name( #name, value, min, max, true )
#	define devvarptr( type, name, value )							::Sig::tDevVar<type*> name( #name, value )
#	define devvarptr_p( type, name, value, precision )				::Sig::tDevVar<type*> name( #name, value, precision )
#	define devvarptr_clamp( type, name, value, min, max, precision ) ::Sig::tDevVar<type*> name( #name, value, min, max, precision )
#	define devrgbptr( name, value )									::Sig::tColorDevVarPtr3 name( #name, value, 0.f, 1.f )
#	define devrgbptr_clamp( name, value, min, max )					::Sig::tColorDevVarPtr3 name( #name, value, min, max )
#	define devrgbaptr( name, value )								::Sig::tColorDevVarPtr4 name( #name, value, 0.f, 1.f )
#	define devrgbaptr_clamp( name, value, min, max )				::Sig::tColorDevVarPtr4 name( #name, value, min, max )
#	define devcb( name, valueText, cb )								::Sig::tDevCallback name( #name, valueText, cb )
#	define devvar_update_value( name, value )						name.fSetFromVector( value )
#	define devcb_update_value( name, valueText )					name.fSetValueText( valueText )

#else//sig_devmenu

#ifdef _PREFAST_ // indicates code analysis is occuring
// Abuse fFileExists to inhibit code analysis from determining this code involves e.g. constant zeros
#	include "FileSystem.hpp"
#	define devvar( type, name, value )								const type name = ::Sig::FileSystem::fFileExists(::Sig::tFilePathPtr("C:\\asdcoijasdoifjawe4iopufhha49n7y")) ? type(value) : type( value )
#	define devvar_p( type, name, value, precision )					const type name = ::Sig::FileSystem::fFileExists(::Sig::tFilePathPtr("C:\\asdcoijasdoifjawe4iopufhha49n7y")) ? type(value) : type( value )
#	define devvar_clamp( type, name, value, min, max, precision )	const type name = ::Sig::FileSystem::fFileExists(::Sig::tFilePathPtr("C:\\asdcoijasdoifjawe4iopufhha49n7y")) ? type(value) : type( value )
#	define devrgb( name, value )									const ::Sig::Math::tVec3f name = ::Sig::FileSystem::fFileExists(::Sig::tFilePathPtr("C:\\asdcoijasdoifjawe4iopufhha49n7y")) ? ::Sig::Math::tVec3f(9001) : value
#	define devrgb_clamp( name, value, min, max, precision )			const ::Sig::Math::tVec3f name = ::Sig::FileSystem::fFileExists(::Sig::tFilePathPtr("C:\\asdcoijasdoifjawe4iopufhha49n7y")) ? ::Sig::Math::tVec3f(9001) : value
#	define devrgba( name, value )									const ::Sig::Math::tVec4f name = ::Sig::FileSystem::fFileExists(::Sig::tFilePathPtr("C:\\asdcoijasdoifjawe4iopufhha49n7y")) ? ::Sig::Math::tVec4f(9001) : value
#	define devrgba_clamp( name, value, min, max, precision )		const ::Sig::Math::tVec4f name = ::Sig::FileSystem::fFileExists(::Sig::tFilePathPtr("C:\\asdcoijasdoifjawe4iopufhha49n7y")) ? ::Sig::Math::tVec4f(9001) : value
#	define devvarptr( type, name, value )							const type& name = value
#	define devvarptr_p( type, name, value, precision )				const type& name = value
#	define devvarptr_clamp( type, name, value, min, max, precision ) const type& name = value
#	define devrgbptr( name, value )									const ::Sig::Math::tVec3f& name = value
#	define devrgbptr_clamp( name, value, min, max, precision )		const ::Sig::Math::tVec3f& name = value
#	define devrgbaptr( name, value )								const ::Sig::Math::tVec4f& name = value
#	define devrgbaptr_clamp( name, value, min, max, precision )		const ::Sig::Math::tVec4f& name = value
#	define devcb( name, valueText, cb )
#	define devvar_update_value( name, value )
#	define devcb_update_value( name, valueText )

#else
#	define devvar( type, name, value )								const type name = type( value )
#	define devvar_p( type, name, value, precision )					const type name = type( value )
#	define devvar_clamp( type, name, value, min, max, precision )	const type name = type( value )
#	define devrgb( name, value )									const ::Sig::Math::tVec3f name = value
#	define devrgb_clamp( name, value, min, max )					const ::Sig::Math::tVec3f name = value
#	define devrgba( name, value )									const ::Sig::Math::tVec4f name = value
#	define devrgba_clamp( name, value, min, max )					const ::Sig::Math::tVec4f name = value
#	define devvarptr( type, name, value )							const type& name = value
#	define devvarptr_p( type, name, value, precision )				const type& name = value
#	define devvarptr_clamp( type, name, value, min, max, precision ) const type& name = value
#	define devrgbptr( name, value )									const ::Sig::Math::tVec3f& name = value
#	define devrgbptr_clamp( name, value, min, max )					const ::Sig::Math::tVec3f& name = value
#	define devrgbaptr( name, value )								const ::Sig::Math::tVec4f& name = value
#	define devrgbaptr_clamp( name, value, min, max )				const ::Sig::Math::tVec4f& name = value
#	define devcb( name, valueText, cb )
#	define devvar_update_value( name, value )
#	define devcb_update_value( name, valueText )
#endif

#endif//sig_devmenu

#endif//__tDevVarTypes__
