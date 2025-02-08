# not-your-usual-shell
This is a project assigned in CS 202 by Professor Tang at NYU.
This project demonstrates my ability to navigate the Linux programming environment and shell, write interactive command-line programs, manage process creation and destruction, and handle signals and I/O redirection effectively.

## Built in Commands

Every shell has a few built-in commands. When the user issues a command, the shell should first check if it is a built-in command. If so, it should not be executed like other programs.

Here, four built-in commands- cd, jobs, fg, and exit- are implemented.

cd <dir>
This command changes the current working directory of the shell. It takes exactly one argument: the directory, which may be an absolute or relative path. For example:
```
[nyush lab2]$ cd /usr/local
[nyush local]$ cd bin
[nyush bin]$ █
```
If cd is called with 0 or 2+ arguments, the shell prints the following error message to STDERR and prompt for the next command.
```
Error: invalid command
```
If the directory does not exist, the shell prints the following error message to STDERR and prompt for the next command.
```
Error: invalid directory
```
jobs
This command prints a list of currently suspended jobs to STDOUT, one job per line. Each line has the following format: [index] command. For example:
```
[nyush lab2]$ jobs
[1] ./hello
[2] /usr/bin/top -c
[3] cat > output.txt
[nyush lab2]$ █
```
(If there are no currently suspended jobs, this command should print nothing.)

A job is the whole command, including any arguments and I/O redirections. A job may be suspended by Ctrl-Z, the SIGTSTP signal, or the SIGSTOP signal. This list is sorted by the time each job is suspended (oldest first), and the index starts from 1.



For simplicity, we have the following assumptions:

There are no more than 100 suspended jobs at one time.
There are no pipes in any suspended jobs.
The only way to resume a suspended job is by using the fg command (see below). We will not try to resume or terminate a suspended job by other means. We will not try to press Ctrl-C or Ctrl-D while there are suspended jobs.
The jobs command takes no arguments. If it is called with any arguments, the shell should print the following error message to STDERR and prompt for the next command.
```
Error: invalid command
```
fg <index>
This command resumes a job in the foreground. It takes exactly one argument: the job index, which is the number inside the bracket printed by the jobs command. For example:
```
[nyush lab2]$ jobs
[1] ./hello
[2] /usr/bin/top -c
[3] cat > output.txt
[nyush lab2]$ fg 2
```
The last command would resume /usr/bin/top -c in the foreground. Note that the job index of cat > output.txt would become 2 as a result. Should the job /usr/bin/top -c be suspended again, it would be inserted to the end of the job list:
```
[nyush lab2]$ jobs
[1] ./hello
[2] cat > output.txt
[3] /usr/bin/top -c
[nyush lab2]$ █
```
If fg is called with 0 or 2+ arguments, the shell prints the following error message to STDERR and prompt for the next command.

```
Error: invalid command
```
If the job index does not exist in the list of currently suspended jobs, the shell should print the following error message to STDERR and prompt for the next command.

```
Error: invalid job
exit
```
This command terminates the shell. However, if there are currently suspended jobs, the shell does not terminate. Instead, it should print the following error message to STDERR and prompt for the next command.
```
Error: there are suspended jobs
The exit command takes no arguments. If it is called with any arguments, the shell prints the following error message to STDERR and prompt for the next command.
```
```
Error: invalid command
```
Note that if the STDIN of the shell is closed (e.g., by pressing Ctrl-D at the prompt), the shell terminates regardless of whether there are suspended jobs.
