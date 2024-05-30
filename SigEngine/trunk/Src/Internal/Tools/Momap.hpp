#ifndef __Momap__
#define __Momap__

#include "tDAGNodeCanvas.hpp"
#include "DerivedAnimNodes.hpp"

namespace Sig { 
	
	class tMotionMapFile;
	
namespace Momap
{
	/*
		
		The momap consists of a blend tree. It's data members are anim inputs, blends, and a single anim output.
		The momap is sort of the abstract interface of motion. It says i'm capable of idling, running, aiming, etc.
		The animap is what correlates actual animations with these higher level motions.

	*/

	tools_export const char* fGetFileExtension( );
	tools_export b32 fIsMomapFile( const tFilePathPtr& path );


	struct tools_export tConnection
	{
		u32 mInput;
		u32 mOutput;
		u32 mInputIndex;
		u32 mOutputIndex;

		tConnection( u32 i = ~0, u32 idi = ~0, u32 o = ~0, u32 ido = ~0 )
			: mInput( i )
			, mOutput( o )
			, mInputIndex( idi )
			, mOutputIndex( ido )
		{ }
	};

	typedef tGrowableArray< tConnection >  tConnectionList;
	typedef tGrowableArray< tAnimBaseNodePtr >  tNodeList;

	// This context system has been pulled out in favor of real GameFlags:: enums, so they can be set more universally in tools and c++
	///*
	//
	//	The context functionality of the momap is sort of like an enum.
	//	There is a Context with a name, and then it has value options below it.

	//	In the animap, per anim input, the user will be able to build these contexts into a tree
	//	 When the anim input is evaluated, the current context values will be used to choose the appropriate animation.

	//*/
	//struct tools_export tContextOption
	//{
	//	std::string mName;
	//	u32 mKey;

	//	tContextOption( const std::string& name = "", u32 key = ~0 )
	//		: mName( name )
	//		, mKey( key )
	//	{ }

	//	b32 operator == ( const std::string& name ) const { return mName == name; }
	//	b32 operator == ( u32 key ) const { return mKey == key; }
	//};

	//struct tools_export tContext
	//{
	//	std::string mName;
	//	u32 mKey;

	//	tGrowableArray< tContextOption > mOptions;

	//	tContext( const std::string& name = "", u32 key = ~0 )
	//		: mName( name )
	//		, mKey( key )
	//		, mNextFreeKey( 0 )
	//	{ }

	//	b32 operator == ( const std::string& name ) const { return mName == name; }
	//	b32 operator == ( u32 key ) const { return mKey == key; }

	//	u32 fFindValueIndexByName( const std::string& name );
	//	u32 fFindValueIndexByKey( u32 key ) { return mOptions.fIndexOf( key ); }

	//	void fInsertValue( const std::string& name, u32 index );

	//	template<class tSerializer>
	//	void fSerializeXml( tSerializer& s )
	//	{
	//		s( "n", mName );
	//		s( "k", mKey );
	//		s( "o", mOptions );
	//		s( "nf", mNextFreeKey );
	//	}

	//private:
	//	u32 mNextFreeKey;
	//};

	//struct tools_export tContextData
	//{
	//	tGrowableArray< tContext > mContexts;

	//	tContextData( )
	//		: mNextFreeKey( 0 )
	//	{ }

	//	void fAddContext( const std::string& name );
	//	tContext* fFindByKey( u32 key ) { return mContexts.fFind( key ); }

	//	template<class tSerializer>
	//	void fSerializeXml( tSerializer& s )
	//	{
	//		s( "c", mContexts );
	//		s( "nf", mNextFreeKey );
	//	}

	//private:
	//	u32 mNextFreeKey;
	//};

	struct tools_export tMoState
	{
		tNodeList			mNodes;
		tConnectionList		mConnections;
		u32					mNextUniqueNodeId;

		tMoState( ) 
			: mNextUniqueNodeId( 0 ) 
		{ }
		
		void fCollect( tDAGNodeCanvas* canvas, tDAGNodeCanvas::tDAGNodeList& nodes, tDAGNodeCanvas::tDAGNodeConnectionList& conn, b32 alreadyConstructed ) const;
	};

	class tools_export tFile
	{
	public:
		tMoState mMoState; //just one for now but may support many.

	public:
		tFile( );

		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );

		tMotionMapFile* fBuildMoMapFile( b32 freshlyLoaded ) const;
	};

}}

#endif//__Momap__
