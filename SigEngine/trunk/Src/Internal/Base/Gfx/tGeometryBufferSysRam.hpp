#ifndef __tGeometryBufferSysRam__
#define __tGeometryBufferSysRam__
#include "tVertexFormat.hpp"

namespace Sig { namespace Gfx
{

	///
	/// \brief Encapsulates a buffer of vertices in a format specified by a tVertexFormat object.
	/// These vertices are stored in system ram, but should be in a gpu-friendly format. Use
	/// tGeometryBufferVRam to convert/transfer to the gpu.
	class base_export tGeometryBufferSysRam
	{
		tVertexFormat			mFormat;
		tDynamicBuffer			mBuffer;
	public:

		///
		/// \brief Base functor type for iterating over vertices.
		class base_export tForEachVertexElement
		{
		public:
			virtual ~tForEachVertexElement( ) { }
			virtual void operator( )( u32 ithVertex, Sig::byte* vertexElem, const tVertexElement& vertexElemDesc ) = 0;
		};

		///
		/// \brief Base functor type for iterating over vertices. This version provides const-only access.
		class base_export tForEachVertexElementConst
		{
		public:
			virtual ~tForEachVertexElementConst( ) { }
			virtual void operator( )( u32 ithVertex, const Sig::byte* vertexElem, const tVertexElement& vertexElemDesc ) = 0;
		};

	public:
		void fAllocate( const tVertexFormat& format, u32 numVerts );
		void fDeallocate( );

		///
		/// \brief Removes and consolidates all duplicate vertices. Returns an array whose
		/// length will be equal to the number of vertices in the buffer prior to the call
		/// to fRemoveDuplicates. Each entry in the array will contain an index into the new
		/// shrunken buffer corresponding to the location of the old vertex in the new buffer.
		void fRemoveDuplicates( tDynamicArray< u32 >& reorderedVertexIndices );

		inline b32						fAllocated( ) const		{ return mFormat.fVertexSize( ) > 0 && mBuffer.fCount( ) > 0; }
		inline const tVertexFormat&		fVertexFormat( ) const	{ return mFormat; }
		inline u32						fVertexSize( ) const	{ return mFormat.fVertexSize( ); }
		inline u32						fSizeInBytes( ) const	{ return mBuffer.fCount( ); }
		inline u32						fVertexCount( ) const	{ return mBuffer.fCount( ) / mFormat.fVertexSize( ); }
		inline Sig::byte*				fBegin( )				{ return mBuffer.fBegin( ); }
		inline const Sig::byte* 		fBegin( ) const			{ return mBuffer.fBegin( ); }
		inline Sig::byte*				fEnd( )					{ return mBuffer.fEnd( ); }
		inline const Sig::byte* 		fEnd( ) const			{ return mBuffer.fEnd( ); }
		inline const tDynamicBuffer&	fGetBuffer( ) const		{ return mBuffer; }

		///
		/// \brief Endian-swap a buffer that was acquired via fGetBuffer( ). I.e., this buffer
		/// must be identical to the underlying buffer in 'this'.
		void							fEndianSwapBuffer( tDynamicBuffer& buffer, tPlatformId targetPlatform ) const;

		inline Sig::byte*				fGetVertexBase( u32 ithVertex ) { return mBuffer.fBegin( ) + ithVertex * mFormat.fVertexSize( ); }
		inline const Sig::byte*			fGetVertexBase( u32 ithVertex ) const { return mBuffer.fBegin( ) + ithVertex * mFormat.fVertexSize( ); }

		void fCopyVertex( const tGeometryBufferSysRam& src, u32 srcIndex, u32 dstIndex ) { memcpy( fGetVertexBase( dstIndex ), src.fGetVertexBase( srcIndex ), fVertexSize( ) ); }

		///
		/// \brief Access and "safely" cast the ith vertex's element (i.e., position, normal, etc) to a vertex element iterator.
		/// You can then safely use the iterator to iterate/index, treating the geometry buffer as though it were a simple
		/// array of the specified vertex element (i.e., a virtualized array of positions, or normals, etc).
		template<class t>
		tVertexElementIterator<t>		fGetVertexElementIterator( const tVertexElement& vertexElem, u32 ithVertex = 0 )
		{
			fValidateElement( vertexElem );
			return tVertexElementIterator<t>( fGetVertexBase( ithVertex ), fEnd( ), &vertexElem, fVertexSize( ) );
		}

		///
		/// \brief This method will iterate over all vertices, and then, for each vertex, all the elements in that vertex,
		/// calling the callback at each vertex element. If your vertex contained a position, normal, and uv, your callback
		/// would first fire for the position, normal, and uv of the first vertex, then the 2nd vertex, etc.
		void fForEachVertexElement( tForEachVertexElement& forEach );
		void fForEachVertexElement( tForEachVertexElementConst& forEach ) const;

		///
		/// \brief This version iterates ONLY over the specific vertex element; i.e., this would be equivalent
		/// to stepping through all the vertex positions; it will skip over all other elements.
		/// \note This method will sigassert at runtime if the vertexElemDesc is not found in the currently bound format.
		void fForEachVertexElement( const tVertexElement& vertexElemDesc, tForEachVertexElement& forEach );
		void fForEachVertexElement( const tVertexElement& vertexElemDesc, tForEachVertexElementConst& forEach ) const;

	private:

		///
		/// \brief Validates that the specified element is contained in the current format.
		b32 fValidateElement( const tVertexElement& elem, b32 assertIfNotValid=true ) const;
	};


}}


#endif//__tGeometryBufferSysRam__

