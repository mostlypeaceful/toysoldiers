#include "ToolsPch.hpp"
#include "tHlslViewerDialog.hpp"
#include "tWxTextEditor.hpp"

namespace Sig
{
	tHlslViewerDialog::tHlslViewerDialog( wxWindow* parent, const HlslGen::tHlslOutput& hlsl )
		: wxDialog( parent, wxID_ANY, wxString("hlsl") )
		, mNotebook( 0 )
	{
		SetIcon( wxIcon( "appicon" ) );
		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		const u32 notebookFlags = wxAUI_NB_TAB_MOVE | wxAUI_NB_CLOSE_BUTTON | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_WINDOWLIST_BUTTON | wxAUI_NB_SCROLL_BUTTONS;
		mNotebook = new wxAuiNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, notebookFlags );
		sigassert( mNotebook );
		GetSizer( )->Add( mNotebook, 1, wxEXPAND | wxALL );

		//TODO: display cost of each shader in instruction/cycle count

#define addshaders( shaderList ) for(u32 i = 0; i < hlsl.shaderList.fCount( ); ++i) \
									fAddPage( hlsl.shaderList[ i ] );
		addshaders(mStaticVShaders);
		addshaders(mStaticVShaders_Instanced);
		addshaders(mSkinnedVShaders);
		addshaders(mSkinnedVShaders_Instanced);
		addshaders(mColorPShaders);
		addshaders(mColorShadowPShaders);
		fAddPage(hlsl.mDepthOnlyPShader);
		fAddPage(hlsl.mDepthWithAlphaPShader);
		fAddPage(hlsl.mGBufferStaticVShader);
		fAddPage(hlsl.mGBufferStaticVShader_Instanced);
		fAddPage(hlsl.mGBufferSkinnedVShader);
		fAddPage(hlsl.mGBufferSkinnedVShader_Instanced);
		fAddPage(hlsl.mGBufferPShader);
		addshaders(mStaticVShaders_DP);
		addshaders(mStaticVShaders_DP_Instanced);
		addshaders(mSkinnedVShaders_DP);
		addshaders(mSkinnedVShaders_DP_Instanced);
		fAddPage(hlsl.mDepthOnlyPShader_DP);
		fAddPage(hlsl.mDepthWithAlphaPShader_DP);

#undef addshaders

		const wxSize size = wxGetDisplaySize( );
		const wxSize newSize( size.x * 0.9f, size.y * 0.9f );
		SetSize( newSize );
		const wxPoint p( (size.x - newSize.x) / 2, (size.y - newSize.y) / 2 );
		SetPosition( p );

		GetSizer( )->Layout( );
		Layout( );
	}

	void tHlslViewerDialog::fAddPage( const HlslGen::tShaderOutputBase& shader )
	{
		tWxTextEditor* text = new tWxTextEditor( mNotebook );
		text->fConfigureForSquirrel( );
		text->SetText( shader.mHlsl );
		mNotebook->AddPage( text, shader.mName );
	}
}
