#ifndef __Anifig__
#define __Anifig__

#include "Math/Math.hpp"

namespace Sig { namespace Anifig
{
	tools_export const char* fGetFileExtension( );
	tools_export b32 fIsAnifigFile( const tFilePathPtr& path );

	struct tObject
	{
		tFilePathPtr	mSgFile;
		tFilePathPtr	mSkelFile;
		Math::tMat3f	mWorldXform;
	};

	class tools_export tFile
	{
	public:
		
		tGrowableArray<tObject> mObjects;

		tFile( );
		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );
	};

}}

#endif//__Anifig__
