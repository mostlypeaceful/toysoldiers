#ifndef __tEditableObjectProperties__
#define __tEditableObjectProperties__
#include "Editor/tEditableProperty.hpp"
#include "tEditorDialog.hpp"

namespace Sig
{
	class tEditorAppWindow;
	class tEditablePropertyCustomString;

	class tEditableObjectProperties : public tEditorDialog
	{
		tEditablePropertyTable::tOnPropertyChanged::tObserver		mOnGlobalPropertyChanged;
		tEditablePropertyTable::tOnPropertyChanged::tObserver		mOnCommonPropertyChanged;
		tEditablePropertyTable::tOnPropertyWantsRemoval::tObserver	mOnPropertyWantsRemoval;
		tDelegate<b32 ( tEditablePropertyCustomString& property, std::string& boneNameOut )> mBrowseForBoneName;
		tDelegate<b32 ( tEditablePropertyCustomString& property, std::string& objectNameOut )> mBrowseForSkeletonBindingName;
		wxScrolledWindow*		mMainPanel;
		wxChoice*				mUserPropType;
		wxChoice*				mUserPropName;
		tEditablePropertyTable	mCommonProps;
		tEditablePropertyTable	mGlobalProperties;
		tGrowableArray<std::string> mBoneNames;
		b32						mDoGlobalProps;
	public:
		tEditableObjectProperties( tEditorAppWindow* editorWindow, b32 globalProps );
		void fSetGlobalProperties( const tEditablePropertyTable& globalProps );
		const tEditablePropertyTable& fGlobalProperties( ) const { return mGlobalProperties; }
		void fOnSelectionChanged( );
	private:
		void fAddProperty( tEditablePropertyTable& table, int propType, const std::string& name );
		void fSetAppearance( b32 globalProps );
		void fRemoveProperty( tEditableProperty& property );
		void fClear( b32 refreshScrollBars = false );
		void fOnAction( wxCommandEvent& event );
		void fNotifyGlobalPropertyChanged( tEditableProperty& property );
		void fNotifyCommonPropertyChanged( tEditableProperty& property );
		void fRefreshBoneNames( tEditableProperty& property );
		b32 fBrowseForBoneName( tEditablePropertyCustomString& property, std::string& boneNameOut );
		b32 fBrowseForSkeletonBindingName( tEditablePropertyCustomString& property, std::string& boneNameOut );
		void fRefreshUserPropertyNames( );
		DECLARE_EVENT_TABLE()
	};

}

#endif//__tEditableObjectProperties__
