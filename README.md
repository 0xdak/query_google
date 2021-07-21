# Query Google
The general purpose of Query Google is to search the given -key words- in google and fetch/parse this sites to user.

## Using
```
Usage: query_google {..arg1..} {..arg2..} {..arg3..} {..arg4..} {..arg5..}
  {..arg1..}
      --google-search    --> get results with given keywords
      --fetch-site       --> fetch site of given url
  {..arg2..}
      if arg1 == --google-search
          {..arg2..}     --> "google keywords"(in quotes)
      if arg1 == --fetch-site
          {..arg2..}     --> site to fetch
  {..arg3..}
      --print-all        --> prints html content of site
      --print-text       --> prints texts of site
      if arg1 == --google-search
          --print-url    --> prints urls in google result
  {..arg4..}
      if arg1 == --google-search
          {..arg4..}     --> number of sites that will fetch
                             ex: 5
      if arg1 == --fetch-site
          {..arg4..}     --> page range (with its parameter) 
                             ex: "?p=1-10"      --> fetchs sites that in [1,10] (1 and 10 is included)
                             ex: ""             --> no page range
  {..arg5..}             --> filename, if not entered default is stdout
Ex:  query_google --google-search "rober hatemo" --print-all 5
     query_google --fetch-site https://eksisozluk.com/rem-uykusu--932646 --print-text "?p=1-10"
     query_google --fetch-site https://artbma.org/about/press/release/bma-security-officers-take-center-stage-as-guest-curators-of-a-new-exhibition-opening-in-march-2022 --print-text
 ```
 ## For Development
 Look up to the source codes, and ask me what you want. I am open to any suggestion, help and development together. 
    
