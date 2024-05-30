#ifndef __XAudio360Util__
#define __XAudio360Util__

namespace Sig { namespace XAudio360Util
{
	toolsxdk_export b32 fWaveFileToXma( tDynamicBuffer& obuffer, const tFilePathPtr& wavePath, s32 quality = -1 );
}}

#endif//__XAudio360Util__
