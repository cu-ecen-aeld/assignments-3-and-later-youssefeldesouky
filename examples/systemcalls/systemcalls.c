#include "systemcalls.h"

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{
    if(!cmd) return false;

    int status = system(cmd);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return true;
    }
    return false;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    va_end(args);

    if(command[0][0] != '/'){
        return false;
    }
    char *path = command[0];
    for(i = strlen(command[0]) - 1; i >= 0 && command[0][i] != '/'; i--);
    command[0] = &(command[0][i + 1]);

    pid_t pid = fork();
    if (pid == (pid_t)-1) {
        return false;
    } else if (pid == (pid_t)0) {
        //inside of child process
        int res = execv(path, command);

        if(res == -1){
            return -1;
        }
    }

    int status = 0;
    pid_t ret_pid = waitpid(pid, &status, 0);
    
    if (ret_pid == (pid_t)-1) {
        return false;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return true;
    }
    return false;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    va_end(args);

    if(command[0][0] != '/'){
        return false;
    }

    char *path = command[0];
    for(i = strlen(command[0]) - 1; i >= 0 && command[0][i] != '/'; i--);
    command[0] = &(command[0][i + 1]);

    pid_t pid = fork();
    if (pid == (pid_t)-1) {
        return false;
    } else if(pid == (pid_t)0) {
        //inside of child process
        int tmp = 0;
        int fd = open(outputfile, O_WRONLY);
        if (fd == -1) {
            return -1;
        }

        tmp = close(STDOUT_FILENO);
        if(tmp == -1){
            return -1;
        }

        tmp = dup(fd);
        if(tmp == -1){
            return -1;
        }

        tmp = close(fd);
        if(tmp == -1){
            return -1;
        }

        int res = execv(path, command);
        if (res == -1) {
            return -1;
        }
    }

    int status = 0;
    pid_t ret_pid = waitpid(pid, &status, 0);
    
    if (ret_pid == (pid_t)-1) {
        return false;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return true;
    }
    return false;
}
