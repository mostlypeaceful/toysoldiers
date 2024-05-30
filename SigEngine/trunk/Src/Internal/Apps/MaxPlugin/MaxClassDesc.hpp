#ifndef __MaxClassDesc_hpp__
#define __MaxClassDesc_hpp__
#include "MaxInclude.hpp"

namespace Sig { namespace MaxPlugin
{

	class tBaseClassDesc : public ClassDesc2
	{
	public:

		static int				fNumberOfClasses( );
		static ClassDesc*		fGetClassDesc( int i );

		tBaseClassDesc( );
		~tBaseClassDesc( );
	};

}}


#endif//__MaxClassDesc_hpp__

