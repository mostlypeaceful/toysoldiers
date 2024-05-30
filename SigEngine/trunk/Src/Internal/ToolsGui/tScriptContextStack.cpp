#include "ToolsGuiPch.hpp"
#include "tScriptContextStack.hpp"
#include "wx/listbox.h"


namespace Sig { namespace ScriptData
{

	u32 tScriptContext::gNextUniqueID = 0;


	tSymbol::tSymbol( const std::string& name )
		: mName( name ) 
		, mParam( NULL )
		, mMember( NULL )
		, mClass( NULL )
		, mNamespace( NULL )
	{ }

	tSymbol::tSymbol( tParameterDesc* desc )
		: mName( desc->mName )
		, mParam( desc )
		, mMember( NULL )
		, mClass( NULL )
		, mNamespace( NULL )
	{ }

	tSymbol::tSymbol( tMemberDesc* desc )
		: mName( desc->mName )
		, mParam( NULL )
		, mMember( desc )
		, mClass( NULL )
		, mNamespace( NULL )
	{ }

	tSymbol::tSymbol( tClassDesc* desc )
		: mName( desc->mBinding.mName )
		, mParam( NULL )
		, mMember( NULL )
		, mClass( desc )
		, mNamespace( NULL )
	{ }

	tSymbol::tSymbol( tNamespaceDesc* desc )
		: mName( desc->mName )
		, mParam( NULL )
		, mMember( NULL )
		, mClass( NULL )
		, mNamespace( desc )
	{ }




	tNamespaceContext::tNamespaceContext( tNamespaceDesc* desc )
		: mDesc( desc )
	{ }

	std::string tNamespaceContext::fStatus( ) const
	{
		return "Namespace: " + mDesc->mName;
	}

	tCollectStatus tNamespaceContext::fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const
	{
		controlOut.fPushBack( '(' );

		if( status != cCollectNoMore )
		{
			for( u32 i = 0; i < mDesc->mClasses.fCount( ); ++i )
				symbolsOut.fPushBack( &mDesc->mClasses[ i ] );

			if( mDesc->mName == "" )
			{
				//default namespace add sub namespaces
				tGrowableArray<tNamespaceDesc>& namespaces = mStack->fData( )->mNameSpaces;
				for( u32 i = 0; i < namespaces.fCount( ); ++i )
					if( namespaces[ i ].mName != mDesc->mName )
						symbolsOut.fPushBack( &namespaces[ i ] );
			}
		}

		return cCollectNoMore;
	}

	tContextAction tNamespaceContext::fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const
	{
		if( symbol && controlCharacter == '(' )
		{
			if( symbol->mClass )
				return tContextAction( fID( ), cActionPush, new tFunctionArgsContext( &symbol->mClass->mConstructor, 0 ) );	
		}

		return tContextAction( cActionNothing );
	}

	///////////////////////////////////////////////////////////////////////

	tClassNamespaceContext::tClassNamespaceContext( tClassDesc* desc, b32 staticOnly )
		: mDesc( desc )
		, mStaticOnly( staticOnly )
	{ }

	std::string tClassNamespaceContext::fStatus( ) const
	{
		return "Class Namespace: " + mDesc->fTrimmedCPPName( );
	}

	tCollectStatus tClassNamespaceContext::fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const
	{
		controlOut.fPushBack( '(' );

		if( status != cCollectNoMore )
		{
			for( u32 i = 0; i < mDesc->mMembers.fCount( ); ++i )
			{
				if( !mStaticOnly || mDesc->mMembers[ i ].mType == tMemberDesc::cStaticFuncType )
					symbolsOut.fPushBack( &mDesc->mMembers[ i ] );
			}
		}

		return cCollectNoMore;
	}

	tContextAction tClassNamespaceContext::fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const
	{
		if( symbol && controlCharacter == '(' )
		{
			if( symbol->mMember )
				return tContextAction( fID( ), cActionPush, new tFunctionArgsContext( symbol->mMember, 0 ) );	
		}

		return tContextAction( cActionNothing );
	}

	///////////////////////////////////////////////////////////////////////
	tFunctionBodyContext::tFunctionBodyContext( tMemberDesc* desc )
		: mDesc( desc )
	{ }

	std::string tFunctionBodyContext::fStatus( ) const
	{
		return "Function Body: " + mDesc->fScriptSignature( );
	}

	tCollectStatus tFunctionBodyContext::fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const
	{
		controlOut.fPushBack( '.' );
		controlOut.fPushBack( '=' );
		controlOut.fPushBack( '(' );

		if( status != cCollectNoMore )
		{
			for( u32 i = 0; i < mDesc->mParams.fCount( ); ++i )
			{
				symbolsOut.fPushBack( &mDesc->mParams[ i ] );
			}
		}

		return cCollectEverything;
	}

	tContextAction tFunctionBodyContext::fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const
	{
		if( symbol )
		{
			if( controlCharacter == '.' )
			{
				if( symbol->mParam )
				{
					return tInstanceContext::fActionFromParameter<tInstanceContext>( fID( ), cActionPush, symbol->mParam, mStack );
				}
				else if( symbol->mClass )
				{
					return tContextAction( fID( ), cActionPush, new tClassNamespaceContext( symbol->mClass, true ) );	
				}
				else if( symbol->mNamespace )
				{
					return tContextAction( fID( ), cActionPush, new tNamespaceContext( symbol->mNamespace ) );	
				}
			}
			else if( controlCharacter == '(' )
			{
				if( symbol->mClass )
					return tContextAction( fID( ), cActionPush, new tFunctionArgsContext( &symbol->mClass->mConstructor, 0 ) );	
			}
		}

		return tContextAction( cActionNothing );
	}

	///////////////////////////////////////////////////////////////////////

	tInstanceContext::tInstanceContext( tClassDesc* desc )
		: mDesc( desc )
	{ }

	std::string tInstanceContext::fStatus( ) const
	{
		return "Instance: " + mDesc->fTrimmedCPPName( );
	}

	tCollectStatus tInstanceContext::fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const
	{
		controlOut.fPushBack( '(' );
		controlOut.fPushBack( '=' );
		controlOut.fPushBack( '.' );

		if( status == cCollectEverything )
		{
			fCollectClassSymbols( symbolsOut, mDesc );
		}

		return cCollectNoMore;
	}

	void tInstanceContext::fCollectClassSymbols( tGrowableArray<tSymbol>& symbolsOut, tClassDesc* desc ) const
	{
		for( u32 i = 0; i < desc->mMembers.fCount( ); ++i )
			symbolsOut.fPushBack( &desc->mMembers[ i ] );

		std::string baseName = desc->fTrimmedBaseName( );
		tClassDesc* baseClass = mStack->fData( )->fFindClassByCPPName( baseName );
		if( baseClass )
			fCollectClassSymbols( symbolsOut, baseClass );
	}

	tContextAction tInstanceContext::fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const
	{
		if( symbol )
		{
			if( controlCharacter == '=' )
			{
				//symbol must be an assignable type.
				tMemberDesc* desc = symbol->mMember;

				if( desc && desc->fIsAssignable( ) )
				{
					return tContextAction( fID( ), cActionReplace, new tAssignmentContext( desc->mReturnParam.fTrimmedCPPType( ) ) );
				}
				else
					mStack->fError( "Member not assignable." );

			}
			else if( controlCharacter == '(' )
			{
				if( symbol->mMember )
				{
					if( symbol->mMember->fIsFunction( ) )
						return tContextAction( fID( ), cActionReplace, new tFunctionArgsContext( symbol->mMember, 0 ) );
					else
						mStack->fError( "Member not a function." );
				}
			}
			else if( controlCharacter == '.' )
			{
				if( symbol->mMember )
				{
					return tInstanceContext::fActionFromParameter<tInstanceContext>( fID( ), cActionReplace, &symbol->mMember->mReturnParam, mStack );
				}
			}
		}

		return tContextAction( cActionNothing );
	}

	///////////////////////////////////////////////////////////////////////

	tReferenceContext::tReferenceContext( tClassDesc* desc )
		: mDesc( desc )
	{ }

	std::string tReferenceContext::fStatus( ) const
	{
		return "Reference: " + mDesc->fTrimmedCPPName( );
	}

	tCollectStatus tReferenceContext::fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const
	{
		controlOut.fPushBack( '.' );

		return cCollectNoMore;
	}

	tContextAction tReferenceContext::fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const
	{
		if( controlCharacter == '.' )
		{
			return tContextAction( fID( ), cActionReplace, new tInstanceContext( mDesc ) );
		}

		return tContextAction( cActionNothing );
	}

	///////////////////////////////////////////////////////////////////////

	tAssignmentContext::tAssignmentContext( const std::string& type )
		: mCppType( type )
	{ }

	std::string tAssignmentContext::fStatus( ) const
	{
		return "Assignment: " + mCppType;
	}

	tCollectStatus tAssignmentContext::fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const
	{
		if( status != cCollectNoMore )
		{
		}

		return cCollectParentNamespaces;
	}

	tContextAction tAssignmentContext::fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const
	{
		return tContextAction( cActionNothing );
	}

	////////////////////////////////////////////////////////////////

	tFunctionArgsContext::tFunctionArgsContext( tMemberDesc* desc, u32 index )
		: mDesc( desc )
		, mIndex( index )
	{ }

	std::string tFunctionArgsContext::fStatus( ) const
	{
		if( !mDesc->mParams.fCount( ) )
			return "Function Args: None";
		if( mIndex >= mDesc->mParams.fCount( ) )
			return "Function Args: Doesnt Exist";
		else
			return "Function Args: " + mDesc->mParams[ mIndex ].fScriptSignature( );
	}

	tCollectStatus tFunctionArgsContext::fCollectSymbols( tCollectStatus status, tGrowableArray<tSymbol>& symbolsOut, tGrowableArray<char>& controlOut ) const
	{
		controlOut.fPushBack( ')' );
		controlOut.fPushBack( ',' );
		controlOut.fPushBack( '.' );

		if( status != cCollectNoMore )
		{
		}

		return cCollectParentNamespaces;
	}

	tContextAction tFunctionArgsContext::fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const
	{
		if( controlCharacter == ',' )
		{
			if( mIndex + 1 >= mDesc->mParams.fCount( ) )
				mStack->fError( "Parameter does not exist." );
			else
				return tContextAction( fID( ), cActionReplace, new tFunctionArgsContext( mDesc, mIndex + 1 ) );
		}
		else if( controlCharacter == ')' )
		{
			if( mDesc->mParams.fCount( ) && mIndex != mDesc->mParams.fCount( ) - 1 )
			{
				mStack->fError( "Not enough parameters." );
			}
			else
			{
				return tInstanceContext::fActionFromParameter<tReferenceContext>( fID( ), cActionReplace, &mDesc->mReturnParam, mStack );
			}
		}
		else if( controlCharacter == '.' )
		{
			if( symbol->mClass )
			{
				return tContextAction( fID( ), cActionPush, new tClassNamespaceContext( symbol->mClass, true ) );	
			}
			else if( symbol->mParam )
			{
				return tInstanceContext::fActionFromParameter<tInstanceContext>( fID( ), cActionReplace, symbol->mParam, mStack );
			}
		}

		return tContextAction( cActionNothing );
	}





	////////////////////////////////////////////////////////////////	
	
	tScriptContextStack::tScriptContextStack( )
		: mError( false )
	{ }

	tContextAction tScriptContextStack::fNewContextFromSymbol( tSymbol* symbol, char controlCharacter ) const
	{
		mErrorString = "";
		mError = false;

		for( s32 i = mStack.fCount( ) - 1; i >= 0 ; --i )
		{
			tContextAction action  = mStack[ i ]->fNewContextFromSymbol( symbol, controlCharacter );
			if( mError )
				break;

			if( action.mAction != cActionNothing )
				return action;
		}
		
		return tContextAction( cActionNothing );
	}

	void tScriptContextStack::fReset( )
	{
		mActionList.fSetCount( 0 );
		mStack.fSetCount( 0 );
		mPotentialSymbols.fSetCount( 0 );
		mControlChars.fSetCount( 0 );
	}

	void tScriptContextStack::fPushContext( const tContextAction& context, u32 charPos, u32 delLength )
	{
		if( context.mAction != cActionNothing )
		{
			context.mNextContext->mStack = this;
			context.mNextContext->mDelLength = delLength;

			u32 lenSoFar = 0;
			for( u32 i = 0; i < mActionList.fCount( ); ++i )
				lenSoFar += mActionList[ i ].mNextContext->mLength + mActionList[ i ].mNextContext->mDelLength;

			u32 myLen = charPos - lenSoFar;
			context.mNextContext->mLength = myLen;

			mActionList.fPushBack( context );
		}

		mContextPosition = charPos;

		fCollectSymbols( );
	}

	void tScriptContextStack::fBuildStack( )
	{
		mStack.fSetCount( 0 );

		s32 posLeft = (s32)mContextPosition;

		for( u32 i = 0; i < mActionList.fCount( ); ++i )
		{
			tContextAction& action = mActionList[ i ];

			s32 len = action.mNextContext->mLength;
			s32 totLen = len + action.mNextContext->mDelLength;
			if( totLen && len > posLeft ) //the if( totLen )  here is to handle debug injection of zero length contexts at the begining of the line
				break;
				
			posLeft -= totLen;

			if( action.mAction != cActionNothing )
			{
				u32 indexInStack = ~0;
				b32 debugStackElement = (action.mContextID == ~0); //this action will not have match and should just be pushed.

				if( !debugStackElement )
				{
					for( s32 s = mStack.fCount( ) - 1; !debugStackElement && s >= 0 ; --s )
						if( mStack[ s ]->fVID( ) == action.mContextID )
						{
							indexInStack = s;
							break;
						}
				}
						
				if( !debugStackElement && indexInStack == ~0 )
				{
					mErrorString += "Stack error!";
					break;
				}
				else
				{
					if( !debugStackElement )
					{
						// remove top stack elements as necessary
						if( action.mAction == cActionPop || action.mAction == cActionReplace )
							mStack.fSetCount( indexInStack ); //clear me and above
						else if( action.mAction == cActionPush )
							mStack.fSetCount( indexInStack + 1 ); //clear above
					}

					if( action.mAction == cActionPush || action.mAction == cActionReplace )
						mStack.fPushBack( tScriptContextPtr( action.mNextContext ) );
				}
			}
		}
	}

	void tScriptContextStack::fFillContextList( wxListBox* list )
	{
		list->Clear( );
		for( u32 i = 0; i < mStack.fCount( ); ++i )
			list->Append( mStack[ i ]->fStatus( ) );
	}

	void tScriptContextStack::fSetData( const tScriptExportDataPtr& data )
	{
		mData = data;
		fReset( );
	}

	void tScriptContextStack::fSetContextPosition( u32 position )
	{
		mContextPosition = position;
		fCollectSymbols( );
	}

	std::string tScriptContextStack::fStatus( ) const
	{
		std::string message = "";

		if( mStack.fCount( ) )
			message = mStack.fBack( )->fStatus( );

		if(  mErrorString.length( ) )
			message += " Error: " + mErrorString;

		return message;
	}

	void tScriptContextStack::fCollectSymbols( )
	{
		mPotentialSymbols.fSetCount( 0 );
		mControlChars.fSetCount( 0 );
		
		if( mError )
		{
			mPotentialSymbols.fPushBack( tSymbol( "Error." ) );
			return;
		}

		fBuildStack( );

		tCollectStatus status = cCollectEverything;
		for( s32 i = mStack.fCount( ) - 1; i >= 0 ; --i )
		{
			tCollectStatus newStatus = mStack[ i ]->fCollectSymbols( status, mPotentialSymbols, mControlChars );

			if( status == cCollectEverything )
			{
				status = newStatus;
			}
		}

		if( mPotentialSymbols.fCount( ) == 0 )
			mPotentialSymbols.fPushBack( tSymbol( "No suggestions." ) );
	}

} }
