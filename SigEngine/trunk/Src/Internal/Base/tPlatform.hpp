#ifndef __tPlatform__
#define __tPlatform__

namespace Sig
{
	/// Retained for backwards compatability, implemented in and in terms of tPlatform
	base_export const char* fPlatformIdString( tPlatformId pid );
	base_export char fPlatformFilePathSlash( tPlatformId pid );
	base_export b32 fPlatformFileCaseSensitive( tPlatformId pid );
	base_export tEndianType fPlatformEndianType( tPlatformId pid );

	inline bool fPlatformNeedsEndianSwap( tPlatformId srcPid, tPlatformId dstPid )
	{
		return fPlatformEndianType( srcPid ) != fPlatformEndianType( dstPid );
	}

	enum tShaderFamily
	{
		cShaderFamilyMask	= 0xFFFF0000,
		cShaderFamilyGlsl	= 0x00010000,
		cShaderFamilyGlslEs	= 0x00020000,
		cShaderFamilyHlsl	= 0x00030000,
	};

	enum tShaderModel
	{
		cFirstGlsl = cShaderFamilyGlsl,
		cGlsl1_20 = cFirstGlsl,
		cGlsl1_30,
		cGlsl1_40,
		cGlsl1_50,
		cGlsl4_10,
		cLastGlsl = cGlsl4_10,


		cFirstGlslEs = cShaderFamilyGlslEs,
		cGlslEs1 = cFirstGlslEs,
		cGlslEs1_1,
		cGlslEs2,
		cLastGlslEs = cGlslEs2,


		cFirstHlsl = cShaderFamilyHlsl,
		cHlsl1,
		cHlsl2,
		cHlsl2a,
		cHlsl2b,
		cHlsl3,
		cHlsl4,
		cLastHlsl = cHlsl4,
	};

	inline b32 fIsGlsl	( tShaderModel sm ) { return (cFirstGlsl <= sm) && (sm <= cLastGlsl); }
	inline b32 fIsGlslEs( tShaderModel sm ) { return (cFirstGlslEs <= sm) && (sm <= cLastGlslEs); }
	inline b32 fIsHlsl	( tShaderModel sm ) { return (cFirstHlsl <= sm) && (sm <= cLastHlsl); }

	class base_export tPlatformInfo
	{
		declare_uncopyable( tPlatformInfo );
		class tImpl;
		tRefCounterPtr<tImpl> mImpl;
	public:
		tPlatformInfo();
		explicit tPlatformInfo( tPlatformId pid );
		~tPlatformInfo();

		b32 fIsTextureFormatSupported( u32 format ) const;
		b32 fIsVertexFormatSupported( u32 format ) const;

		b32 fIsVertexShaderModelSupported( tShaderModel sm ) const;
		b32 fIsPixelShaderModelSupported( tShaderModel sm ) const;
		b32 fIsCommonShaderModelSupported( tShaderModel sm ) const; ///< Prefer VS/PS-specific checks where possible?

		tStringPtr fFolderName() const;
		tStringPtr fFlagName() const;
		tStringPtr fShortDisplayName() const;
		tStringPtr fLongDisplayName() const;

		tEndianType fEndian() const;
		b32 fCaseSensitiveFilesystem() const;
		char fFilesystemSeperator() const;

		static void fInitAll();
	};
}

#endif //ndef __tPlatform__
