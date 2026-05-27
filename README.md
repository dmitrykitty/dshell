# DShell

DShell is a small Unix-like shell written in C. It supports built-in commands, external programs, one pipe, input/output redirection, append redirection, background jobs, command history, signal handling, and an asynchronous logger.

## Build

```sh
make
```

Run it:

```sh
./dshell
```

Run the example test commands from this README:

```sh
make test
```

## Main Ideas

The shell loop is implemented in `src/shell.c`. It reads a line with `getline`, trims it, stores it in history, parses it into a command or pipeline, then either runs a built-in or forks external commands.

Parsing is intentionally simple. `src/parser.c` splits input by whitespace with `strtok`. This means commands like `echo hello` work, but quotes, escapes, variable expansion and globbing are not implemented. DShell supports only one pipe per line.

External commands are handled in `src/executor.c`. The parent process calls `fork`. The child restores default signal handlers, applies redirection with `open` and `dup2`, then replaces itself with the target program using `execvp`. The parent waits for foreground commands with `waitpid`, or records background commands in the job table.

Pipes use `pipe`, `fork`, and `dup2`. The left child writes to the pipe write end through `STDOUT_FILENO`; the right child reads from the pipe read end through `STDIN_FILENO`. DShell supports input redirection on the left command and output redirection on the right command.

Redirections use normal file descriptors:

- `< file` opens the file read-only and moves it to stdin.
- `> file` creates/truncates the file and moves it to stdout.
- `>> file` creates/appends to the file and moves it to stdout.

Signals are in `src/signals.c`. The shell catches `SIGINT` so Ctrl+C redraws the prompt instead of killing the shell. Children restore default signal handling so external commands behave normally. `SIGCHLD` only sets a small atomic flag; the shell loop later refreshes background job status with `waitpid(..., WNOHANG)`.

Jobs are tracked in `src/jobs.c`. A background command created with `&` gets an id, pid, status and saved command text. Use `jobs` to list them and `kill %ID` or `kill PID` to terminate a running job.

History is stored in memory as a ring buffer and appended to `~/.dshell_history`. The `history` built-in prints the current in-memory history.

The logger is implemented in `src/logger.c`. The main shell thread pushes messages into a fixed-size queue protected by a mutex. A logger thread waits on a semaphore, pops messages, adds timestamps, and writes to `.dshell.log`. Logging is done from the shell process, not from child processes while external commands are running.

## Signals

DShell installs signal handlers once during startup with `signals_init`.

`SIGINT` is the signal normally produced by Ctrl+C. In the shell process, DShell handles it by writing only a newline. The handler uses `write`, not `printf`, because only async-signal-safe functions should be used inside signal handlers. `SA_RESTART` is intentionally not used for `SIGINT`; this lets `getline` return `EINTR`, after which the shell clears the stream error and shows a fresh prompt.

`SIGCHLD` is delivered when a child process exits or is terminated. The handler does not call `waitpid` directly. It only sets a `volatile sig_atomic_t` flag, which is safe to modify inside a signal handler. The main shell loop checks that flag and calls `job_table_refresh` later, outside signal-handler context.

Child processes call `signals_restore_defaults_for_child` before `execvp`. This matters because external programs should receive normal Ctrl+C behavior instead of inheriting the shell's custom handlers.

## Jobs

Background execution is enabled with `&` at the end of a command:

```sh
./test/bin/wait_and_print &
```

For background commands, the parent shell does not wait immediately. It stores a `Job` with:

- shell job id, used as `%1`, `%2`, and so on;
- process id, used by the operating system;
- status: running, done, or terminated;
- exit code or terminating signal;
- original command text.

`job_table_refresh` uses `waitpid(pid, &status, WNOHANG)`. `WNOHANG` is important because it lets the shell check job status without blocking the prompt. When a child is done, `waitpid` also reaps it, preventing zombie processes.

The `jobs` built-in prints every known job. Completed jobs currently remain in the table, so repeated `jobs` calls can still show finished background commands.

The `kill` built-in supports both operating-system pids and DShell job ids:

```sh
kill 12345
kill %1
```

## Built-In Commands

```text
help
pwd
cd [path]
exit
env
setenv NAME VALUE
unsetenv NAME
printenv NAME
history
jobs
kill PID
kill %JOB_ID
```

## Example Commands

Copy these commands inside DShell after running `make test` once, because the test target compiles helper programs into `test/bin`.

```sh
help
pwd
setenv DSHELL_MODE demo
printenv DSHELL_MODE
unsetenv DSHELL_MODE
./test/bin/shift_numbers < test/fixtures/numbers.txt > test/tmp/shifted.txt
./test/bin/shift_numbers < test/fixtures/numbers.txt | ./test/bin/sum_pairs > test/tmp/pipeline.txt
./test/bin/shift_numbers < test/fixtures/numbers.txt >> test/tmp/shifted.txt
./test/bin/wait_and_print &
jobs
sleep 2
jobs
history
exit
```

The most complete example is:

```sh
./test/bin/shift_numbers < test/fixtures/numbers.txt | ./test/bin/sum_pairs > test/tmp/pipeline.txt
```

It uses input redirection, a pipe, two external programs, and output redirection.

## Current Limitations

- Only one pipe is supported.
- No quoting, escaping, command substitution, variables or glob expansion.
- Built-ins do not currently support redirection.
- Background pipelines are rejected.
- Job entries stay in the table after completion.
