//------------------------------------------------------------------------------
// \file tDevVarLogFlag.hpp - 8 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tDevVarLogFlag__
#define __tDevVarLogFlag__

namespace Sig
{
#ifdef sig_devmenu
	/// \class	tDevVarLogFlag
	/// \brief	Expose a logging flag to the dev menu for runtime tweaking.
	class tDevVarLogFlag : public tDevMenuItem
	{
	public:
		/// \brief	Create a devvar controlling the log flag specified.
		/// \param	path	The dev menu path of this devvar.
		/// \param	flag	The log flag to control with this devvar.
		tDevVarLogFlag( const char* path, const Log::tFlagsType& flag );

		// Implement tDevVarBase
		virtual std::string fIthItemValueString( u32 i ) const OVERRIDE;
		virtual void fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt ) OVERRIDE;
		virtual void fSetFromVector( const Math::tVec4f& v ) OVERRIDE;
		virtual void fSetItemValue( u32 itemIndex, const std::string & value ) OVERRIDE;

	private:
		/// \brief	Return if the logging flag is set.
		/// \return	True = set (logging), False = unset (not logging)
		b32 fIsLogging( ) const;

		/// \brief	Change wheither or not the logging flag controlled by this devvar is set.
		/// \param	value	True = set (log), False = unset (don't log)
		void fSetLogging( b32 value ) const;

	private:
		/// \brief	The logging flag controlled by this devvar.
		const Log::tFlagsType& mFlag;
	};
#endif
}

#endif //ndef __tDevVarLogFlag__
