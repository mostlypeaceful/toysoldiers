#ifndef __tMaterialFile__
#define __tMaterialFile__

namespace Sig { namespace Gfx
{
	class tDevicePtr;
	class tMaterial;
	class tShadeMaterialGlueValues;
	class tRenderContext;
	class tRenderInstance;
	class tDrawCall;

	class base_export tShadeMaterialGlue : public Rtti::tSerializableBaseClass, public tUncopyable, public tRefCounter
	{
		declare_reflector( );
		implement_rtti_serializable_base_class(tShadeMaterialGlue, 0x1118A4FB);
	public:
		virtual tShadeMaterialGlue* fClone( ) const = 0;
		virtual b32 fIsUnique( ) const { return true; } // most parameters require being the only one of the given type; if yours supports multiple instances of the same, be sure to override this and return false
		virtual b32 fIsShared( ) const { return true; } // most parameters use the fApplyShared method; if yours doesn't, be sure to override this and return false
		virtual void fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const;
		virtual void fApplyInstance( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tShadeMaterialGlue );
	typedef tDynamicArray< tLoadInPlacePtrWrapper<tShadeMaterialGlue> > tShadeMaterialGlueArray;
	typedef tDynamicArray<tShadeMaterialGluePtr> tShadeMaterialGluePtrArray;

	///
	/// TODO document
	class base_export tMaterialFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		declare_lip_version( );
		implement_rtti_serializable_base_class(tMaterialFile, 0x26980D95);
	public:
		static const char*		fGetFileExtension( );

		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return path; }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tFilePathPtr( ); }
	public:

		enum tShaderBufferType
		{
			cShaderBufferTypeVertex,
			cShaderBufferTypePixel,

			// last
			cShaderBufferTypeCount
		};

		typedef Sig::byte* tShaderPlatformHandle;

		class base_export tShaderPointer
		{
			declare_reflector( );
		public:
			tShadeMaterialGlueArray			mGlueShared;
			tShadeMaterialGlueArray			mGlueInstance;
			tEnum<tShaderBufferType,u32>	mType; ///< meaning is platform-specific
			u32								mBufferOffset; ///< offset from the start of the file to the raw shader buffer
			u32								mBufferSize;
			tShaderPlatformHandle			mPlatformHandle;
		};

		typedef tDynamicArray< tShaderPointer > tShaderList;

		Rtti::tClassId						mMaterialCid;
		u32									mHeaderSize; ///< size of the file up to but not including the shader buffers
		b32									mDiscardShaderBuffers;
		tDynamicArray< tShaderList >		mShaderLists;

	public:
		tMaterialFile( );
		tMaterialFile( tNoOpTag );

		virtual void fOnFileLoaded( const tResource& ownerResource );
		virtual void fResizeAfterLoad( tGenericBuffer* fileBuffer );
		virtual void fOnFileUnloading( const tResource& ownerResource );

		void fApplyShader( const tDevicePtr& device, u32 ithShaderList, u32 ithShader ) const;
		void fApplyNullShader( const tDevicePtr& device, tShaderBufferType type ) const;

	private:
		void fCreateShaderPlatformSpecific( const tDevicePtr& device, tShaderPointer& shPtr, const Sig::byte* shaderBuffer );
		void fDestroyShaderPlatformSpecific( tShaderPointer& shPtr );
	};

}} // ::Sig::Gfx

#endif//__tMaterialFile__
