#ifndef __tFxFile__
#define __tFxFile__
#include "tEntityDef.hpp"

namespace Sig { namespace FX
{

	class base_export tFxFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		declare_lip_version( );
		implement_rtti_serializable_base_class(tFxFile, 0x0D00A932);

	public:
		typedef tDynamicArray< tLoadInPlacePtrWrapper< tEntityDef > > tObjectArray;
		tObjectArray mObjects;
		f32 mLifetime;
		u32 mFlags; // Currently, these are not passed on to the tFxFileRefEntity. They are used once.
		Math::tAabbf mBounds;

	public:
		static u32			fGetNumBinaryExtensions( );
		static const char*	fGetBinaryExtension( u32 i = 0 );
		static b32			fIsBinary( const tFilePathPtr& path );

		static u32			fGetNumFileExtensions( );
		static const char*	fGetFileExtension( u32 i = 0 );
		static b32			fIsFile( const tFilePathPtr& path );

		static tFilePathPtr fFxmlPathToFxb( const tFilePathPtr& path );
		static tFilePathPtr fFxbPathToFxml( const tFilePathPtr& path );

		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathML2B( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathB2ML( path ); }

	public:
		tFxFile( );
		tFxFile( tNoOpTag );
		~tFxFile( );

		virtual void fOnFileLoaded( const tResource& ownerResource ) OVERRIDE;
		virtual void fOnSubResourcesLoaded( const tResource& ownerResource, b32 success ) OVERRIDE;
		virtual void fOnFileUnloading( const tResource& ownerResource ) OVERRIDE;

		void fCollectEntities( const tCollectEntitiesParams& params ) const;

		f32 fLifetime( ) const { return mLifetime; }
	};

}} // ::Sig::Fx

#endif	// __tFxFile__

