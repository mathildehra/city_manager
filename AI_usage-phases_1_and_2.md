# AI Usage

## Phase 1
### AI tool used 
I used ChatGPT to help me with two functions:  `int parse_condition()` and `int match_condition()`

### Prompts given to AI
***First prompt for first function:***
I have a city management system in C where each district is a different directory and in each directory there are three files: reports.dat a binary report file with issues related to the city, district.cfg storing a severity threshold, and logged_district containing all actions performed in the system. Can you implement a function int pars_condition(const char *input, char *field, char *op, char *value) which splits a field:operator:value (ex severity:>=:2) string into its three parts.

***Second prompt for second function:***
Now implement a int match_condition(Report *r, const char *field, const char *op, const char *value) where field can either be int severity, char category, char inspector, time_t timestamp. The operations can be: ==, !=, <, <=, >, >=. The function returns 1 if the record satisfies the condition and 0 otherwise. 

### What the AI generated
***First prompt:***
The AI gave me a function with:
- pointers for finding the ':'
- null pointer checks
- search for first then second ':' in the input
- associates field, op and value to the right value

***Second prompt:***
The function had:
- comparisons for field to match (severity, category, inspector or timestamp)
- checks for the operations for each field
- returns 1 if the condition is true and 0 if not

### What I changed and why
***First prompt:***
- renamed some variables for easier read ("colon1/2" to "c1/2")
- restructured the if conditions for easier read also

***Second prompt:***
- for the category and inspector fields, I deleted some operations (<, <=, >, >=) because they are strings therefore only == and != are necessary. It made the code shorter and more efficient


### What I learned
- the difference between strncpy() and strcpy() and also strchr()
- how to parse strings to find a specific character
- how to write a .md file


## Phase 2
### AI tool used

### Prompts given to AI

### What the AI generated

### What I changed and why

### What I learned
