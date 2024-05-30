#ifndef __tFxFile__
#define __tFxFile__
#include "tEntityDef.hpp"

namespace Sig { namespace FX
{

	class base_export tFxFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tFxFile, 0xD00A932 );
	public:
		typedef tDynamicArray< tLoadInPlacePtrWrapper< tEntityDef > > tObjectArray;
		tObjectArray mObjects;
		f32 mLifetime;
		Math::tAabbf mBounds;
	public:
		static const u32 cVersion;
	public:
		static u32			fGetNumBinaryExtensions( );
		static const char*	fGetBinaryExtension( u32 i = 0 );
		static b32			fIsBinary( const tFilePathPtr& path );

		static u32			fGetNumFileExtensions( );
		static const char*	fGetFileExtension( u32 i = 0 );
		static b32			fIsFile( const tFilePathPtr& path );

		static tFilePathPtr fFxmlPathToFxb( const tFilePathPtr& path );
		static tFilePathPtr fFxbPathToFxml( const tFilePathPtr& path );

	public:
		tFxFile( );
		tFxFile( tNoOpTag );
		~tFxFile( );

		virtual void fOnFileLoaded( const tResource& ownerResource );
		virtual void fOnSubResourcesLoaded( const tResource& ownerResource );
		virtual void fOnFileUnloading( );

		void fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const;

		f32 fLifetime( ) const { return mLifetime; }
	};

}}


namespace Sig
{
	template<>
	class tResourceConvertPath<FX::tFxFile>
	{
	public:
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathML2B( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathB2ML( path ); }
	};
}

#endif	// __tFxFile__

