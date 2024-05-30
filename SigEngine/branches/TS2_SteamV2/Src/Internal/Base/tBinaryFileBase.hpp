#ifndef __tBinaryFileBase__
#define __tBinaryFileBase__

namespace Sig
{
	class tResource;

	///
	/// \brief Base class and byte-stream header for all Sig binary files.
	///
	/// This class provides a common header for all Sig binary files,
	/// so that the first N (see below for how big N is) bytes of the
	/// file can be consistent between all Sig binary files. Provides:
	/// a) validating that a file is one of ours 
	/// b) figuring out the derived type just from the header 
	/// c) basic version validation
	class base_export tBinaryFileBase : public Rtti::tSerializableBaseClass
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tBinaryFileBase, 0x418E645C );
	public:
		typedef tFixedArray<char,8> tSignature;

	protected:
		tSignature		mSignature;
		Rtti::tClassId	mFileType;
		u32				mVersion;

	protected:
		tBinaryFileBase( );
		tBinaryFileBase( tNoOpTag );

		static void fCreateSignature( tSignature& sig, tPlatformId pid );

	public:

		b32 fVerifyResource( 
			const tResource& resource, 
			tFilePathPtr (*fConvertToSource)( const tFilePathPtr& path ), 
			u32 expectedVersion, 
			Rtti::tClassId classId  );

		inline const tSignature&	fSignature( ) const			{ return mSignature; }
		inline Rtti::tClassId		fFileClassId( ) const		{ return mFileType; }
		inline u32					fVersion( ) const			{ return mVersion; }


#ifdef target_tools
		void fSetSignature( tPlatformId pid, Rtti::tClassId cid, u32 version );
#endif//target_tools
	};


}

#endif//__tBinaryFileBase__