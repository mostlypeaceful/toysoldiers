#ifndef __tFixedBody__
#define __tFixedBody__

#include "tPhysicsBody.hpp"

namespace Sig { namespace Physics
{

	class tFixedBody : public tPhysicsBody
	{
		debug_watch( tFixedBody );
		declare_uncopyable( tFixedBody );
		define_dynamic_cast( tFixedBody, tPhysicsBody );
	public:
		tFixedBody( )
			: tPhysicsBody( cPhysicsObjectTypeFixed )
		{ }

		void fSetTransform( const Math::tMat3f& xform ) 
		{ 
			tPhysicsBody::fSetTransform( xform );
			fUpdateBroadphase( Math::tVec3f::cZeroVector );
		}
	};

}}

#endif//__tFixedBody__
