#ifndef __exporter_hpp__
#define __exporter_hpp__
#include "MaxInclude.hpp"
#include "tXmlFile.hpp"

namespace Sig { namespace MaxPlugin
{

	class tExporter : public SceneExport
	{
	public:

		static void fMakeLinkerRespectCode( );

		tExporter( );
		~tExporter( );

		// overrides from SceneExport
		virtual int					ExtCount( );
		virtual const TCHAR *		Ext( int i );
		virtual const TCHAR *		LongDesc( );
		virtual const TCHAR *		ShortDesc( );
		virtual const TCHAR *		AuthorName( );
		virtual const TCHAR *		CopyrightMessage( );
		virtual const TCHAR *		OtherMessage1( );
		virtual const TCHAR *		OtherMessage2( );
		virtual unsigned int		Version( );
		virtual void				ShowAbout( HWND hWnd );
		virtual BOOL				SupportsOptions( int ext, DWORD options );
		virtual int					DoExport( const TCHAR* name, ExpInterface* ei, Interface* i, BOOL suppressPrompts=FALSE, DWORD options=0 );

	private:

		void update_progress( Interface* maxIface );
		void export_tree( Interface* maxIface, INode* node );
		void export_inode( tXmlNode& output_node, Interface* maxIface, INode* node );
		void export_reference( std::string& referenceNameOut, Interface* maxIface, INode* node );
		b32	 export_reference_geometry( tXmlNode ref_node, Interface* maxIface, INode* node, Object* ref );
		b32	 export_reference_light( tXmlNode ref_node, Interface* maxIface, INode* node, Object* ref );
		void export_material( std::string& materialNameOut, Interface* maxIface, Mtl* mtl );

		enum ext_types
		{
			EXT_XAM,
			EXT_COUNT,
		};

		static const TCHAR* g_extensions[EXT_COUNT];

		u32			m_total_nodes;
		u32			m_current_node;
		b32			m_previous_quiet_mode;
		tXmlFile	m_output_file;
		tXmlNode	m_instance_tree_root;
		tXmlNode	m_reference_tree_root;
		tXmlNode	m_materials_tree_root;
		tXmlNode	m_current_instance_root;

		std::vector<Object*>	m_refs_written;
		std::vector<Mtl*>		m_mtls_written;

	};


}}


#endif//__exporter_hpp__

