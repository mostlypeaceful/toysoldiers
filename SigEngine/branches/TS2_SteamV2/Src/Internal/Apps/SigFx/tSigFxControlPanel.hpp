#ifndef __tSigFxControlPanel__
#define __tSigFxControlPanel__
#include "tSGFileRefEntity.hpp"

namespace Sig
{
	class tSigFxMainWindow;

	class tSigFxControlPanel : public wxScrolledWindow, public Win32Util::tRegistrySerializer
	{
		// main window
		tSigFxMainWindow* mMainWindow;
		std::string mRegKeyName;
		
	public:
		
		tSigFxControlPanel( tSigFxMainWindow* mainWindow );
		virtual ~tSigFxControlPanel( );
		
		void fOnTick( f32 dt );
		void fLoadSettings( );
		void fResetSolidColorObjects( 
			const Gfx::tDevicePtr& device,
			const Gfx::tMaterialPtr& material, 
			const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator );

	private: // from Win32Util::tRegisterSerializer
		
		virtual std::string fRegistryKeyName( ) const { return mRegKeyName; }
		virtual void fSaveInternal( HKEY hKey );
		virtual void fLoadInternal( HKEY hKey );

	private:

		void fBuildGui( );
		void fUpdateWindowTitle( );
	};

}

#endif//__tSigFxControlPanel__

