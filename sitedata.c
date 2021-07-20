#include "sitedata.h"
#include "sslcon.h"
#include "uri_encode.h"

CHOICE ch = {0};
FILE *fp;


// takes urls from given google's SITEDATA
void get_urls(SITEDATA* sdGg)
{
  unsigned char *head, *end, *data_end;
  char query[50] = {0};
  int urlSize;

  strcpy(query, "class=\"kCrYT\"><a href=\"");       // kCrYT class'i sahip olan divlerde arama sonuclarinin url'leri var

  head = sdGg->data;
  data_end = sdGg->data + sdGg->dtrv;
  while (head < data_end) {
    head = strstr(head, query);   
    if (head == NULL) break;
    
    head = strstr(head, "http");    // "...[h]ttps://www.example.com/index.html&amp...."
    end  = strstr(head, "&amp");    // "https://www.example.com/index.html[&]amp...."

    urlSize = end - head;
    if (urlSize > URSZ || urlSize < 0) continue;

    char url[urlSize+1];
    memcpy(url, head, urlSize);
    url[urlSize] = '\0';

    char dec_url[urlSize+1];
    uri_decode(url, urlSize, dec_url);

    fprintf(ch.fp, "%s\n", url);
    fprintf(ch.fp, "%s\n", dec_url);
    if (ch.print == 3) { goto go; }       // print urls only

    SITEDATA *sd = fetch_site(url);

    if (sd == NULL) { fprintf(ch.fp, "Couldn't establish ssl connection with given url\n"); continue;}

    // sitenin html icerigi yazilacaksa parse'a gerek yok
    if (ch.print == 1) { fprintf(ch.fp, "%s\n", sd->data); goto free_and_go;} 
    parse_sitedata(sd);

    free_and_go:
    free_SITEDATA(sd);
    fprintf(ch.fp, "------------------------------------------------------------------\n");

    go:
    ch.gGstart++; ch.done++;
    if (ch.requ == ch.done) break;
  }
}


// verilen urlnin icerigini getirip SITEDATA doldurur
// doldurdugu SITEDAYAyi dondurur
SITEDATA* fetch_site(char* url)
{
  SITEDATA *sd = SITEDATA_constructor();
  strcat(sd->url, url);

  char hostname[HNSZ] = {0};
  char segments[SGSZ] = {0};
  char protocol[PRSZ] = {0};

  get_part_of_url(0, url, protocol);
  get_part_of_url(1, url, hostname);
  get_part_of_url(2, url, segments);
  
  int rv = secure_connect(hostname, segments, sd);
  if (rv == -1) { free_SITEDATA(sd); return NULL; }

  return sd;
}


// memory'den SITEDATA icin yer alir, memory'leri memsetler
// geriye SITEDAYAyi dondurur
SITEDATA* SITEDATA_constructor()
{
  SITEDATA *sd = (SITEDATA*)malloc(3*sizeof(int) + DTSZ + URSZ + TXSZ);
  
  sd->dtsz = DTSZ;
  sd->ursz = URSZ;
  sd->txsz = TXSZ;
  sd->dtrv = 0;
  sd->txrv = 0;
  sd->data = (unsigned char*)malloc(DTSZ);
  sd->text = (unsigned char*)malloc(TXSZ);
  sd->url  = (unsigned char*)malloc(URSZ);

  memset(sd->data, 0, DTSZ);
  memset(sd->url , 0, URSZ);
  memset(sd->text , 0, TXSZ);

  return sd;
}


// delete given SITEDATA from memory
int free_SITEDATA(SITEDATA* sd)
{
  free(sd->data);
  free(sd->url);
  free(sd->text);
  free(sd);
  return 1;
}


// takes text content from given SITEDATA
// and pushs to sd->text
int parse_sitedata(SITEDATA *sd)
{
  char* ep[6] = {"<p ", "<p>" , "<div class=\"content\">"};   // better results without <span>
  char* dp[6] = {"</p>", "</p>", "</div>"};
  
  unsigned char *head, *end, *tmp, *min; 
  int size, parse_index, total = 0;

  head = strstr(sd->data, "<body");
  if (head == NULL) {
    fprintf(ch.fp, "Site content is not printable...\n");
    return -1;
  }
  while (1) {
    min = sd->data + sd->dtrv;
    parse_index = -1;
    for (int i = 0; i < 3; i++) {
      tmp = strstr(head, ep[i]);
      if (tmp == NULL) { continue; }
      if (tmp < min) {
        min = tmp; 
        parse_index = i; 
      }
    }
    if (parse_index == -1) break;    // hicbir parser bulunamadiysa while'i bitir

    head = min + strlen(ep[parse_index]);
    end  = strstr(head, dp[parse_index]);
    size = end - head;      //SIZE 'in cok yuksek gelme olasiligi ??
    total += size;

    if (total >= sd->txsz) {  // TODO --> REALLOC WHOLE STRUCT or is it necessary??
      //fprintf(stdout, "reallocing: %d bytes, dtsz is %d\n", total, sd->txsz);
      sd->txsz += TXSZ;
      char* tmp = (char*)realloc(sd->text, sd->txsz);
      if (tmp != NULL)
        sd->text = tmp;
      else {
        perror("reallocing failed in parse_sitedata()");
        return -1;
      }
    }
    strncat(sd->text, head, size);
  }
  sd->txrv = total;

  beautify_text(sd);
  return 1;
}


void beautify_text(SITEDATA *sd)
{
  char* ep[20] = {"<a " , "</a>", "<br/>", "<br />", "<sup ", "<strong>", "class=\"", "class='", "<img", "<textarea", "<label", "<input", "<b>", "<span", "<i ", "</i>", "<!--"};
  char* dp[20] = {">", "/a>", "<br/>", "<br />", "</sup", "</strong>", ">", ">", ">", "</textarea>","</label>", ">", "</b>", ">", ">", "</i>", "-->"};
  
  unsigned char *head, *end, *min, *tmp;
  int parse_index, size;

  head = sd->text;
  
  while (1) {
    min = sd->text + sd->txrv;
    parse_index = -1;
    for (int i = 0; i < 17; i++)
    {
      tmp = strstr(head, ep[i]);
      if (tmp == NULL) { continue; }
      if (tmp < min) {
        min = tmp; 
        parse_index = i; 
      }
    }
    if (parse_index == -1) break;    // hicbir parser bulunamadiysa while'i bitir
    
    head = min;
    end  = strstr(head, dp[parse_index]) + strlen(dp[parse_index]);
    memmove(head, end, sd->text + sd->txrv - end);
    sd->txrv -= end - head;
    sd->text[sd->txrv] = '\0';
  }
  if (ch.print == 2) {
    fprintf(ch.fp, "\n\n%s\n", sd->text);
  }
}

