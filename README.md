# concise_json_schema

master: [![Build Status](https://travis-ci.com/sizmailov/concise_json_schema.svg?branch=master)](https://travis-ci.com/sizmailov/concise_json_schema)


Introduction 
------------

Schema has 15 basic types and allows to define user types (see #define section).

| Type name     | Schema Example|  JSON example    | 
| ------------- |:-------------| :-----|
| **allOf**     | `allOf(int)`    | `2`|
| **any**       |  `any`    | `null`|
| **anyOf**     |  `anyOf(int,double)`    | `3.14`|
| **array**     |  `[any]`    | `[1,2,3]`|
| **bool**      |  `bool`    | `true`|
| **enum**      |  `enum(2,12,85,0,6)`    | `85`|
| **int**       |  `int(1..42)`    | `42`|
| **not**       |  `not(int)`    | `"hello"`|
| **null**      |  `null`    | `null`|
| **double**    |  `double`    | `1e-9`|
| **oneOf**     |  `oneOf(int,null)`    | `1`|
| **object**    |  `{"x": str}`    | `{"x":"y"}`|
| **reference** |  `@example`    | -- |
| **string**    |  `str`    | `"hello world"` |
| **tuple**     |  `(int,double)`    | `[1,2]`|



Comparison with JSON Schema
---------------------------

Note: JSON Schema `{"items" : {"type":"integer"}}` matches JSON objects `1` or `null`, so field `"type" : "array"` is 
required to constrain type. 

#### to JSON Schema

| Comment | Schema Example|  JSON Schema analog    | 
| ------------- |:-------------| :-----|
| array of ints   | `[int]`  |  `{ "type": "array", "items" : {"type": "integer"} }`|
| array of 5 ints  | `[int]{5}`  |  `{ "type": "array", "items" : {"type" :"integer"}, "minItems" : 5 }`|
| pair of int and double| `(int,double)`  |  `{ "type": "array", "items" : [ { "type" : "integer"}, { "type" : "number"}], "additionalItems": false }`|
| object with two int fields | `{"x":int, "y":int}`  |  `{ "type": "object", "properties" : {"x" : { "type" : "integer" }, "y": {"type" : "integer"}}, "additionalProperties" : false}`|

#### from JSON Schema

| Comment | JSON Schema | Schema Example |  
|---------|:--------------| :-----|
|negative number or not number| `{"maximum":0, "exclusiveMaximum":0}` | `allOf(double(..0),not(enum(0.0,0))`|
|negative int or not int | `{"maximum":0, "exclusiveMaximum":0}` | `oneOf(int(..-1),not(int))`|
|not array or array of (not arrays or arrays of integers)| `{"items":{"items": { "type" : "integer"}}}` | `oneOf(not([any]),[oneOf(not([any]),[int])])`|




There are several differences with JSON Schema. 

#### Advantages over JSON Schema
* It's shorter. Like ... a lot.
* Schema looks pretty much like valid data (e.g `{"x":int, "y":int}`)
* Easier to read / write
* Intrinsic differentiation between tuples and arrays
* Recursive structures are not allowed (i.e. infinite loops are impossible)

#### Disadvantages over JSON Schema
* Schema itself is not a JSON, but separated format, so separate parser is required
* Cumbersome expression of open intervals (e.g. `allOf(double(0..1), not(enum(0.0)))`)  
* No property dependencies feature (yet?)
* No external references (yet, see roadmap)
* Extensions/Format update will brake older parsers
* No `multipleOf` analog


Grammar
----
<todo>



Inline examples
----


| Schema |  JSON |  Matches |  Comment | 
|:-------------| :-----|:-----| ------|
|`any`   | `null ` |   true | _any_ matches anything 
|`any`   | `"str"` |   true |
|`any`   | `1` | true |  
|`allOf(str,any,str("he.*"))`| `"hello"`| true| _allOf_ matches iff all items-schemas match
|`allOf(int,any)` |  `1`  | true 
| `int       ` | `1`| true | matches any _int_ 
| `int(1..10)` | `1`| true | limits are inclusive
| `int()     ` | `1`| true | same as `int` 
| `int(..)   ` | `1`| true | same as `int`
| `anyOf(int,str,bool)` |`true`| true | _anyOf_ matches if one or more items-schemas match
| `anyOf(int,str,bool)` |`3.14`| false  
| `bool` |  `true` | true
| `bool` |  `false` | true
| `enum(1,"2")`| `2` | false | _enum_ matches only listed jsons   
| `enum(1,"2")`| `"2"` | true |    
| `not(int)` |  `5.5` | true | not matches iff items-schema fails
| `null` |  `null` | true | _null_ present for completeness 
| `double(1.5..10.0)` | `1.5` | true | rules for _int_ and _double_ are same  
| `oneOf(int,str,bool)` |  `true` | true | _oneOf_ matches only if exactly only one items-schema matches 
| `oneOf(int,double)` | `42` | false | _int_ is valid _double_ as well  
| `{}` | `{}` | true | empty object
| `{ "x" : int}` |  `{ }` | false| mandatory property missed
| `{ "x" : int}` |  `{ "x":2 }` | true | mandatory property provided 
| `{ ?"x" : int}` |  `{ }` | true | optional property is not provided
| `{ ?"x" : int}` |  `{ "x":2}` | true | optional property provided
| `{ ?"x" : int = 5}` |  `{ }` | true | defaulted property is not provided 
| `{ ?"x" : int = 5}` |  `{"x":2 }` | true | defaulted property provided
| `{ re"dbl_.+" : double}` |  `{"dbl_x": 2}` | true | pattern property provided
| `{ "x":int, re".*":double}` |  `{"x": 2}` | false | pattern property mismatch
| `{ }` |  `{"z":2 }` | false | unknown property
| `str` |  `"foo"` | true| matches any string
| `str{3}` |  `"bar"` | true| string of length 3
| `str{,3}` |  `"bar"` | true| string of length 3 or less 
| `str{3,}` |  `"bar"` | true| string of length 3 or more
| `str{3,10}` |  `"foobar"` | true| string of length from 3 to 10
| `str("[A-Z]")` |  `"FOO"` | true| string with regex constraint (matches whole string)  
| `str("A")` |  `"AAA"` | false | regex does not match
| `str("A*"){,5}` |  `"AAA"` | true | matches up to 5 "A"
| `[any]` |  `[1,"s",{}]` | true | matches any array   
| `[int]{1,5}` |  `[1,"s",{}]` | true | matches _int_ array of size 1 to 5   
| `[ unique int]{,5}` |  `[1,2,3]` | true | matches array of unique ints with size <=5   
| `[ unique int]` |  `[1,2,3,4,1]` | false | matches array of unique ints    
| `(int,int)` |  `[1,2]` | true | matches pair of ints   
| `(int,int,str)` |  `[1,2,"s"]` | true | matches tuple of two ints and string   
| `not(null)` |  `[]` | true | matches all except null   
| `not(anyOf(bool,null))` |  `12345` | true | doesn't match null or bool   

Multiline examples
----

```
/* --- Simple define example --- */
#posInt int(1..)#
/**array of positive integers */
[@posInt]


/* --- Defintion scope --- */
/*definitions are visible in arbirtrary nested scopes*/
#posInt int(1..)#
/**multidimensional array of positive ints*/
[[[[[[[@posInt]]]]]]]


/* --- Multiple definitions --- */
/** schema may have arbitrary number of definitions */
#int25  int(2..5)#
#str4 str{4}#
/**tuple of int and two strings*/
( 
  @int25,
  @str4,
  @str4
)


/* --- Shadow(i.e. hide) definition --- */
/*outter definition might be hided by the nested one*/
#x int#
/**tuple of str and int arrays*/
( 
  #x str#
  /* local define hides outer definition, `@x` matches string */
  [@x],
  /* @x matches int again*/
  [@x]
)


/* --- Docstrins concatenation --- */
/**docstrings may appear before definition [1]*/
#x int#
/**, between definitions [2]*/
#y int#
/** and after definition [3]*/
/** The resulting docstring will contain concatenated docstring*/
(@x, @y)


/* --- Definition inside definition--- */
#y
  #invisible 
    /*@invisible is visible only inside @y*/
    str
  #
  (int, @invisible)
#
/*@invisible is inaccessible here*/
[@y]


```


What is matched 
---------------

 

### **allOf**     


**Match criteria:**  JSON value matches all sub-schemas. 

**Note:** Match procedure goes from first sub-schema to last one. Order may matter 
if sub-schemas perform default value substitution, see "Defaults substitution" section for further details

**Examples:**

```
  allOf(int,enum(2,3,5,7,11)) /* dummy example */
  
  allOf(extensible { "x" : int(1..)}, 
        extensible { "y" : int(1..)}
       )
  
  allOf({?"x" : int = 2 },
        { "x" : int(1..) }
       ) /* matches {} and {"x": 1} */
       
       
  allOf({ "x" : int(1..), 
        {?"x" : int = 2 }}
       ) /* matches {"x": 1}, but not {} */
```

### **any**

**Match criteria:**  JSON value always matches. 


### **anyOf**     

**Match criteria:** JSON value matches at least one sub-schema. 

**Note:** Match procedure is short-circuit and goes from first sub-schema to last one. Order may matter 
if sub-schemas perform default value substitution, see "Defaults substitution" section for further details. 


C++ Examples
------------

to be done 

```c++

```


Known issues
------------
* schemas with default arguments alters validated JSON during check
