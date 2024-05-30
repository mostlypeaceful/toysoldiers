#ifndef __tVarProperty__
#define __tVarProperty__
#include "Memory/tPool.hpp"

namespace Sig
{
	///
	/// \brief Simple variant-style property base type. A property is a (name,value) pair,
	/// where the value is of varying type.
	class base_export tVarProperty : public tRefCounter
	{
	protected:
		tStringPtr mName;

	public:
		virtual ~tVarProperty( ) { }

		const tStringPtr& fName( ) const { return mName; }

		template<class t>
		b32 fValueIs( ) const { return fValueAs<t>( ) != NULL; }

		template<class t>
		t* fValueAs( ) { return ( t* )fValuePtr( Rtti::fGetClassId< t > ( ) ); }

		template<class t>
		const t* fValueAs( ) const { return ( const t* )fValuePtr( Rtti::fGetClassId< t > ( ) ); }

		template<class t>
		b32 fSetValue( const t& v )
		{
			if( t* vptr = fValueAs<t>( ) )
			{
				*vptr = v;
				return true;
			}

			return false;
		}

		template<class t>
		b32 fEqual( const t& v ) const
		{
			const t* vptr = fValueAs<t>( );
			return vptr ? Sig::fEqual( *vptr, v ) : false;
		}

		virtual Rtti::tClassId fClassId( ) const = 0;

	protected:

		explicit tVarProperty( const tStringPtr& name ) : mName( name ) { }
		virtual void* fValuePtr( Rtti::tClassId cid ) const { return 0; }
	};

	///
	/// \brief Template facilitating instantiation of basic property types (int, float, other types supporting basic operators).
	template<class t>
	class tVarPropertyTemplate : public tVarProperty
	{
		define_class_pool_new_delete( tVarPropertyTemplate, 1024 );
	public:
		static const Rtti::tClassId cCid;
	private:
		t mValue;
	public:
		explicit tVarPropertyTemplate( const tStringPtr& name, const t& value ) : tVarProperty( name ), mValue( value ) { }
		virtual Rtti::tClassId fClassId( ) const { return cCid; }
	protected:
		virtual void* fValuePtr( Rtti::tClassId cid ) const { return cid == cCid ? ( void* )&mValue : 0; }
	};

	template<class t>
	const Rtti::tClassId tVarPropertyTemplate<t>::cCid = Rtti::fGetClassId<t>( );

	typedef tVarPropertyTemplate<s32> tVarPropertyS32;
	typedef tVarPropertyTemplate<u32> tVarPropertyU32;
	typedef tVarPropertyTemplate<f32> tVarPropertyF32;
	
	typedef tVarPropertyTemplate<Math::tVec2f> tVarPropertyVec2f;
	typedef tVarPropertyTemplate<Math::tVec3f> tVarPropertyVec3f;
	typedef tVarPropertyTemplate<Math::tVec4f> tVarPropertyVec4f;
	typedef tVarPropertyTemplate<tStringPtr> tVarPropertyStringPtr;
	

	///
	/// \brief Property type whose name is equivalent to the value.
	class base_export tVarPropertyTag : public tVarProperty
	{
	public:
		static const Rtti::tClassId cCid;
	public:
		explicit tVarPropertyTag( const tStringPtr& tag ) : tVarProperty( tag ) { }
		virtual Rtti::tClassId fClassId( ) const { return cCid; }
	protected:
		virtual void* fValuePtr( Rtti::tClassId cid ) const { return cid == cCid ? ( void* )&mName : 0; }
	};

	define_smart_ptr( base_export, tRefCounterPtr, tVarProperty );

	///
	/// \class tVarPropertyBag
	/// \brief Collection of properties
	class tVarPropertyBag
	{
	public:

		template< class T >
		void fAdd( const tStringPtr& name, const T& value )
		{
			fAdd( tVarPropertyPtr( NEW_TYPED(tVarPropertyTemplate<T>)( name, value ) ) );
		}
		
		void fAdd( const tVarPropertyPtr& prop );
		b32 fRemove( const tStringPtr& name );

		tVarProperty* fFind( const tStringPtr& name ) const;

		u32 fCount( ) const { return mProperties.fCount( ); }
		tVarProperty& fIndex( u32 idx ) const { return *mProperties[ idx ]; }

	private:

		s32 fIndexOf( const tStringPtr& name ) const;

	private:

		tGrowableArray< tVarPropertyPtr > mProperties;
	};
}


#endif//__tVarProperty__

