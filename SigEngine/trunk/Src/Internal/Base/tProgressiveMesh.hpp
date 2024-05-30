//------------------------------------------------------------------------------
// \file tProgressiveMesh.hpp - 18 Jul 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tProgressiveMesh__
#define __tProgressiveMesh__

#include "Gfx/tGeometryBufferSysRam.hpp"
#include "Gfx/tIndexBufferSysRam.hpp"

namespace Sig
{
	///
	/// \class tProgressiveMesh
	/// \brief 
	class base_export tProgressiveMesh
	{
	public:

		struct base_export tProgressiveIndexBuffer
		{
			struct tWindow
			{
				u32 mFirstIndex;
				u32 mNumFaces;
				u32 mNumVerts;
			};

			Gfx::tIndexBufferSysRam mIndexBuffer;
			tGrowableArray< tWindow > mWindows;
		};

	public:

		const Gfx::tGeometryBufferSysRam & fM0Geometry( ) const { return mM0.mVerts; }
		const Gfx::tIndexBufferSysRam & fM0Indices( ) const { return mM0.mIndices; }

		// Applys expansions to meet the specifications and then captures the vb/ib of the built mesh
		void fExpandByRatio( f32 ratio, Gfx::tGeometryBufferSysRam & vb, Gfx::tIndexBufferSysRam & ib ); 
		void fExpandByCount( u32 expandCount, Gfx::tGeometryBufferSysRam & vb, Gfx::tIndexBufferSysRam & ib );

		// Expands the system fully so that vb contains all verts and builds a series
		// of progressive index buffers that can be used for VIPM display. 
		// NOTE!!!: For this algorithm to function the progressive mesh must have been
		//			built with cascade restrictions and the subset policy
		void fBuildProgressiveWindows( 
			Gfx::tGeometryBufferSysRam & vb, 
			tGrowableArray< tProgressiveIndexBuffer > & ibs ) const;

	protected:

		struct tExpansion
		{
			tExpansion( ) 
				: mNewVertDataOffset( ~0 )
				, mUpdateVertIndex( ~0 )
				, mUpdateVertDataOffset( ~0 ) { }

			u32 mNewVertDataOffset;

			u32 mUpdateVertIndex;
			u32 mUpdateVertDataOffset;

			tDynamicArray< u32 > mNewFaces; // triplets of indices
			tDynamicArray< u32 > mIndicesToChange; // indices to change to new vert index
		};

	private:

		void fExpandVertices( u32 expandCount, Gfx::tGeometryBufferSysRam & vb ) const;
		void fExpandIndices( u32 expandCount, Gfx::tIndexBufferSysRam & ib ) const;

	protected:

		struct
		{
			Gfx::tGeometryBufferSysRam mVerts;
			Gfx::tIndexBufferSysRam mIndices;
		} mM0;

		tDynamicBuffer mExpansionVertData;
		tDynamicArray< tExpansion > mExpansions;
		tDynamicArray< u32 > mCascadeMarkers;
		
	};

} // ::Sig

#endif//__tProgressiveMesh__
