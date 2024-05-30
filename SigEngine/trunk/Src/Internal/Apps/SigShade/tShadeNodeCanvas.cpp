#include "ShadePch.hpp"
#include "tShadeNodeCanvas.hpp"
#include "DerivedShadeNodes.hpp"
#include "tPhongShadeNode.hpp"

namespace Sig
{
	namespace
	{
		struct tSortNodesById
		{
			inline b32 operator( )( const tShadeNodePtr& a, const tShadeNodePtr& b ) const
			{
				return a->fUniqueNodeId( ) < b->fUniqueNodeId( );
			}
		};
	}

	class tCreateNodeContextAction : public tEditorContextAction
	{
	public:

		struct tShadeNodeDesc
		{
			std::string mName;
			u32 mActionId;
			Rtti::tClassId mClassId;

			tShadeNodeDesc( ) { }

			tShadeNodeDesc( const std::string& name, u32 aid, Rtti::tClassId cid )
				: mName( name ), mActionId( aid ), mClassId( cid )
			{
			}
		};

		typedef tGrowableArray<tShadeNodeDesc> tShadeNodeDescList;

		struct tNodeCategory
		{
			wxString mName;
			tShadeNodeDescList mNodeDescs;
		};

		typedef tGrowableArray<tNodeCategory> tNodeCategoryList;
	private:
		tShadeNodeCanvas& mCanvas;
		u32 mBaseId;
		u32 mMaxId;
		u32 mDefaultOutputId;
		tNodeCategoryList mCategories;
	public:
		tCreateNodeContextAction( tShadeNodeCanvas& canvas )
			: mCanvas( canvas )
			, mBaseId( fNextUniqueActionId( ) )
			, mMaxId( mBaseId )
			, mDefaultOutputId( ~0 )
		{
			mCategories.fSetCount( 9 );
			u32 i = 0;

			fCreateFinalOutputs( mCategories[ i++ ] );
			fCreateLightingModels( mCategories[ i++ ] );
			fCreateTextureSamples( mCategories[ i++ ] );
			fCreateBlendOps( mCategories[ i++ ] );
			fCreateColors( mCategories[ i++ ] );
			fCreateNumbers( mCategories[ i++ ] );
			fCreateNormals( mCategories[ i++ ] );
			fCreateUtility( mCategories[ i++ ] );
			fCreateSpatialTests( mCategories[ i++ ] );

			sigassert( i == mCategories.fCount( ) );

			//reserve the rest of the unique ids used
			for( s32 id = 0; id < s32(mMaxId - mBaseId); ++id )
				fNextUniqueActionId( );
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			for( u32 i = 0; i < mCategories.fCount( ); ++i )
			{
				const tNodeCategory& category = mCategories[ i ];
				wxMenu* subMenu = new wxMenu;
				menu.AppendSubMenu( subMenu, category.mName );
				for( u32 j = 0; j < category.mNodeDescs.fCount( ); ++j )
				{
					const u32 actionId = category.mNodeDescs[ j ].mActionId;
					wxMenuItem* menuItem = subMenu->Append( actionId, category.mNodeDescs[ j ].mName );
					if( mDefaultOutputId == actionId && mCanvas.fOutputs( )[ tShadeNode::cOutputColor0 ] )
						menuItem->Enable( false );
				}
			}
			return true;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId < mBaseId || actionId >= mMaxId )
				return false;
			const tShadeNodeDesc* desc = fFindNodeDesc( actionId );
			if( !desc )
				return false;

			const wxPoint p = mCanvas.fScreenToAbsolute( fRightClickPos() );
			fAddNode( *desc, p );
			return true;
		}
		void fAddDefaultOutputNode( const wxPoint& p = wxPoint( 120, 120 ) )
		{
			const tShadeNodeDesc* desc = fFindNodeDesc( mDefaultOutputId );
			if( !desc )
				return;
			fAddNode( *desc, p ); // add default output node by default
		}
	private:
		void fAddNode( const tShadeNodeDesc& desc, const wxPoint& p )
		{
			tDAGNodePtr dagNode = tDAGNodePtr( ( tShadeNode* )Rtti::fNewClass( desc.mClassId ) );
			dagNode->fSetPosition( p );
			mCanvas.fEditorActions( ).fAddAction( tEditorActionPtr( new tAddDeleteDAGNodeAction( mCanvas, dagNode, true ) ) );
		}
		const tShadeNodeDesc* fFindNodeDesc( u32 actionId ) const
		{
			if( mDefaultOutputId == actionId && mCanvas.fOutputs( )[ tShadeNode::cOutputColor0 ] )
				return 0; // don't allow multiple outputs

			for( u32 i = 0; i < mCategories.fCount( ); ++i )
			{
				const tNodeCategory& category = mCategories[ i ];
				for( u32 j = 0; j < category.mNodeDescs.fCount( ); ++j )
				{
					if( category.mNodeDescs[ j ].mActionId == actionId )
						return &category.mNodeDescs[ j ];
				}
			}
			return 0;
		}
		void fCreateFinalOutputs( tNodeCategory& category )
		{
			category.mName = "FinalOutputs";

			mDefaultOutputId = mMaxId;
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "DefaultOutput", mMaxId++, Rtti::fGetClassId<tDefaultOutputShadeNode>( ) ) );
		}
		void fCreateLightingModels( tNodeCategory& category )
		{
			category.mName = "LightingModels";

			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Phong", mMaxId++, Rtti::fGetClassId<tPhongShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "FlatParticle", mMaxId++, Rtti::fGetClassId<tFlatParticleShadeNode>( ) ) );
		}
		void fCreateTextureSamples( tNodeCategory& category )
		{
			category.mName = "TextureMapping";

			category.mNodeDescs.fPushBack( tShadeNodeDesc( "UvChannel", mMaxId++, Rtti::fGetClassId<tUvChannelShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "ColorMap", mMaxId++, Rtti::fGetClassId<tColorMapShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "NormalMap", mMaxId++, Rtti::fGetClassId<tNormalMapShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "ColorMapAtlas", mMaxId++, Rtti::fGetClassId<tColorMapAtlasShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "CubeMap", mMaxId++, Rtti::fGetClassId<tCubeMapShadeNode>( ) ) );
		}
		void fCreateBlendOps( tNodeCategory& category )
		{
			category.mName = "BlendOps";

			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Add", mMaxId++, Rtti::fGetClassId<tAddShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Subtract", mMaxId++, Rtti::fGetClassId<tSubtractShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Multiply", mMaxId++, Rtti::fGetClassId<tMultiplyShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Divide", mMaxId++, Rtti::fGetClassId<tDivideShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Pow", mMaxId++, Rtti::fGetClassId<tPowShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "OneMinus", mMaxId++, Rtti::fGetClassId<tOneMinusShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Blend", mMaxId++, Rtti::fGetClassId<tBlendShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Dot", mMaxId++, Rtti::fGetClassId<tDotShadeNode>( ) ) );
		}
		void fCreateColors( tNodeCategory& category )
		{
			category.mName = "Colors";

			category.mNodeDescs.fPushBack( tShadeNodeDesc( "ColorRGB", mMaxId++, Rtti::fGetClassId<tColorRGBShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "ColorRGBA", mMaxId++, Rtti::fGetClassId<tColorRGBAShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "VertexColor", mMaxId++, Rtti::fGetClassId<tVertexColorShadeNode>( ) ) );
		}
		void fCreateNumbers( tNodeCategory& category )
		{
			category.mName = "Numbers";

			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Number", mMaxId++, Rtti::fGetClassId<tNumberShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Vector2", mMaxId++, Rtti::fGetClassId<tVector2ShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Vector3", mMaxId++, Rtti::fGetClassId<tVector3ShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Vector4", mMaxId++, Rtti::fGetClassId<tVector4ShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "DynamicValue", mMaxId++, Rtti::fGetClassId<tDynamicVec4ShadeNode>( ) ) );
		}
		void fCreateNormals( tNodeCategory& category )
		{
			category.mName = "Normals Etc.";

			category.mNodeDescs.fPushBack( tShadeNodeDesc( "GeometryNormal", mMaxId++, Rtti::fGetClassId<tGeometryNormalShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "PerPixelNormal", mMaxId++, Rtti::fGetClassId<tPerPixelNormalShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "ReflectionVec", mMaxId++, Rtti::fGetClassId<tReflectionVectorShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "WorldSpacePos", mMaxId++, Rtti::fGetClassId<tWorldSpacePosShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "EdgeDetect", mMaxId++, Rtti::fGetClassId<tEdgeShadeNode>( ) ) );
		}
		void fCreateUtility( tNodeCategory& category )
		{
			category.mName = "Utility";

			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Swizzle", mMaxId++, Rtti::fGetClassId<tSwizzleShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Saturate", mMaxId++, Rtti::fGetClassId<tSaturateShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Clamp", mMaxId++, Rtti::fGetClassId<tClampShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Threshold", mMaxId++, Rtti::fGetClassId<tThresholdShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Pulse", mMaxId++, Rtti::fGetClassId<tPulseShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Scroll", mMaxId++, Rtti::fGetClassId<tScrollShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Frac", mMaxId++, Rtti::fGetClassId<tFracShadeNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "Trunc", mMaxId++, Rtti::fGetClassId<tTruncShadeNode>( ) ) );
		}

		void fCreateSpatialTests( tNodeCategory & category )
		{
			category.mName = "Spatial Tests";

			category.mNodeDescs.fPushBack( tShadeNodeDesc( "TransitionObjects", mMaxId++, Rtti::fGetClassId<tTransitionObjectsNode>( ) ) );
			category.mNodeDescs.fPushBack( tShadeNodeDesc( "SplitPlane", mMaxId++, Rtti::fGetClassId<tSplitPlaneNode>( ) ) );

		}
	};
}

namespace Sig
{
	namespace
	{
		inline tShadeNode::tOutputSemantic fOutputSemantic( const tDAGNodePtr& dagNode )
		{
			tShadeNode* asShadeNode = dynamic_cast<tShadeNode*>( dagNode.fGetRawPtr( ) );
			if( !asShadeNode )
				return tShadeNode::cOutputNotAnOutput;
			return asShadeNode->fOutputSemantic( );
		}
		inline void fResetOutput( tShadeNodeCanvas::tOutputArray& outputs, const tDAGNodePtr& dagNode )
		{
			tShadeNode* asShadeNode = dynamic_cast<tShadeNode*>( dagNode.fGetRawPtr( ) );
			if( !asShadeNode )
				return ;
			return outputs[ asShadeNode->fOutputSemantic( ) ].fReset( asShadeNode );
		}
	}


	tShadeNodeCanvas::tShadeNodeCanvas( wxWindow* parent )
		: tDAGNodeCanvas( parent )
		, mNodeCreator( new tCreateNodeContextAction( *this ) )
	{
		mRenderOutputStyle = tDAGNode::cRenderOutputsBeforeInputs;
		mPreventCycles = true;
		mContextActions.fPushBack( tEditorContextActionPtr( mNodeCreator ) );
	}
	void tShadeNodeCanvas::fAddNode( const tDAGNodePtr& shadeNode )
	{
		const tShadeNode::tOutputSemantic os = fOutputSemantic( shadeNode );
		if( os < tShadeNode::cOutputCount )
		{
			if( mOutputs[ os ] )
				return; // can't place two outputs of the same type in a file
			else
				fResetOutput( mOutputs, shadeNode );
		}

		tDAGNodeCanvas::fAddNode( shadeNode );
	}
	void tShadeNodeCanvas::fDeleteNode( const tDAGNodePtr& shadeNode )
	{
		const tShadeNode::tOutputSemantic os = fOutputSemantic( shadeNode );
		if( os < tShadeNode::cOutputCount && mOutputs[ os ] == shadeNode )
			mOutputs[ os ].fRelease( );

		tDAGNodeCanvas::fDeleteNode( shadeNode );
	}
	void tShadeNodeCanvas::fReset( const tDAGNodeList& nodes, const tDAGNodeConnectionList& connections, b32 addTo )
	{
		if( !addTo )
			fClearOutputs( );
		else
		{
			// Give all the nodes added to the scene new unique ids.
			// Ensures adds don't break uniqueness contract.
			for( u32 i = 0; i < nodes.fCount( ); ++i )
			{
				tShadeNode* newNode = dynamic_cast< tShadeNode* >( nodes[ i ].fGetRawPtr( ) );
				newNode->fGrabNextUniqueNodeId( );
			}
		}

		for( u32 i = 0; i < nodes.fCount( ); ++i )
		{
			const tShadeNode::tOutputSemantic os = fOutputSemantic( nodes[ i ] );
			if( os < tShadeNode::cOutputCount )
			{
				sigassert( !mOutputs[ os ] );
				fResetOutput( mOutputs, nodes[ i ] );
			}
		}

		tDAGNodeCanvas::fReset( nodes, connections, addTo );
	}
	void tShadeNodeCanvas::fClearCanvas( )
	{
		fClearOutputs( );
		tDAGNodeCanvas::fClearCanvas( );
		tShadeNode::fSetNextUniqueNodeId( 1 );
	}
	void tShadeNodeCanvas::fAddDefaultOutputNode( const wxPoint& p )
	{
		wxPoint realP;
		if( p.x < 0 || p.y < 0 )
		{
			const wxSize s = GetSize( );
			realP.x = s.x / 2 - 60;
			realP.y = s.y / 2 - 60;
		}
		else
			realP = p;

		mNodeCreator->fAddDefaultOutputNode( realP );

		fFrame( );
	}

	namespace
	{
		static const tFilePathPtr cScratchFilePath = ToolsPaths::fCreateTempEngineFilePath( ".derml", tFilePathPtr("shade"), "scratch" );

		class tPasteNodesAction : public tEditorAction
		{
			tShadeNodeCanvas& mCanvas;
			tDAGNodeCanvas::tDAGNodeList mPrevSelected;
			tDAGNodeCanvas::tDAGNodeList mPrevNodes, mNewNodes;
			tDAGNodeCanvas::tDAGNodeConnectionList mPrevConnections, mNewConnections;
		public:
			tPasteNodesAction( tShadeNodeCanvas& canvas, Derml::tFile& clipboard )
				: mCanvas( canvas )
				, mPrevSelected( canvas.fSelectedNodes( ) )
				, mPrevNodes( canvas.fAllNodes( ) )
				, mNewNodes( clipboard.mNodes.fCount( ) )
				, mPrevConnections( canvas.fAllConnections( ) )
				, mNewConnections( clipboard.mConnections.fCount( ) )
			{
				sigassert( clipboard.mNodes.fCount( ) > 0 );
				for( u32 i = 0; i < mNewNodes.fCount( ); ++i )
				{
					mNewNodes[ i ].fReset( clipboard.mNodes[ i ].fGetRawPtr( ) );
					mNewNodes[ i ]->fSetOwner( &mCanvas );
				}

				for( u32 i = 0; i < mNewConnections.fCount( ); ++i )
				{
					const Derml::tConnection& connec = clipboard.mConnections[ i ];
					mNewConnections[ i ].fReset( new tDAGNodeConnection( mNewNodes[ connec.mInput ]->fInput( connec.mIndex ), mNewNodes[ connec.mOutput ]->fOutput( 0 ) ) );
				}
				fRedo( );
			}
			virtual void fUndo( )
			{
				mCanvas.fReset( mPrevNodes, mPrevConnections );
				mCanvas.fSetSelectedNodes( mPrevSelected );
			}
			virtual void fRedo( )
			{
				mCanvas.fClearSelection( true, true, false );
				mCanvas.fReset( mNewNodes, mNewConnections, true );
				mCanvas.fSetSelectedNodes( mNewNodes );
			}
		};
	}	
	
	void tShadeNodeCanvas::fCopy( )
	{
		mClipboard = Derml::tFile( );
		fToDermlFile( mClipboard, true, true );
		mClipboard.fSaveXml( cScratchFilePath, false );
	}

	void tShadeNodeCanvas::fPaste( )
	{
		if( !mClipboard.fLoadXml( cScratchFilePath ) )
			return;

		for( u32 i = 0; i < mClipboard.mNodes.fCount( ); ++i )
			mClipboard.mNodes[ i ]->fMove( wxPoint( 8, 8 ) );

		// save again so that if they paste again, they're in the shifted position
		mClipboard.fSaveXml( cScratchFilePath, false );

		if( mClipboard.mNodes.fCount( ) > 0 )
			fEditorActions( ).fAddAction( tEditorActionPtr( new tPasteNodesAction( *this, mClipboard ) ) );
	}

	void tShadeNodeCanvas::fToDermlFile( Derml::tFile& file, b32 selectedOnly, b32 ignoreOutputs )
	{
		for( u32 i = 0; i < mDAGNodes.fCount( ); ++i )
		{
			tShadeNode* shadeNode = dynamic_cast< tShadeNode* >( mDAGNodes[ i ].fGetRawPtr( ) );
			if( !shadeNode )
				continue;
			if( selectedOnly && !mSelectedNodes.fFind( mDAGNodes[ i ] ) )
				continue;
			if( ignoreOutputs && shadeNode->fOutputSemantic( ) < tShadeNode::cOutputCount )
				continue;
			file.mNodes.fPushBack( tShadeNodePtr( shadeNode ) );
		}

		// Sort the nodes by id so the files are more consistent
		std::sort( file.mNodes.fBegin( ), file.mNodes.fEnd( ), tSortNodesById( ) );

		for( u32 i = 0; i < mConnections.fCount( ); ++i )
		{
			tDAGNodeInputPtr input = mConnections[ i ]->fInput( );
			tDAGNodeOutputPtr output = mConnections[ i ]->fOutput( );
			if( !input || !output )
				continue;
			tShadeNodePtr* findInput = file.mNodes.fFind( dynamic_cast<const tShadeNode*>( &input->fOwner( ) ) );
			tShadeNodePtr* findOutput = file.mNodes.fFind( dynamic_cast<const tShadeNode*>( &output->fOwner( ) ) );
			if( !findInput || !findOutput )
				continue;
			if( selectedOnly && ( !mSelectedNodes.fFind( &input->fOwner( ) ) || !mSelectedNodes.fFind( &output->fOwner( ) ) ) )
				continue;

			file.mConnections.fPushBack( Derml::tConnection( fPtrDiff( findInput, file.mNodes.fBegin( ) ), input->fIndex( ), fPtrDiff( findOutput, file.mNodes.fBegin( ) ) ) );
		}

		file.mNextUniqueNodeId = tShadeNode::fNextUniqueNodeId( );
		sigassert( !file.mNodes.fCount( ) || file.mNodes.fBack( )->fUniqueNodeId( ) < file.mNextUniqueNodeId );
	}

	void tShadeNodeCanvas::fFromDermlFile( const Derml::tFile& file )
	{
		mDAGNodes.fSetCount( file.mNodes.fCount( ) );
		for( u32 i = 0; i < mDAGNodes.fCount( ); ++i )
		{
			const tShadeNode::tOutputSemantic os = file.mNodes[ i ]->fOutputSemantic( );
			if( os < tShadeNode::cOutputCount )
			{
				sigassert( !mOutputs[ os ] );
				mOutputs[ os ] = file.mNodes[ i ];
			}

			mDAGNodes[ i ].fReset( file.mNodes[ i ].fGetRawPtr( ) );
			mDAGNodes[ i ]->fSetOwner( this );
		}

		mConnections.fSetCount( file.mConnections.fCount( ) );
		for( u32 i = 0; i < mConnections.fCount( ); ++i )
		{
			const Derml::tConnection& connec = file.mConnections[ i ];
			mConnections[ i ].fReset( new tDAGNodeConnection( mDAGNodes[ connec.mInput ]->fInput( connec.mIndex ), mDAGNodes[ connec.mOutput ]->fOutput( 0 ) ) );
		}

		fClearSelection( true, true, false );

		if( file.mNextUniqueNodeId > 0 )
			tShadeNode::fSetNextUniqueNodeId( file.mNextUniqueNodeId );

		Refresh( );
	}

	void tShadeNodeCanvas::fClearOutputs( )
	{
		for( u32 i = 0; i < mOutputs.fCount( ); ++i )
			mOutputs[ i ].fRelease( );
	}

}
