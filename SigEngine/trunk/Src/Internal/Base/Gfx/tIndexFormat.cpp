#include "BasePch.hpp"
#include "tIndexFormat.hpp"

namespace Sig { namespace Gfx
{

	u32 tIndexFormat::fGetIndexSize( tStorageType storageType )
	{
		u32 o = 0;
		switch( storageType )
		{
		case cStorageU16:	o = sizeof( u16 ); break;
		case cStorageU32:	o = sizeof( u32 ); break;
		default:			sigassert( !"invalid index format" ); break;
		}
		return o;
	}

	u32 tIndexFormat::fGetMaxAllowableValue( tStorageType storageType )
	{
		u32 o = 0;
		switch( storageType )
		{
		case cStorageU16:	o = 0xfffe; break; // last value 0xffff is saved for special significance
		case cStorageU32:	o = 0xfffff; break; // this may vary by platform; my dx caps say 1048575
		default:			sigassert( !"invalid index format" ); break;
		}
		return o;
	}

	tIndexFormat tIndexFormat::fCreateAppropriateFormat( tPrimitiveType primType, u32 highestVertexIndex )
	{
		if( highestVertexIndex <= fGetMaxAllowableValue( cStorageU16 ) )
			return tIndexFormat( cStorageU16, primType );
		else if( highestVertexIndex <= fGetMaxAllowableValue( cStorageU32 ) )
			return tIndexFormat( cStorageU32, primType );

		sigassert( !"cannot create an appropriate index format!" );
		return tIndexFormat( );
	}

}}
