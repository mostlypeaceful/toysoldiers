#include "BasePch.hpp"
#include "tTintStack.hpp"

using namespace Sig::Math;

namespace Sig { namespace Gfx
{

	const f32 tTintEntry::cActiveThreshold = 0.05f;
	const f32 tTintEntry::cFullStrengthThreshold = 0.95f;

	tTintStack::tTintStack( )
		: mChanged( false )
		, mForceChanged( false )
		, mCurrentTint( tVec4f::cOnesVector )
	{ }

	void tTintStack::fStep( f32 dt )
	{
		mChanged = mForceChanged;
		mCurrentTint = tVec4f::cOnesVector;
		mForceChanged = false;

		f32 blend = 1.f;
		b32 keepBlending = true;
	
		for( s32 i = mStack.fCount( ) - 1; i >= 0; --i )
		{
			const tTintEntryPtr& tint = mStack[ i ];

			//there may be empty items in the stack.
			if( tint )
			{
				if( tint->fStep( dt ) )
					mChanged = true;

				if( keepBlending )
				{
					f32 thisBlend = tint->fBlendStrength( );
					keepBlending = thisBlend < blend;

					if( keepBlending ) blend -= thisBlend;
					else thisBlend = blend;

					mCurrentTint += mStack[ i ]->fCurrentTint( ) * thisBlend;
				}
			}
		}
	}

}}
