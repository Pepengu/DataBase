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
./Client
~~~

## Server
Open root directory in a console and type
~~~
./Server
~~~

# Usage
> ## **Integer overflow behaviour is unpredictable** 

## Configuration files:
Examples of client and server configs with all explanaions are in config.cfg files in coresponding folders

## Client commands:
~~~
add field_name1_1: field_value_1, field_name_2: field_value_2, ... field_name_n: field_value_n 
edit idx field_name1_1: field_value_1, field_name_2: field_value_2, ... field_name_n: field_value_n 
~~~ 
- Commands above can be used with either all names provided or none (if none, values are used in order written in config file). If name is missing, default value will be used insted. If no names are provided, skipping value is impossible.
Name Skippint looks like: field_value_1, field_value_2, ... field_value_n
~~~
remove idx
~~~
~~~
get idx
~~~
~~~
dump
~~~
- Used to get all records in DataBase.
~~~
filter field_name1_1: field_value_1, field_name_2: field_value_2, ... field_name_n: field_value_n 
~~~
- Shows all records with fields same as provided. If name is skipped it will be ignored.

## Server:
No user actions needed.