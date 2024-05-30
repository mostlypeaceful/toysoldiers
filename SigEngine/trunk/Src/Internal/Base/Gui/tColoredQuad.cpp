#include "BasePch.hpp"
#include "tColoredQuad.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tDevice.hpp"

namespace Sig { namespace Gui
{
	tColoredQuad::tColoredQuad( )
		: mAllocators( Gfx::tDefaultAllocators::fInstance( ) )
	{
		fCommonCtor( );
	}
	tColoredQuad::tColoredQuad( Gfx::tDefaultAllocators& allocators )
		: mAllocators( allocators )
	{
		fCommonCtor( );
	}
	void tColoredQuad::fCommonCtor( )
	{
		mRenderState = Gfx::tRenderState::cDefaultColorTransparent;
		mGeometry.fSetRenderStateOverride( &mRenderState );
		mGeometry.fSetPrimTypeOverride( Gfx::tIndexFormat::cPrimitiveTriangleList );
		fResetDeviceObjects( *Gfx::tDevice::fGetDefaultDevice( ) );
		fSetRect( Math::tRect( ) );
		fMemSet( mVertColors, 0xff, sizeof( mVertColors ) );
	}
	tColoredQuad::~tColoredQuad( )
	{
	}
	void tColoredQuad::fOnDeviceReset( Gfx::tDevice* device )
	{
		fSetRect( fLocalXform( ).mRect );
	}
	void tColoredQuad::fSetRect( const Math::tVec2f& topLeft, const Math::tVec2f& widthHeight )
	{
		fSetRect( Math::tRect( topLeft, widthHeight ) );
	}
	void tColoredQuad::fSetRect( const Math::tVec2f& widthHeight )
	{
		fSetRect( Math::tRect( widthHeight ) );
	}
	void tColoredQuad::fSetRect( const Math::tRect& rect )
	{
		fSetBounds( rect );
		fUpdateGeometry( );
	}
	void tColoredQuad::fSetVertColors( const Math::tVec4f& topLeft, const Math::tVec4f& topRight, const Math::tVec4f& botLeft, const Math::tVec4f& botRight )
	{
		mVertColors[ 0 ] = Gfx::tVertexColor( topLeft ).fForGpu( );
		mVertColors[ 1 ] = Gfx::tVertexColor( topRight ).fForGpu( );
		mVertColors[ 2 ] = Gfx::tVertexColor( botLeft ).fForGpu( );
		mVertColors[ 3 ] = Gfx::tVertexColor( botRight ).fForGpu( );
		fUpdateGeometry( );
	}
	void tColoredQuad::fResetDeviceObjects( Gfx::tDevice& device )
	{
		mMaterial.fReset( mAllocators.mSolidColorMaterial->fDynamicCast< Gfx::tSolidColorMaterial >( ) );

		mGeometry.fResetDeviceObjects( mAllocators.mSolidColorGeomAllocator, mAllocators.mIndexAllocator );
		mGeometry.fChangeMaterial( *mMaterial );

		// generate indices (as these never change)
		const u16 indices[] = { 0, 2, 1, 1, 2, 3 };
		mGeometry.fAllocateIndices( *mMaterial, array_length( indices ), 2 );
		mGeometry.fCopyIndicesToGpu( indices, array_length( indices ) );

		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
		fRegisterWithDevice( &device );
	}
	void tColoredQuad::fUpdateGeometry( )
	{
		const Math::tRectf& rect = fLocalXform( ).mRect;
		const Math::tVec2f topLeft = rect.fTopLeft( );
		const Math::tVec2f widthHeight = rect.fWidthHeight( );

		const u32 numVerts	= 4;

		if( !mGeometry.fAllocateVertices( *mMaterial, numVerts ) )
		{
			fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
			return; // couldn't get geometry
		}

		sigassert( mGeometry.fGetRenderBatch( )->fBatchData( ).mGeometryBuffer->fVertexFormat( ).fVertexSize( ) == sizeof( Gfx::tSolidColorRenderVertex ) );

		// we're using a solid color material
		tFixedArray<Gfx::tSolidColorRenderVertex,4> verts;
		verts[ 0 ].mP = Math::tVec3f( topLeft.x,						topLeft.y,						0.f );
		verts[ 0 ].mColor = mVertColors[ 0 ];
		verts[ 1 ].mP = Math::tVec3f( topLeft.x + widthHeight.x,		topLeft.y,						0.f );
		verts[ 1 ].mColor = mVertColors[ 1 ];
		verts[ 2 ].mP = Math::tVec3f( topLeft.x,						topLeft.y + widthHeight.y,		0.f );
		verts[ 2 ].mColor = mVertColors[ 2 ];
		verts[ 3 ].mP = Math::tVec3f( topLeft.x + widthHeight.x,		topLeft.y + widthHeight.y,		0.f );
		verts[ 3 ].mColor = mVertColors[ 3 ];
		mGeometry.fCopyVertsToGpu( verts.fBegin( ), verts.fCount( ) );

		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
	}
}}

namespace Sig { namespace Gui
{
	void tColoredQuad::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tColoredQuad, tRenderableCanvas, Sqrat::NoCopy<tColoredQuad> > classDesc( vm.fSq( ) );

		classDesc
			.Overload<void (tColoredQuad::*)(const Math::tVec2f&, const Math::tVec2f&)>( "SetRect", &tColoredQuad::fSetRect )
			.Overload<void (tColoredQuad::*)(const Math::tVec2f&)>( "SetRect", &tColoredQuad::fSetRect )
			// Don't enable next line: currently (v.0.8.1) Sqrat can't distiniguish by type, the overloads must have a different number of parameters
					//.Overload<void (tTexturedQuad::*)(const Math::tRect&)>( "SetRect", &fSetRect )
			.Func(_SC("SetVertColors"), &tColoredQuad::fSetVertColors)
			;

		vm.fNamespace(_SC("Gui")).Bind(_SC("ColoredQuad"), classDesc);
	}
}}
