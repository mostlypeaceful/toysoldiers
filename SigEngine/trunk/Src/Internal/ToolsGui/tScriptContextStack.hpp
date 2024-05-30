#ifndef __tScriptContextStack__
#define __tScriptContextStack__

#include "Scripts/tScriptExportData.hpp"

class wxListBox;

namespace Sig { namespace ScriptData
{

	class tScriptContextStack;
	struct tScriptContext;
	typedef tRefCounterPtr<tScriptContext> tScriptContextPtr;

	struct tSymbol
	{
		std::string mName;

		// these are all optional and are context sensitive
		tParameterDesc*	mParam;
		tMemberDesc*	mMember;
		tClassDesc*		mClass;
		tNamespaceDesc* mNamespace;

		tSymbol( const std::string& name = "" );
		tSymbol( tParameterDesc* desc );
		tSymbol( tMemberDesc* desc );
		tSymbol( tClassDesc* desc );
		tSymbol( tNamespaceDesc* desc );

		b32 operator == ( const std::string& name ) const
		{
			return mName == name;
		}
	};

	enum tCollectStatus
	{
		cCollectEverything,
		cCollectParentNamespaces,
		cCollectNoMore,
	};

	enum tContextActionEnum
	{
		cActionPush,
		cActionPop,
		cActionReplace,
		cActionNothing,
	};

	struct tContextAction
	{
		u32 mAction;
		u32 mContextID;
		tScriptContextPtr mNextContext;

		tContextAction( u32 contextID = ~0, u32 action = cActionNothing, tScriptContext* nextContext = NULL )
			: mContextID( contextID ), mAction( action ), mNextContext( nextContext )
		{ }
	};

	struct tScriptContext : public tRefCounter
	{
		tScriptContext( ) 
			: mStack( NULL )
			, mLength( 0 )
			, mDelLength( 0 )
		{ }

		virtual ~tScriptContext( ) { }
		virtual std::string fStatus( ) const { return ""; }
		virtual tCollectStatus fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const { return cCollectEverything; }
		virtual tContextAction fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const { return tContextAction( cActionNothing ); }

		tScriptContextStack* mStack;
		u32 mLength;
		u32 mDelLength;

		virtual u32 fVID( ) const { return ~0; }
	protected:
		static u32 gNextUniqueID;
	};

	#define UniqueID( ) \
		u32 fID( ) const { static u32 mID = ~0; if( mID == ~0 ) mID = gNextUniqueID++; return mID; } \
		virtual u32 fVID( ) const { return fID( ); }

	struct tNamespaceContext : public tScriptContext
	{
		UniqueID( );
		tNamespaceContext( tNamespaceDesc* desc );
		
		tNamespaceDesc* mDesc;

		virtual std::string fStatus( ) const;
		virtual tCollectStatus fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const;
		virtual tContextAction fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const;
	};

	struct tClassNamespaceContext : public tScriptContext
	{
		UniqueID( );
		tClassNamespaceContext( tClassDesc* desc, b32 staticOnly ); //false to call base class functions

		tClassDesc* mDesc;
		b32			mStaticOnly;

		virtual std::string fStatus( ) const;
		virtual tCollectStatus fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const;
		virtual tContextAction fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const;
	};

	struct tFunctionBodyContext : public tScriptContext
	{
		UniqueID( );
		tFunctionBodyContext( tMemberDesc* desc );

		tMemberDesc* mDesc;

		virtual std::string fStatus( ) const;
		virtual tCollectStatus fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const;
		virtual tContextAction fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const;
	};

	struct tInstanceContext : public tScriptContext
	{
		UniqueID( );
		tInstanceContext( tClassDesc* desc );

		template<class t>
		static tContextAction fActionFromParameter( u32 id, u32 action, tParameterDesc* param, tScriptContextStack* stack )
		{
			if( param->fIsVoid( ) )
				return tContextAction( id, action, new tVoidTerminator( "End of statement, void return type." ) );

			tClassDesc* classDesc = stack->fData( )->fClassFromParameter( param );
			if( classDesc ) 
				return tContextAction( id, action, new t( classDesc ) );	
			else
				return tContextAction( id, action, new tVoidTerminator( std::string("No info for type: ") + param->fTrimmedCPPType( ) ) );
		}

		tClassDesc* mDesc;

		virtual std::string fStatus( ) const;
		virtual tCollectStatus fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const;
		virtual tContextAction fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const;

		// recursively adds base class members
		void fCollectClassSymbols( tGrowableArray<tSymbol>& symbolsOut, tClassDesc* desc ) const;
	};

	// this one just waits to push an instance if dereferenced
	struct tReferenceContext : public tScriptContext
	{
		UniqueID( );
		tReferenceContext( tClassDesc* desc );

			
		tClassDesc* mDesc;

		virtual std::string fStatus( ) const;
		virtual tCollectStatus fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const;
		virtual tContextAction fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const;
	};

	struct tVoidTerminator : public tScriptContext
	{
		UniqueID( );
		tVoidTerminator( const std::string& status ) : mStatus( status ) { }

		std::string mStatus;

		virtual std::string fStatus( ) const { return mStatus; }
		virtual tCollectStatus fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const { return cCollectNoMore; }
		virtual tContextAction fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const { return tContextAction( cActionNothing ); }
	};

	struct tAssignmentContext : public tScriptContext
	{
		UniqueID( );
		tAssignmentContext( const std::string& type );

		std::string mCppType; //trimmed type

		virtual std::string fStatus( ) const;
		virtual tCollectStatus fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const;
		virtual tContextAction fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const;
	};

	struct tFunctionArgsContext : public tScriptContext
	{
		UniqueID( );
		tFunctionArgsContext( tMemberDesc* desc, u32 index );

		tMemberDesc* mDesc;
		u32 mIndex;

		virtual std::string fStatus( ) const;
		virtual tCollectStatus fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const;
		virtual tContextAction fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const;
	};


	class toolsgui_export tScriptContextStack
	{
	public:
		tScriptContextStack( );

		void fSetData( const tScriptExportDataPtr& data );

		void fReset( );
		void fPushContext( const tContextAction& context, u32 charPos, u32 delLength );
		tContextAction fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const;
		void fError( const std::string& error ) const { mErrorString += error + " "; mError = true; }

		void fSetContextPosition( u32 position );
		std::string fStatus( ) const;
		void fFillContextList( wxListBox* list );

		const tGrowableArray<tSymbol>& fSymbols( ) const { return mPotentialSymbols; }
		tGrowableArray<tSymbol>& fSymbols( ) { return mPotentialSymbols; }
		const tGrowableArray<char>& fControlChars( ) const { return mControlChars; }

		tSymbol* fFindSymbol( const std::string& text ) { return mPotentialSymbols.fFind( text ); }

		tScriptExportData* fData( ) const { return mData.fGetRawPtr( ); }

	private:
		tScriptExportDataPtr mData;
		tGrowableArray<tScriptContextPtr> mStack;
		tGrowableArray<tContextAction> mActionList;
		tGrowableArray<tSymbol> mPotentialSymbols;
		tGrowableArray<char> mControlChars;
		u32 mContextPosition;
		mutable std::string mErrorString;
		mutable b32 mError;

		void fCollectSymbols( );

		void fBuildStack( );
	};

} }

#endif//__tScriptContextStack__
