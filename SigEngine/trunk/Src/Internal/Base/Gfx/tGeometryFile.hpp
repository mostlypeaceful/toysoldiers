//------------------------------------------------------------------------------
// \file tGeometryFile.hpp - 10 Aug 2011
// \author jwittner & cbramwell
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tGeometryFile__
#define __tGeometryFile__
#include "tGeometryBufferVRam.hpp"
#include "tIndexBufferVRam.hpp"

namespace Sig { namespace Gfx
{

	//
	/// \class tGeometryFile
	/// \brief 
	class base_export tGeometryFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		declare_lip_version( );
		implement_rtti_serializable_base_class(tGeometryFile, 0xDD7F2370);
	public:
		static const char*		fGetFileExtension( );
		static tFilePathPtr		fConvertToBinary( const tFilePathPtr& path ) { return path; }
		static tFilePathPtr		fConvertToSource( const tFilePathPtr& path ) { return tFilePathPtr( ); }
	public:

		enum tBufferState
		{
			cBufferStateNull = 0,
			cBufferStateLoading,
			cBufferStateReady
		};

		struct base_export tBufferPointer
		{
			declare_reflector( );
			u32						mBufferOffset; ///< offset from the start of the file to the raw shader buffer
			u32						mBufferSize;
			u16						mRefCount;
			u8						mState;
			u8						pad;
		};

		struct base_export tGeometryPointer : public tBufferPointer
		{
			declare_reflector( );
			tGeometryBufferVRam mVRamBuffer;
		};

		struct base_export tIndexListPointer : public tBufferPointer
		{
			declare_reflector( );
			tIndexBufferVRam mVRamBuffer;
		};

		struct base_export tIndexWindow
		{
			declare_reflector( );
			u32 mFirstIndex;
			u16 mNumFaces;
			u16 mNumVerts;
		};

		struct base_export tIndexLod
		{
			declare_reflector( );
			tDynamicArray<tIndexWindow> mWindows;
		};

		struct base_export tIndexLodGroup
		{
			declare_reflector( );
			u16 mPointerStart; ///< The index of the first lod's Pointer
			u16 mTotalWindowCount;
			tDynamicArray<tIndexLod> mLods;
		};

	public:

		u32										mHeaderSize; ///< size of the file up to but not including the geometry buffers
		tDynamicArray<tGeometryPointer>			mGeometryPointers;
		tDynamicArray<tIndexListPointer>		mIndexListPointers;
		tDynamicArray<tIndexLodGroup>			mIndexGroups; ///< Organized info about sub mesh lods

	public:
		tGeometryFile( );
		tGeometryFile( tNoOpTag );
		~tGeometryFile( );

		// LOD Requests
		const tGeometryBufferVRam *	fRequestGeometry( u32 index );
		const tIndexBufferVRam *	fRequestIndices( u32 subMesh, f32 lodRatio, tGeometryFile::tIndexWindow & window );


	private:

		// Select the Lod
		void fSelectLod( u32 subMesh, f32 ratio, u32 & idx, tGeometryFile::tIndexWindow & window );

	public:
		//------------------------------------------------------------------------------
		// tLoadInPlaceFile overloads
		//------------------------------------------------------------------------------
		virtual void fOnFileLoaded( const tResource& ownerResource ) OVERRIDE;
		virtual void fOnFileUnloading( const tResource& ownerResource ) OVERRIDE;

		// Computes the total storage used and builds a display string
		u32 fComputeStorage( std::string & display ) const;
		u32 fLODMemoryInBytes( ) const;

		u32 fVramUsage( u32& geoVRamUsed, u32& geoVRamMax, u32& indiVRamUsed, u32& indiVRamMax ) const;
		u32 fMainUsage( ) const { return mHeaderSize; }

	};

}} // ::Sig::Gfx

#endif//__tGeometryFile__
