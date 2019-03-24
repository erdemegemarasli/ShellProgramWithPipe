# ShellProgramWithPipe
CS342-Operating System Project 1

  Simple Shell Program implemented in C to execute linux commands.
This shell program also support composition of two commands where the output of
one command will be given to another command as input. For example, there is
“ps aux” program in Linux that is listing the current processes, and there is “sort”
program in Linux that is sorting a text file. When we write “ps aux | sort” in
Linux shell, it prints to the screen the sorted list of processes. Similarly, when
we would write such a command line in your shell, it should also print a listing
of processes in sorted order. Such a command line consists of two commands, with
possible parameters, separated with | symbol. The symbol | is called the pipe symbol.
This shell program supports use of only one pipe symbol in a command line, hence the
compound execution of two commands in a command line.
