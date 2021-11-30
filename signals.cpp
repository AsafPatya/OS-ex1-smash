#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <string.h>
#include <sys/wait.h>

using namespace std;
extern SmallShell &smash;

//void smashError(string errMsg, bool isKernelError = false){
//    string baseMsg = "smash error: ";
//    if (isKernelError){
//        string msg = baseMsg + errMsg;
//        perror(msg.c_str());
//    }
//    else
//        cerr << baseMsg << errMsg << endl;
//}

void ctrlZHandler(int sig_num) {
    signal(SIGSTOP, &ctrlCHandler);
//    signal(SIGSTOP, &ctrlZHandler);todo: use this line instead?
    std::cout << "smash: got ctrl-Z" << std::endl;
    int fgprocess = smash.get_fg_process();
    if (fgprocess != 0) {
        auto jobs = smash.get_ptr_to_jobslist();
        int jobID = jobs->get_job_id_by_pid(fgprocess);
        jobs->get_map().find(jobID)->second.setStopped(true);
        if (kill(fgprocess, SIGSTOP) == -1) {
//            smashError("kill failed", true);
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
//            smashError("smash error: kill failed", true);
            perror("smash error: kill failed");
            return;
        }
        std::cout << "smash: process " << fgprocess << " was killed" << std::endl;
    }
}
void alarmHandler(int sig_num) {
    std::cout << "smash: got an alarm" << std::endl;
    time_t time_now = time(nullptr);
    TimeList* time_list = smash.get_ptr_to_Timelist();
    int time_Id = time_list->get_TimeId_Of_finished_Timeout(time_now);
    if (time_Id != -1) {

        TimeList tl = smash.getTimeList();
        JobsList* jobs_list = smash.get_ptr_to_jobslist();
        TimeList* time_list = smash.get_ptr_to_Timelist();

        int pid = tl.getTimeMap().find(time_Id)->second.getPid();
        int job_id = tl.getTimeMap().find(time_Id)->second.getJobId();
        int status;
        int ans = waitpid(pid, &status, WNOHANG);
        if (ans != 0) {
            jobs_list->removeJobById(job_id);
            time_list->removeTimeById(time_Id);
            time_list->change_Max_TimeId();

        }
        else{
            TimeList tl = smash.getTimeList();
            JobsList jl = smash.getJobsList();
            JobsList* jobs_list = smash.get_ptr_to_jobslist();
            TimeList* time_list = smash.get_ptr_to_Timelist();

            int ans_2 = killpg(pid, SIGINT);
            if (ans_2 == -1) {
//                smashError("smash error: kill failed", true);
                perror("smash error: kill failed");
                return;
            }
            std::cout << "smash: " << tl.getTimeMap().find(time_Id)->second.getCommand() << " timed out!" << std::endl;
            if (jl.get_map().find(job_id)->second.if_is_background() || jl.get_map().find(job_id)->second.if_is_stopped()) {
                jobs_list->removeJobById(job_id);
            }
            time_list->removeTimeById(time_Id);
            time_list->change_Max_TimeId();
        }
    }
    if (time_list->getMaxId() != 0) {
        time_list->What_is_the_Next_Timeout(time_now);
    }
}

