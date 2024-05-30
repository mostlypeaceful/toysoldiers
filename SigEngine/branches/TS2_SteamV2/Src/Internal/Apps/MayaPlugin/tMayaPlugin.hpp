#ifndef __tMayaPlugin__
#define __tMayaPlugin__

class MObject;
class MFnPlugin;

namespace Sig
{

	///
	/// \brief Provides a callback style interface for registering maya plugins; this plugin
	/// will be invoked from within the master MayaPlugin dll, allowing you to register custom maya plugin types.
	class iMayaPlugin
	{
	public:

		virtual ~iMayaPlugin( ) { }

		///
		/// \brief Called when the plugin is loaded into Maya. This is where you initialize/create
		/// your custom maya plugins using the Maya API.
		virtual b32		fOnMayaInitialize( MObject& mayaObject, MFnPlugin& mayaPlugin ) = 0;

		///
		/// \brief Called before the plugin is unloaded from Maya. This where you'd shutdown/destroy
		/// any custom maya plugins that you created in fOnMayaInitialize.
		virtual b32		fOnMayaShutdown( MObject& mayaObject, MFnPlugin& mayaPlugin )	= 0;
	};

}


#endif//__tMayaPlugin__
