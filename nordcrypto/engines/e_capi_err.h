﻿/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * NOTE: this file was auto generated by the mkerr.pl script: any changes
 * made to it will be overwritten when the script next updates this file,
 * only reason strings will be preserved.
 */

#ifndef HEADER_CAPI_ERR_H
# define HEADER_CAPI_ERR_H

#ifdef  __cplusplus
extern "C" {
#endif

/* BEGIN ERROR CODES */
static void ERR_load_CAPI_strings(void);
static void ERR_unload_CAPI_strings(void);
static void ERR_CAPI_error(int function, int reason, char *file, int line);
# define CAPIerr(f,r) ERR_CAPI_error((f),(r),OPENSSL_FILE,OPENSSL_LINE)

/* Error codes for the CAPI functions. */

/* Function codes. */
# define CAPI_F_CAPI_CERT_GET_FNAME                       99
# define CAPI_F_CAPI_CTRL                                 100
# define CAPI_F_CAPI_CTX_NEW                              101
# define CAPI_F_CAPI_CTX_SET_PROVNAME                     102
# define CAPI_F_CAPI_DSA_DO_SIGN                          114
# define CAPI_F_CAPI_GET_KEY                              103
# define CAPI_F_CAPI_GET_PKEY                             115
# define CAPI_F_CAPI_GET_PROVNAME                         104
# define CAPI_F_CAPI_GET_PROV_INFO                        105
# define CAPI_F_CAPI_INIT                                 106
# define CAPI_F_CAPI_LIST_CONTAINERS                      107
# define CAPI_F_CAPI_LOAD_PRIVKEY                         108
# define CAPI_F_CAPI_OPEN_STORE                           109
# define CAPI_F_CAPI_RSA_PRIV_DEC                         110
# define CAPI_F_CAPI_RSA_PRIV_ENC                         111
# define CAPI_F_CAPI_RSA_SIGN                             112
# define CAPI_F_CAPI_VTRACE                               118
# define CAPI_F_CERT_SELECT_DIALOG                        117
# define CAPI_F_CLIENT_CERT_SELECT                        116
# define CAPI_F_WIDE_TO_ASC                               113

/* Reason codes. */
# define CAPI_R_CANT_CREATE_HASH_OBJECT                   99
# define CAPI_R_CANT_FIND_CAPI_CONTEXT                    100
# define CAPI_R_CANT_GET_KEY                              101
# define CAPI_R_CANT_SET_HASH_VALUE                       102
# define CAPI_R_CRYPTACQUIRECONTEXT_ERROR                 103
# define CAPI_R_CRYPTENUMPROVIDERS_ERROR                  104
# define CAPI_R_DECRYPT_ERROR                             105
# define CAPI_R_ENGINE_NOT_INITIALIZED                    106
# define CAPI_R_ENUMCONTAINERS_ERROR                      107
# define CAPI_R_ERROR_ADDING_CERT                         125
# define CAPI_R_ERROR_CREATING_STORE                      126
# define CAPI_R_ERROR_GETTING_FRIENDLY_NAME               108
# define CAPI_R_ERROR_GETTING_KEY_PROVIDER_INFO           109
# define CAPI_R_ERROR_OPENING_STORE                       110
# define CAPI_R_ERROR_SIGNING_HASH                        111
# define CAPI_R_FILE_OPEN_ERROR                           128
# define CAPI_R_FUNCTION_NOT_SUPPORTED                    112
# define CAPI_R_GETUSERKEY_ERROR                          113
# define CAPI_R_INVALID_DIGEST_LENGTH                     124
# define CAPI_R_INVALID_DSA_PUBLIC_KEY_BLOB_MAGIC_NUMBER  122
# define CAPI_R_INVALID_LOOKUP_METHOD                     114
# define CAPI_R_INVALID_PUBLIC_KEY_BLOB                   115
# define CAPI_R_INVALID_RSA_PUBLIC_KEY_BLOB_MAGIC_NUMBER  123
# define CAPI_R_PUBKEY_EXPORT_ERROR                       116
# define CAPI_R_PUBKEY_EXPORT_LENGTH_ERROR                117
# define CAPI_R_UNKNOWN_COMMAND                           118
# define CAPI_R_UNSUPPORTED_ALGORITHM_NID                 119
# define CAPI_R_UNSUPPORTED_PADDING                       120
# define CAPI_R_UNSUPPORTED_PUBLIC_KEY_ALGORITHM          121
# define CAPI_R_WIN32_ERROR                               127

#ifdef  __cplusplus
}
#endif
#endif
