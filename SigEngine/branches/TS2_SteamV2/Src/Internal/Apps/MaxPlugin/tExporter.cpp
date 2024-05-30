#include "tExporter.hpp"
#include "MaxClassDesc.hpp"
#include "MaxPlugin.hpp"
#include "ExporterUtil.hpp"
#include "XyzFileUtil.hpp"
#include <algorithm>

namespace Sig { namespace MaxPlugin
{
	static class tExporterClassDesc : public tBaseClassDesc
	{
	public:
		int 			IsPublic() { return TRUE; }
		void *			Create(BOOL loading = FALSE) { return new tExporter( ); }
		const TCHAR *	ClassName() { return TEXT("Sig::exporter"); }
		SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
		Class_ID		ClassID() { return Class_ID(0x4d2a7370, 0x4ebf115a); }
		const TCHAR* 	Category() { return TEXT("Sig"); }
		const TCHAR*	InternalName() { return TEXT("exporter"); }
		HINSTANCE		HInstance() { extern HINSTANCE g_Hinst; return g_Hinst; }
	} gExporterClassDesc;

	DWORD WINAPI ProgressUpdateFunc(LPVOID arg)
	{
		return 0;
	}

	void tExporter::fMakeLinkerRespectCode( )
	{
	}

	const TCHAR* tExporter::g_extensions[tExporter::EXT_COUNT]=
	{
		TEXT( "xyz" ),
	};

	tExporter::tExporter( )
		: m_total_nodes( 0 ), m_current_node( 0 ), m_previous_quiet_mode( false )
	{
	}

	tExporter::~tExporter( )
	{
		fSetQuietModeEnabled( m_previous_quiet_mode );
	}

	int					tExporter::ExtCount( )				{ return EXT_COUNT; }
	const TCHAR *		tExporter::Ext( int i )				{ if ( i < 0 || i >= EXT_COUNT ) return 0; return g_extensions[i]; }
	const TCHAR *		tExporter::LongDesc( )				{ return TEXT( "XAM Exporter" ); }
	const TCHAR *		tExporter::ShortDesc( )				{ return TEXT( "XAM Exporter" ); }
	const TCHAR *		tExporter::AuthorName( )				{ return TEXT( "Max Wagner" ); }
	const TCHAR *		tExporter::CopyrightMessage( )		{ return TEXT( "Copyright 2007 (c) All Rights Reserved" ); }
	const TCHAR *		tExporter::OtherMessage1( )			{ return TEXT( "other message 1" ); }
	const TCHAR *		tExporter::OtherMessage2( )			{ return TEXT( "other message 2" ); }
	unsigned int		tExporter::Version( )				{ return 100; }
	void				tExporter::ShowAbout( HWND hWnd )	{ }
	BOOL				tExporter::SupportsOptions( int ext, DWORD options ) { return FALSE; }
	int					tExporter::DoExport( const TCHAR* path, ExpInterface* ei, Interface* i, BOOL suppressPrompts, DWORD options )
	{
		m_previous_quiet_mode = fGetQuietModeEnabled( );

		fSetQuietModeEnabled( suppressPrompts );

		i->ProgressStart( "Export Progress", TRUE, &ProgressUpdateFunc, this );

		int result = IMPEXP_SUCCESS;

		try
		{
			tXmlNode xyz			= m_output_file.fCreateRoot(		xyz::xyz_string( ) );
			m_instance_tree_root	= xyz.fCreateChild(	m_output_file,	xyz::instances_string( ) );
			m_reference_tree_root	= xyz.fCreateChild(	m_output_file,	xyz::references_string( ) );
			m_materials_tree_root	= xyz.fCreateChild(	m_output_file,	xyz::materials_string( ) );
			m_current_instance_root = m_instance_tree_root;
			m_total_nodes			= fComputeNumINodes( i->GetRootNode( ) );
			export_tree( i, i->GetRootNode( ) );
		}
		catch( tCancelException& )
		{
			// export cancelled
			result = IMPEXP_CANCEL;
		}
		catch( const char* descrip )
		{
			// known exception
			result = IMPEXP_FAIL;
			Log::tOutput( ) << descrip;
		}
		catch( ... )
		{
			// unknown exception
			result = IMPEXP_FAIL;
		}

		if( result == IMPEXP_SUCCESS )
		{
			m_output_file.fSave( path );
		}

		i->ProgressEnd( );

		return result;
	}

	void tExporter::update_progress( Interface* maxIface )
	{
		++m_current_node;

		if( maxIface->GetCancel( ) )
			throw( tCancelException( ) );

		const u32 progress = ( u32 )( 100.f * ( f32 )m_current_node / ( f32 )m_total_nodes );

		maxIface->ProgressUpdate( progress );
	}

	void tExporter::export_tree( Interface* maxIface, INode* node )
	{
		update_progress( maxIface );

		std::string safeNodeName;
		fMakeINodeNameSafeForXml( safeNodeName, node->GetName( ) );

		tXmlNode saved_instance_root = m_current_instance_root;

		if( node != maxIface->GetRootNode( ) )
		{
			m_current_instance_root = m_current_instance_root.fCreateChild( m_output_file, safeNodeName.c_str( ) );

			export_inode( m_current_instance_root, maxIface, node );
		}

		for( u32 ichild = 0; ichild < node->NumberOfChildren( ); ++ichild )
			export_tree( maxIface, node->GetChildNode( ichild ) );

		m_current_instance_root = saved_instance_root;
	}

	void tExporter::export_inode( tXmlNode& output_node, Interface* maxIface, INode* node )
	{
		Log::tOutput( ) << "exporting inode [" << node->GetName( ) << "]";

		std::string referenceName;
		export_reference( referenceName, maxIface, node );

		std::string materialName;
		export_material( materialName, maxIface, node->GetMtl( ) );

		output_node.fGetAttribute( m_output_file, xyz::name_string( ),			node->GetName( ) );
		output_node.fGetAttribute( m_output_file, xyz::references_string( ),	referenceName.c_str( ) );
		output_node.fGetAttribute( m_output_file, xyz::material_string( ),		materialName.c_str( ) );

		ExporterUtil::fExportUserProps( m_output_file, output_node.fCreateChild( m_output_file, xyz::user_props_string( ) ), maxIface, node );
		ExporterUtil::fExportKeyFrames( m_output_file, output_node.fCreateChild( m_output_file, xyz::key_frames_string( ) ), maxIface, node );
	}

	void tExporter::export_reference( std::string& referenceNameOut, Interface* maxIface, INode* node )
	{
		Object* ref = fEvaluateINode( maxIface, node );

		fMakePointerSafeForXml( referenceNameOut, ref );

		if( !ref )
			return;

		std::vector<Object*>::iterator alreadyWritten = std::find( m_refs_written.begin( ), m_refs_written.end( ), ref );
		if( alreadyWritten != m_refs_written.end( ) )
			return; // reference has already been written
		m_refs_written.push_back( ref );

		tXmlNode ref_node = m_reference_tree_root.fCreateChild( m_output_file, referenceNameOut.c_str( ) );
		ref_node.fGetAttribute( m_output_file, xyz::first_instance_string( ), node->GetName( ) );

		b32 handled=false;

		const SClass_ID superClassID = ref->SuperClassID( );
		switch( superClassID )
		{
		case GEOMOBJECT_CLASS_ID:
			handled = export_reference_geometry( ref_node, maxIface, node, ref );
			if( handled )
				ref_node.fGetAttribute( m_output_file, xyz::type_string( ), xyz::geometry_string( ) );
			break;
		case LIGHT_CLASS_ID:
			handled = export_reference_light( ref_node, maxIface, node, ref );
			if( handled )
				ref_node.fGetAttribute( m_output_file, xyz::type_string( ), xyz::light_string( ) );
			break;
		default:
			handled = false;
			break;
		}

		if( !handled )
			ref_node.fGetAttribute( m_output_file, xyz::type_string( ), xyz::unknown_string( ) );

	}

	b32 tExporter::export_reference_geometry( tXmlNode ref_node, Interface* maxIface, INode* node, Object* ref )
	{
		TriObject* triobj = fGetTriobjFromObject( maxIface, ref );
		if( !triobj )
			return false;
		tAutoDeleteTriObject _auto_delete_( triobj, ref );

		Mesh* mesh = &triobj->mesh;

		if( mesh->getNumVerts( ) == 0 || mesh->getNumFaces( ) == 0 )
		{
			Log::tWarning( ) << "The object '" << node->GetName( ) << "' has no vertices (this is illegal).  This object was skipped.";
			return false;
		}

		ExporterUtil::fExportVerts(			m_output_file, ref_node.fCreateChild( m_output_file, xyz::verts_string( ) ),			mesh );
		ExporterUtil::fExportVertTris(		m_output_file, ref_node.fCreateChild( m_output_file, xyz::vert_tris_string( ) ),		mesh );
		ExporterUtil::fExportMtlIds(		m_output_file, ref_node.fCreateChild( m_output_file, xyz::tri_mtl_ids_string( ) ),		mesh );
		ExporterUtil::fExportTriNormals(	m_output_file, ref_node.fCreateChild( m_output_file, xyz::tri_normals_string( ) ),		mesh );
		ExporterUtil::fExportUvws(			m_output_file, ref_node.fCreateChild( m_output_file, xyz::uvws_string( ) ),				mesh );
		ExporterUtil::fExportVertColors(	m_output_file, ref_node.fCreateChild( m_output_file, xyz::vert_colors_string( ) ),		mesh );
		ExporterUtil::fExportVertAlphas(	m_output_file, ref_node.fCreateChild( m_output_file, xyz::vert_alphas_string( ) ),		mesh );
		ExporterUtil::fExportAuxChannels(	m_output_file, ref_node.fCreateChild( m_output_file, xyz::aux_channels_string( ) ),		mesh );
		ExporterUtil::fExportBoneWeights(	m_output_file, ref_node.fCreateChild( m_output_file, xyz::bone_weights_string( ) ),		mesh, node );

		return true;
	}

	b32 tExporter::export_reference_light( tXmlNode ref_node, Interface* maxIface, INode* node, Object* ref )
	{
		LightObject* lightObj = ( LightObject* )ref;

		LightState lightState;
		Interval ival = maxIface->GetAnimRange( );
		lightObj->EvalLightState( maxIface->GetAnimRange( ).Start( ), ival, &lightState );

		// TODO output light state

		return true;
	}

	void tExporter::export_material( std::string& materialNameOut, Interface* maxIface, Mtl* mtl )
	{
		fMakePointerSafeForXml( materialNameOut, mtl );

		if( !mtl )
			return;

		std::vector<Mtl*>::iterator alreadyWritten = std::find( m_mtls_written.begin( ), m_mtls_written.end( ), mtl );
		if( alreadyWritten != m_mtls_written.end( ) )
			return; // material has already been written
		m_mtls_written.push_back( mtl );

		tXmlNode mtl_node = m_materials_tree_root.fCreateChild( m_output_file, materialNameOut.c_str( ) );
		mtl_node.fGetAttribute( m_output_file, xyz::name_string( ), mtl->GetName( ).data( ) );

		// TODO get information about the material (texture maps, derived shader info, etc.)

		if( mtl->IsMultiMtl( ) )
		{
			tXmlNode sub_mtls_node = mtl_node.fCreateChild( m_output_file, xyz::sub_materials_string( ) );

			for( u32 ichild = 0; ichild < mtl->NumSubMtls( ); ++ichild )
			{
				Mtl* subMtl = mtl->GetSubMtl( ichild );
				if( !subMtl )
					continue;

				std::string subMtlName;
				export_material( subMtlName, maxIface, subMtl );

				tXmlNode sub_mtl = sub_mtls_node.fCreateChild( m_output_file, xyz::sub_material_string( ) );
				sub_mtl.fGetAttribute( m_output_file, xyz::mtl_id_string( ),	ichild );
				sub_mtl.fGetAttribute( m_output_file, xyz::mtl_name_string( ),	subMtlName.c_str( ) );
			}
		}
	}

}}

