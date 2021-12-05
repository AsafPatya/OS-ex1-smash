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
        auto je = jobs->get_map().find(jobID);
        auto job = je->second;
//        if(je == jobs->get_map().end()){
//            JobsList::addJob();
//        }
        if (kill(fgprocess, SIGSTOP) == -1) {
//            smashError("kill failed", true);
            perror("smash error: kill failed");
            return;
        }
        job.setStopped(true);
        job.setBackground(true);
        std::cout << "smash: process " << fgprocess << " was stopped" << std::endl;
    }
}

void ctrlCHandler(int sig_num) {
    signal(SIGINT, &ctrlCHandler);
    std::cout << "smash: got ctrl-C" << std::endl;
    int fgProcessId = smash.get_fg_process();
    if (fgProcessId != 0) {
        if (killpg(fgProcessId, SIGKILL) == -1) {
//            smashError("smash error: kill failed", true);
            perror("smash error: kill failed");
            return;
        }
        std::cout << "smash: process " << fgProcessId << " was killed" << std::endl;
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

        auto alarm = tl.getTimeMap().find(time_Id)->second;

        int pid = alarm.getPid();
        int job_id = alarm.getJobId();
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

            if (killpg(pid, SIGINT) == -1) {
//                smashError("smash error: kill failed", true);
                perror("smash error: kill failed");
                return;
            }
            auto alarm = tl.getTimeMap().find(time_Id)->second;
            auto  job = jl.get_map().find(job_id)->second;

            std::cout << "smash: " << alarm.getCommand() << " timed out!" << std::endl;
            if (job.if_is_background() || job.if_is_stopped()) {
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

