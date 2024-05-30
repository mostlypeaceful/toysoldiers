#include "BasePch.hpp"
#include "tLineList.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tDevice.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "Gfx/tMaterial.hpp"
#include "Gfx/tDefaultAllocators.hpp"

namespace Sig { namespace Gui
{
	void tLineList::fSetGeometry( tGrowableArray<Gfx::tSolidColorRenderVertex>& solidColorVerts )
	{
		mSysMemVerts.fSwap( solidColorVerts );
		fSetGeometryInternal( mSysMemVerts.fBegin( ), mSysMemVerts.fCount( ) );
	}
	template<class tIndex>
	void fGenerateLineList( tIndex* ids, u32 numPrims )
	{
		sigassert( sizeof( tIndex ) == sizeof( u16 ) || sizeof( tIndex ) == sizeof( u32 ) );

		for( u32 iline = 0; iline < numPrims; ++iline )
		{
			ids[ iline * 2 + 0 ] = iline * 2 + 0;
			ids[ iline * 2 + 1 ] = iline * 2 + 1;
		}
	}
	void tLineList::fSetGeometryInternal( const Gfx::tSolidColorRenderVertex* solidColorVerts, u32 numVerts )
	{
		const u32 numPrims	= numVerts / 2;
		const u32 numIds	= numVerts;

		mGeometry.fSetRenderStateOverride( &Gfx::tRenderState::cDefaultColorTransparent );
		Gfx::tDefaultAllocators& alloc = Gfx::tDefaultAllocators::fInstance( );

		if( !alloc.mSolidColorMaterial )
			return;

		const Gfx::tMaterialPtr material( alloc.mSolidColorMaterial->fDynamicCast< Gfx::tSolidColorMaterial >( ) );

		mGeometry.fResetDeviceObjects( 
			Gfx::tDevice::fGetDefaultDevice( ), 
			material,
			alloc.mSolidColorGeomAllocator, 
			alloc.mIndexAllocator );

		mGeometry.fSetPrimTypeOverride( Gfx::tIndexFormat::cPrimitiveLineList );

		if( !mGeometry.fAllocateGeometry( *mGeometry.fMaterial( ), numVerts, numIds, numPrims ) )
		{
			fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
			return; // couldn't get geometry
		}

		// copy vert data to gpu
		mGeometry.fCopyVertsToGpu( ( Sig::byte* )solidColorVerts, numVerts );

		// generate indices
		if( mGeometry.fIndexFormat( ).mStorageType == Gfx::tIndexFormat::cStorageU16 )
		{
			tDynamicArray<u16> ids( numIds );
			fGenerateLineList( ids.fBegin( ), numPrims );
			mGeometry.fCopyIndicesToGpu( ids.fBegin( ), numIds );
		}
		else
		{
			tDynamicArray<u32> ids( numIds );
			fGenerateLineList( ids.fBegin( ), numPrims );
			mGeometry.fCopyIndicesToGpu( ids.fBegin( ), numIds );
		}

		tRenderableCanvas::fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
	}

	namespace
	{
		void fCircle( tLineList* list, const Math::tVec3f& xyzCenter, f32 radius, const Math::tVec4f& rgba )
		{
			tGrowableArray<Gfx::tSolidColorRenderVertex> verts;

			u32 color = Gfx::tVertexColor( rgba ).fForGpu( );
			const u32 steps = 20;
			const f32 delta = Math::c2Pi / steps;

			for( u32 i = 0; i < steps; ++i )
			{
				f32 angle1 = delta * i;
				f32 angle2 = delta * (i+1);
				verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f( cos( angle1 ) * radius, sin( angle1 ) * radius, 0.0f ) + xyzCenter, color ) );
				verts.fPushBack( Gfx::tSolidColorRenderVertex( Math::tVec3f( cos( angle2 ) * radius, sin( angle2 ) * radius, 0.0f ) + xyzCenter, color ) );
			}

			list->fSetGeometry( verts );
		}

		void fLine( tLineList* list, const Math::tVec3f& p1, const Math::tVec3f& p2, const Math::tVec4f& rgba )
		{
			tGrowableArray<Gfx::tSolidColorRenderVertex> verts;

			u32 color = Gfx::tVertexColor( rgba ).fForGpu( );
			verts.fPushBack( Gfx::tSolidColorRenderVertex( p1, color ) );
			verts.fPushBack( Gfx::tSolidColorRenderVertex( p2, color ) );

			list->fSetGeometry( verts );	
		}
	}

	void tLineList::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tLineList, tRenderableCanvas, Sqrat::NoCopy<tLineList> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Circle"),	&fCircle)
			.GlobalFunc(_SC("Line"),	&fLine)
			;
		vm.fNamespace( "Gui" ).Bind( _SC("LineList"), classDesc );
	}
}}//Sig::Gui

