#ifndef __tAnimPackFile__
#define __tAnimPackFile__
#include "tKeyFrameAnimation.hpp"
#include "tHashTable.hpp"

namespace Sig
{

	///
	/// \brief Contains a set of key framed skeletal (bone-based) animations that were authored
	/// in a modelling package (ex: Maya); all animations in the pack correspond to the same skeleton.
	class base_export tAnimPackFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		declare_lip_version( );
		implement_rtti_serializable_base_class(tAnimPackFile, 0xD246627F);
	public:
		static const char*	fGetFileExtension( );
		static tFilePathPtr fAnipkPathToAnib( const char* anipkPath );
		static tFilePathPtr fAnipkPathToAnib( const tFilePathPtr& anipkPath );
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathPK2B( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathB2PK( path ); }
	public:
		typedef tDynamicArray<tKeyFrameAnimation>													tAnimList;
		typedef tHashTable<tHashTablePtrInt, const tKeyFrameAnimation*, tHashTableNoResizePolicy>	tAnimMap;
		typedef tLoadInPlaceRuntimeObject<tAnimMap>													tAnimMapStorage;
	public:
		tLoadInPlaceResourcePtr*	mSkeletonResource;
		tAnimList					mAnims;
		tAnimMapStorage				mAnimsByName;
	public:
		tAnimPackFile( );
		tAnimPackFile( tNoOpTag );

		virtual void fOnFileLoaded( const tResource& ownerResource );
		virtual void fOnFileUnloading( const tResource& ownerResource );

		const tAnimMap& fGetAnimMap( ) const { return mAnimsByName.fTreatAsObject( ); }
		const tKeyFrameAnimation* fFindAnim( const tStringPtr& name ) const;
		const tKeyFrameAnimation* fFindAnim( const tLoadInPlaceStringPtr* name ) const { return fFindAnim( name->fGetStringPtr( ) ); }

		s32 fIndexOfAnim( const tStringPtr& name ) const;
		s32 fIndexOfAnim( const tKeyFrameAnimation& anim ) const;

		u32 fComputeStorage( std::string& display ) const;
		u32 fLODMemoryInBytes( ) const { return 0; }

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	};

} // ::Sig

#endif//__tAnimPackFile__
