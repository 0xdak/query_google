#ifndef SSLCON_H
#define SSLCON_H

#include <openssl/bio.h> /* BasicInput/Output streams */
#include <openssl/err.h> /* errors */
#include <openssl/ssl.h> /* core library */

#include "sitedata.h"

void report_and_exit(const char* msg);
void init_ssl();
void cleanup(SSL_CTX* ctx, BIO* bio);
int  secure_connect(char* hostname, char* segments, SITEDATA* sd);
int  get_part_of_url(int part, char* src, char* dst);


#endif /* SSLCON_H */