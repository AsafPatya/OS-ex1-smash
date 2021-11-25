#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;
extern SmallShell &smash;

void ctrlZHandler(int sig_num) {
    signal(SIGSTOP, &ctrlCHandler);
//    signal(SIGSTOP, &ctrlZHandler);todo: use this line instead?
    std::cout << "smash: got ctrl-Z" << std::endl;
    int fgprocess = smash.get_fg_process();
    if (fgprocess != 0) {
        auto jobs = smash.get_ptr_to_jobslist();
        int jobID = jobs->get_job_id_by_pid(fgprocess);
        jobs.get_map().find(jobID)->second.setStopped(true);
        if (kill(fgprocess, SIGSTOP) == -1) {
            perror("smash error: kill failed");
            return;
        }
        std::cout << "smash: process " << fgprocess << " was stopped" << std::endl;
    }
}

void ctrlCHandler(int sig_num) {
    signal(SIGINT, &ctrlCHandler);
    std::cout << "smash: got ctrl-C" << std::endl;
    int fgprocess = smash.get_fg_process();
    if (fgprocess != 0) {
        int ans = killpg(fgprocess, SIGKILL);
        if (ans == -1) {
            perror("smash error: kill failed");
            return;
        }
        std::cout << "smash: process " << fgprocess << " was killed" << std::endl;
    }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

