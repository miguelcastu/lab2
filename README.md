# C Minishell — Operating Systems Coursework

A historical POSIX C exercise exploring process creation, command execution,
pipelines, redirection and shell control flow. This is a learning implementation,
not a replacement for a production shell.

[Engineering portfolio](https://github.com/miguelcastu/miguelcastu) · [Project directory](https://github.com/miguelcastu/miguelcastu/blob/main/PROJECTS.md)

## Source map

- [msh.c](p2_minishell_2023/msh.c): command processing and process/thread logic.
- [Makefile](p2_minishell_2023/Makefile): compilation and parser linkage.
- [libparser.so](p2_minishell_2023/libparser.so): supplied binary parser dependency.
- The checker script and authors file retain the original coursework context.

## Build and explore

Use Linux or a compatible WSL environment with GCC, Make, pthreads and an ABI
compatible with the supplied parser library:

```shell
cd p2_minishell_2023
make
./msh
```

The library is a platform-specific dependency; native Windows execution and a
fresh Linux build have not been verified in the portfolio audit. Inspect the
source before running arbitrary commands: this shell executes local processes.
The historical checker is preserved, not presented as passing CI.
