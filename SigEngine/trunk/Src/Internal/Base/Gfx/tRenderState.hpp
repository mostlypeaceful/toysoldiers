#ifndef __tRenderState__
#define __tRenderState__

namespace Sig { namespace Gfx
{
	class tDevicePtr;
	class tRenderContext;

	class base_export tRenderState
	{
		declare_reflector( );

	public:

		enum tGlobalFillMode
		{
			cGlobalFillDefault,
			cGlobalFillSmooth,
			cGlobalFillWire,
			cGlobalFillEdgedFace,
			cGlobalFillModeCount
		};

		enum tRenderPassMode
		{
			cRenderPassLighting,
			cRenderPassShadowMap,
			cRenderPassDepth,
			cRenderPassXparentDepthPrepass,

			cRenderPassGBuffer,
			cRenderPassGLighting
		};

		enum tFlags
		{
			cPolyFlipped			= ( 1 << 0 ),
			cPolyTwoSided			= ( 1 << 1 ),
			cAlphaBlend				= ( 1 << 2 ),
			cCutOut					= ( 1 << 3 ),
			cColorBuffer			= ( 1 << 4 ), // The game shouldn't need to control this.
			cDepthBuffer			= ( 1 << 5 ),
			cDepthWrite				= ( 1 << 6 ),
			cStencilBuffer			= ( 1 << 7 ), // The game shouldn't need to control this.
			cFillWireFrame			= ( 1 << 8 ),
			cDepthModeGreater		= ( 1 << 9 ),
			cWriteProperAlpha		= ( 1 << 10 ),

			cBlendOpSubtract		= ( 1 << 28 ),
			cBlendOpRevSubtract		= ( 1 << 29 ),
			cBlendOpMin				= ( 1 << 30 ),
			cBlendOpMax				= ( 1 << 31 ),
			cBlendOpMask			= ( cBlendOpSubtract | cBlendOpRevSubtract | cBlendOpMin | cBlendOpMax ),
		};

		enum tBlendMode
		{
			cBlendOne,
			cBlendZero,
			cBlendSrcAlpha,
			cBlendOneMinusSrcAlpha,
			cBlendSrcColor,
			cBlendOneMinusSrcColor,
			cBlendDstAlpha,
			cBlendOneMinusDstAlpha,
			cBlendDstColor,
			cBlendOneMinusDstColor,

			// last
			cBlendModeCount,
		};

	private:

		u32		mFlags;
		u8		mCutOutThreshold;
		u8		mSrcDstBlend;
		s8		mDepthBias;
		s8		mSlopeScaleBias;

	public:

		static const tRenderState cDefaultColorOpaque;
		static const tRenderState cDefaultColorTransparent;
		static const tRenderState cDefaultColorCutOut;
		static const tRenderState cDefaultColorAdditive;
		static b32 gInvertedViewportDepth;
		static b32 gConsiderCutoutXParent;
		static b32 gWriteProperAlphaWhenWritingCutOut;

		static void fEnableDisableColorWrites( const tDevicePtr& device, b32 enable );

		inline tRenderState( ) { fMemSet( *this, 0xff ); }
		inline tRenderState( tNoOpTag ) { }
		explicit tRenderState( u32 flags, tBlendMode srcBlend = cBlendOne, tBlendMode dstBlend = cBlendZero, u8 cutOut = 0 );

		inline void 		fEnableDisable( u32 flags, b32 enable )	{ mFlags = ( enable ? ( mFlags | flags ) : ( mFlags & ~flags ) ); }
		inline b32  		fQuery( u32 flags )	const				{ return ( mFlags & flags ) ? true : false; }

		void				fSetBlendOpFromIndex( u32 index );
		u32					fGetBlendOpAsIndex( ) const;
		inline u32			fGetBlendOpFlags( ) const				{ return mFlags & cBlendOpMask; }

		inline void 		fSetCutOutThreshold( u8 thresh )		{ mCutOutThreshold = thresh; }
		inline u8			fGetCutOutThreshold( ) const			{ return mCutOutThreshold; }

		inline void			fSetSrcBlendMode( tBlendMode mode )		{ mSrcDstBlend = fMakeByte( ( u8 )mode, ( u8 )fGetDstBlendMode( ) ); }
		inline tBlendMode	fGetSrcBlendMode( ) const				{ return ( tBlendMode )fLowNibble( mSrcDstBlend ); }

		inline void			fSetDstBlendMode( tBlendMode mode )		{ mSrcDstBlend = fMakeByte( ( u8 )fGetSrcBlendMode( ), ( u8 )mode ); }
		inline tBlendMode	fGetDstBlendMode( ) const				{ return ( tBlendMode )fHighNibble( mSrcDstBlend ); }

		inline void			fSetDepthBias( s8 bias )				{ mDepthBias = bias; }
		inline s8			fGetDepthBias( ) const					{ return mDepthBias; }

		inline void			fSetSlopeScaleBias( s8 bias )			{ mSlopeScaleBias = bias; }
		inline s8			fGetSlopeScaleBias( ) const				{ return mSlopeScaleBias; }

		inline b32			fHasTransparency( ) const				{ return (fQuery( cAlphaBlend ) && fGetDstBlendMode( ) != cBlendZero) || (gConsiderCutoutXParent && fQuery( cCutOut )); }

		///
		/// \brief Compare two render states
		inline b32			operator==( const tRenderState& other ) const { return fMemCmp( this, &other, sizeof( *this ) ) == 0; }
		inline b32			operator!=( const tRenderState& other ) const { return !operator==( other ); }

		///
		/// \brief This should only be called immediately prior to rendering geometry.
		void fApply( const tDevicePtr& device, const tRenderContext& context ) const;

		

	private:

		void fApplyCopy( const tDevicePtr& device, const tRenderContext& context ) const;
		void fApplyInternal( const tDevicePtr& device, const tRenderContext& context ) const;
	};

}}

#endif//__tRenderState__
