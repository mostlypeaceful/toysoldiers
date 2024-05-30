#ifndef __tContactPointOld__
#define __tContactPointOld__
#include "tShapeEntity.hpp"

namespace Sig { 

	
	namespace PhysicsOld
	{

		class tRigidBody;

		class tContactPoint
		{
		public:
			Math::tVec3f mPoint;
			Math::tVec3f mNormal;
			Math::tVec3f mVelocity;
			f32			 mDepth;

			u32 mUID;

			tShapeEntityPtr mShape; 
			tRigidBody *mRigidBody; //other persons rigid body if it exists

			tContactPoint( const Math::tVec3f& point = Math::tVec3f::cZeroVector
						 , u32 uid = ~0
						 , const Math::tVec3f& normal = Math::tVec3f::cXAxis
						 , f32 depth = 0.f )
						 : mPoint( point )
						 , mUID( uid )
						 , mNormal( normal )
						 , mVelocity( Math::tVec3f::cZeroVector )
						 , mDepth( depth )
						 , mRigidBody( NULL )
			{ }

		};

	}
}

#endif //__tContactPointOld__