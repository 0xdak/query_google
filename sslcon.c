#include "sslcon.h"

void report_and_exit(const char* msg) {
  perror(msg);
  ERR_print_errors_fp(stderr);
  exit(-1);
}

void init_ssl() {
  SSL_load_error_strings();
  SSL_library_init();  
}

void cleanup(SSL_CTX* ctx, BIO* bio) {
  SSL_CTX_free(ctx);
  BIO_free_all(bio);
}
#define BFSZ 10000

// establishs ssl connection with given sd->url
// and fetchs that site to sd->data
int secure_connect(char* hostname, char* segments, SITEDATA* sd) {
  char name[HNSZ];
  char request[URSZ];

  const SSL_METHOD* method = TLS_client_method();
  //const SSL_METHOD* method = TLSv1_2_client_method();  //‘TLSv1_2_client_method’ is deprecated
  if (NULL == method) report_and_exit("TLS_client_method...");
  
  SSL_CTX* ctx = SSL_CTX_new(method);
  if (NULL == ctx) report_and_exit("SSL_CTX_new...");

  BIO* bio = BIO_new_ssl_connect(ctx);
  if (NULL == bio) report_and_exit("BIO_new_ssl_connect...");

  SSL* ssl = NULL;

  /* link bio channel, SSL session, and server endpoint */
  sprintf(name, "%s:%s", hostname, "https");
  BIO_get_ssl(bio, &ssl);                   /* session */
  SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);   /* robustness */
  BIO_set_conn_hostname(bio, name);         /* prepare to connect */


//  BIO_set_nbio(bio, 0);

  /* try to connect */
  if (BIO_do_connect(bio) <= 0) {
    cleanup(ctx, bio);
    return -1;
  }


  /* verify truststore, check cert */
  if (!SSL_CTX_load_verify_locations(ctx,
				     "/etc/ssl/certs/ca-certificates.crt", /* truststore */
				     "/etc/ssl/certs/")) /* more truststore */
    report_and_exit("SSL_CTX_load_verify_locations...");

  long verify_flag = SSL_get_verify_result(ssl);
  /*
  if (verify_flag != X509_V_OK) 
    /fprintf(stderr,
	    "##### Certificate verification error (%i) but continuing...\n",
	    (int) verify_flag);
  */
 
  /* now fetch the homepage as sample data */
  sprintf(request,
	  "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n\r\n",
	  segments, hostname);
  BIO_puts(bio, request);
  //- SSL_CTX_set_timeout(ctx, 2);     // set timeout as 2 seconds , --> it sets session's timeout
  
  int total = 0; 
  char response[BFSZ] = {0};
  while (1) {
    //fprintf(stderr, "total: %d\n", total);
    memset(response, '\0', sizeof(response));
    int recvd = BIO_read(bio, response, BFSZ);
    //fprintf(stderr, "recvd: %d\n", recvd);
    
    if (recvd <= 0)  
    {
      break;
    }
      // 0 is end-of-stream, < 0 is an error
    total += recvd;
    if (total >= sd->dtsz) {  // TODO --> REALLOC WHOLE STRUCT or is it necessary??
      //fprintf(stdout, "reallocing: %d bytes, dtsz is %d\n", total, sd->dtsz);
      sd->dtsz += DTSZ;
      char* tmp = (char*)realloc(sd->data, sd->dtsz);
      if (tmp != NULL)
        sd->data = tmp;
      else {
        perror("reallocing failed in secure_connect()");
        return -1;
      }
    }
    strcat(sd->data, response);
  }
  sd->dtrv = total;
  cleanup(ctx, bio);
  return 1;
}


// TODO CAN BE BEAUTIFY BY USING MACRO
// 0 ---> protocol, 1 ---> hostname, 2 ---> segments
// return -1  --> error, return > 0 --> strlen(dst)
int get_part_of_url(int part, char* src, char* dst)
{
  unsigned char *tmp, *head, *end;
  if      (part == 0) {
    head = src;

    end  = strstr(src, "://");
    if (end == NULL)    { return - 1;       }    // unavailable protocol
    end += 3;

    memcpy(dst, head, end - head);
  }
  else if (part == 1) {
    head = strstr(src, "://");    // https[://]www.example.net/index.html
    if   (head != NULL) { head += 3;        }
    else                { head  = src;      }

    end  = strstr(head, "/");     // https://www.example.net[/]index.html
    if   (end == NULL)  { end   = src + strlen(src); }

    memcpy(dst, head, end - head);
  }
  else if (part == 2) {
    head = strstr(src, ".");
    if   (head == NULL)   { head = src;       }

    head = strstr(head, "/");
    if   (head == NULL)   { strcat(dst, "/"); }
    else {
      end = src + strlen(src);
      memcpy(dst, head, end - head);
    }
  }
  else                { return -1; }
  return strlen(dst);
}

