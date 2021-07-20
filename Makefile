all: query_google.c sitedata.c sslcon.c
	gcc -g -O0 query_google.c sitedata.c sslcon.c uri_encode.c -lssl -lcrypto  -o query_google




