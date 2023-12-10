# Build

Open root directory in a console and type
~~~
make server
make client
~~~

# Run

## Client
Open root directory in a console and type
~~~
./Client "path to config file"
~~~

## Server
Open root directory in a console and type
~~~
./Server "path to config file"
~~~

# Usage
> ## **Integer overflow behaviour is unpredictable** 

## Configuration files:
Examples of client and server configs with all explanaions are in config.cfg files in coresponding folders

## Client commands:
~~~
add field_name1_1: field_value_1, field_name_2: field_value_2, ... field_name_n: field_value_n 
~~~
- Adds record to the end of DataBase
~~~
edit idx field_name1_1: field_value_1, field_name_2: field_value_2, ... field_name_n: field_value_n 
~~~ 
- Edits record with id equal to idx if exists
- If field is skipped, then it reamins unchanged
- If record does not exists shows error message in console
### **2 Commands above can be used with either all names provided or none (if none, values are used in order written in config file). If name is missing, default value will be used insted. If no names are provided, skipping value is impossible.**
### **Name Skipping looks like: field_value_1, field_value_2, ... field_value_n**

~~~
remove idx
~~~
- Removes record with id value equal to idx if exists
- If record does not exists does nothing
~~~
get idx
~~~
- Gives record with id value equal to idx if exists
- If record does not exists shows error message in console
~~~
dump
~~~
- Gives all records in DataBase.
~~~
filter field_name1_1: field_value_1, field_name_2: field_value_2, ... field_name_n: field_value_n 
~~~
- Shows all records with fields same as provided. If name is skipped it will be ignored.
~~~
size
~~~
- Gives size of data base (including gaps)
~~~
save
~~~
- Saves database
~~~
stop
~~~
- Saves database and stops the server

## Server:
No user actions needed.