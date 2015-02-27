#ifndef __SIGLAP_H__
#define __SIGLAP_H__

extern const char *gitversion;

#define SIGLAP_OS_VERSION_MAJOR 1
#define SIGLAP_OS_VERSION_MINOR 0

#define serialDriver (&SD1)
#define stream_driver ((BaseSequentialStream *)serialDriver)
extern void *stream;

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))
#endif

#endif /* __SIGLAP_H__ */
