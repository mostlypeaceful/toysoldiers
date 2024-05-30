//------------------------------------------------------------------------------
// \file tEditableScriptNodeEntity.hpp - 16 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tEditableScriptNodeEntity__
#define __tEditableScriptNodeEntity__
#include "Gfx/tWorldSpaceQuads.hpp"

namespace Sig
{
	class tEditableTileDb;
	class tEditableScriptNodeDef;
	class tEditableScriptNodeEntity;
	typedef tRefCounterPtr< tEditableScriptNodeEntity > tEditableScriptNodeEntityPtr;

	class tools_export tEditableScriptNodeEntity : public tRefCounter, public tUncopyable
	{
		Gfx::tWorldSpaceQuadsPtr	mPanel;
		tResourcePtr				mTextureFile;
		u32							mScriptGuid;

	public:
		tEditableScriptNodeEntity( u32 scriptGuid, const tEditableTileDb* tileDb );

		virtual ~tEditableScriptNodeEntity( );

		Math::tMat3f	fObjectToWorld( ) const { return mPanel->fObjectToWorld( ); }
		u32				fGuid( ) const { return mScriptGuid; }

		void fSetGuid( u32 newGuid, const tEditableTileDb* tileDb );
		void fRefresh( const tEditableScriptNodeDef* thisNodeType, const Math::tVec3f& newPos );

		void fMoveTo( const Math::tVec3f& newPos );

		void fShowPanel( tEntity& parent );
		void fHidePanel( );

	private:
		void fMoveTo( const Math::tMat3f& newXform );
		void fCreateSquare( );
	};
}

#endif //__tEditableScriptNodeEntity__
