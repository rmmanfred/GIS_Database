# GIS_Database
Ross Manfred
## Premise
- Create a searchable database for GIS records
- Supports the following functions on the database:
  - exists  <feature> <state abbrev>: Reports number of occurrences of matching records
  - details_of  <feature> <state abbrev>: Reports feature ID, type, state abbreviation, county, primary longitude and latitude for each matching record
  - distance_between  <feature ID>  <featureID>: Displays orthodromic distance between features at each ID
- Full instruction specifications can be found [here](C_GIS.pdf)
## Execution
- Stores offsets of GIS entries in a hashtable linked to feature ID
- Takes filename of records from script file input and creates and stores links in the hash table
  - Performing functions on the data searches the table and pulls the records
- Entries are hashed by their feature ID (entries limited to table size provided in script file), duplicate hashes are stored in a sorted list at that location
## Usage and Testing
- Demo to come
- Individual code for project found in the [dev folder](main/dev)
  - Hash Table code provided but personal implementation located [here](main)
- Test scripts can be found in the [testing folder](main/testing)
  - run as "gis <script filename> <wtf>"
- Reference outputs labeled "reflog", can be compared to found output with included "compare"
  - "compare <file 1> <file 2>"
- "VA" and "NM" files contain contents used to build database
