#ifndef __tVertexFormat__
#define __tVertexFormat__

namespace Sig { namespace Gfx
{
	struct base_export tVertexColor
	{
		union
		{
			struct { u8 mR, mG, mB, mA; };
			struct { u32 mAsU32; };
		};

		inline tVertexColor( )
		{
		}

		inline explicit tVertexColor( u32 colorAsU32 )
			: mAsU32( colorAsU32 )
		{
		}

		inline tVertexColor( int r, int g, int b, int a = 0xff )
			: mR( ( u8 )fClamp( r, 0x00, 0xff ) )
			, mG( ( u8 )fClamp( g, 0x00, 0xff ) )
			, mB( ( u8 )fClamp( b, 0x00, 0xff ) )
			, mA( ( u8 )fClamp( a, 0x00, 0xff ) )
		{
		}

		inline tVertexColor( f32 r, f32 g, f32 b, f32 a = 1.0f )
			: mR( ( u8 )fClamp( r * 255.f, 0.f, 255.f ) )
			, mG( ( u8 )fClamp( g * 255.f, 0.f, 255.f ) )
			, mB( ( u8 )fClamp( b * 255.f, 0.f, 255.f ) )
			, mA( ( u8 )fClamp( a * 255.f, 0.f, 255.f ) )
		{
		}

		inline tVertexColor( const Math::tVec4f& rgba )
			: mR( ( u8 )fClamp( rgba.x * 255.f, 0.f, 255.f ) )
			, mG( ( u8 )fClamp( rgba.y * 255.f, 0.f, 255.f ) )
			, mB( ( u8 )fClamp( rgba.z * 255.f, 0.f, 255.f ) )
			, mA( ( u8 )fClamp( rgba.w * 255.f, 0.f, 255.f ) )
		{
		}

		u32 fForGpu( tPlatformId pid = cCurrentPlatform ) const;
	};


	///
	/// \brief Describes an individual element in a vertex (i.e., position, or uvs, etc.).
	class base_export tVertexElement
	{
		declare_reflector( );
	public:

		enum tSemantic
		{
			cSemanticPosition,
			cSemanticNormal,
			cSemanticTangent,
			cSemanticBinormal,
			cSemanticTexCoord,
			cSemanticColor,
			cSemanticBoneWeights,
			cSemanticBoneIndices,

			// last
			cSemanticCount,
			cSemanticInvalid = cSemanticCount
		};

		enum tFormat
		{
			cFormat_f32_1,
			cFormat_f32_2,
			cFormat_f32_3,
			cFormat_f32_4,
			cFormat_f16_2,
			cFormat_f16_4,
			cFormat_u8_4,
			cFormat_u8_4_Color,
			cFormat_u8_4_Normalized,

			// last
			cFormatCount,
			cFormatInvalid = cFormatCount
		};

		tEnum<tSemantic,u8>		mSemantic;
		tEnum<tFormat,u8>		mFormat;
		u8						mSemanticIndex; // i.e., if you have multiple tex coords
		u8						mStreamIndex; // used to specify that the vertex element comes from a separate stream (i.e., for instanced rendering)
		u16						mOffsetFromBase; // this gets filled in by tVertexFormat's ctor
		u16						mSize; // this gets filled in by tVertexElement's ctor; if you don't use the ctor use fSizeFromFormat

		///
		/// \brief Query for the size in bytes of an element by format type.
		static u16 fSizeFromFormat( tFormat format );

		inline tVertexElement( )
			: mSemantic( cSemanticInvalid )
			, mSemanticIndex( 0 )
			, mStreamIndex( 0 )
			, mFormat( cFormatInvalid )
			, mOffsetFromBase( 0 )
			, mSize( 0 )
		{
		}

		inline tVertexElement( tSemantic semantic, tFormat format, u8 semanticIndex=0, u8 streamIndex=0 )
			: mSemantic( semantic )
			, mFormat( format )
			, mSemanticIndex( semanticIndex )
			, mStreamIndex( streamIndex )
			, mOffsetFromBase( 0 )
			, mSize( fSizeFromFormat( format ) )
		{
		}

		///
		/// \brief See if a vertex element is valid (tests the semantic and format fields).
		inline b32 fValid( ) const { return mSemantic != cSemanticInvalid && mFormat != cFormatInvalid; }

		///
		/// \brief Compare two vertex elements by semantics only.
		/// \return true if the two elements have equivalent semantics.
		inline b32 fSemanticsEqual( const tVertexElement& other ) const { return mSemantic==other.mSemantic && mSemanticIndex ==other.mSemanticIndex; }

		///
		/// \brief Compare two vertex elements by full equality (i.e., compares each variable of each vertex element).
		/// \return true if the two elements are completely equivalent in all respects.
		inline b32 fFullyEqual( const tVertexElement& other ) const { return fMemCmp( this, &other, sizeof( *this ) ) == 0; }
	};


	///
	/// \brief Describes the format of an individual vertex in a geometry buffer.
	class base_export tVertexFormat
	{
		declare_reflector( );
	public:
		static const u32 cStreamIndex_Instanced = 1;
	private:
		tDynamicArray<tVertexElement> mVertexElements;
	public:
		inline tVertexFormat( ) { }
		tVertexFormat( tNoOpTag );
		tVertexFormat( const tVertexElement* elems, u32 numElems );

		u32 fVertexSize( u32 stream=0 ) const;
		inline u32 fElementCount( ) const { return mVertexElements.fCount( ); }
		inline const tVertexElement& operator[]( u32 ithElement ) const { return mVertexElements[ ithElement ]; }
		inline const tVertexElement* fBegin( ) const { return mVertexElements.fBegin( ); }
		inline const tVertexElement* fEnd( ) const { return mVertexElements.fEnd( ); }

		///
		/// \brief Test if two vertex formats are equivalent based on the semantics of the underlying vertex elements.
		b32 fSemanticsEqual( const tVertexFormat& other ) const;

		///
		/// \brief Test if two vertex formats are FULLY equivalent, as in each vertex element is identical.
		b32 fFullyEqual( const tVertexFormat& other ) const;

		///
		/// \brief Find an element in this vertex format with equivalent semantics to 'other'.
		/// \return 0 if not found, otherwise a pointer to an element contained in 'this'.
		const tVertexElement* fFindBySemantics( const tVertexElement& other ) const;

		///
		/// \brief Find an element in this vertex format that is completely equivalent to 'other'.
		/// \return 0 if not found, otherwise a pointer to an element contained in 'this'.
		const tVertexElement* fFindByFullEquality( const tVertexElement& other ) const;

		///
		/// \brief Reset the current format with a new set of vertex elements.
		void fReset( const tVertexElement* elems, u32 numElems );

		///
		/// \brief Consider this method as the union of 'this' and 'other'. It will
		/// also do little things like re-order the elements to keep memory packing happy.
		void fCombine( const tVertexFormat& other );

		///
		/// \brief Will remove any elements in 'this' that aren't in 'other'.
		void fIntersect( const tVertexFormat& other );

		///
		/// \brief Will add required elements for skinning.
		void fAddSkinningElements( );

		///
		/// \brief Relocate the vertex format following a load-in-place operation. If you don't know what
		/// this means, then you shouldn't be using it.
		void fRelocateInPlace( ptrdiff_t delta );
	};

	///
	/// \brief Class for aiding in safely indexing/iterating through a raw geometry buffer. Acts as a
	/// "view" or adaptor on top of the geometry buffer that allows you to treat the geometry buffer
	/// as an array of a single element (i.e., position, normal, uv, whatever).
	template<class t>
	class tVertexElementIterator
	{
	private:
		mutable Sig::byte*			mCurElem;
		Sig::byte*					mVertsEnd;
		const tVertexElement*		mVertexElement;
		u32							mVertexSize;

	public:
		tVertexElementIterator( )
			: mCurElem( 0 ), mVertsEnd( 0 ), mVertexElement( 0 ), mVertexSize( 0 )
		{
		}

		tVertexElementIterator( Sig::byte* vertsBase, Sig::byte* vertsEnd, const tVertexElement* vertexElem, u32 vertexSize )
			: mCurElem( vertsBase ), mVertsEnd( vertsEnd ), mVertexElement( vertexElem ), mVertexSize( vertexSize )
		{
			sigassert( mCurElem && mVertsEnd && mVertexElement && mVertexSize > 0 );
			sigassert( sizeof( t ) == mVertexElement->mSize );

			// advance the base pointer to the start of the element
			mCurElem += mVertexElement->mOffsetFromBase;
		}

		inline b32						fNull( )						{ return !mCurElem || mCurElem >= mVertsEnd; }
		inline void						fAdvance( u32 numElems )		{ mCurElem += numElems * mVertexSize; }
		inline void						fReverse( u32 numElems )		{ mCurElem -= numElems * mVertexSize; }
		inline tVertexElementIterator&	operator++( )					{ fAdvance( 1 ); return *this; }
		inline tVertexElementIterator	operator++( int )				{ tVertexElementIterator copy = *this; operator++( ); return copy; }
		inline tVertexElementIterator&	operator--( )					{ fReverse( 1 ); return *this; }
		inline tVertexElementIterator	operator--( int )				{ tVertexElementIterator copy = *this; operator--( ); return copy; }
		inline t&						operator[]( u32 i )				{ return *reinterpret_cast<t*>( mCurElem + i * mVertexSize ); }
		inline const t&					operator[]( u32 i ) const		{ return *reinterpret_cast<const t*>( mCurElem + i * mVertexSize ); }
		inline t&						fAsElement( )					{ return *reinterpret_cast<t*>( mCurElem ); }
		inline const t&					fAsElement( ) const				{ return *reinterpret_cast<const t*>( mCurElem ); }
	};


}}


#endif//__tVertexFormat__
