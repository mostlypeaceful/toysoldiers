#ifndef __iSigEdPlugin__
#define __iSigEdPlugin__

namespace Sig
{

	///
	/// \brief SigEd plugin entry point.

	class tEditorAppWindow;
	class tEditorPluginData;
	class tEntityData;

	class tools_export iSigEdPlugin
	{
	public:
		virtual ~iSigEdPlugin( ) { }

		// Called once when plugin is loaded.
		virtual void fConstruct( tEditorAppWindow* editor ) { }

		virtual std::string fName( ) const { return "ReplaceWithName"; }

		// This must be unique from plugin to plugin. 
		virtual u32 fUniqueID( ) const { sigassert( !"Every plugin must generate a unique ID, use RUIDGen." ); return ~0; }

		// When the user clicks the plugin menu option.
		virtual void fToggle( ) { }

		// Called for each tEditorPluginData found with this fUniqueID( )
		virtual tEntityData* fSerializeData( tLoadInPlaceFileBase* fileOut, tEditorPluginData* baseDataPtr ) { return NULL; }

		// Refresh dialogs and stuff.
		virtual void fFileOpened( ) { }
	};

	class tXmlSerializer;
	class tXmlDeserializer;

	///
	/// \brief iSigEdPlugin can use this structure to store data on entities in the Sigml.
	class tools_export tEditorPluginData : public Rtti::tSerializableBaseClass, public tRefCounter
	{
		declare_null_reflector( );
		implement_rtti_serializable_base_class( tEditorPluginData, 0x516D15D );
	public:
		tEditorPluginData( u32 pluginID ) : mPluginID( pluginID ) { }
		virtual ~tEditorPluginData( ) { }

		virtual void fSerialize( tXmlSerializer& s ) { }
		virtual void fSerialize( tXmlDeserializer& s ) { }
		
		virtual tEditorPluginData* fClone( ) const = 0;

		// The plugin who created this data.
		u32 mPluginID;
	};

	typedef tRefCounterPtr< tEditorPluginData > tEditorPluginDataPtr;

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tEditorPluginData& o )
	{
		s( "Plugin", o.mPluginID );
		o.fSerialize( s );
	}

	class tools_export tEditorPluginDataContainer : public tGrowableArray<tEditorPluginDataPtr>
	{
	public:
		void fUnion( const tEditorPluginDataContainer& other );

		// Assignment operator clones other.
		tEditorPluginDataContainer& operator = ( const tEditorPluginDataContainer& other );
		tEditorPluginDataContainer fClone( ) const;
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tEditorPluginDataContainer& o )
	{
		// This class should ideally not add any more data.
		// If you need to add more data you'll need to sub child it here
		//  adnd convert all the existing assets' xml.
		fSerializeXmlObject( s, (tGrowableArray<tEditorPluginDataPtr>&)o );
	}

}


#endif//__iSigEdPlugin__
