#include "BasePch.hpp"
#include "tQuaternion.hpp"

#include "cml/cml.h"

namespace Sig { namespace Math
{
	
	namespace
	{		
		cml::matrix33f_c fToCML( const tMat3f& mat )
		{
			tVec3f aX = mat.fXAxis( );
			tVec3f aY = mat.fYAxis( );
			tVec3f aZ = mat.fZAxis( );
			return cml::matrix33f_c( aX.x, aX.y, aX.z, aY.x, aY.y, aY.z, aZ.x, aZ.y, aZ.z );
		}

		tMat3f fToMat3( const cml::matrix33f_c& mat )
		{
			return tMat3f( tVec3f( mat( 0, 0 ), mat( 0, 1 ), mat( 0, 2 ) )
				, tVec3f( mat( 1, 0 ), mat( 1, 1 ), mat( 1, 2 ) )
				, tVec3f( mat( 2, 0 ), mat( 2, 1 ), mat( 2, 2 ) )
				, tVec3f::cZeroVector );
		}
	}


	tEulerAnglesf fCMLDecompose( u32 order, const tQuatf& delta )
	{
		tMat3f rot( delta );
		cml::matrix33f_c m = fToCML( rot );

		tEulerAnglesf ea;
		cml::matrix_to_euler( m, ea.x, ea.y, ea.z, (cml::EulerOrder)order );

		return ea;
	}

	tQuatf fCMLCompose( u32 order, const tEulerAnglesf& ea )
	{
		cml::matrix33f_c m;
		cml::matrix_rotation_euler( m, ea.x, ea.y, ea.z, (cml::EulerOrder)order );

		tMat3f rot = fToMat3( m );
		return tQuatf( rot );
	}

}}