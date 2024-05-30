#ifndef __tLoadInPlaceFileBase__
#define __tLoadInPlaceFileBase__
#include "tBinaryFileBase.hpp"
#include "tResource.hpp"
#include "Base.rtti.hpp"
#include "BaseExplicit.rtti.hpp"

namespace Sig
{

	typedef tDynamicArray<char> tLoadInPlaceString;

	class base_export tLoadInPlaceStringPtr
	{
		declare_reflector( );
	public:
		tLoadInPlaceRuntimeObject<tStringPtr>	mStringPtr;
		tLoadInPlaceString						mRawString;

		inline const tStringPtr&				fGetStringPtr( ) const { return mStringPtr.fTreatAsObject( ); }

		void fRelocateInPlace( ptrdiff_t delta )
		{
			mRawString.fRelocateInPlace( delta );
			mStringPtr.fTreatAsObject( ) = tStringPtr( mRawString.fBegin( ) );
		}
	};

	class base_export tLoadInPlaceFilePathPtr
	{
		declare_reflector( );
	public:
		tLoadInPlaceRuntimeObject<tFilePathPtr>	mFilePathPtr;
		tLoadInPlaceString						mRawString;

		inline const tFilePathPtr&				fGetFilePathPtr( ) const { return mFilePathPtr.fTreatAsObject( ); }

		void fRelocateInPlace( ptrdiff_t delta )
		{
			mRawString.fRelocateInPlace( delta );
			mFilePathPtr.fTreatAsObject( ) = tFilePathPtr( mRawString.fBegin( ) );
		}
	};

	class base_export tLoadInPlaceResourceId
	{
		declare_reflector( );
	public:
		Rtti::tClassId							mClassId;
		tLoadInPlaceRuntimeObject<tFilePathPtr>	mFilePathPtr;
		tLoadInPlaceString						mRawPath;

		inline const tFilePathPtr&				fGetFilePathPtr( ) const { return mFilePathPtr.fTreatAsObject( ); }
		inline tResourceId						fGetResourceId( ) const { return tResourceId::fMake( mClassId, fGetFilePathPtr( ) ); }
		inline tResourceId						fGetResourceIdRawPath( ) const { return tResourceId::fMake( mClassId, tFilePathPtr( mRawPath.fBegin( ) ) ); }

		void fRelocateInPlace( ptrdiff_t delta )
		{
			mRawPath.fRelocateInPlace( delta );
			mFilePathPtr.fTreatAsObject( ) = tFilePathPtr( mRawPath.fBegin( ) );
		}
	};

	class base_export tLoadInPlaceResourcePtr : public tLoadInPlaceResourceId
	{
		declare_reflector( );
	public:
		tLoadInPlaceRuntimeObject<tResourcePtr>	mResourcePtr;
		inline const tResourcePtr&				fGetResourcePtr( ) const { return mResourcePtr.fTreatAsObject( ); }

		void fRelocateInPlace( ptrdiff_t delta )
		{
			tLoadInPlaceResourceId::fRelocateInPlace( delta );
			mResourcePtr.fTreatAsObject( ).fRelease( );
		}
	};

	///
	/// Extends tBinaryFileBase to aid in storing string and path tables
	/// for the entire file (i.e., consolidate all strings/filepaths, and at runtime
	/// get real tStringPtr and tFilePathPtr objects so as to be compatible with all
	/// runtime functionality).
	class base_export tLoadInPlaceFileBase : public tBinaryFileBase
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tLoadInPlaceFileBase, 0xD03B7854 );
	public:
		typedef tDynamicArray< tLoadInPlacePtrWrapper<tLoadInPlaceStringPtr> >		tStringPtrTable;
		typedef tDynamicArray< tLoadInPlacePtrWrapper<tLoadInPlaceFilePathPtr> >	tFilePathPtrTable;
		typedef tDynamicArray< tLoadInPlacePtrWrapper<tLoadInPlaceResourceId> >		tResourceIdTable;
		typedef tDynamicArray< tLoadInPlacePtrWrapper<tLoadInPlaceResourcePtr> >	tResourcePtrTable;
	private:
		tStringPtrTable		mStringPtrTable;
		tFilePathPtrTable	mFilePathPtrTable;
		tResourceIdTable	mResourceIdTable;
		tResourcePtrTable	mResourcePtrTable; // This could actually just be IDs
		tLoadInPlaceRuntimeObject< tGrowableArray< tResourcePtr > > mTotalSubResourcePtrTable;
		tLoadInPlaceRuntimeObject<tResource::tOnLoadComplete::tObserver> mOnSubResourceLoaded;
		Sig::byte*			mOwnerResource;
		u32					mNumSubResourcesLoadedSuccess;
		u32					mNumSubResourcesLoadedFailed;
	protected:
		tLoadInPlaceFileBase( );
		tLoadInPlaceFileBase( tNoOpTag );
	private:
		inline u32 fNumSubResourcesFinished( ) const { return mNumSubResourcesLoadedSuccess + mNumSubResourcesLoadedFailed; }
		void fOnSubResourceLoaded( tResource& resource, b32 success );
	public:
		void fInitializeLoadInPlaceTables( tResourceDepot& depot );
		void fInitializeLoadInPlaceTables( tResource& ownerResource );
		void fCleanUpLoadInPlaceTables( );
		///
		/// \brief Derived types should call this method after performing relocation
		void fRelocateLoadInPlaceTables( ptrdiff_t delta );
		b32  fLoadSubResources( tResource& ownerResource );
		void fUnloadSubResources( );
		b32  fIsSubResource( const tResource& sub, tResource::tVisitedList& visited ) const;
		void fGatherSubResources( tResourcePtrList& subs ) const;
		tResourcePtr fOwnerResource( ) const;

		/// 
		/// \brief Implement this to trigger the load of any runtime resources that you need to load. Gets triggered 
		/// after the tables are initialized and before sub resources are loaded.
		virtual void fGatherRuntimeResources( tResource& ownerResource ) { }
		void fAddRuntimeResourcePtr( tResourcePtr& ptr );

		///
		/// \brief Called after the file is loaded and the all load-in-place initialization is complete.
		/// Provides a mechanism for custom one-time initialization.
		virtual void fOnFileLoaded( const tResource& ownerResource ) { }

		///
		/// \brief Called after the file is loaded, all load-in-place initialization is complete, and all sub-resources have loaded.
		/// Provides a mechanism for custom one-time initialization, while ensuring all sub-resources are also loaded.
		/// \note If there are no sub-resources, this method will NOT be invoked.
		virtual void fOnSubResourcesLoaded( const tResource& ownerResource, b32 success ) { }

		///
		/// \brief Called following the fOnFileLoaded method. This allows the file to resize itself, usually
		/// in order to remove any uneeded data at the end of the file once it has been safely processed and/or
		/// transferred elsewhere. Be careful, calling fFree/fAlloc/fResize on the 'fileBuffer' parameter will end
		/// up deleting the current instance ('this').
		virtual void fResizeAfterLoad( tGenericBuffer* fileBuffer ) { }

		///
		/// \brief Called before any part of the file is unloaded or de-allocated.
		/// Provides a mechanism to clean up any of the custom one-time initialization 
		/// performed in fOnFileLoaded.
		virtual void fOnFileUnloading( const tResource & ownerResource ) { }

#ifdef target_tools
	private:
		void fCleanUpTables( );
		void fCopyTables( const tLoadInPlaceFileBase& copyFrom );
	public:
		tLoadInPlaceFileBase( const tLoadInPlaceFileBase& other );
		tLoadInPlaceFileBase& operator=( const tLoadInPlaceFileBase& other );
		~tLoadInPlaceFileBase( );
		tLoadInPlaceStringPtr*		fAddLoadInPlaceStringPtr( const char* s );
		tLoadInPlaceFilePathPtr*	fAddLoadInPlaceFilePathPtr( const char* s );
		tLoadInPlaceResourceId*		fAddLoadInPlaceResourceId( const tResourceId& rid );
		tLoadInPlaceResourcePtr*	fAddLoadInPlaceResourcePtr( const tResourceId& rid );

		// This simpler method is used for tools only
		// Used to populate the tLoadInPlaceResourcePtrs.
		void fQueryResourcePtrs( tResourceDepot& depot );
#endif//target_tools
	};
}

#endif//__tLoadInPlaceFileBase__

