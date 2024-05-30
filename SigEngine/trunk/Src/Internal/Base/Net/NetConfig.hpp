#ifndef __NetConfig__
#define __NetConfig__

//#if !defined( platform_metro )
#define use_enet
//#endif

#if defined( use_enet )
#include <enet/enet.h>
#endif // defined( use_enet )

#endif //ndef __NetConfig__
