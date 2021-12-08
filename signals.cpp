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
    int fgprocess = smash.get_foreground_proccess_id();
    if (fgprocess != 0) {
        auto jobs = smash.get_jobs_list_pointer();
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
        job.set_stopped(true);
        job.set_background(true);
        std::cout << "smash: process " << fgprocess << " was stopped" << std::endl;
    }
}

void ctrlCHandler(int sig_num) {
    signal(SIGINT, &ctrlCHandler);
    std::cout << "smash: got ctrl-C" << std::endl;
    int fgProcessId = smash.get_foreground_proccess_id();
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
    TimeList* time_list = smash.get_time_list_pointer();
    int time_Id = time_list->get_ids_of_finished_timeouts(time_now);
    if (time_Id != -1) {

        TimeList tl = smash.get_time_list();
        JobsList* jobs_list = smash.get_jobs_list_pointer();
        TimeList* time_list = smash.get_time_list_pointer();

        auto alarm = tl.get_map().find(time_Id)->second;

        int pid = alarm.get_pid();
        int job_id = alarm.get_job_id();
        int status;
        int ans = waitpid(pid, &status, WNOHANG);
        if (ans != 0) {
            jobs_list->removeJobById(job_id);
            time_list->remove_by_id(time_Id);
            time_list->refresh_time_list();
        }
        else{
            TimeList tl = smash.get_time_list();
            JobsList jl = smash.get_jobs();
            JobsList* jobs_list = smash.get_jobs_list_pointer();
            TimeList* time_list = smash.get_time_list_pointer();

            if (killpg(pid, SIGINT) == -1) {
//                smashError("smash error: kill failed", true);
                perror("smash error: kill failed");
                return;
            }
            auto alarm = tl.get_map().find(time_Id)->second;
            auto  job = jl.get_map().find(job_id)->second;

            std::cout << "smash: " << alarm.get_command() << " timed out!" << std::endl;
            if (job.is_background() || job.is_stopped()) {
                jobs_list->removeJobById(job_id);
            }
            time_list->remove_by_id(time_Id);
            time_list->refresh_time_list();
        }
    }
    if (time_list->get_max_id() != 0) {
        time_list->get_next_time_out(time_now);
    }
}

