#ifndef __SentientConfig__
#define __SentientConfig__

#if defined( platform_metro )
#define sig_use_sentient // Current sentient built against MoLIVE M14 APIs removed in M15 which we're using

#endif

#if defined( sig_use_sentient )
#define if_sentient( x ) x
#else
#define if_sentient( x )
#endif

#endif //ndef __SentientConfig__
