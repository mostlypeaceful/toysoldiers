#ifndef __tEmitters__
#define __tEmitters__

namespace Sig
{
	class tMersenneGenerator;

namespace FX
{
	class tParticleSystem;

	enum tEmitterType
	{
		cPoint,
		cSphere,
		cBox,
		cFountain,
		cShockwave,
		cCylinder,
		cEmitterTypeCount,
	};

	class base_export tParticleEmitter : public tRefCounter
	{
		friend class tParticleSystem;

	protected:
		tMersenneGenerator&		mRand;
		f32						mParentLifeDelta;
		Math::tVec3f			mScale;
		Math::tVec3f			mTranslation;
		Math::tQuatf			mRotation;

	public:
		tParticleEmitter( tMersenneGenerator& rand );
		virtual ~tParticleEmitter( ) { }

		void fReset( );
		virtual void fEmit( Math::tVec3f& direction, Math::tVec3f& position, b32 randInVolume ) = 0;
		
		const Math::tVec3f& fScale( ) const { return mScale; }
		void fSetScale( f32 x, f32 y, f32 z ) { mScale = Math::tVec3f( x, y, z ); }
		void fSetScale( const Math::tVec3f& scale ) { mScale = scale; }

		void fSetTranslation( const Math::tVec3f& trans ) { mTranslation = trans; }
		const Math::tVec3f& fTranslation( ) const { return mTranslation; }

		void fSetRotation( const Math::tQuatf& rot ) { mRotation = rot; }
		const Math::tQuatf& fRotation( ) const { return mRotation; }

		void fSetParentSystemDelta( const f32 delta ) { mParentLifeDelta = delta; }
		f32 fGetParentSystemDelta( ) { return mParentLifeDelta; }
	};

	typedef tRefCounterPtr< tParticleEmitter > tParticleEmitterPtr;


	class tPointEmitter : public tParticleEmitter
	{
	public:
		tPointEmitter( tMersenneGenerator& rand )
			: tParticleEmitter( rand )
		{
		}
		virtual void fEmit( Math::tVec3f& direction, Math::tVec3f& position, b32 randInVolume );
	};
	class tSphereEmitter : public tParticleEmitter
	{
	public:
		tSphereEmitter( tMersenneGenerator& rand )
			: tParticleEmitter( rand )
		{
		}
		virtual void fEmit( Math::tVec3f& direction, Math::tVec3f& position, b32 randInVolume );
	};	
	class tBoxEmitter : public tParticleEmitter
	{
	public:
		tBoxEmitter( tMersenneGenerator& rand )
			: tParticleEmitter( rand )
		{
		}
		virtual void fEmit( Math::tVec3f& direction, Math::tVec3f& position, b32 randInVolume );
	};
	class tFountainEmitter : public tParticleEmitter
	{
	public:
		tFountainEmitter( tMersenneGenerator& rand )
			: tParticleEmitter( rand )
		{
		}
		virtual void fEmit( Math::tVec3f& direction, Math::tVec3f& position, b32 randInVolume );
	};
	class tShockwaveEmitter: public tParticleEmitter
	{
	public:
		tShockwaveEmitter( tMersenneGenerator& rand )
			: tParticleEmitter( rand )
		{
		}
		virtual void fEmit( Math::tVec3f& direction, Math::tVec3f& position, b32 randInVolume );
	};
	class tCylinderEmitter : public tParticleEmitter
	{
	public:
		tCylinderEmitter( tMersenneGenerator& rand )
			: tParticleEmitter( rand )
		{
		}
		virtual void fEmit( Math::tVec3f& direction, Math::tVec3f& position, b32 randInVolume );
	};


}}

#endif// __tEmitters__

