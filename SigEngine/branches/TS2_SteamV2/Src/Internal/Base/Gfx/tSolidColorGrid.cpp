#include "BasePch.hpp"
#include "tSolidColorGrid.hpp"
#include "tSolidColorMaterial.hpp"

namespace Sig { namespace Gfx
{
	tSolidColorGrid::tSolidColorGrid( )
		: mGridLinesPerHalfAxis( 0 )
		, mMinorIncrement( 0 )
		, mMajorIncrement( 0 )
	{
	}

	void tSolidColorGrid::fOnDeviceReset( tDevice* device )
	{
		fGenerate( mGridLinesPerHalfAxis, mMinorIncrement, mMajorIncrement );
	}

	void tSolidColorGrid::fGenerate( u32 gridLinesPerHalfAxis, u32 inc, u32 major, Math::tVec3f xAxis, Math::tVec3f zAxis )
	{
		mGridLinesPerHalfAxis = gridLinesPerHalfAxis;
		mMinorIncrement = inc;
		mMajorIncrement = major;

		const f32 min = -( f32 )gridLinesPerHalfAxis * inc;
		const f32 max = +( f32 )gridLinesPerHalfAxis * inc;

		const u32 numLines = 2 + 2 * 2 * gridLinesPerHalfAxis;
		tDynamicArray< Gfx::tSolidColorRenderVertex > lineVerts( 2 * numLines );

		const u32 red	= Gfx::tVertexColor( 0x99, 0x11, 0x11, 0xff ).fForGpu( );
		//const u32 green = Gfx::tVertexColor( 0x11, 0x99, 0x11, 0xff ).fForGpu( );
		const u32 blue	= Gfx::tVertexColor( 0x22, 0x22, 0xaa, 0xff ).fForGpu( );
		const u32 white = Gfx::tVertexColor( 0x11, 0x11, 0x11, 0xff ).fForGpu( );
		const u32 black = Gfx::tVertexColor( 0x99, 0x99, 0x99, 0xff ).fForGpu( );

		// x-axis
		lineVerts[ 0 ] = Gfx::tSolidColorRenderVertex( xAxis * min, red );
		lineVerts[ 1 ] = Gfx::tSolidColorRenderVertex( xAxis * max, red );

		// z-axis
		lineVerts[ 2 ] = Gfx::tSolidColorRenderVertex( zAxis * min, blue );
		lineVerts[ 3 ] = Gfx::tSolidColorRenderVertex( zAxis * max, blue );

		u32 vertex = 4;
		for( s32 i = 1; i <= ( s32 )gridLinesPerHalfAxis; ++i )
		{
			const u32 color = ( i % major ) == 0 ? black : white;
			const f32 axisXing = ( f32 )i * inc;

			// parallel to x-axis
			lineVerts[ vertex++ ] = Gfx::tSolidColorRenderVertex( xAxis * min + zAxis * -axisXing, color );
			lineVerts[ vertex++ ] = Gfx::tSolidColorRenderVertex( xAxis * max + zAxis * -axisXing, color );
			lineVerts[ vertex++ ] = Gfx::tSolidColorRenderVertex( xAxis * min + zAxis * +axisXing, color );
			lineVerts[ vertex++ ] = Gfx::tSolidColorRenderVertex( xAxis * max + zAxis * +axisXing, color );

			// parallel to z-axis
			
			lineVerts[ vertex++ ] = Gfx::tSolidColorRenderVertex( xAxis * -axisXing + zAxis * min, color );
			lineVerts[ vertex++ ] = Gfx::tSolidColorRenderVertex( xAxis * -axisXing + zAxis * max, color );
			lineVerts[ vertex++ ] = Gfx::tSolidColorRenderVertex( xAxis * +axisXing + zAxis * min, color );
			lineVerts[ vertex++ ] = Gfx::tSolidColorRenderVertex( xAxis * +axisXing + zAxis * max, color );
		}

		sigassert( vertex == lineVerts.fCount( ) );
		fBake( ( Sig::byte* )lineVerts.fBegin( ), lineVerts.fCount( ), false );
	}

}}

