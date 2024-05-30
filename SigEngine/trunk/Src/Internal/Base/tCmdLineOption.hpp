#ifndef __tCmdLineOption__
#define __tCmdLineOption__

namespace Sig
{
	class base_export tCmdLineOption
	{
		b32			mFound;
		std::string mOption;

	public:

		tCmdLineOption( );
		tCmdLineOption( const char* option, const std::string& cmdLineBuffer );

		inline b32					fFound		( ) const		{ return mFound; }
		inline const std::string&	fGetOption	( ) const		{ return mOption; }

		template<class t>
		t fGetTypedOption( ) const
		{
			std::stringstream ss; ss << mOption;
			t o; ss >> o;
			return o;
		}
	};

}

#endif//__tCmdLineOption__
