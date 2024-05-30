#ifndef __tContactPoint__
#define __tContactPoint__

namespace Sig { 
	
	namespace Physics
	{
		class tContactPoint
		{
		public:
			Math::tVec3f mPoint;
			Math::tVec3f mNormal;
			f32			 mDepth;

			tContactPoint( const Math::tVec3f& point = Math::tVec3f::cZeroVector
						 , const Math::tVec3f& normal = Math::tVec3f::cXAxis
						 , f32 depth = 0.f )
						 : mPoint( point )
						 , mNormal( normal )
						 , mDepth( depth )
			{ }

			void fFlip( );

			// The physics requires the default operation, but for outside
			// physics the opposite normal is needed to compute the other body pt
			Math::tVec3f fOtherBodyPt( ) const;
			void fRender( const Math::tVec4f& color = Math::tVec4f( 0,0,1,1 ) ) const;
		};

		class tRigidBody;

		struct tFullContactWitness
		{
			tRigidBody* mA;			//Vertex body
			tRigidBody* mB;			//Face body
			f32			mDepth;		//Distance along normal from vertex to face. Negative for separation.
			Math::tVec3f mPoint;	// Vertex location, on A for edge collision.
			Math::tVec3f mNormal;	// Outwards face normal, or B to A for edge.
			Math::tVec3f mEa, mEb;	// Edges for edge collision only.
			b32			mEdgeCollision;

			tFullContactWitness( )
				: mA( NULL )
				, mB( NULL )
				, mEdgeCollision( false )
			{ }

			// Bodies have moved:
			void fReCompute( );

			static b32 fClosestPointOnEdges( const Math::tVec3f& oA, const Math::tVec3f& dirA, const Math::tVec3f& oB, const Math::tVec3f& dirB, const Math::tVec3f& sepNormal, Math::tVec3f& output );
		};

	}
}

#endif //__tContactPoint__