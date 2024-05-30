#include "BasePch.hpp"
#include "tVertexFormat.hpp"

namespace Sig { namespace Gfx
{
	u32 tVertexColor::fForGpu( tPlatformId pid ) const
	{
		switch( pid )
		{
		case cPlatformPcDx9:	return tVertexColor( mB, mG, mR, mA ).mAsU32;
		case cPlatformPcDx10:	return tVertexColor( mB, mG, mR, mA ).mAsU32;
#ifdef platform_xbox360
		case cPlatformXbox360:	return tVertexColor( mA, mR, mG, mB ).mAsU32;
#else
		case cPlatformXbox360:	return tVertexColor( mB, mG, mR, mA ).mAsU32;
#endif//platform_xbox360
		default: sigassert( !"tVertexColor::fForGpu( ): need to update for other platforms" ); break;
		}

		// swizzle out components to make d3d9 happy
		return tVertexColor( mB, mG, mR, mA ).mAsU32;
	}

	u16 tVertexElement::fSizeFromFormat( tFormat format )
	{
		u16 size = 0;
		switch( format )
		{
		case cFormat_f32_1:				size = sizeof(f32) * 1; break;
		case cFormat_f32_2:				size = sizeof(f32) * 2; break;
		case cFormat_f32_3:				size = sizeof(f32) * 3; break;
		case cFormat_f32_4:				size = sizeof(f32) * 4; break;
		case cFormat_f16_2:				size = sizeof(u16) * 2; break;
		case cFormat_f16_4:				size = sizeof(u16) * 4; break;
		case cFormat_u8_4:				size = sizeof(u8)  * 4; break;
		case cFormat_u8_4_Color:		size = sizeof(u8)  * 4; break;
		case cFormat_u8_4_Normalized:	size = sizeof(u8)  * 4; break;
		default:						sigassert(!"Invalid vertex element format!"); break;
		}
		return size;
	}


	tVertexFormat::tVertexFormat( const tVertexElement* elems, u32 numElems )
		: mVertexSize( 0 )
	{
		fReset( elems, numElems );
	}

	tVertexFormat::tVertexFormat( tNoOpTag )
		: mVertexElements( cNoOpTag )
	{
	}

	b32 tVertexFormat::fSemanticsEqual( const tVertexFormat& other ) const
	{
		if( mVertexSize != other.mVertexSize )
			return false;
		if( mVertexElements.fCount( ) != other.mVertexElements.fCount( ) )
			return false;

		for( u32 i = 0; i < mVertexElements.fCount( ); ++i )
		{
			if( !mVertexElements[ i ].fSemanticsEqual( other.mVertexElements[ i ] ) )
				return false;
		}

		return true;
	}

	b32 tVertexFormat::fFullyEqual( const tVertexFormat& other ) const
	{
		if( mVertexSize != other.mVertexSize )
			return false;
		if( mVertexElements.fCount( ) != other.mVertexElements.fCount( ) )
			return false;

		for( u32 i = 0; i < mVertexElements.fCount( ); ++i )
		{
			if( !mVertexElements[ i ].fFullyEqual( other.mVertexElements[ i ] ) )
				return false;
		}

		return true;
	}

	const tVertexElement* tVertexFormat::fFindBySemantics( const tVertexElement& other ) const
	{
		for( u32 i = 0; i < mVertexElements.fCount( ); ++i )
		{
			if( mVertexElements[ i ].fSemanticsEqual( other ) )
				return &mVertexElements[ i ];
		}

		return 0;
	}

	const tVertexElement* tVertexFormat::fFindByFullEquality( const tVertexElement& other ) const
	{
		for( u32 i = 0; i < mVertexElements.fCount( ); ++i )
		{
			if( mVertexElements[ i ].fFullyEqual( other ) )
				return &mVertexElements[ i ];
		}

		return 0;
	}

	void tVertexFormat::fReset( const tVertexElement* elems, u32 numElems )
	{
		// copy vertex elements, and while we do, 
		// compute element offsets and total vertex size

		mVertexSize = 0;
		mVertexElements.fNewArray( numElems );
		for( u32 i = 0; i < mVertexElements.fCount( ); ++i )
		{
			sigassert( elems[ i ].fValid( ) );
			mVertexElements[ i ] = elems[ i ];
			mVertexElements[ i ].mOffsetFromBase = mVertexSize;
			mVertexSize += mVertexElements[ i ].mSize;
		}
	}

	void tVertexFormat::fCombine( const tVertexFormat& other )
	{
		tGrowableArray<tVertexElement> elems;
		elems.fSetCapacity( mVertexElements.fCount( ) + other.mVertexElements.fCount( ) );

		// first add all my elements
		elems.fInsert( 0, mVertexElements.fBegin( ), mVertexElements.fCount( ) );

		for( u32 iotherElement = 0; iotherElement < other.mVertexElements.fCount( ); ++iotherElement )
		{
			if( !fFindBySemantics( other.mVertexElements[ iotherElement ] ) )
			{
				// i don't have this element, add it
				elems.fPushBack( other.mVertexElements[ iotherElement ] );
			}
		}

		// re-order the elements
			// TODO use the size of each element to try and neatly pack each quadword

		// recreate my vertex element array
		fReset( elems.fBegin( ), elems.fCount( ) );
	}

	void tVertexFormat::fIntersect( const tVertexFormat& other )
	{
		tGrowableArray<tVertexElement> elems;
		elems.fSetCapacity( mVertexElements.fCount( ) + other.mVertexElements.fCount( ) );

		for( u32 i = 0; i < mVertexElements.fCount( ); ++i )
		{
			if( other.fFindBySemantics( mVertexElements[ i ] ) )
			{
				// we both have this element, so add it to new list
				elems.fPushBack( mVertexElements[ i ] );
			}
		}

		// recreate my vertex element array; N.B.! we don't call fReset( ), as that would
		// modify the vertex size and element offsets; in this case, we want to preserve those,
		// so we simply copy the elements directly
		mVertexElements.fNewArray( elems.fCount( ) );
		for( u32 i = 0; i < mVertexElements.fCount( ); ++i )
			mVertexElements[ i ] = elems[ i ];
	}

	void tVertexFormat::fAddSkinningElements( )
	{
		tGrowableArray<tVertexElement> elems;
		elems.fInsert( 0, mVertexElements.fBegin( ), mVertexElements.fCount( ) );

		elems.fPushBack( tVertexElement( tVertexElement::cSemanticBoneWeights, tVertexElement::cFormat_u8_4_Normalized ) );
		elems.fPushBack( tVertexElement( tVertexElement::cSemanticBoneIndices, tVertexElement::cFormat_u8_4 ) );

		// recreate my vertex element array
		fReset( elems.fBegin( ), elems.fCount( ) );
	}

	void tVertexFormat::fRelocateInPlace( ptrdiff_t delta )
	{
		mVertexElements.fRelocateInPlace( delta );
	}

}}
