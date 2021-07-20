#include "sitedata.h"

/*
gcc -g -O0 query_google.c sitedata.c sslcon.c uri_encode.c -lssl -lcrypto  -o query_web
*/


void print_help_exit();

int main(int argc, char *argv[])
{
  init_ssl();
  ch.gGstart = 0;
  ch.fp = stdout;

  if (argc < 4 || argc > 6 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
    print_help_exit();
    return 1;
  }
  if      (strcmp(argv[1], "--google-search") == 0) { ch.search = 1; }
  else if (strcmp(argv[1], "--fetch-site")    == 0) { ch.search = 2; }
  else                                              { print_help_exit(); }
  
  if      (strcmp(argv[3], "--print-all")     == 0) { ch.print  = 1; }
  else if (strcmp(argv[3], "--print-text")    == 0) { ch.print  = 2; }
  else if (strcmp(argv[3], "--print-url")     == 0) { ch.print  = 3; }
  else                                              { print_help_exit(); }

  if (argc == 6) { 
    if (strlen(argv[5]) > FLSZ) { fprintf(stderr, "Length of filename must be smaller than 255!\n"); return 1;}
    ch.fp = fopen(argv[5], "w"); 
  }
  
  if (ch.search == 1) {      // --google-search
    if (argc > 4) { ch.requ = atoi(argv[4]); }
    else          { ch.requ = 5;             }    // default is 5

    while (ch.done < ch.requ) {
      char google_query[URSZ] = {0};
      char startPrm[20] = {0};
      strcat(google_query, "https://www.google.com/search?q=");
      const char* delp = strtok(argv[2], " ");
      while (delp!= NULL) {
        strcat(google_query, delp);
        strcat(google_query, "+");
        delp = strtok(NULL, " ");
      }
      sprintf(startPrm, "&start=%d", ch.gGstart);
      strcat(google_query, startPrm);

      SITEDATA *sdGg = fetch_site(google_query);
      if (sdGg == NULL) {
        perror("Couldn't connect google");  // log dosyasi
        return 1;
      }
      get_urls(sdGg);

      free_SITEDATA(sdGg);
    }
  }
  else {                     // --fetch-site
    int start_page = 0, end_page = 0;
    char* param;
    char url[URSZ] = {0};
    strcat(url, argv[2]);
    
    if (argc == 5 && strstr(argv[4], "=") != NULL && strstr(argv[4], "-") != NULL) {
      param = strtok(argv[4], "=");
      char* start = strtok(NULL, "-");
      char* end = strtok(NULL, "\0");
      if (start != NULL && end != NULL) {
        start_page = atoi(start);
        end_page = atoi(end);
        strcat(url, param);
        strcat(url, "=");
      }
    }
    for (int i = start_page; i <= end_page; i++)
    {
      char url_p[URSZ] = {0};
      strcpy(url_p, url);
      if (end_page != 0) {
        char buf[64] = {0};
        sprintf(buf, "%d", i);
        strcat(url_p, buf);
      }
      fprintf(ch.fp, "%s\n", url_p);
      SITEDATA* sd = fetch_site(url_p);
      if (sd == NULL) {
        fprintf(ch.fp, "Couldn't establish ssl connection with given url\n");
        return 1;
      }
      parse_sitedata(sd);
      if      (ch.print == 1) { fprintf(ch.fp, "%s\n", sd->data); }
      free_SITEDATA(sd);
      fprintf(ch.fp, "---------------------------------------------------------\n");
    }
    
    
  }
  fclose(ch.fp);
  return 0;
}

void print_help_exit()
{
  printf("Usage: query_web {..arg1..} {..arg2..} {..arg3..} {..arg4..} {..arg5..}\n");
  printf("  {..arg1..}\n");
  printf("      --google-search    --> get results with given keywords\n");
  printf("      --fetch-site       --> fetch site of given url\n");
  printf("  {..arg2..}\n");
  printf("      if arg1 == --google-search\n");
  printf("          {..arg2..}     --> \"google keywords\"(in quotes)\n");
  printf("      if arg1 == --fetch-site\n");
  printf("          {..arg2..}     --> site to fetch\n");
  printf("  {..arg3..}\n");
  printf("      --print-all        --> prints html content of site\n");
  printf("      --print-text       --> prints texts of site\n");
  printf("      if arg1 == --google-search\n");
  printf("          --print-url     --> prints urls in google result\n");
  printf("  {..arg4..}\n");
  printf("      if arg1 == --google-search\n");
  printf("          {..arg4..}     --> number of sites that will fetch\n");
  printf("                             ex: 5\n");
  printf("      if arg1 == --fetch-site\n");
  printf("          {..arg4..}     --> page range (with its parameter) \n");
  printf("                             ex: \"?p=1-10\"      --> fetchs sites that in [1,10] (1 and 10 is included)\n");
  printf("                             ex: \"\"             --> no page range\n");
  printf("  {..arg5..}             --> filename, if not entered default is stdout\n");
  printf("Ex:  query_google --google-search \"rober hatemo\" --print-all 5\n");
  printf("     query_google --fetch-site https://eksisozluk.com/rem-uykusu--932646 --print-text \"?p=1-10\" \n"); // couldn't fetch 'eksisozluk'
  exit(1);
}
