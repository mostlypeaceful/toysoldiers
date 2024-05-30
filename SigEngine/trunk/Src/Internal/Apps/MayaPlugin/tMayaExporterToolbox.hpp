#ifndef __tMayaExporterToolbox__
#define __tMayaExporterToolbox__
#include "tExporterToolbox.hpp"
#include "tMayaEvent.hpp"

namespace Sig
{
	class tMayaExporterToolbox : public tExporterToolbox
	{
		tMayaEventPtr mOnMayaIdle;
		tMayaEventPtr mOnMayaSelChanged;
		tMayaEventPtr mSceneOpened;
		tMayaEventPtr mSceneImported;

		u32 mNumGeomObjectsSelected;
		u32 mNumBoneObjectsSelected;
		u32 mNumLightObjectsSelected;
		u32 mNumMaterialsSelected;

	public:
		tMayaExporterToolbox( const tFilePathPtr& iconBmpPath );
		~tMayaExporterToolbox( );
	private:
		void fOnMayaIdle( );
		void fOnMayaSelChanged( );
		void fOnSceneOpened( );
		b32	 fConvertNode( MDagPath& path, MObject& component );
		b32  fCountObject( MDagPath& path, MObject& component );
	};
}

#endif//__tMayaExporterToolbox__
