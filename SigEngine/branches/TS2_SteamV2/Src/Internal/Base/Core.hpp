#ifndef __Core__
#define __Core__

#ifdef assert
#	undef assert
#endif

#ifdef false
#	undef false
#endif

#ifdef true
#	undef true
#endif

#define false	0u
#define true	1u

namespace Sig
{
	#define join_macros( X, Y )		join_macros_1( X, Y )
	#define join_macros_1( X, Y )	join_macros_2( X, Y )
	#define join_macros_2( X, Y )	X##Y

    template<bool>	struct tStaticAssert;

	template<>		struct tStaticAssert<true> { typedef int type; };

	#define sig_static_assert(condition) typedef ::Sig::tStaticAssert<(condition)>::type join_macros(SIG_STATIC_ASSERTION_FAILED_, __LINE__)

	#define array_length( x ) (sizeof(x)/sizeof(x[0]))

	#define sizeof_member( c, m ) (sizeof( ((c*)0)->m ))

	#define call_member_fn_ptr( o, m ) ((o).*(m))

	class base_export tNoOpTag { };
	base_export extern const tNoOpTag cNoOpTag;

	#define declare_global_object( t ) \
		public: static t& fInstance( ) { static t gT; return gT; }

	#define declare_singleton_define_own_ctor_dtor( t ) \
		private: t(const t&); \
		private: t& operator=(const t&); \
		declare_global_object( t )

	#define declare_singleton( t ) \
		private: t( ) { } ~t( ) { } \
		declare_singleton_define_own_ctor_dtor( t )

	#define define_smart_ptr( exportTag, ptrType, className ) \
		class exportTag className##Ptr : public ptrType< className > \
		{ \
		public: \
			inline className##Ptr( ) { } \
			inline explicit className##Ptr( className* p ) : ptrType< className >( p ) { } \
			inline className##Ptr( const ptrType< className >& base ) : ptrType< className >( base ) { } \
		}

	#define define_smart_const_ptr( exportTag, ptrType, className ) \
		class exportTag className##ConstPtr : public ptrType< const className > \
		{ \
		public: \
			inline className##ConstPtr( ) { } \
			inline explicit className##ConstPtr( const className* p ) : ptrType< const className >( p ) { } \
			inline className##ConstPtr( const ptrType< const className >& base ) : ptrType< const className >( base ) { } \
		}

	struct tCString { tCString( const char* cstr ) : mCStr( cstr ) { } operator bool() const { return mCStr!=0; } const char* mCStr; };

	template<class tA, class tB>
	class tPair
	{
	public:
		tA mA;
		tB mB;

		inline tPair( ) { }
		inline tPair( const tA& a, const tB& b ) : mA( a ), mB( b ) { }
		inline b32 operator==( const tPair<tA,tB>& rhs ) const
		{
			return fEqual( mA, rhs.mA ) && fEqual( mB, rhs.mB );
		}
	};
	template<class tA, class tB>
	inline tPair<tA,tB> fMakePair( const tA& a, const tB& b )
	{
		return tPair<tA,tB>( a, b );
	}

	class base_export tStaticFunctionCall
	{
	public:
		typedef void (*function_signature)( );
		tStaticFunctionCall( function_signature pf ) { pf( ); }
	};
	#define define_static_function(name) \
		void name( ); static ::Sig::tStaticFunctionCall _##name( name ); \
		void name( )

	template<class b, class a>
	class tInheritsFrom
	{
	private:

		typedef u8		tSmall;
		typedef u16		tBig;
		static tSmall	fTestIfBIsA( const a& );
		static tBig		fTestIfBIsA( ... );
		static b		fMakeB( );

	public:

		enum { cIsSubClass = ( sizeof( fTestIfBIsA( fMakeB( ) ) ) == sizeof( tSmall ) ) };
	};

	#define can_convert(b,a) tInheritsFrom<const b*, const a*>::cIsSubClass

	template<class t>
	class tIsBuiltInType
	{
	public:
		static const b32 cIs = false;
	};

	template<class t>
	class tIsBuiltInType<t*>
	{
	public:
		static const b32 cIs = tIsBuiltInType<t>::cIs;
	};

	template<> class tIsBuiltInType<char>				{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<u8>					{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<s8>					{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<u16> 				{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<s16> 				{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<u32> 				{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<s32> 				{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<u64> 				{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<s64> 				{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<f32> 				{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<f64> 				{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<const char>			{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<const u8>			{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<const s8>			{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<const u16> 			{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<const s16> 			{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<const u32> 			{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<const s32> 			{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<const u64> 			{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<const s64> 			{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<const f32> 			{ public: static const b32 cIs = true; };
	template<> class tIsBuiltInType<const f64> 			{ public: static const b32 cIs = true; };

	template<class t>
	inline b32 fIsBuiltinType( )
	{
		return tIsBuiltInType<t>::cIs;
	}

	template<class t>
	class tIsPointer
	{
	public: static const b32 cIs = false;
	};

	template<class t>
	class tIsPointer<t*>
	{
	public: static const b32 cIs = true;
	};

	template<class t>
	inline b32 fIsPointer( )
	{
		return tIsPointer<t>::cIs;
	}

#	define offsetof_base( derived, base ) (( u32 )(( size_t )( base* )( derived* )1 - 1))

	template<class derived, class base>
	inline u32 fOffsetOfBase( )
	{
		return offsetof_base( derived, base );
	}
	template<class t>
	inline t fMin( const t& a, const t& b )
	{
		return a < b ? a : b;
	}
	template<class t>
	inline t fMin( const t& a, const t& b, const t& c )
	{
		return fMin( fMin( a, b ), c );
	}
	template<class t>
	inline t fMin( const t& a, const t& b, const t& c, const t& d )
	{
		return fMin( fMin( fMin( a, b ), c ), d );
	}
	template<class t>
	inline t fMax( const t& a, const t& b )
	{
		return a > b ? a : b;
	}
	template<class t>
	inline t fMax( const t& a, const t& b, const t& c )
	{
		return fMax( fMax( a, b ), c );
	}
	template<class t>
	inline t fMax( const t& a, const t& b, const t& c, const t& d )
	{
		return fMax( fMax( fMax( a, b ), c ), d );
	}

	template<class t>
	inline t fClamp( const t& toClamp, const t& min, const t& max )
	{
		return fMax( fMin( toClamp, max ), min );
	}

	template<class t>
	inline t fSaturate( const t& toClamp ) // same as fClamp( value, 0, 1 )
	{
		return fMax( fMin( toClamp, 1.f ), 0.f );
	}

	template<class t>
	inline t fWrap( const t& toClamp, const t& min, const t& max )
	{
		const f32 s = ( f32 )( max - min );
		const f32 r = std::fmod( ( f32 )( toClamp - min ), s );
		return min + ( r < 0.f ? ( s + r ) : r );
	}

	template<class t>
	inline t fPulseFromZero( const t& toPulse ) // 0 => 0
	{
		return fAbs( std::sin( toPulse ) );
	}

	template<class t>
	inline t fPulseFromOne( const t& toPulse ) // 0 => 1
	{
		return fAbs( std::cos( toPulse ) );
	}

	template<class t>
	inline b32 fInBounds( const t& toCheck, const t& min, const t& max )
	{
		return ( toCheck >= min ) && ( toCheck <= max );
	}

	template<class t>
	inline t fAbs( const t& a )
	{
		return a < 0 ? -a : a;
	}

	template<class t>
	inline void fSwap( t& a, t& b )
	{
		t c = a;
		a = b;
		b = c;
	}

	template<class t>
	inline s32 fPtrDiff( const t* high, const t* low )
	{
		return ( s32 )( ptrdiff_t )( high - low );
	}

	template<class tReturnType>
	inline tReturnType fRound( const f32 f )
	{
		return ( tReturnType )floor( f + 0.5f );
	}

	template<class tReturnType>
	inline tReturnType fRoundUp( const f32 f )
	{
		return ( tReturnType )ceil( f );
	}

	template<class tReturnType>
	inline tReturnType fRoundDown( const f32 f )
	{
		return ( tReturnType )floor( f );
	}

	template<class t>
	inline t fSign( const t& value )
	{
		return ( value > t(0) ? t(1) : ( value < t(0) ? t(-1) : t(0) ) );
	}

	template<class t>
	inline t fAlignHigh( t valueToAlign, t alignment )
	{
		return ( ( valueToAlign + alignment - 1 ) / alignment ) * alignment;
	}

	template<class t>
	inline t fAlignLow( t valueToAlign, t alignment )
	{
		return ( valueToAlign / alignment ) * alignment;
	}

	template<class t>
	inline b32 fIsAligned( t valueInQuestion, t alignment )
	{
		return ( valueInQuestion % alignment ) == 0;
	}

	inline void fMemCpy( void* dst, const void* src, size_t byteCount )
	{
#ifdef platform_xbox360
		XMemCpy( dst, src, byteCount );
#else
		memcpy( dst, src, byteCount );
#endif
	}

	inline void fMemCpyToGpu( void* dst, const void* src, size_t byteCount )
	{
#ifdef platform_xbox360
		XMemCpyStreaming_WriteCombined( dst, src, byteCount );
#else
		memcpy( dst, src, byteCount );
#endif
	}

	inline void fMemMove( void* dst, const void* src, size_t byteCount )
	{
		memmove( dst, src, byteCount );
	}

	template<class t>
	inline void fMemSet( t& object, byte value )
	{
		memset( &object, value, sizeof( object ) );
	}

	inline void fMemSet( void* dst, byte value, size_t byteCount )
	{
		memset( dst, value, byteCount );
	}

	inline s32 fMemCmp( const void* a, const void* b, size_t byteCount )
	{
		return memcmp( a, b, byteCount );
	}

	template<class t>
	inline void fZeroOut( t& object )
	{
		fMemSet( object, 0 );
	}

	template<class t>
	inline void fZeroOut( t* objectPtr, u32 numInARow=1 )
	{
		fMemSet( objectPtr, 0, numInARow * sizeof(t) );
	}

	template<class t, class u>
	inline b32 fTestBits( t mask, u flags )
	{
		return ( mask & flags ) ? true : false;
	}

	template<class t, class u>
	inline t fSetBits( t mask, u flags )
	{
		return mask | flags;
	}

	template<class t, class u>
	inline t fClearBits( t mask, u flags )
	{
		return mask & ~flags;
	}

	template<class t, class u>
	inline t fSetClearBits( t mask, u flags, b32 set )
	{
		return set ? fSetBits( mask, flags ) : fClearBits( mask, flags );
	}


	inline u8 fLowNibble( u8 value )
	{
		return ( value & 0x0f );
	}

	inline u8 fHighNibble( u8 value )
	{
		return ( value & 0xf0 ) >> 4;
	}

	inline u8 fMakeByte( u8 low, u8 high )
	{
		return ( ( high << 4 ) & 0xf0 ) | ( low & 0x0f );
	}

	inline u8 fLowByte( u16 value )
	{
		return u8( value & 0x00ff );
	}

	inline u8 fHighByte( u16 value )
	{
		return u8( ( value & 0xff00 ) >> 8 );
	}

	inline u16 fMakeU16( u8 low, u8 high )
	{
		return ( ( high << 8 ) & 0xff00 ) | ( low & 0x00ff );
	}

	inline u16 fLowU16( u32 value )
	{
		return u16( value & 0x0000ffff );
	}

	inline u16 fHighU16( u32 value )
	{
		return u16( ( value & 0xffff0000 ) >> 16 );
	}

	inline u32 fMakeU32( u16 low, u16 high )
	{
		return ( ( high << 16 ) & 0xffff0000 ) | ( low & 0x0000ffff );
	}

	template<class tInt>
	inline b32 fIsOdd( tInt i )
	{
		return ( i & 1 ) ? true : false;
	}

	template<class tInt>
	inline b32 fIsEven( tInt i )
	{
		return ( i & 1 ) ? false : true;
	}

	template<class t, class u>
	inline b32 fEqual( const t& a, const u& b, const f32 epsilon=0.00001f )
	{
		return a == b;
	}
	template<>
	inline b32 fEqual<f32,f32>( const f32& a, const f32& b, const f32 epsilon )
	{
		return ( a <= b + epsilon ) && ( a >= b - epsilon );
	}
	template<>
	inline b32 fEqual<f64,f64>( const f64& a, const f64& b, const f32 epsilon )
	{
		return ( a <= b + epsilon ) && ( a >= b - epsilon );
	}

	template<class t, class tBoolReturn=b32>
	struct tEqual
	{
		inline tBoolReturn operator( )( const t& a, const t& b ) const { return fEqual( a, b ) ? true : false; }
	};

	template<class t, class tBoolReturn=b32>
	struct tLess
	{
		inline tBoolReturn operator( )( const t& a, const t& b ) const { return a < b ? true : false; }
	};

	template<class t, class tBoolReturn=b32>
	struct tLessEqual
	{
		inline tBoolReturn operator( )( const t& a, const t& b ) const { return a <= b ? true : false; }
	};

	template<class t, class tBoolReturn=b32>
	struct tGreater
	{
		inline tBoolReturn operator( )( const t& a, const t& b ) const { return a > b ? true : false; }
	};

	template<class t, class tBoolReturn=b32>
	struct tGreaterEqual
	{
		inline tBoolReturn operator( )( const t& a, const t& b ) const { return a >= b ? true : false; }
	};

	template<class t>
	inline u32 fNullTerminatedLength( const t* nullTerminatedArray )
	{
		if( !nullTerminatedArray ) return 0;
        const t* tmp = nullTerminatedArray;
        while( *tmp ) ++tmp;
        return tmp - nullTerminatedArray;
	}

	base_export void fSleep( u32 numMs=0 );

	class tScriptVm;
}

#endif//__Core__
