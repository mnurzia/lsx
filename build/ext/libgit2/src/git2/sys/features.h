#ifndef INCLUDE_features_h__
#define INCLUDE_features_h__

/* #undef GIT_DEBUG_POOL */
/* #undef GIT_TRACE */
#define GIT_THREADS 1
/* #undef GIT_MSVC_CRTDBG */

#define GIT_ARCH_64 1
/* #undef GIT_ARCH_32 */

/* #undef GIT_USE_ICONV */
#define GIT_USE_NSEC 1
#define GIT_USE_STAT_MTIM 1
/* #undef GIT_USE_STAT_MTIMESPEC */
/* #undef GIT_USE_STAT_MTIME_NSEC */
#define GIT_USE_FUTIMENS 1

/* #undef GIT_REGEX_REGCOMP_L */
/* #undef GIT_REGEX_REGCOMP */
#define GIT_REGEX_PCRE
/* #undef GIT_REGEX_PCRE2 */
/* #undef GIT_REGEX_BUILTIN */

/* #undef GIT_SSH */
/* #undef GIT_SSH_MEMORY_CREDENTIALS */

/* #undef GIT_NTLM */
/* #undef GIT_GSSAPI */
/* #undef GIT_WINHTTP */
/* #undef GIT_NTLM */

/* #undef GIT_HTTPS */
/* #undef GIT_OPENSSL */
/* #undef GIT_SECURE_TRANSPORT */
/* #undef GIT_MBEDTLS */

#define GIT_SHA1_COLLISIONDETECT 1
/* #undef GIT_SHA1_WIN32 */
/* #undef GIT_SHA1_COMMON_CRYPTO */
/* #undef GIT_SHA1_OPENSSL */
/* #undef GIT_SHA1_MBEDTLS */

#endif
