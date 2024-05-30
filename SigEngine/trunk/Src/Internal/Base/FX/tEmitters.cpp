#include "BasePch.hpp"
#include "tEmitters.hpp"
#include "tParticle.hpp"
#include "tMeshSystem.hpp"
#include "tParticleSystem.hpp"
#include "tRandom.hpp"

namespace Sig
{
namespace FX
{

	tParticleEmitter::tParticleEmitter( tMersenneGenerator& rand )
		: mRand( rand )
		, mParentLifeDelta( 0.f )
		, mScale( Math::tVec3f::cOnesVector )
		, mTranslation( Math::tVec3f::cZeroVector )
		, mRotation( Math::tQuatf::cIdentity )
	{		
	}
	void tParticleEmitter::fReset( )
	{
	}
	void tPointEmitter::fEmit( Math::tVec3f& direction, Math::tVec3f& position, b32 randInVolume )
	{
		direction	= mRotation.fRotate( mRand.fNormalizedVec3( ) * fScale( ) );
		position	= fTranslation( );
	}
	void tSphereEmitter::fEmit( Math::tVec3f& direction, Math::tVec3f& position, b32 randInVolume )
	{
		f32 o = mRand.fFloatZeroToOne( ) * Math::cPi;
		f32 t = mRand.fFloatZeroToOne( ) * Math::c2Pi;

		const f32 sino = Math::fSin( o );
		const f32 x = sino * Math::fCos( t );
		const f32 y = sino * Math::fSin( t );
		const f32 z = Math::fCos( o );

		Math::tVec3f surface = Math::tVec3f( x, y, z );
		direction = mRotation.fRotate( surface );
		position = fTranslation( ) + mRotation.fRotate( ( randInVolume ? mRand.fFloatZeroToOne( ) : 1.f ) * surface * fScale( ) );
	}
	void tBoxEmitter::fEmit( Math::tVec3f& direction, Math::tVec3f& position, b32 randInVolume )
	{
		f32 x = mRand.fFloatMinusOneToOne( );//Math::fLerp( -1.f, 1.f, mRand.fFloatZeroToOne( ) );
		f32 y = mRand.fFloatMinusOneToOne( );//Math::fLerp( -1.f, 1.f, mRand.fFloatZeroToOne( ) );
		f32 z = mRand.fFloatMinusOneToOne( );//Math::fLerp( -1.f, 1.f, mRand.fFloatZeroToOne( ) );

		Math::tVec3f volume = Math::tVec3f( x, y, z );
		Math::tVec3f surface = volume;

		u32 axis = surface.fMaxAxisMagnitudeIndex( );

		switch( axis )
		{
		case 0:	surface.x = fSign( surface.x );	break;
		case 1:	surface.y = fSign( surface.y );	break;
		case 2:	surface.z = fSign( surface.z );	break;
		}

		direction = mRotation.fRotate( surface );
		position = fTranslation( ) + mRotation.fRotate( ( randInVolume ? volume : surface ) * fScale( ) );
	}
	void tFountainEmitter::fEmit( Math::tVec3f& direction, Math::tVec3f& position, b32 randInVolume )
	{
		Math::tVec3f vec = mRand.fNormalizedVec3( );
		vec.y = 1.f;
		Math::tVec3f nom = vec;
		nom.fNormalize( );

		//Math::tVec3f nom( x, 1.f, z );
		direction = mRotation.fRotate( nom * fScale( ) );
		position = fTranslation( );
		if( randInVolume )
			position += mRand.fFloatZeroToOne( ) * direction;
	}
	void tShockwaveEmitter::fEmit( Math::tVec3f& direction, Math::tVec3f& position, b32 randInVolume )
	{
		Math::tVec3f surface = mRand.fNormalizedVec3( );
		surface.y = 0.f;
		surface.fNormalize( );
		surface = mRotation.fRotate( surface );
		position = fTranslation( ) + ( surface * fScale( ) );		//( surface * fScale( ) + back );
		direction = surface;
	}
	void tCylinderEmitter::fEmit( Math::tVec3f& direction, Math::tVec3f& position, b32 randInVolume )
	{
		f32 o = mRand.fFloatMinusOneToOne( );
		f32 t = mRand.fFloatZeroToOne( ) * Math::c2Pi;

		const f32 volumeFactor = randInVolume ? mRand.fFloatZeroToOne( ) : 1.f;
		const f32 x = volumeFactor * Math::fCos( t );
		const f32 y = o;
		const f32 z = volumeFactor * Math::fSin( t );

		Math::tVec3f surface = Math::tVec3f( x, y, z );
		direction = mRotation.fRotate( surface );
		position = fTranslation( ) + ( direction * fScale( ) );
	}
}}

