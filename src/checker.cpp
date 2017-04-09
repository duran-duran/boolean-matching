#include "checker.h"
#include "defines.h"

#include <cstring>
#include <unistd.h>

// procedures to start and stop the ABC framework
// (should be called before and after the ABC procedures are called)
extern "C" void   Abc_Start();
extern "C" void   Abc_Stop();

// procedures to get the ABC framework and execute commands in it
extern "C" void * Abc_FrameGetGlobalFrame();
extern "C" int    Cmd_CommandExecute( void * pAbc, char * sCommand );

int checkMiter(const char *miter_file)
{
    // save stdout's descriptor
    int old_fd = dup(1);

    // declare variables
    static void * pAbc = nullptr;
    char Command[BUF_LEN];
    char Result[BUF_LEN];

    // start the ABC framework
    if (!pAbc) {
        Abc_Start();
        pAbc = Abc_FrameGetGlobalFrame();
    }

    sprintf(Command, "read_verilog %s", miter_file);

    // perform verification
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
        dup2(old_fd, 1);
        close(old_fd);
        return FAIL;
    }

    memset(Command, 0, strlen(Command));
    sprintf(Command, "sat");

    // redirect ABC's output into Result[]
    if (freopen("/dev/null", "a", stdout) == NULL) {
        dup2(old_fd, 1);
        close(old_fd);
        return FAIL;
    }
    setbuffer(stdout, Result, BUF_LEN);

    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
        dup2(old_fd, 1);
        close(old_fd);
        return FAIL;
    }

    if (freopen("/dev/tty", "a", stdout) == NULL) {
        dup2(old_fd, 1);
        close(old_fd);
        return FAIL;
    }

    // stop the ABC framework
    //Abc_Stop();

    dup2(old_fd, 1);
    close(old_fd);

    printf("Miter check: %s\n", Result);
    if (strstr(Result, "UNSATISFIABLE")) {
        return 1;
    }
    return 0;
}

bool checkMiter(Circuit *miter)
{
    int old_fd = dup(1);
    const char *tmp_file = "tmp_file";
    freopen(tmp_file, "w", stdout);

    miter->print();
    bool result = checkMiter(tmp_file);

    unlink(tmp_file);
    dup2(old_fd, 1);
    close(old_fd);

    return result;
}
