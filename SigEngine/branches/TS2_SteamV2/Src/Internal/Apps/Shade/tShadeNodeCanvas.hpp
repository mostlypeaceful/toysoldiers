#ifndef __tShadeNodeCanvas__
#define __tShadeNodeCanvas__
#include "tDAGNodeCanvas.hpp"
#include "Derml.hpp"

namespace Sig
{
	class tCreateNodeContextAction;

	class tShadeNodeCanvas : public tDAGNodeCanvas
	{
	public:
		typedef tShadeNode::tOutputArray tOutputArray;
	private:
		tCreateNodeContextAction* mNodeCreator;
		Derml::tFile mClipboard;
		tOutputArray mOutputs;
	public:
		explicit tShadeNodeCanvas( wxWindow* parent );
		virtual void fAddNode( const tDAGNodePtr& shadeNode );
		virtual void fDeleteNode( const tDAGNodePtr& shadeNode );
		virtual void fReset( const tDAGNodeList& nodes, const tDAGNodeConnectionList& connections, b32 addTo = false );
		virtual void fClearCanvas( );
		const tOutputArray& fOutputs( ) const { return mOutputs; }
		void fAddDefaultOutputNode( const wxPoint& p = wxPoint( -1, -1 ) );
		void fCopy( );
		void fPaste( );
		void fToDermlFile( Derml::tFile& file, b32 selectedOnly = false, b32 ignoreOutputs = false );
		void fFromDermlFile( const Derml::tFile& file, b32 addToScene = false );
	private:
		b32  fPrepareClipboardForNextPaste( );
		void fClearOutputs( );
	};
}

#endif//__tShadeNodeCanvas__
