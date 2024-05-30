#ifndef __BannedApiConfig__
#define __BannedApiConfig__

#ifdef build_release
#define sig_use_banned_apis

#else // !build_release
#define sig_use_banned_apis

#endif

#endif //ndef __BannedApiConfig__
