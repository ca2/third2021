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

#ifndef HEADER_AFALG_ERR_H
# define HEADER_AFALG_ERR_H

# ifdef  __cplusplus
extern "C" {
# endif

/* BEGIN ERROR CODES */
void ERR_load_AFALG_strings(void);
void ERR_unload_AFALG_strings(void);
void ERR_AFALG_error(int function, int reason, char *file, int line);
# define AFALGerr(f,r) ERR_AFALG_error((f),(r),__FILE__,__LINE__)

/* Error codes for the AFALG functions. */

/* Function codes. */
# define AFALG_F_AFALG_CHK_PLATFORM                       100
# define AFALG_F_AFALG_CREATE_BIND_SK                     106
# define AFALG_F_AFALG_CREATE_BIND_SOCKET                 105
# define AFALG_F_AFALG_CREATE_SK                          108
# define AFALG_F_AFALG_INIT_AIO                           101
# define AFALG_F_AFALG_SETUP_ASYNC_EVENT_NOTIFICATION     107
# define AFALG_F_AFALG_SET_KEY                            109
# define AFALG_F_AFALG_SOCKET                             102
# define AFALG_F_AFALG_START_CIPHER_SK                    103
# define AFALG_F_BIND_AFALG                               104

/* Reason codes. */
# define AFALG_R_EVENTFD_FAILED                           108
# define AFALG_R_FAILED_TO_GET_PLATFORM_INFO              111
# define AFALG_R_INIT_FAILED                              100
# define AFALG_R_IO_SETUP_FAILED                          105
# define AFALG_R_KERNEL_DOES_NOT_SUPPORT_AFALG            101
# define AFALG_R_KERNEL_DOES_NOT_SUPPORT_ASYNC_AFALG      107
# define AFALG_R_MEM_ALLOC_FAILED                         102
# define AFALG_R_SOCKET_ACCEPT_FAILED                     110
# define AFALG_R_SOCKET_BIND_FAILED                       103
# define AFALG_R_SOCKET_CREATE_FAILED                     109
# define AFALG_R_SOCKET_OPERATION_FAILED                  104
# define AFALG_R_SOCKET_SET_KEY_FAILED                    106

#ifdef  __cplusplus
}
#endif
#endif
