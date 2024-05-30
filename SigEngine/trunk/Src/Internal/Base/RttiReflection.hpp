#ifndef __tRttiReflection__
#define __tRttiReflection__

namespace Sig { namespace Rtti
{
	template<class t>
	tReflector fDefineReflector( const char* type, const tBaseClassDesc* baseClassesArray, const tClassMemberDesc* membersArray )
	{
		sigassert( !fIsBuiltinType<t>( ) );
		return tReflector( sizeof(t), fGetClassId<t>( ), type, false, can_convert( t, tSerializableBaseClass ), baseClassesArray, membersArray );
	}

	template<class t>
	tReflector fDefineBuiltInTypeReflector( const char* type )
	{
		return tReflector( sizeof(t), fGetClassId<t>( ), type, true, false, 0, 0 );
	}

#	define	define_built_in_type_reflector(type) fDefineBuiltInTypeReflector<type>(#type)

	template<class t>
	class tReflectorAccessor
	{
	public: static inline const tReflector* fAccess( ) { return &t::fGetReflector( ); }
	};

	template<class t>
	class tReflectorAccessor<t*>
	{
	public: static inline const tReflector* fAccess( ) { return tReflectorAccessor<t>::fAccess( ); }
	};

	template<> class tReflectorAccessor<void> { public: 
		static inline const tReflector* fAccess( ) { static tReflector gR( 0, fGetClassId<void>( ), "void", true, false, 0, 0 ); return &gR; } };
	template<> class tReflectorAccessor<char> { public: 
		static inline const tReflector* fAccess( ) { static tReflector gR = define_built_in_type_reflector(byte); return &gR; } };
	template<> class tReflectorAccessor<u8> { public: 
		static inline const tReflector* fAccess( ) { static tReflector gR = define_built_in_type_reflector(u8); return &gR; } };
	template<> class tReflectorAccessor<s8> { public: 
		static inline const tReflector* fAccess( ) { static tReflector gR = define_built_in_type_reflector(s8); return &gR; } };
	template<> class tReflectorAccessor<u16> { public: 
		static inline const tReflector* fAccess( ) { static tReflector gR = define_built_in_type_reflector(u16); return &gR; } };
	template<> class tReflectorAccessor<s16> { public: 
		static inline const tReflector* fAccess( ) { static tReflector gR = define_built_in_type_reflector(s16); return &gR; } };
	template<> class tReflectorAccessor<u32> { public: 
		static inline const tReflector* fAccess( ) { static tReflector gR = define_built_in_type_reflector(u32); return &gR; } };
	template<> class tReflectorAccessor<s32> { public: 
		static inline const tReflector* fAccess( ) { static tReflector gR = define_built_in_type_reflector(s32); return &gR; } };
	template<> class tReflectorAccessor<u64> { public: 
		static inline const tReflector* fAccess( ) { static tReflector gR = define_built_in_type_reflector(u64); return &gR; } };
	template<> class tReflectorAccessor<s64> { public: 
		static inline const tReflector* fAccess( ) { static tReflector gR = define_built_in_type_reflector(s64); return &gR; } };
	template<> class tReflectorAccessor<f32> { public: 
		static inline const tReflector* fAccess( ) { static tReflector gR = define_built_in_type_reflector(f32); return &gR; } };
	template<> class tReflectorAccessor<f64> { public: 
		static inline const tReflector* fAccess( ) { static tReflector gR = define_built_in_type_reflector(f64); return &gR; } };

#	undef define_built_in_type_reflector

	template<class t>
	const tReflector* fGetReflectorFromClass( )
	{
		const tReflector* o = tReflectorAccessor<t>::fAccess( );
		sigassert( o );
		return o;
	}


	///
	/// \brief Stores information pertaining to a base class;
	/// used in conjunction with the tReflector class.
	class base_export tBaseClassDesc
	{
	private:

		u32					mOffsetFromBase;
		const char*			mTypeName;
		const char*			mAccess;
		const tReflector*	mReflector;

	public:

		inline tBaseClassDesc( )
		{
			fZeroOut( this );
		}

		tBaseClassDesc( 
			u32 o, 
			const char* t,
			const char* a,
			const tReflector* r )
			: mOffsetFromBase( o )
			, mTypeName( t ) 
			, mAccess( a )
			, mReflector( r )
		{ }

		inline void* fComputeAddress( const void* object ) const
		{
			return ( void* )( ( const byte* )object + mOffsetFromBase );
		}

		inline u32					fOffsetFromBase( )	const { return mOffsetFromBase; }
		inline const char*			fTypeName( )		const { return mTypeName; }
		inline const char*			fAccess( )			const { return mAccess; }
		inline const tReflector*	fReflector( )		const { return mReflector; }
		inline b32					fNull( )			const { return mReflector ? false : true; }
	};

	inline tBaseClassDesc fDefineBaseClassDesc( )
	{
		return tBaseClassDesc( );
	}

	template<class tContainer, class tBase>
	inline tBaseClassDesc fDefineBaseClassDesc( 
				const char* containerName, 
				const char* baseName, 
				const char* access )
	{
		return tBaseClassDesc( 
			fOffsetOfBase<tContainer,tBase>( ), 
			baseName,
			access,
			fGetReflectorFromClass<tBase>( ) );
	}


	///
	/// \brief Stores information pertaining to a single class member variable;
	/// used in conjunction with the tReflector class.
	class base_export tClassMemberDesc : public tBaseClassDesc
	{
	private:

		b32				mIsBuiltIn;
		b32				mIsPointer;
		u32				mArrayCount;
		u32				mDynamicArrayCountOffset;
		u32				mDynamicArrayCountSize;
		const char*		mName;

	public:

		inline tClassMemberDesc( )
		{
			fZeroOut( this );
		}

		tClassMemberDesc( 
			b32 ib, 
			b32 ip,
			u32 arrayCount,
			u32 dynArrayCountOffset,
			u32 dynArrayCountSize,
			const char* n,
			u32 o, 
			const char* t,
			const char* a,
			const tReflector* r )
			: tBaseClassDesc( o, t, a, r )
			, mIsBuiltIn( ib )
			, mIsPointer( ip )
			, mArrayCount( arrayCount )
			, mDynamicArrayCountOffset( dynArrayCountOffset )
			, mDynamicArrayCountSize( dynArrayCountSize )
			, mName( n )
		{ }

		inline u64 fComputeArrayCount( const void* object ) const
		{
			switch( mDynamicArrayCountSize )
			{
			case sizeof(u8):  return *( const u8* ) ( ( const byte* )object + mDynamicArrayCountOffset );
			case sizeof(u16): return *( const u16* )( ( const byte* )object + mDynamicArrayCountOffset );
			case sizeof(u32): return *( const u32* )( ( const byte* )object + mDynamicArrayCountOffset );
			case sizeof(u64): return *( const u64* )( ( const byte* )object + mDynamicArrayCountOffset );
			default: break;
			}

			return mArrayCount;
		}

		inline b32			fIsBuiltIn( )				const { return mIsBuiltIn; }
		inline b32			fIsPointer( )				const { return mIsPointer; }
		inline u32			fArrayCount( )				const { return mArrayCount; }
		inline u32			fDynamicArrayCountOffset( ) const { return mDynamicArrayCountOffset; }
		inline u32			fDynamicArrayCountSize( )	const { return mDynamicArrayCountSize; }
		inline const char*	fName( )					const { return mName; }

	};

	inline tClassMemberDesc fDefineClassMemberDesc( )
	{
		return tClassMemberDesc( );
	}

	template<class tContainer, class tMember>
	inline tClassMemberDesc fDefineClassMemberDesc( 
				const char* containerName, 
				const char* memberType, 
				const char* memberName, 
				const char* access,
				u32 memberOffset,
				u32 arrayCount,
				u32 dynArrayCountOffset,
				u32 dynArrayCountSize )
	{
		return tClassMemberDesc( 
			fIsBuiltinType<tMember>( ),
			fIsPointer<tMember>( ),
			arrayCount,
			dynArrayCountOffset,
			dynArrayCountSize,
			memberName,
			memberOffset,
			memberType,
			access,
			fGetReflectorFromClass<tMember>( ) );
	}

#	define dynamic_array_count( x )


	///
	/// \brief Macro facilitating implementation of tBaseClass methods in a derived class.
#	define implement_rtti_base_class( t ) \
		public:		virtual ::Sig::Rtti::tClassId		fClassId( )			const	{ return ::Sig::Rtti::fGetClassId<t>( ); } \
		public:		virtual u32							fClassSizeof( )		const	{ return sizeof( t ); }


	///
	/// \brief Macro facilitating implementation of tSerializableBaseClass methods in a derived class.
#	define implement_rtti_serializable_base_class( t, staticId ) \
		implement_rtti_base_class( t ) \
		public:		static void fStaticAssertLegal( ) { static_assert( can_convert( t, ::Sig::Rtti::tSerializableBaseClass ) ); } \
		public:		static const ::Sig::Rtti::tClassId cClassId = staticId; \
		public:		virtual const ::Sig::Rtti::tReflector& fGetDerivedReflector( ) const { return fGetReflector( ); }

#	define declare_lip_version( ) \
		public: \
			static const u32 cLipSaveVersion; \
			static const u32 cLipMinLoadVersion; \
			static const u32 cLipMaxLoadVersion;

#	define define_lip_version( type, saveVer, minLoadVer, maxLoadVer ) \
		const u32 type :: cLipSaveVersion = saveVer ; \
		const u32 type :: cLipMinLoadVersion = minLoadVer ; \
		const u32 type :: cLipMaxLoadVersion = maxLoadVer ;

}}


#endif//__tRttiReflection__
