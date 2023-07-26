# Command Line Shell

This project consists of designing a C program to serve as a shell interface that accepts user commands and then executes each command in a separate process.

## Features

1. **Command Execution**: The shell allows users to execute various commands supported by the underlying operating system.

2. **Multiple Processes**: Each command entered by the user is executed in a separate child process, allowing for concurrent execution.

3. **Input/Output Redirection**: The shell supports input and output redirection, enabling users to read input from a file or write output to a file.

4. **Piping**: Users can chain multiple commands together using pipes, allowing the output of one command to serve as input to another command.

5. **Background Execution**: Users can run commands in the background by appending the `&` symbol to the command, freeing the shell for additional input.
