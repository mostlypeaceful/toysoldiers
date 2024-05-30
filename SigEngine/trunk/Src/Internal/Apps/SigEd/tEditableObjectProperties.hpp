#ifndef __tEditableObjectProperties__
#define __tEditableObjectProperties__
#include "Editor/tEditableProperty.hpp"
#include "tEditorDialog.hpp"

namespace Sig
{
	class tEditorAppWindow;
	class tEditablePropertyCustomString;
	class tEditablePropertyCDCFloat;

	class tEditableObjectProperties : public tEditorDialog
	{
	public:
		typedef void ( tEditorAppWindow::*tOnFocusCallback )( void );
	private:
		tEditablePropertyTable::tOnPropertyChanged::tObserver		mOnGlobalPropertyChanged;
		tEditablePropertyTable::tOnPropertyChanged::tObserver		mOnCommonPropertyChanged;
		tEditablePropertyTable::tOnPropertyWantsRemoval::tObserver	mOnPropertyWantsRemoval;
		tDelegate<b32 ( tEditablePropertyCustomString& property, std::string& boneNameOut )> mBrowseForBoneName;
		tDelegate<b32 ( tEditablePropertyCustomString& property, std::string& objectNameOut )> mBrowseForSkeletonBindingName;
		tDelegate<b32 ( tEditablePropertyCDCFloat& property, f32& cameraDistanceOut )> mCaptureCameraDistance;
		wxScrolledWindow*		mMainPanel;
		wxChoice*				mUserPropType;
		wxChoice*				mUserPropName;
		tEditablePropertyTable	mCommonProps;
		tEditablePropertyTable	mGlobalProperties;
		tGrowableArray<std::string> mBoneNames;
		b32						mDoGlobalProps;
		tOnFocusCallback		mOnFocusCallback;
	public:
		tEditableObjectProperties( tEditorAppWindow* editorWindow, b32 globalProps );
		void fSetGlobalProperties( const tEditablePropertyTable& globalProps );
		const tEditablePropertyTable& fGlobalProperties( ) const { return mGlobalProperties; }
		void fOnSelectionChanged( );
		void fSetOnFocusCallback( tOnFocusCallback focusCB );
		void fSetFocusHooks( );
	private:
		void fAddProperty( tEditablePropertyTable& table, int propType, const std::string& name );
		void fSetAppearance( b32 globalProps );
		void fRemoveProperty( tEditableProperty& property );
		void fClear( b32 refreshScrollBars = false );
		void fOnAction( wxCommandEvent& event );
		void fOnFocus( wxFocusEvent& event );
		void fOnOpenExplorer( wxCommandEvent& );
		void fNotifyGlobalPropertyChanged( tEditableProperty& property );
		void fNotifyCommonPropertyChanged( tEditableProperty& property );
		void fRefreshBoneNames( tEditableProperty& property );
		b32 fBrowseForBoneName( tEditablePropertyCustomString& property, std::string& boneNameOut );
		b32 fBrowseForSkeletonBindingName( tEditablePropertyCustomString& property, std::string& boneNameOut );
		b32 fCaptureCameraDistance( tEditablePropertyCDCFloat& property, f32& cameraDistanceOut );
		void fRefreshUserPropertyNames( );
		void fSetFocusHooksFromList( wxWindowList& list );
		DECLARE_EVENT_TABLE()
	};

}

#endif//__tEditableObjectProperties__
