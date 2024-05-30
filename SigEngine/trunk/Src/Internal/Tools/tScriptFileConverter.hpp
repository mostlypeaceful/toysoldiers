#ifndef __tScriptFileConverter__
#define __tScriptFileConverter__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{

	class tools_export tScriptFileConverter
		: public iAssetPlugin
		, public iAssetGenPlugin
	{
	public:
		virtual iAssetGenPlugin*	fGetAssetGenPluginInterface( )	{ return this; }

        virtual tLoadInPlaceResourcePtr* fAddDependencyInternal( 
            tLoadInPlaceFileBase& outputFileObject, 
            const tFilePathPtr& dependencyPath );

		virtual void fDetermineInputOutputs( 
			tInputOutputList& inputOutputsOut,
			const tFilePathPtrList& immediateInputFiles );

		virtual void fProcessInputOutputs(
			const tInputOutput& inputOutput,
			tFilePathPtrList& indirectGenFilesAddTo );

		enum tVariableType { cVariableTypeString, cVariableTypeEnum, cVariableTypePath, cVariableTypeInt, cVariableTypeFloat, cVariableTypeVector };
		struct tools_export tExportedVariable
		{
		public:
			std::string mExportedName;
			std::string mCallableName;
			s32			mGroupIndex;
			s32			mLineNumber;
			std::string mDescription;
			std::string mCurrentValue;
			std::string mPotentialValues;
			std::string mComment;
			tVariableType mType;
			tGrowableArray<void*> mUIElements; //for inside script editor only.

			std::string fGenerateLine( ) const;
			std::string fValue( ) const;
			tGrowableArray<float> fValues( ) const;
		};

		static void fConvertScript( tDynamicBuffer scriptString, const tFilePathPtr& inputFileName, const tFilePathPtr& output, tPlatformId pid, tFilePathPtrList& indirectGenFilesAddTo, const tFilePathPtrList& additionalDependencies );
		static u32 fGenerateUniqueFileBasedId( const tFilePathPtr& inputFileName );
		static std::string fGenerateUniqueFileBasedTag( u32 uniqueId );
		static b32 fParseAndReplaceSigvars( tScriptFile* scriptFile, tDynamicBuffer& scriptString, const std::string& fileBasedUniqueTag, tGrowableArray< tExportedVariable >& variablesOut, tGrowableArray< std::string >& groupsOut, b32 skipReplacement );

		static u32 fReplaceFilePathReferences( std::string& scriptString, const tFilePathPtr& replace, const tFilePathPtr& with );

	};

}


#endif//__tScriptFileConverter__
