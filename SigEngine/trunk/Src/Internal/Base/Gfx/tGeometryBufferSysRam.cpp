#include "BasePch.hpp"
#include "tGeometryBufferSysRam.hpp"
#include "EndianUtil.hpp"
#include "tPlatform.hpp"

namespace Sig { namespace Gfx
{

	namespace
	{
		///
		/// \brief Provides endian-swapping for arbitrary geometry buffers.
		class tGeometryEndianSwapper : public Gfx::tGeometryBufferSysRam::tForEachVertexElementConst
		{
			tPlatformId mTargetPlatform;
			const Sig::byte* mBase;
			tDynamicBuffer& mBuffer;
		public:
			tGeometryEndianSwapper( tPlatformId targetPlatform, const Sig::byte* base, tDynamicBuffer& buffer )
				: mTargetPlatform( targetPlatform ), mBase( base ), mBuffer( buffer ) { }

			virtual void operator( )( u32 ivtx, const Sig::byte* vertexElem, const Gfx::tVertexElement& vertexElemDesc );
		private:
			void fSwapColor( Sig::byte* myVertexElem );
		};

		void tGeometryEndianSwapper::operator( )( u32 ivtx, const Sig::byte* vertexElem, const Gfx::tVertexElement& vertexElemDesc )
		{
			const ptrdiff_t offset = vertexElem - mBase;
			Sig::byte* myVertexElem = mBuffer.fBegin( ) + offset;

			switch( vertexElemDesc.mFormat )
			{
			case tVertexElement::cFormat_f32_1: EndianUtil::fSwap32( myVertexElem, 1 ); break;
			case tVertexElement::cFormat_f32_2: EndianUtil::fSwap32( myVertexElem, 2 ); break;
			case tVertexElement::cFormat_f32_3: EndianUtil::fSwap32( myVertexElem, 3 ); break;
			case tVertexElement::cFormat_f32_4: EndianUtil::fSwap32( myVertexElem, 4 ); break;

			case tVertexElement::cFormat_f16_2: EndianUtil::fSwap16( myVertexElem, 2 ); break;
			case tVertexElement::cFormat_f16_4: EndianUtil::fSwap16( myVertexElem, 4 ); break;

			case tVertexElement::cFormat_u8_4:				EndianUtil::fSwap8( myVertexElem, 4 ); break;
			case tVertexElement::cFormat_u8_4_Color:		fSwapColor( myVertexElem ); break;
			case tVertexElement::cFormat_u8_4_Normalized:	EndianUtil::fSwap8( myVertexElem, 4 ); break;

			default: sigassert( !"invalid vertex format in tGeometryEndianSwapper::operator( )!" ); break;
			}
		}
		void tGeometryEndianSwapper::fSwapColor( Sig::byte* myVertexElem )
		{
			// color elements require custom swapping, as they're handled a little strangely

			// copy into temp src buffer
			tFixedArray<u8,4> src;
			fMemCpy( src.fBegin( ), myVertexElem, sizeof( src ) );

			// re-arrange src buffer by platform
			switch( mTargetPlatform )
			{
			case cPlatformWii:			break;
			case cPlatformPcDx9:		break;
			case cPlatformPcDx10:		break;
			case cPlatformXbox360:		fSwap( src[0], src[3] ); fSwap( src[1], src[2] ); break;
			case cPlatformPs3Ppu:		break;
			default: sigassert( !"invalid platform in tGeometryEndianSwapper::fSwapColor" ); break;
			}

			// copy back to vertex element
			fMemCpy( myVertexElem, src.fBegin( ), sizeof( src ) );
		}
	}



	void tGeometryBufferSysRam::fAllocate( const tVertexFormat& format, u32 numVerts )
	{
		sigassert( numVerts > 0 );

		mFormat = format;
		mBuffer.fNewArray( numVerts * mFormat.fVertexSize( ) );
	}

	void tGeometryBufferSysRam::fDeallocate( )
	{
		mFormat = tVertexFormat( );
		mBuffer.fDeleteArray( );
	}

	void tGeometryBufferSysRam::fRemoveDuplicates( tDynamicArray< u32 >& reorderedVertexIndices )
	{
		const u32 oldVtxCount = fVertexCount( );

		reorderedVertexIndices.fNewArray( oldVtxCount );

		// create and reserve overly-much data for new buffer (it is guaranteed to be same size or smaller)
		tGrowableArray< Sig::byte > newBuffer;
		newBuffer.fSetCapacity( mBuffer.fCount( ) );

		const u32 vtxSize = fVertexSize( );

		// go through every vertex in current buffer
		for( u32 iold = 0; iold < oldVtxCount; ++iold )
		{
			// look for this vertex in new buffer
			s32 match = -1;
			const u32 newVtxCount = newBuffer.fCount( ) / vtxSize;
			for( u32 inew = 0; inew < newVtxCount; ++inew )
			{
				if( fMemCmp( &mBuffer[ iold * vtxSize ], &newBuffer[ inew * vtxSize ], vtxSize ) == 0 )
				{
					// found a match
					match = inew;
					break;
				}
			}

			if( match < 0 )
			{
				// vertex wasn't yet placed in new buffer, add it
				const u32 addAt = newBuffer.fCount( );
				match = addAt / vtxSize;
				newBuffer.fGrowCount( vtxSize );
				fMemCpy( &newBuffer[ addAt ], &mBuffer[ iold * vtxSize ], vtxSize );
			}

			sigassert( match >= 0 );
			reorderedVertexIndices[ iold ] = ( u32 )match;
		}

		// now replace my buffer
		mBuffer.fNewArray( newBuffer.fCount( ) );
		fMemCpy( mBuffer.fBegin( ), newBuffer.fBegin( ), newBuffer.fCount( ) );
	}

	void tGeometryBufferSysRam::fEndianSwapBuffer( tDynamicBuffer& buffer, tPlatformId targetPlatform ) const
	{
		if( !fPlatformNeedsEndianSwap( cCurrentPlatform, targetPlatform ) )
			return;

		tGeometryEndianSwapper endianSwapper( targetPlatform, fBegin( ), buffer );
		fForEachVertexElement( endianSwapper );
	}

	void tGeometryBufferSysRam::fForEachVertexElement( tForEachVertexElement& forEach )
	{
		// iterate over all verts
		Sig::byte* verts	= fBegin( );
		Sig::byte* vertsEnd = fEnd( );
		for(	u32 ithVertex = 0; 
				verts < vertsEnd; 
				verts += mFormat.fVertexSize( ), ++ithVertex )
		{
			// iterate over each element in the current vertex
			for( const tVertexElement*	vertexElemDesc	= mFormat.fBegin( );
										vertexElemDesc != mFormat.fEnd( );
										++vertexElemDesc )
			{
				forEach( ithVertex, verts + vertexElemDesc->mOffsetFromBase, *vertexElemDesc );
			}
		}
	}

	void tGeometryBufferSysRam::fForEachVertexElement( tForEachVertexElementConst& forEach ) const
	{
		// iterate over all verts
		const Sig::byte* verts	= fBegin( );
		const Sig::byte* vertsEnd = fEnd( );
		for(	u32 ithVertex = 0; 
				verts < vertsEnd; 
				verts += mFormat.fVertexSize( ), ++ithVertex )
		{
			// iterate over each element in the current vertex
			for( const tVertexElement*	vertexElemDesc	= mFormat.fBegin( );
										vertexElemDesc != mFormat.fEnd( );
										++vertexElemDesc )
			{
				forEach( ithVertex, verts + vertexElemDesc->mOffsetFromBase, *vertexElemDesc );
			}
		}
	}

	void tGeometryBufferSysRam::fForEachVertexElement( const tVertexElement& vertexElemDesc, tForEachVertexElement& forEach )
	{
		fValidateElement( vertexElemDesc );

		// iterate over the specific element in each vertex
		Sig::byte* verts	= fBegin( );
		Sig::byte* vertsEnd = fEnd( );
		for(	u32 ithVertex = 0; 
				verts < vertsEnd; 
				verts += mFormat.fVertexSize( ), ++ithVertex )
		{
			forEach( ithVertex, verts + vertexElemDesc.mOffsetFromBase, vertexElemDesc );
		}
	}

	void tGeometryBufferSysRam::fForEachVertexElement( const tVertexElement& vertexElemDesc, tForEachVertexElementConst& forEach ) const
	{
		fValidateElement( vertexElemDesc );

		// iterate over the specific element in each vertex
		const Sig::byte* verts	= fBegin( );
		const Sig::byte* vertsEnd = fEnd( );
		for(	u32 ithVertex = 0; 
				verts < vertsEnd; 
				verts += mFormat.fVertexSize( ), ++ithVertex )
		{
			forEach( ithVertex, verts + vertexElemDesc.mOffsetFromBase, vertexElemDesc );
		}
	}

	b32 tGeometryBufferSysRam::fValidateElement( const tVertexElement& elem, b32 assertIfNotValid ) const
	{
		// first validate that the vertex element descriptor is contained in the current format
		b32 foundElemDesc = false;
		for( const tVertexElement*	find	 = mFormat.fBegin( );
									find	!= mFormat.fEnd( );
									++find )
		{
			if( find == &elem )
			{
				foundElemDesc = true;
				break;
			}
		}

		if( assertIfNotValid && !foundElemDesc )
		{
			sigassert( !"You must pass a vertex element descriptor that is contained in the current format." );
		}

		return foundElemDesc;
	}

}}


