#ifndef __tScriptExportData__
#define __tScriptExportData__

#include "tGameArchive.hpp"
#include "GameArchiveString.hpp"

namespace Sig { namespace ScriptData
{

	// This base class is not used for polymorphic serialization but for polymorphic tree user data
	struct base_export tBaseData
	{
		virtual ~tBaseData( ) { }
	};

	struct base_export tParameterDesc : public tBaseData
	{
		tParameterDesc( ) { }
		tParameterDesc( tNoOpTag ) { }

		std::string mName;
		std::string mCppType;
		std::string mDefaultValue;

		std::string fTrimmedCPPType( ) const;
		std::string fScriptSignature( ) const;
		b32 fIsVoid( ) const;

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mName );
			archive.fSaveLoad( mCppType );
			archive.fSaveLoad( mDefaultValue );
		}
	};

	struct base_export tMemberDesc : public tBaseData
	{
		tMemberDesc( ) { }
		tMemberDesc( tNoOpTag ) { }

		enum tType { cPropType, cVarType, cFuncType, cGlobalFuncType, cStaticFuncType };

		u32			mType;
		std::string	mName;
		std::string	mGet; //used for vars and funcs too
		std::string	mSet;

		tParameterDesc mReturnParam;
		tGrowableArray<tParameterDesc> mParams;

		std::string fGenerateLine( ) const;
		std::string fScriptSignature( ) const;

		b32 fIsFunction( ) const				{ return mType == cFuncType || mType == cGlobalFuncType || mType == cStaticFuncType; }
		b32 fIsAssignable( ) const				{ return (mType == cPropType && mSet.length( )) || mType == tMemberDesc::cVarType; }

		b32 operator < ( const tMemberDesc& other ) const
		{
			return mName < other.mName;
		}

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mType );
			archive.fSaveLoad( mName );
			archive.fSaveLoad( mGet );
			archive.fSaveLoad( mSet );
			archive.fSaveLoad( mReturnParam );
			archive.fSaveLoad( mParams );
		}
	};

	struct base_export tBindingDesc : public tBaseData
	{
		tBindingDesc( ) { }
		tBindingDesc( tNoOpTag ) { }

		enum tType { cClassType, cConstType };
		u32		    mType;
		std::string	mNameSpace;
		std::string	mName;
		std::string	mFoundInFile;

		std::string	fGenerateLine( ) const;
		std::string	fFullScriptName( ) const;
		static std::string fGetHppFile( const std::string& cppfileName, const std::string& aliasFile );

		b32 operator < ( const tBindingDesc& other ) const
		{
			if( mNameSpace == other.mNameSpace )
				return mName < other.mName;
			else
				return mNameSpace < other.mNameSpace;
		}

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mType );
			archive.fSaveLoad( mNameSpace );
			archive.fSaveLoad( mName );
			archive.fSaveLoad( mFoundInFile );
		}
	};

	struct base_export tClassDesc : public tBaseData
	{
		tClassDesc( ) { }
		tClassDesc( tNoOpTag ) { }

		std::string	mCppName;
		std::string	mCppBase;

		tBindingDesc mBinding;
		tGrowableArray<tMemberDesc> mMembers;
		tMemberDesc mConstructor;

		std::string	fCPPName( ) const;
		std::string fTrimmedCPPName( ) const;
		std::string fTrimmedBaseName( ) const;
		void		fSort( );

		tMemberDesc* fFindMember( const std::string& scriptName );

		b32 operator < ( const tClassDesc& other ) const
		{
			return mBinding < other.mBinding;
		}

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mCppName );
			archive.fSaveLoad( mCppBase );
			archive.fSaveLoad( mBinding );
			archive.fSaveLoad( mMembers );
			archive.fSaveLoad( mConstructor );
		}
	};

	struct base_export tNamespaceDesc : public tBaseData
	{
		tNamespaceDesc( ){ }
		tNamespaceDesc( tNoOpTag ) { }

		std::string	mName;
		tGrowableArray<tClassDesc> mClasses;

		void fSort( );

		b32 operator < ( const tNamespaceDesc& other ) const
		{
			return mName < other.mName;
		}

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mName );
			archive.fSaveLoad( mClasses );
		}
	};

	struct base_export tScriptExportData : public tRefCounter
	{
		tScriptExportData( ){ }
		tScriptExportData( tNoOpTag ) { }

		tGrowableArray<tNamespaceDesc> mNameSpaces;
		tGrowableArray<tBindingDesc> mConsts;

		void fSort( );

		tNamespaceDesc* fFindNameSpace( const std::string& ns );
		tClassDesc* fFindClass( const std::string& scriptName );
		tClassDesc* fFindClassByCPPName( const std::string& cppName );
		tClassDesc* fClassFromParameter( tParameterDesc* param );

		static tFilePathPtr fGetDataPath( );
		void fSave( );
		b32  fLoad( );

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			const u32 cVersion = 3;
			u32 version = cVersion;

			archive.fSaveLoad( version );
			if( version != cVersion )
			{
				archive.fFail( );
				return;
			}

			archive.fSaveLoad( mNameSpaces );
			archive.fSaveLoad( mConsts );
		}
	};

	typedef base_export tRefCounterPtr<tScriptExportData> tScriptExportDataPtr;
} }

#endif//__tScriptExportData__
