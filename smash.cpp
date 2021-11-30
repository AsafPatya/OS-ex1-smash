#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

SmallShell& smash = SmallShell::getInstance();

int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    //TODO: setup sig alarm handler
    struct sigaction action;
    action.sa_handler = alarmHandler;
    action.sa_flags = SA_RESTART;

    int res = sigaction(SIGALRM, &action, 0);
    if (res == -1) {
        perror("smash error: failed to set alarm handler");
    }

    while(!smash.isquit) {
        std::cout << smash.getPrompt();
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.get_ptr_to_jobslist()->removeFinishedJobs();
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}