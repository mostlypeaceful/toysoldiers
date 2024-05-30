//------------------------------------------------------------------------------
// \file tScriptedControl.hpp - 10 Aug 2010
// \author Max Wagner
// \author Josh Wittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tScriptedControl__
#define __tScriptedControl__
#include "tCanvas.hpp"

// need to include this for resource name translation (.nut => .nutb)
#include "Scripts/tScriptFile.hpp"

namespace Sig { namespace Gui
{
	//------------------------------------------------------------------------------
	// \class tScriptedControl
	// \brief A simple wrapper around a resource and canvas
	//------------------------------------------------------------------------------
	class tScriptedControl : public tRefCounter
	{
	public:

		explicit tScriptedControl( const tResourcePtr& scriptResource );
		virtual ~tScriptedControl( );

		const tResourcePtr& fScriptResource( ) const { return mScriptResource; }
		b32 fScriptLoadCompleted( ) const 
		{ 
			return mScriptResource.fNull() || !mScriptResource->fLoading( ); 
		}

		tCanvasPtr& fCanvas( ) { return mCanvas; }
		const tCanvasPtr& fCanvas( ) const { return mCanvas; }
		void fReleaseCanvas( ) { mCanvas = tCanvasPtr( ); }

		template<class tDerivedType>
		b32 fCreateControlFromScript( const tStringPtr& entryPointName, tDerivedType * obj ) {
			
			// Query the entry function
			Sqrat::Function entry;
			b32 success = fQueryEntryPoint( entryPointName, entry );
			
			// Failed to find the entry point
			if (!success) {
				fUnloadAndReleaseScript( );
				mCanvas = tCanvasPtr( );
				return false;
			}

			// Evaluate the entry point
			mCanvas = tCanvasPtr( entry.Evaluate<Sqrat::Object>( obj ) );

			// Failed to build the canvas
			if ( mCanvas.fIsNull( ) ) {
				fUnloadAndReleaseScript( );
				return false;
			}

			return true;
		}

	protected:
 
		void fAttachCanvasToFrame( Gui::tCanvasFrame& frame );
		b32 fQueryEntryPoint( const tStringPtr& entryPointName, Sqrat::Function & entryPoint );
		void fUnloadAndReleaseScript( );

	protected:
		tResourcePtr	mScriptResource;
		tCanvasPtr		mCanvas;
		b32				mLetCanvasPersist;
	};

	typedef tRefCounterPtr< tScriptedControl > tScriptedControlPtr;

}}

#endif//__tScriptedControl__
