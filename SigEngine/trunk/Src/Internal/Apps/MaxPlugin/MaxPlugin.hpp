#ifndef __MaxPlugin_hpp__
#define __MaxPlugin_hpp__
#include "MaxInclude.hpp"
#include <string>

namespace Sig { namespace MaxPlugin
{
	class tAutoDeleteTriObject : public tUncopyable
	{
	public:
		tAutoDeleteTriObject( TriObject* triObj, Object* object )
			: mTriObj(triObj), mObject(object) { }
		~tAutoDeleteTriObject( ) { if( mTriObj!=mObject ) mTriObj->DeleteMe( ); }
	private:
		TriObject*		mTriObj;
		Object*			mObject;
	};

	class tCancelException
	{
	};

	b32				fGetQuietModeEnabled( );
	void			fSetQuietModeEnabled( b32 enabled );
	f32				fToSeconds( TimeValue t );
	TimeValue		fToTimeValue( f32 seconds );
	u32				fComputeNumINodes( INode* root );
	INode*			fFindINode( Interface* maxIface, INode* root, const char* nodeName );
	Object*			fEvaluateINode( Interface* maxIface, INode* node, TimeValue t=(TimeValue)-1 );
	TriObject*		fGetTriobjFromObject( Interface* maxIface, Object* object, TimeValue t=(TimeValue)-1 );
	void			fMakeINodeNameSafeForXml( std::string& out, const char* nodeName );
	void			fMakePointerSafeForXml( std::string& out, void* address );

}}


#endif//__MaxPlugin_hpp__

