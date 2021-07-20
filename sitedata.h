#ifndef SITEDATA_H
#define SITEDATA_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define FLSZ    255     // choice file size
#define DTSZ 100000     // dat[] -that contains all recv bufs- is 100000 bytes at first
#define TXSZ  10000     // sitedata->text size (parsed data of sitedata->data)
#define URSZ   1024     // sum of the following 3 macro variables
#define PRSZ     10     // protocol size ---> https:// or http://
#define HNSZ    256     // hostname size ---> www.example.net
#define SGSZ    750     // segments size ---> /index.html

typedef struct SITEDATA_t {
  int            dtsz;     // allocated data size
  int            ursz;     // allocated  url size
  int            txsz;     // allocated text size
  int            dtrv;     // received data size
  int            txrv;     // received text size
  unsigned char*  url;     // url
  unsigned char* data;     // all site content, raw data
  unsigned char* text;     // only texts of data
} SITEDATA;


typedef struct CHOICE_t {   
  int search;        // 0 -> none, 1 -> google_search, 2 -> fetch_site
  int print;         // 0 -> none, 1 -> all, 2 -> text, 3 -> url
  int requ;          // number of sites to process
  int done;          // number of sites processed
  int gGstart;       // ?search=deneme?start=<gGStart>
  FILE *fp;          // output file
} CHOICE;

extern CHOICE ch;

SITEDATA* SITEDATA_constructor();
int       free_SITEDATA(SITEDATA* sd);
void      get_urls(SITEDATA* sd);
SITEDATA* fetch_site(char* url);
int       parse_sitedata(SITEDATA *sd);
void      beautify_text(SITEDATA *sd);

#endif /* SITEDATA_H */