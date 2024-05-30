#ifndef __tSprungMass__
#define __tSprungMass__

namespace Sig { namespace Physics
{

	///
	/// Sprung mass - attaches a mass to a coordinate space by a spring.
	///  ideal the mass would be on a limited damper. To cleanly contrain the mass' 
	///  position (via continuous velocity) to within a defined space. (plane and cylinder?)
	///   mass is assumed to be 1.0f;

	class base_export tSprungMass
	{
	public:
		tSprungMass( );

		void fResetParent( const Math::tMat3f& basis );
		void fSetBasis( const Math::tMat3f& basis ) { mBasis = basis; }
		void fSetDamping( f32 p, f32 d ) { mCurrentPosition.fSetPD( p, d ); }
		void fSetMaxAcc( const Math::tVec3f& maxAcc ) { mMaxAcc = maxAcc; }
		void fSetVerticalRange( f32 compress = 0.5f, f32 extend = 0.75f ) { mMaxCompression = compress; mMaxExtension = extend; }
		void fStep( const Math::tMat3f& newParent, f32 dt );
		
		//only vertical position is integrated here
		Math::tVec3f fGetPosition( ) const { return mBasis.fXformPoint( Math::tVec3f(0,mCurrentPosition.fValue( ).y,0) ); }
		Math::tVec3f fGetCenterPosition( ) const { return mBasis.fGetTranslation( ); }

		const Math::tVec3f& fGetHorizontalAcc( ) const { return mCurrentPosition.fValue( ); }

	private:
		Math::tMat3f mPrevParent; //parent transform
		Math::tMat3f mBasis;  //parent to child transform
		Math::tVec3f mPrevVelocity;
		Math::tPDDampedVec3f mCurrentPosition;
		Math::tVec3f mMaxAcc;

		f32 mMaxCompression, mMaxExtension; //along mAxisDirection
		f32 mCompressionSpring, mExtensionSpring; //to return position to zero

	};

	
}}

#endif//__tSprungMass__
