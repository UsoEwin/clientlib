#ifndef _CLIENTLIB_H_
#define _CLIENTLIB_H_
//*This sturct will be used to store the return JSON format string from using cURL.*/


void initialized();

/* This function will be modified in order to get a string input */
char ***getusercollections(char * id_or_name);


void arra_clean(char ***array);

    
//this function will be used to test the result of arra
void arra_print(char ***array);



void listusercollections(char * id_or_name, char * path, char * coll_id_or_name, int rec); 



void getFile(char * coll_id_or_name, char * doi, char * id_or_name );



void createcollection(char *coll_name,char *owner_id);
#endif
