#include <curl/curl.h>
#include <stdlib.h>
#include <curl/easy.h>
#include <string.h>
#include <jansson.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "clientlib.h"
//*This sturct will be used to store the return JSON format string from using cURL.*/
typedef struct _bufferprototype{
    char *buffer;
    size_t size;    
}BufferStruct;

/* This function willl be used to write the string to data struct*/
static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;

    BufferStruct * mem = (BufferStruct *) data;

    mem->buffer = realloc(mem->buffer, mem->size + realsize + 1);

    if ( mem->buffer )
    {
        memcpy( &( mem->buffer[ mem->size ] ), ptr, realsize );
        mem->size += realsize;
        mem->buffer[ mem->size ] = '\0'; //set terminate condition for the string.
    }

    return realsize;

}


static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t retcode;
  curl_off_t nread;
 
  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */ 
  retcode = fread(ptr, size, nmemb, stream);
 
  nread = (curl_off_t)retcode;
 
  fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
          " bytes from file\n", nread);
 
  return retcode;
}



void initialized()
{
    printf("\nUsage : C iDataClient followed by any one of those options below :\n");
    
    printf("\n1. To get a users collections: getcollections <user-id/user-name>\n");
    
    printf("\n2. To list collections contents at a path(-a/-s will select the work mode,-a will recursively list all contents in(under) the directory): list <coll-id-or-name> <path> <user-id/user-name><-s/-a>\n");
    
    printf("\n3. To get A file: getFile <coll-id-or-name> <doi> <user_id/user__name> \n");
    
    // printf("\n4. To put A file: putFile <filename> <coll_id_or_name> <user_id/user__name> <path > \n");
    
     printf("\n4. To creat a new collection: createcollection <collection_name> <user_id> \n");
    
    return;
}

/* This function will be modified in order to get a string input */
char ***getusercollections(char * id_or_name) 
{
    curl_global_init( CURL_GLOBAL_ALL );
    
    CURL *myHandle;
    
    CURLcode result;
    
    BufferStruct output; // Create an instance of out BufferStruct to accept LCs output
    
    output.buffer = NULL;
    
    output.size = 0;
    
    myHandle = curl_easy_init();

    char url[256];
    //1016
    /*still need error checking*/
    if (id_or_name[0] >= '0' && id_or_name[0] <='9')
    {
        sprintf(url, "https://dev.mygeohub.org/api/idata/collection/list?owner_id=%s", id_or_name);

    }
    else {
    sprintf(url, "https://dev.mygeohub.org/api/idata/collection/list?owner=%s", id_or_name);
    }

    //snprintf(url, URL_SIZE, URL_FORMAT, argv[1], argv[2]);

    curl_easy_setopt(myHandle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); // Passing the function pointer to LC
    
    curl_easy_setopt(myHandle, CURLOPT_WRITEDATA, (void *)&output); // Passing our BufferStruct to LC

    curl_easy_setopt(myHandle, CURLOPT_URL, url);

    result = curl_easy_perform( myHandle );

    curl_easy_cleanup(myHandle);
    curl_global_cleanup();

    /* Next step will be use the step to get decode the data.*/

    json_t *root;

    json_error_t error;

    char *content = output.buffer;

    //printf("\n returned content : %s \n",content);

    root = json_loads(content,0, &error);

    /*check if the root and buffer is valid.*/

    if(!root)
    {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return NULL;
    }
    //First get the Jasson array.
    json_t *collections  = json_object_get(root,"collections");

    // make sure this is a array.
    if(!json_is_array(collections))
    {
        fprintf(stderr, "error: collections is not an array\n");
        json_decref(root);
        return NULL;
    }

    int i = 0;
    char ***arra = (char ***)malloc((json_array_size(collections)+1)*sizeof(char **));
    //the first in arra will be use to
    
    
    arra[0] = (char **) malloc(sizeof(char *));
    arra[0][0] = (char *)malloc(sizeof(int));
    arra[0][0][0] = json_array_size(collections);
    //since this is char, the 
    

    for(i = 1; i <= json_array_size(collections); i++)
    {
        json_t *name, *id, *shared, *val;

        arra[i] = (char **)malloc(3*sizeof(char *));
	//for test
        val = json_array_get(collections, i-1);
        
        if(!json_is_object(val))
        {
        
            fprintf(stderr, "error: val %d is not an object\n", (int)(i + 1));
            
            json_decref(root);
            
            return NULL;
        }

        name = json_object_get(val, "name");
        
        if(!json_is_string(name))
        {
            fprintf(stderr, "error: name %d is not a string\n", (int)(i + 1));
            
            return NULL;
        }

        id = json_object_get(val, "id");
        
        if(!json_is_string(id))
        {
            fprintf(stderr, "error: id %d: commit is not an string\n", (int)(i + 1));
            
            json_decref(root);
            
            return NULL;
	}

        shared = json_object_get(val, "shared");
        
        if(!json_is_string(shared))
        
        {
            fprintf(stderr, "error: shared %d is not a string\n", (int)(i + 1));
           
            json_decref(root);
           
            return NULL;
        }
	
  arra[i][0] = (char *)malloc((strlen(json_string_value(name))+1)*sizeof(char));
	
  arra[i][1] = (char *)malloc((strlen(json_string_value(id))+1)*sizeof(char));
	
  arra[i][2] = (char *)malloc((strlen(json_string_value(shared))+1)*sizeof(char));
	
  sprintf(arra[i][0],"%s", json_string_value(name));
        
        sprintf(arra[i][1],"%s", json_string_value(id));
        
        sprintf(arra[i][2],"%s", json_string_value(shared));
    
    }


    
    json_decref(root);

 
    if(output.buffer)
    {
        free(output.buffer);
        
        output.buffer = NULL;
        
        output.size = 0;

    }

    return arra;
}






void arra_clean(char ***array)
{
    int i = 1;
    
    int size  = array[0][0][0];
    
    //printf("in arra_clean,the size is %d\n",size);
    
    for (;i<=size;i++)
    
    {
      
      free(array[i][0]);
      
      free(array[i][1]);
      
      free(array[i][2]);
      
      free(array[i]);
    }
    free(array[0][0]);
    
    free(array[0]);
    
    free(array);
      
    return;

}
    
//this function will be used to test the result of arra
void arra_print(char ***array)
{
  
  int i = 1;
  
  int size  = array[0][0][0];
  
  for (;i<=size;i++)
    {
      
      printf("name :%s  \t",array[i][0]);
      
      printf("id :%s  \t",array[i][1]);
      
      printf("shared :%s\n",array[i][2]);
    
    }
    
    return ;

}









void listusercollections(char * id_or_name, char * path, char * coll_id_or_name, int rec) 
{
    curl_global_init( CURL_GLOBAL_ALL );
    
    CURL *myHandle;
    
    CURLcode result;

    BufferStruct output; // Create an instance of out BufferStruct to accept LCs output
    
    output.buffer = NULL;
    
    output.size = 0;
    
    myHandle = curl_easy_init();

    char url[512];
    
    char *addr = curl_easy_escape(myHandle,path,0);
    
    char *collname=curl_easy_escape(myHandle,coll_id_or_name,0);
    if (id_or_name[0] >= '0' && id_or_name[0] <='9' && coll_id_or_name[0] >= '0' && coll_id_or_name[0]<='9')
    {
      sprintf(url, "http://dev.mygeohub.org/api/idata/collection/contents?id=%s&path=%s&owner_id=%s", collname,addr,id_or_name);

    }
    else if(id_or_name[0] >= '0' && id_or_name[0] <='9')
      {
	sprintf(url, "http://dev.mygeohub.org/api/idata/collection/contents?name=%s&path=%s&owner_id=%s", collname,addr,id_or_name);
      }
    else if(coll_id_or_name[0] >= '0' && coll_id_or_name[0]<='9')
      {
	sprintf(url, "http://dev.mygeohub.org/api/idata/collection/contents?id=%s&path=%s&owner=%s", collname,addr,id_or_name);
      }
    else 
      {
	sprintf(url, "http://dev.mygeohub.org/api/idata/collection/contents?name=%s&path=%s&owner=%s", collname,addr,id_or_name);
      }
    //Encode the URL
    curl_free(addr);
    
    curl_free(collname);

    curl_easy_setopt(myHandle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); // Passing the function pointer to LC
    
    curl_easy_setopt(myHandle, CURLOPT_WRITEDATA, (void *)&output); // Passing our BufferStruct to LC

    curl_easy_setopt(myHandle, CURLOPT_URL, url);

    result = curl_easy_perform( myHandle );

    /* curl_free(outpt); */
    
    curl_easy_cleanup(myHandle);
    
    curl_global_cleanup();

    /* Next step will be use the step to get decode the data.*/

    json_t *root;

    json_error_t error;

    char *content = output.buffer;


    root = json_loads(content,0, &error);

    /*check if the root and buffer is valid.*/

    if(!root)
    {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return ;
    }
    //First get the Jasson array.


    json_t *collections  = json_object_get(root,"contents");
    // make sure this is a array.
    if(!json_is_array(collections))
    {
        fprintf(stderr, "error: contents is not an array\n");
        json_decref(root);
        return ;
    }

    int i = 0;


    for(i = 0; i < json_array_size(collections); i++)
    {
      json_t *name, *dof, *type, *size, *crtime,*doi,*val;

        val = json_array_get(collections, i);
        if(!json_is_object(val))
        {
            fprintf(stderr, "error: val %d is not an object\n", (int)(i + 1));
            json_decref(root);
            return ;
        }

        name = json_object_get(val, "name");
        if(!json_is_string(name))
        {
            fprintf(stderr, "error: name %d is not a string\n", (int)(i + 1));
          
            return ;
        }

        dof = json_object_get(val, "dir-or-file");
        
        if(!json_is_number(dof))
        {
            
            fprintf(stderr, "error: dof %d: is not an number\n", (int)(i + 1));
            
            json_decref(root);
            
            return ;
	}
	
  fprintf(stdout,"name: %s  dir-or-file:%d   ",json_string_value(name),(int)json_number_value(dof));
	
  if(json_number_value(dof) == 2.0)
	  {
	    
      type = json_object_get(val, "type");
	    
      size = json_object_get(val, "size");
	    
      crtime = json_object_get(val, "ctime");
	    
      doi = json_object_get(val, "doi");
	    
      fprintf(stdout,"type: %s  size:%s  ctime: %s   doi:%s",json_string_value(type),json_string_value(size),json_string_value(crtime),json_string_value(doi));
	
	    
	  }
	//recursively go into the folders
	else if(rec == 1 && json_number_value(dof) == 1.0)
	  {
	    char newpath[128];
	    
      sprintf(newpath,"%s",path);
	    
      strcat(newpath,"/");
	    
      strcat(newpath,json_string_value(name));
	    
      fprintf(stdout,"in the path:%s the files are:\n",newpath);
	    
      listusercollections(id_or_name,newpath,coll_id_or_name,rec); 
	    
      fprintf(stdout,"back to the orgin directory\n");
	    
      fprintf(stdout,"orgin directory name:%s\n",json_string_value(name));
	  }

	fprintf(stdout,"\n");
    }


    
    json_decref(root);

 
    if(output.buffer)
    {
        free(output.buffer);
        output.buffer = NULL;
        output.size = 0;

    }

    return ;
}



void getFile(char * coll_id_or_name, char * doi, char * id_or_name )
{  

  CURL *curl_handle;
  
  CURLcode res;
  
  BufferStruct chunk,header;
  
  chunk.buffer = malloc(1);  /* will be grown as needed by the realloc above */ 
  
  chunk.size = 0;    /* no data at this point */ 
  
  //This is for the header.
  
  header.buffer = malloc(1);
  
  header.size = 0;
  
  curl_global_init(CURL_GLOBAL_ALL);
 
  curl_handle = curl_easy_init();
 
  char url[512];

  char *collname=curl_easy_escape(curl_handle,coll_id_or_name,0);
  
  char *doid=curl_easy_escape(curl_handle,doi,0);
    /*still need error checking*/
    
    if (id_or_name[0] >= '0' && id_or_name[0] <='9' && coll_id_or_name[0] >= '0' && coll_id_or_name[0]<='9')
    {
      sprintf(url, "http://dev.mygeohub.org/api/idata/collection/get?coll=%s&doi=%s&owner_id=%s", collname,doid,id_or_name);
      //printf("\n%s\n",url);
    }
    else if(id_or_name[0] >= '0' && id_or_name[0] <='9')
      {
        sprintf(url, "http://dev.mygeohub.org/api/idata/collection/get?id=%s&doi=%s&owner_id=%s", collname,doid,id_or_name);
	//	printf("\n%s\n",url);
      }
    else if(coll_id_or_name[0] >= '0' && coll_id_or_name[0]<='9')
      {
        sprintf(url, "http://dev.mygeohub.org/api/idata/collection/get?id=%s&doi=%s&owner=%s", collname,doid,id_or_name);
	//	printf("\n%s\n",url);
      }
    else 
      {
        sprintf(url, "http://dev.mygeohub.org/api/idata/collection/get?coll=%s&doi=%s&owner=%s", collname,doid,id_or_name);
	//	printf("\n%s\n",url);
      }

    curl_free(collname);
    curl_free(doid);
    //

  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
 
 
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
 

  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "iDataClient");
   
  //This is for header.
  curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, (void *)&header);

  res = curl_easy_perform(curl_handle);
 


  


  /* check for errors */ 
  if(res != CURLE_OK) {
     fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  }
  else {
    /*
     * Now, our chunk.memory points to a memory block that is chunk.size
     * bytes big and contains the remote file.
     */ 

    //This part is for test of header
    //get the name of file.
    char *wrkptr = NULL;
    char name[100];
    int key = 0;
    wrkptr = header.buffer;

    do{
     

    if(!strncmp("filename",wrkptr,8) && key == 0)
      {
	wrkptr +=10;
	int j = 0;
	while(*wrkptr!='\n')
	  {
	    name[j] = *wrkptr;
	    j++;
	    wrkptr++;
	  }
	name[j-1] = '\0';
      }

    if(*wrkptr == '\n')
      key = 0;
    else 
      key = 1;


      wrkptr++;
  
    }while(*wrkptr!='\0');
    


     FILE *outptr = fopen(name,"w");
  
     if(!outptr)
  
       goto clean;
     
     rewind(outptr);
  
     fwrite(chunk.buffer,1,chunk.size,outptr);
  
     fclose(outptr);


    printf("%lu bytes retrieved\n", (long)chunk.size);
  
    printf("%lu bytes header retrieved\n", (long)header.size);
  
  clean: printf("end of write data\n");
 
  }
 
  /* cleanup curl stuff */ 
  curl_easy_cleanup(curl_handle);
  
  printf("%s\n",header.buffer);
  
  free(chunk.buffer);
  
  free(header.buffer);
  
  /* we're done with libcurl, so clean it up */ 
  
  curl_global_cleanup();


 
  return ;



}





 void createcollection(char *coll_name,char *owner_id)
{

  CURL *curl_handle;
  
  CURLcode res;
  
  char url[512];
  
  curl_global_init(CURL_GLOBAL_ALL);
 
  curl_handle = curl_easy_init();
  
  char *collname=curl_easy_escape(curl_handle,coll_name,0);
  
  sprintf(url, "http://dev.mygeohub.org/api/idata/collection/create?name=%s&owner_id=%s", collname,owner_id);
  
  fprintf(stdout, "%s",url);
  
  curl_free(collname);
  
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);

    res = curl_easy_perform( curl_handle );
    
    curl_easy_cleanup(curl_handle);
    
    curl_global_cleanup();
    
    return;
}

