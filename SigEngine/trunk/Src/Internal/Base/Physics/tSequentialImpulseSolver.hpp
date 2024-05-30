#ifndef __tSequentialImpulseSolver__
#define __tSequentialImpulseSolver__

#include "tPhysicsObject.hpp"
#include "tLogicThreadPool.hpp"

namespace Sig { namespace Physics
{

	class tSequentialImpulseSolver
	{
	public:
		void fSolve( tGrowableArray<tPhysicsObjectPtr>& islands, tPhysicsWorld& world, f32 dt );

	private:
		tGrowableArray<tPhysicsObjectPtr>* mIslandsToSolve;
		f32 mDtInv;

		b32 fProcessIsland( u32 index );

		Threads::tDistributedForLoopCallback fMakeCallback( )
		{
			return make_delegate_memfn(  Threads::tDistributedForLoopCallback, tSequentialImpulseSolver, fProcessIsland );
		}
	};

}}

#endif//__tSequentialImpulseSolver__
