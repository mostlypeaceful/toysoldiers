#ifndef __tLocmlConvertDictionary__
#define __tLocmlConvertDictionary__

#include "Locml.hpp"

namespace Sig
{
	class tLocmlConvertDictionary
	{
	public:
		enum tLangTargets
		{
			cEN, // THIS IS NOT A TARGET!
			cBR,
			cCHS,
			cDE,
			cES,
			cFR,
			cIT,
			cJA,
			cKO,
			cRU,
			cZH,

			cNumLangTargets
		};

		// All strings must be UTF-8 encoded std::strings.
		void			fExtractFromLocml( std::string lang, const Locml::tFile& loc );
		void			fClear();
		std::string		fGetReplacementText( std::string tag, std::string lang ) const;
		b32				fBuilt() const;

		void			fPrint( );

	private:
		class tRecord
		{
		public:
			std::string mTag;
			tHashTable< std::string, std::string > mTexts;
		};

		tHashTable< std::string, tRecord > mRecords;

		void fExtractRecord( std::string lang, const Locml::tStringEntry& entry );
	};
}

#endif // __tLocmlConvertDictionary__
