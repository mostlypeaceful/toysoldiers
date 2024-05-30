#ifndef __tScriptFile__
#define __tScriptFile__

namespace Sig
{
	class base_export tScriptFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		declare_lip_version( );
		implement_rtti_serializable_base_class(tScriptFile, 0x08139E89);

	public:
		static const char*		fGetFileExtension( );
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathAddB( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathSubB( path ); }

	public:
		enum tFlags
		{
			cFlagCompiledByteCode = 1 << 0,
		};
		enum tStandardExportedFunction
		{
			cEntityOnCreated,
			cEntityOnChildrenCreated,
			cEntityOnSiblingsCreated,
			cStandardExportedFunctionCount,
		};

		typedef tLoadInPlaceRuntimeObject<Sqrat::Function> tScriptObjectStorage;

		struct base_export tExportedFunction
		{
			declare_reflector( );
		public:
			tLoadInPlaceStringPtr* mCallableName;
			tLoadInPlaceStringPtr* mExportedName;
			tScriptObjectStorage   mScriptObject;
		public:
			tExportedFunction( ) : mScriptObject( cNoOpTag ) { }
			tExportedFunction( tNoOpTag ) : mScriptObject( cNoOpTag ) { }
		};

		typedef tExportedFunction tExportedVariable;

	public:
		u32																	mFlags;
		u32																	mUniqueId;
		tLoadInPlaceStringPtr*												mScriptClass;
		tDynamicBuffer														mByteCode;
		tDynamicArray< tLoadInPlacePtrWrapper<tLoadInPlaceResourcePtr> >	mScriptImports;
		tDynamicArray< tExportedFunction >									mExportedFunctions;
		tDynamicArray< tExportedVariable >									mExportedVariables;
		tFixedArray< tExportedFunction, cStandardExportedFunctionCount >	mStandardExportedFunctions;

	public:
		tScriptFile( );
		tScriptFile( tNoOpTag );
		void fInitScriptObjects( tScriptVm& vm );
		virtual void fOnSubResourcesLoaded( const tResource& ownerResource, b32 success ) OVERRIDE;
		virtual void fOnFileUnloading( const tResource& ownerResource ) OVERRIDE;

		const Sqrat::Function& fIndexStandardExportedFunction( tStandardExportedFunction func ) const { return mStandardExportedFunctions[ func ].mScriptObject.fTreatAsObject( ); }
		const Sqrat::Function& fFindExportedFunction( const tStringPtr& exportName ) const;
	};

} // ::Sig

#endif//__tScriptFile__

