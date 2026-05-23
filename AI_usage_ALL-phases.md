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
I used ChatGPT to help me with the signal handling setup in `monitor_reports.c` and with the `notify_monitor()` function in `city_manager.c`
### Prompts given to AI
***First prompt for signal handling:***
I need to write a C program on Linux called monitor_reports. It should write its own PID to a file called .monitor_pid on startup, delete that file on exit, print a message when it receives SIGUSR1, and print a shutdown message and exit when it receives SIGINT. It must use sigaction() and not signal(). Can you show me how to set up the signal handlers?

***Second prompt for notify_monitor():***
I have a C program that needs to read a PID from a file called .monitor_pid and then send SIGUSR1 to that process using kill(). It should return 1 on success and 0 if anything fails. How should I write this using open() and read() instead of fopen()? 

### What the AI generated
***First prompt:***
The AI gave me:
- two `volatile sig_atomic_t` flags (`got_sigusr1` and `got_sigint`) set inside the handlers
- `sigaction()` calls for both signals with `SA_RESTART` set on both
- a `while (!got_sigint) { pause(); }` loop that checks the flags after each signal

***Second prompt:***
The AI gave me:
- `open()` and `read()` to read the PID file into a char buffer
- `strtol()` to convert the string to a `pid_t`
- a `kill(pid, SIGUSR1)` call with the return value checked
- returns 0 if the file can't be opened, the PID is invalid, or `kill()` fails

### What I changed and why
***First prompt:***
- removed `SA_RESTART` from the SIGINT handler because with it set, `pause()` would restart instead of returning, so the loop would never actually check `got_sigint` and the program would not shut down
- added `fflush(stdout)` after every `printf` because when the program runs in the background stdout can be buffered and messages would not appear immediately

***Second prompt:***
- added a `pid <= 0` check before calling `kill()` because if the file contains garbage or a zero, `kill(0, SIGUSR1)` would send the signal to every process in the process group which is dangerous
- the logging of success or failure into `logged_district` was written manually since the AI only generated the function itself and not the calling code

### What I learned
- the difference between `SA_RESTART` and not using it, and why it matters depending on which system call the signal is supposed to interrupt
- that signal handlers should only set a flag and that all real work should be done in the main loop to stay async-signal-safe
- how `kill()` works and why checking the PID before sending a signal is important
- how `fork()`, `execlp()`, and `waitpid()` work together to run an external command from a C program

## Phase 3
### AI tool used
I used ChatGPT to help me understand how to chain two `fork()` calls to create the hub_mon intermediate process in `city_hub.c`, and how to set up `dup2()` correctly for the scorer pipes.

### Prompts given to AI
***First prompt for hub_mon pipe setup:***
I have a C program called city_hub. When the user types start_monitor, I need to fork a background child (hub_mon) that itself forks the monitor_reports executable and reads its stdout through a pipe. How do I set up the pipe and dup2 so that monitor_reports writes to the pipe and hub_mon reads from it?

***Second prompt for scorer pipes:***
I need to spawn multiple external processes in parallel, each with their stdout redirected to a separate pipe, then read all their output from the parent. How do I use pipe() and dup2() to do this in C?

### What the AI generated
***First prompt:***
The AI gave me:
- a `pipe(pipefd)` call before the second `fork()`
- in the monitor child: `close(pipefd[0])`, then `dup2(pipefd[1], STDOUT_FILENO)`, then `close(pipefd[1])`, then `execl()`
- in hub_mon: `close(pipefd[1])` then reading from `pipefd[0]` with `fgets()` in a loop

***Second prompt:***
The AI gave me:
- a loop that calls `pipe()` and `fork()` for each district
- in each child: `dup2(pipefd[1], STDOUT_FILENO)` then `execl()` for the scorer
- in the parent: storing all read-end fds in an array, then reading from each one after all children are launched

### What I changed and why
***First prompt:***
- the AI used `read()` byte by byte to read lines from the pipe, which works but is very slow. Replaced with `fdopen()` + `fgets()` for cleaner line-by-line reading
- the AI did not add `setlinebuf(stdout)` in `monitor_reports.c`. Without this, stdout is fully buffered when it goes to a pipe and messages would be delayed or never appear. Added `setlinebuf(stdout)` at the start of `monitor_reports`
- the AI did not handle the case where the pipe closes without an END message (monitor crash). Added a fallback message for that case

***Second prompt:***
- the AI collected scorer output one child at a time (spawn one, read it, then spawn the next). Changed this to spawn all children first, then read them all, so the scorers actually run in parallel as the spec requires
- added `waitpid()` after reading each scorer's pipe to properly reap the child process and avoid zombies

### What I learned
- how `dup2(pipefd[1], STDOUT_FILENO)` replaces file descriptor 1 with the write end of the pipe, so any `printf` in the child process automatically goes to the pipe
- why you must close the write end of the pipe in the parent after forking, otherwise `fgets()` on the read end never returns EOF even when the child exits
- the difference between buffered output (stdio) and unbuffered output (raw `write()`), and why `setlinebuf` matters when a program's stdout is connected to a pipe instead of a terminal
- how to run multiple child processes in parallel and collect their results in order using an array of pipe file descriptors
