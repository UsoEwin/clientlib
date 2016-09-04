##Readme

#All urls used in those function are on the virtual machine (dev)


#1.WriteMemoryCallback
This function will be used to write the buffer with information, it was used in the getusercollection function.

#2.getusercollections
This function will use the user's id or username as input and returns a pointer which points to a array,
the array stores the decoded JSON response, in order to display the information of the array, the arra_print function
will be used, when finish using this function, the arra_clean must be called to clean the memory.

#3.arra_clean
This function will take a *** type array, which is created by getusercollections, as input and clean the memory.

#4.arra_print
This function will take a *** type array, which is created by getusercollections, as input and print the result to stdout,
if user want the result to be print in certain buffer, please change the code (basically the fprintf function ).

#5.listusercollection
This function will take users <coll-id-or-name> <path> <user-id/user-name><-s/-a> as input, list the contents in certain path 
recursively or not. The result will be print to stdout, if want the result to be print in any other buffer, please modify the 
fprintf function.

#6.getFile
This function will simply download the content at given address(doi)

#7.createcollection
This function will create a collection.

#8.initialized 
This function will display needed information to stdout buffer, if user need to know the 
input of those function, please refer to this.
