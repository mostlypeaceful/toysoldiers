#ifndef __tGeometryFile__
#define __tGeometryFile__
#include "tGeometryBufferVRam.hpp"
#include "tIndexBufferVRam.hpp"

namespace Sig { namespace Gfx
{

	class base_export tGeometryFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		implement_rtti_serializable_base_class(tGeometryFile, 0xDD7F2370);
	public:
		static const u32		cVersion;
		static const char*		fGetFileExtension( );
	public:

		struct base_export tBufferPointer
		{
			declare_reflector( );
			u32						mBufferOffset; ///< offset from the start of the file to the raw shader buffer
			u32						mBufferSize;
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

		u32										mHeaderSize; ///< size of the file up to but not including the geometry buffers
		b32										mDiscardSysRamBuffers;
		tDynamicArray<tGeometryPointer>			mGeometryPointers;
		tDynamicArray<tIndexListPointer>		mIndexListPointers;

	public:
		tGeometryFile( );
		tGeometryFile( tNoOpTag );
		~tGeometryFile( );

		virtual void fOnFileLoaded( const tResource& ownerResource );
		virtual void fResizeAfterLoad( tGenericBuffer* fileBuffer );
		virtual void fOnFileUnloading( );

		s32 fComputeStorage( ) const;
		f32 fComputeStorageInMB( ) const;
	};

}}

namespace Sig
{
	template< >
	class tResourceLoadBehavior<Gfx::tGeometryFile>
	{
	public:
		static b32 fWillResizeAfterLoad( ) { return true; }
	};
}

#endif//__tGeometryFile__
