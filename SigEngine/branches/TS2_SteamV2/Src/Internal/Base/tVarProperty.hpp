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
	private:
		tStringPtr mName;

	public:
		explicit tVarProperty( const tStringPtr& name ) : mName( name ) { }
		virtual ~tVarProperty( ) { }

		virtual Rtti::tClassId fClassId( ) const = 0;
		virtual tVarProperty* fClone( ) const = 0;

		const tStringPtr& fName( ) const { return mName; }

		template<class t>
		b32 fCheckType( ) const
		{
			const t* vptr = ( const t* )fValuePtr( Rtti::fGetClassId< t > ( ) );
			return vptr != 0;
		}

		template<class t>
		const t& fValue( ) const
		{
			static const t gDefault;
			const t* vptr = ( const t* )fValuePtr( Rtti::fGetClassId< t > ( ) );
			return vptr ? *vptr : gDefault;
		}

		template<class t>
		b32 fSetValue( const t& v )
		{
			t* vptr = ( t* )fValuePtr( Rtti::fGetClassId< t > ( ) );
			if( vptr )
			{
				*vptr = v;
				return true;
			}
			return false;
		}

		template<class t>
		b32 fEqual( const t& v ) const
		{
			const t* vptr = ( const t* )fValuePtr( Rtti::fGetClassId< t > ( ) );
			return vptr ? Sig::fEqual( *vptr, v ) : false;
		}

	protected:
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
		virtual tVarProperty* fClone( ) const { return NEW tVarPropertyTemplate( fName( ), mValue ); }
	protected:
		virtual void* fValuePtr( Rtti::tClassId cid ) const { if( cid == cCid ) return ( void* )&mValue; return 0; }
	};

	template<class t>
	const Rtti::tClassId tVarPropertyTemplate<t>::cCid = Rtti::fGetClassId<t>( );

	typedef tVarPropertyTemplate<s32> tVarPropertyInt;
	typedef tVarPropertyTemplate<f32> tVarPropertyFloat;
	typedef tVarPropertyTemplate<Math::tVec2f> tVarPropertyVec2f;
	typedef tVarPropertyTemplate<Math::tVec3f> tVarPropertyVec3f;
	typedef tVarPropertyTemplate<Math::tVec4f> tVarPropertyVec4f;
	typedef tVarPropertyTemplate<tStringPtr> tVarPropertyStringPtr;
	

	///
	/// \brief Property type whose name is equivalent to the value.
	class base_export tVarPropertyTag : public tVarPropertyStringPtr
	{
	public:
		explicit tVarPropertyTag( const tStringPtr& name ) : tVarPropertyStringPtr( name, name ) { }
		virtual tVarProperty* fClone( ) const { return NEW tVarPropertyTag( fName( ) ); }
	};

	define_smart_ptr( base_export, tRefCounterPtr, tVarProperty );

//#define _var_property_use_hash_table_
#ifdef _var_property_use_hash_table_
	typedef tHashTable< tStringPtr, tVarPropertyPtr > tVarPropertyTableBase;

	///
	/// \brief Container for properties (hash table, for O(k) find by name).
	class base_export tVarPropertyTable : public tVarPropertyTableBase
	{
		define_class_pool_new_delete( tVarPropertyTable, 128 );
	public:
		u32 mTagMask;
	public:
		tVarPropertyTable( ) : tVarPropertyTableBase( 4 ), mTagMask( 0 ) { }
		void fAddClones( tVarPropertyTable& addTo ) const;
	};
#else//_var_property_use_hash_table_
	typedef tGrowableArray< tVarPropertyPtr > tVarPropertyTableBase;

	///
	/// \brief Container for properties (hash table, for O(k) find by name).
	class base_export tVarPropertyTable : public tVarPropertyTableBase
	{
		define_class_pool_new_delete( tVarPropertyTable, 128 );
	public:
		u32 mTagMask;
	public:
		tVarPropertyTable( ) : mTagMask( 0 ) { }
		void fAddClones( tVarPropertyTable& addTo ) const;
		void fInsert( const tStringPtr& name, const tVarPropertyPtr& prop );
		tVarPropertyPtr* fFind( const tStringPtr& name );
		b32 fRemove( const tStringPtr& name );
		void fRemove( tVarPropertyPtr* prop );
	};
#endif//_var_property_use_hash_table_

}


#endif//__tVarProperty__

