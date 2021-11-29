#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <unistd.h>
#include <algorithm>
#include <fcntl.h>




using namespace std;
const std::string WHITESPACE = " \n\r\t\f\v";
extern SmallShell &smash;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

///
/// helper functions
///

string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = (char*)malloc(s.length()+1);
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

///new functions

static vector<string> splitStringToWords(const string &str) {
    vector<string> wordsvec;
    string word = "";
    for(unsigned int i=0;i<str.length();i++){
        if (str.at(i) == ' ' && word != "") {
            wordsvec.push_back(word);
            word = "";
        }
        else {
            word += str.at(i) ;
        }
    }
    if (word != "") {
        wordsvec.push_back(word);
    }
    return wordsvec;
}

bool checkIfInt(const string &str) {
    if (str.empty() || ( (!isdigit(str[0]))&&(str[0] != '-') && (str[0] != '+'))) return false;
    for(unsigned int i=1;i<str.length();i++){
        if(isdigit(str[i])==0){
            return false;
        }
    }
    return true;
}

vector<string> get_param_of_pipe(const string &str) {

    vector<string> result = {"", ""};
    bool is_second_arg = false;

    for(auto ch : str){
        if (ch != '|') {
            if (is_second_arg == false) {
                result[0].push_back(ch);
            } else {
                if (result[1].size() == 0 && ch== ' ') {
                    continue;
                }
                result[1].push_back(ch);
            }
        }
        else {
            is_second_arg = true;
            int last_index = result[0].length()-1;
            if (result[0][last_index] == ' ') {
                result[0].erase(last_index, 1);
            }
        }
    }

    return result;
}


vector<string> get_param_of_pipe_with_arr(const string &str) {

    vector<string> result = {"", ""};
    bool is_second_arg = false;
    bool found_first= false;


    for(auto ch:str){
        if(found_first==true){
            found_first=false;
            continue;
        }
        else if (ch != '|') {
            if (is_second_arg == false) {
                result[0].push_back(ch);
            } else {
                if (result[1].size() == 0 && ch == ' ') {
                    continue;
                }
                result[1].push_back(ch);
            }
        }
        else {
            is_second_arg = true;
            found_first= true;
            int last_index = result[0].length()-1;
            if (result[0][last_index]== ' ') {
                result[0].erase(last_index, 1);
            }
        }
    }

    return result;
}


vector<string> get_param_case_override(const string &str) {

    vector<string> result = {"", ""};
    bool is_second_arg = false;

    for(auto ch:str){
        if (ch != '>') {
            if (is_second_arg == false) {
                result[0].push_back(ch);
            } else {
                if (result[1].size() == 0 && ch == ' ') {
                    continue;
                }
                result[1].push_back(ch);
            }
        }
        else {
            is_second_arg = true;
            int last_index=result[0].length()-1;
            if (result[0][last_index] == ' ') {
                result[0].erase(last_index, 1);
            }
        }
    }
    return result;
}
vector<string> get_param_case_append(const string &str) {

    vector<string> result = {"", ""};
    bool is_second_arg = false;
    bool found_first= false;
    for(auto ch:str){
        if(found_first== true){
            found_first= false;
            continue;
        }
        else if (ch != '>') {
            if (is_second_arg == false) {
                result[0].push_back(ch);
            } else {
                if (result[1].size() == 0 && ch == ' ') {
                    continue;
                }
                result[1].push_back(ch);
            }
        }
        else {
            is_second_arg = true;
            found_first= true;
            int last_index=result[0].length()-1;
            if (result[0][last_index]== ' ') {
                result[0].erase(last_index, 1);
            }
        }
    }
    return result;
}

bool is_ovveride(const string &str) {
    return (str.find(">") != str.npos);
}

bool is_append(const string &str) {
    return (str.find(">>") != str.npos);
}
bool is_out(const string &str) {
    return (str.find("|") != str.npos);
}
bool is_err(const string &str) {
    return (str.find("|&") != str.npos);
}

///
/// smash helper functions
///

bool xxx(string command, string s){ //todo: change the name
    //todo: remove spaces from the start of the command
    if (s.length() < command.length())
        return false;
    for (unsigned int i = 0; i < command.length(); ++i) {
        if (command.at(i) != s.at(i))
            return false;
    }
    //todo: make sure the command ends with space
    return true;
}

bool isStringCommand(string s, string command){
//    cout << "looking for the command \"" << command << "\" in the string s:  " << s << endl;
    bool b = xxx(command, s);
//    cout << "the command has been" << (b ? "" : " not") << " founded" <<'\n' << endl;
    return b;
    //return command.find(s) == 0;// && s.at(command.length() + 1) == ' ';
}

void smashError(string errMsg, bool isKernelError = false){
    string baseMsg = "smash error: ";
    if (isKernelError){
        string msg = baseMsg + errMsg;
        perror(msg.c_str());
    }
    else
        cerr << baseMsg << errMsg << endl;
}

///
/// commands implementation
///


///
/// #Command (abstract command)
/// \param cmd_line

Command::Command(const char *cmd_line) {
    int len = strlen(cmd_line);
    char *command_to_insert = new char[len + 1];
    strcpy(command_to_insert, cmd_line);
    this->commandLine = command_to_insert;
    vector<string> split_params_to_words = splitStringToWords(this->commandLine);
    for (unsigned int i = 1; i < split_params_to_words.size(); i++) {
        this->params.push_back(split_params_to_words[i]);
    }
}

Command::~Command() {
    delete this->commandLine;
}
bool Command::if_is_stopped() const {
    return this->stopped;
}
void Command::setStopped(bool stopped) {
    this->stopped = stopped;
}
//bool Command::isExternal() const {
//    return this->external;
//}
bool Command::if_is_background() const {
    return this->background;
}
void Command::setBackground(bool background) {
    this->background = background;
}
const char *Command::getCommandLine() const {
    return this->commandLine;
}

bool Command::isExternal() const {
    return this->external;
}

///
/// #BuiltInCommand
/// \param cmd_line

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}

///
/// #ChpromptCommand
/// \param cmd_line

ChpromptCommand::ChpromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
void ChpromptCommand::execute() {
    if (this->params.empty()) {
        smash.setPrompt("smash> ");
    }
    else {
        string newPromptName = this->params.at(0);
        newPromptName.append("> ");
        smash.setPrompt(newPromptName);
    }
}

///
/// #ShowPidCommand
/// \param cmd_line

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
void ShowPidCommand::execute() {
    string res="smash pid is ";
    cout << res<<smash.getPid()<< endl;
}

///
/// #GetCurrDirCommand
/// \param cmd_line

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
void GetCurrDirCommand::execute() {
    char *currDirCommand = get_current_dir_name();
    if (currDirCommand == nullptr) {
        perror("error : get_current_dir_name failed");
        return;
    }
    string result = currDirCommand;
    free(currDirCommand);
    if (result == "") {
        return;
    }
    else {
        cout << result << endl;
    }
}

///
/// #ChangeDirCommand
/// \param cmd_line


ChangeDirCommand::ChangeDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
void ChangeDirCommand::execute() {
    string last_dir = smash.getLastDir();
    string curr_dir = "";

    if (this->params.size() > 1) {
        cerr<<"smash error: cd: too many arguments"<<endl;
        return;
    }
    else if(this->params.size()==0){ //Todo: ask about zero arguments
        cerr << "smash error: cd: zero arguments" << endl;
    }
    else if (this->params[0] == "-") {
        if (last_dir == "") {
            cerr<<"smash error: cd: OLDPWD not set"<<endl;
            return;
        }
        last_dir = smash.getCurrDir();
        curr_dir = smash.getLastDir();

        int result = chdir(curr_dir.c_str());//curr_dir.c_str()
        if (result == -1) {
            perror("smash error: chdir failed");
            return;
        }

        smash.setCurrDir(curr_dir);
        smash.setLastDir(last_dir);
        return;
    }
    else if (this->params.empty()) {
        return;
    }
    else {
        char *currDirCommand = get_current_dir_name();
        if (currDirCommand == nullptr) {
            perror("ERROR : get_current_dir_name failed");
            return ;
        }

        last_dir = currDirCommand;
        free(currDirCommand);

        ///todo: check about (last_dir == curr_dir)
        int ans = chdir(this->params[0].c_str());
        if (ans == -1) {
            perror("smash error: chdir failed");
            return;
        }
        curr_dir = this->params[0];
        smash.setCurrDir(curr_dir);
        smash.setLastDir(last_dir);
        return;
    }
}

///
/// #JobsCommand
/// \param cmd_line
/// \param jobs

JobsCommand::JobsCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs_list(jobs) {}
void JobsCommand::execute() {
    JobsList* jobs_list=smash.get_ptr_to_jobslist();
    jobs_list->removeFinishedJobs();
    jobs_list->printJobsList();
}

///
/// #KillCommand
///

KillCommand::KillCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line) ,jobs_list(jobs){}
void KillCommand::execute() {
    if (this->params.size() != 2) {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }

    int sig_num = 0;
    int job_id = 0;
    if (!checkIfInt(this->params[0]) || !checkIfInt(this->params[1])) {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    else {
        sig_num = stoi(this->params[0]);
        job_id = stoi(this->params[1]);
    }
    if (job_id < 0) {
        cerr << "smash error: kill: job-id " << job_id << " does not exist" << endl;
        return;
    }
    if (sig_num >= 0) {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    map<int, JobsList::JobEntry> map=this->jobs_list->get_map();
    if (map.find(job_id) == map.end()) {
        smashError("kill: job-id " + to_string(job_id) + " does not exist");
        return;
    }

    int abs_sig_num = abs(sig_num);
    int pid_of_job = map.find(job_id)->second.getPid();

    if (kill(pid_of_job, abs_sig_num) == -1) {
        perror("smash error: kill failed");
        return;
    }
    else if (abs_sig_num == 9) {
        this->jobs_list->removeJobById(job_id);
    }
    else if(abs_sig_num==19){
        smash.getJobsList().get_map().find(job_id)->second.setStopped(true);
    }
    cout << "signal number " << abs_sig_num << " was sent to pid " << pid_of_job << endl;
}

///
/// #ForegroundCommand
/// \param cmd_line
/// \param jobs

ForegroundCommand::ForegroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line),jobs_list(jobs) {}
void ForegroundCommand::execute() {
    int job_id;
    map<int, JobsList::JobEntry> map = this->jobs_list->get_map();

    if (this->params.size() > 1) {
        smashError("fg: invalid arguments");
        return;
    }

    ///getting the jobId
    if (this->params.empty()) {
        if (map.size() == 0) {
            smashError("fg: jobs list is empty");
            return;
        }
        job_id = this->jobs_list->return_max_job_id_in_Map();
    }
    else {
        if (!checkIfInt(this->params[0])) {
            smashError("fg: invalid arguments");
            return;
        }
        job_id = stoi(this->params[0]);
        if (map.find(job_id) == map.end()) {
            smashError("fg: job-id " + this->params[0] + " does not exist");
            return;
        }
    }
    ///execute the command

    this->jobs_list->removeFinishedJobs();
    JobsList::JobEntry currentJob = map.find(job_id)->second;

    int pid = currentJob.getPid();
//    string command = currentJob.getCommand();
//
//    currentJob.setBackground(false);
//    smash.set_fg_process(pid_of_job);

    cout << currentJob.toString() << endl;
    smash.bringJobToForeGround(currentJob);

    if (killpg(pid, SIGCONT) == -1) {
        smashError("kill failed", true);
        return;
    }
//    map.find(job_id)->second.setStopped(false);
    waitpid(pid, nullptr, WUNTRACED);
//    todo: implement. do we need the if statement?
//    if (!map.find(job_id)->second.if_is_stopped()) {
    this->jobs_list->removeJobById(job_id);
//    }
//
//    this->jobs_list->change_last_stopped_job_id();

    smash.set_fg_process(0);
}

///
/// #BackgroundCommand
/// \param cmd_line
/// \param jobs
BackgroundCommand::BackgroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line),jobs_list(jobs) {}
void BackgroundCommand::execute() {
    map<int, JobsList::JobEntry> map = this->jobs_list->get_map();
    int job_id = 0;

    if (this->params.size() > 1) {
        smashError("bg: invalid arguments");
        return;
    }

    ///getting the jobId

    if(this->params.size() == 0){
//        job_id = this->jobs_list->get_max_from_stopped_jobs_id();//todo get the jobId
        if (job_id == 0) {//todo
            smashError("bg: there is no stopped jobs to resume");
            return;
        }
    }
    else {
        if (!checkIfInt(this->params[0])) {
            smashError("bg: invalid arguments");
            return;
        }
        job_id = stoi(this->params[0]);
        if (map.find(job_id) == map.end()) {
            smashError("bg: job-id " + this->params[0] + " does not exist");
            return;
        }
    }

    ///start the command

    JobsList::JobEntry jobEntry = map.find(job_id)->second;

    if (jobEntry.if_is_background() && !jobEntry.if_is_stopped()) {
        smashError(("bg: job-id " + std::to_string(job_id) + " is already running in the background"));
        return;
    }
    int pid_of_job = jobEntry.getPid();
    cout << jobEntry.toString() << endl;
    if (jobEntry.if_is_background() && jobEntry.if_is_stopped()) {
        if (killpg(pid_of_job, SIGCONT) == -1) { // syscall failed
            smashError(" kill failed");
            return;
        }
    } else{
        if (kill(pid_of_job, SIGCONT) == -1) {
            smashError(" kill failed");
            return;
        }
    }
    smash.sendJobToBackground(jobEntry);
}

///
/// #QuitCommand
/// \param cmd_line
/// \param jobs
QuitCommand::QuitCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line),jobs_list(jobs) {}
void QuitCommand::execute() {
    this->jobs_list->removeFinishedJobs();
    auto params = this->params;
    if (!params.empty() && params[0] == "kill") {
        this->jobs_list->removeFinishedJobs();
        map<int, JobsList::JobEntry> jobs = this->jobs_list->get_map();
        cout << "smash: sending SIGKILL signal to " << jobs.size() << " jobs:" << endl;
        for(auto job : jobs){
            int pid = job.second.getPid();
            string command = job.second.getCommand();
            cout << pid << ": " << command << endl;
            if (kill(pid, SIGKILL) == -1) {
                smashError("kill failed");
            }
        }
    }
    smash.isquit = true;
}




///
/// #jobs section starts
///

///
/// #jobs list start
///

void JobsList::printJobsList() {
    for (auto &jobEntry : this->map_of_smash_jobs) {
        time_t now = time(nullptr);
        if (now == -1) {
            smashError("time failed", true);
            return;
        }
        auto job = jobEntry.second;
        cout << "[" << job.getJobId() << "] " << job.getCommand() << " : "
             << job.getPid() << " "
             << difftime(now, job.get_time_of_command()) << " secs";
        if (job.if_is_stopped()) {
            cout << " (stopped)";
        }
        cout << endl;
    }
}

void JobsList::removeFinishedJobs() {
    int status;
    int childPid = waitpid(-1, &status, WNOHANG);
    while (childPid > 0) {
        int jobId = get_job_id_by_pid(childPid);
        if (jobId != 0) {
            removeJobById(jobId);
        }
        childPid = waitpid(-1, &status, WNOHANG);
    }
}

void JobsList::removeJobById(int jobId) {
    JobEntry job = this->map_of_smash_jobs.find(jobId)->second;
    job.deleteCommand();

    this->map_of_smash_jobs.erase(jobId);
    int maxJob = return_max_job_id_in_Map();

    set_max_from_jobs_id(maxJob);
}

int JobsList::return_max_job_id_in_Map() {
    if (this->map_of_smash_jobs.size() == 0) {
        return 0;
    }
    int max = 0;
    for (const auto &job : this->map_of_smash_jobs) {
        if (job.first > max) {
            max = job.first;
        }
    }

    return max;
}

void JobsList::set_max_from_jobs_id(int max_job_id) {
    this->max_from_jobs_id = max_job_id;
}

int JobsList::get_job_id_by_pid(int pid) {
    if (this->map_of_smash_jobs.size() == 0) {
        return 0;
    }
    for (const auto &job : this->map_of_smash_jobs) {
        if (job.second.getPid() == pid) {
            return job.first;
        }
    }
    return 0;
}

int JobsList::JobEntry::getJobId() const {
    return this->jobID;
}

const map<int, JobsList::JobEntry> &JobsList::get_map() const {
    return this->map_of_smash_jobs;
}

int JobsList::addJob(int pid, Command *cmd, bool isStopped, int jobId) {
    if (jobId == -1){
        jobId = 0;
        for (auto jobEntry : this->get_map()) {
            int currentJobId = jobEntry.second.getJobId();
            if (currentJobId > jobId)
                jobId = currentJobId;
        }

//        jobId = max todo
//        set new max todo
    }
    jobId++;
    JobEntry new_job(jobId, pid, cmd);
    this->map_of_smash_jobs.insert(std::pair<int, JobEntry>(jobId, new_job));
    return jobId;
}

void JobsList::change_last_stopped_job_id() {
    ///checkvalgrind
    JobsList jl = smash.getJobsList();
    map<int, JobsList::JobEntry> Map = jl.get_map();
    if (Map.size() == 0) {
        this->max_from_stopped_jobs_id = 0;
    }
    int Max = 0;
    for (auto job : Map ){
        if (job.first > Max && job.second.if_is_stopped()) {
            Max = job.first;
        }
    }
    this->max_from_stopped_jobs_id  = Max;
}

/// #job entry begin

JobsList::JobEntry::JobEntry(int jobId, int pid, Command *cmd) : command(cmd) {
    this->time_of_command = time(nullptr);
    this->jobID=jobId;
    this->pid=pid;
    if (this->time_of_command == -1) {
        perror("smash error: time failed");
    }
}

pid_t JobsList::JobEntry::getPid() const {
    return this->pid;
}

const char *JobsList::JobEntry::getCommand() const {
    return this->command->getCommandLine();
}

void JobsList::JobEntry::deleteCommand() {
    delete this->command;
}

bool JobsList::JobEntry::if_is_background() const {
    return this->command->if_is_background();
}

void JobsList::JobEntry::setBackground(bool mode) const {
    this->command->setBackground(mode);
}

void JobsList::JobEntry::setStopped(bool stopped) const {
    this->command->setStopped(stopped);
}

bool JobsList::JobEntry::if_is_stopped()const {
    return this->command->if_is_stopped();
}

time_t JobsList::JobEntry::get_time_of_command() const {
    return this->time_of_command;
}

void JobsList::JobEntry::set_time_of_command(time_t time)  {
    this->time_of_command=time;
}

string JobsList::JobEntry::toString() const {
    int pid = this->getPid();
    string command = this->getCommand();
    return command + " : " + std::to_string(pid);
}

///
/// #jobs section ends
///


///
/// #ExternalCommand
/// \param cmd_line
/// \param is_bg
ExternalCommand::ExternalCommand(const char *cmd_line, bool is_bg)  : Command(cmd_line) {
    this->external = true;
    this->background=is_bg;
}
void ExternalCommand::execute() {
    int pid = fork();
    if (pid == -1) {
        smashError("fork failed", true);
        return;
    }
    if (pid == 0) {
        setpgrp();
        char external_params[200] = {0};

        strcpy(external_params, this->commandLine);
        _trim(external_params);//todo: make sure what this command do
        _removeBackgroundSign(external_params);

        char* param0=(char *) "/bin/bash";
        char*param1=(char *) "-c";
        char *const params_for_exec[] = {param0, param1, external_params, nullptr};
        int ans = execv("/bin/bash", params_for_exec);

        if (ans == -1) {
            smashError("execv failed", true);
            return;
        }
    }
    else {
        auto jobs = smash.get_ptr_to_jobslist();
        jobs->removeFinishedJobs();
        int new_job_id = jobs->addJob(pid, this, false);

        if (!(this->if_is_background())) {
            smash.set_fg_process(pid);
            waitpid(pid, nullptr, WUNTRACED);
            if (!jobs->get_map().find(new_job_id)->second.if_is_stopped()) {
                // The process was not stopped while it was running, so it is safe to remove it from the jobs list
                jobs->removeJobById(new_job_id);
            }

//            jobs->change_last_stopped_job_id();todo
            smash.set_fg_process(0);
        }
    }
}



///
/// special commands start
///

///
/// pipe command
///
PipeCommand::PipeCommand(const char *cmd_line, bool out1): Command(cmd_line), ifout(out1) {}
void PipeCommand::execute() {
    vector<string> params_of_pipe;
    if (ifout) {
        params_of_pipe = get_param_of_pipe(this->commandLine);
    }
    else {
        params_of_pipe = get_param_of_pipe_with_arr(this->commandLine);
    }

    if (params_of_pipe.size() != 2) {
        smashError("invalid arguments");
        return;
    }

    if (params_of_pipe[0].empty() || params_of_pipe[1].empty()) {
        smashError("invalid arguments");
        return;
    }

    int mypipe[2];

    if (pipe(mypipe) == -1) {
        smashError("pipe failed");
        return;
    }

    int channel;
    if (ifout == 1) {
        channel = 1;
    }
    else{
        channel = 2;
    }

//    int sec_place_of_channel = dup(channel);
//    if (sec_place_of_channel == -1) {
//        perror("smash error: dup failed");
//        return;
//    }

    int pid1 = fork();
    int pid2;
    if (pid1 == -1) {
        smashError("fork failed" );
        return;
    }
    if (pid1 == 0) {
        setpgrp();
        if (dup2(mypipe[1], channel) == -1) {
            smashError("dup2 failed");
            return;
        }
        if (close(mypipe[0]) == -1) {
            smashError("close failed");
            return;
        }
        if (close(mypipe[1]) == -1) {
            smashError("close failed");
            return;
        }
        smash.executeCommand(params_of_pipe[0].c_str());
        exit(0);
    }
    else {
        pid2 = fork();
        //setrpgp
        if (pid2 == -1) {
            smashError("fork failed");
            return;
        }
        if (pid2 == 0) {
            if (dup2(mypipe[0], 0) == -1) {
                smashError("dup2 failed");
                return;
            }
            if (close(mypipe[0]) == -1) {
                smashError("close failed");
                return;
            }
            if (close(mypipe[1]) == -1) {
                smashError("close failed");
                return;
            }
            smash.executeCommand(params_of_pipe[1].c_str());
            exit(0);
        }

    }

    if (close(mypipe[0]) == -1) {
        smashError("close failed");
        return;
    }
    if (close(mypipe[1]) == -1) {
        smashError("close failed");
        return;
    }
//    if (dup2(sec_place_of_channel, channel) == -1) {
//        perror("smash error: dup2 failed");
//        return;
//    }
    //   wait(pid1);
    // wait(pid2);
    waitpid(pid1, NULL, WUNTRACED);
    waitpid(pid2, NULL, WUNTRACED);
}


///
/// redirection command
///

RedirectionCommand::RedirectionCommand(const char *cmd_line, bool background_flag, bool override_flag):Command(cmd_line),
                                                                                             override(override_flag){
    this->background=background_flag;
}

void RedirectionCommand::execute() {
    vector<string> params_to_rc;
    if (this->override) {
        params_to_rc = get_param_case_override(this->commandLine);
    }
    else {
        params_to_rc = get_param_case_append(this->commandLine);
    }
    if (params_to_rc.size() != 2) {
        smashError("invalid argument");
        return;
    }
    if (params_to_rc[1].empty() || params_to_rc[0].empty()) {
        smashError("invalid argument");
        return;
    }

    if (this->background) {

        params_to_rc[0]+=" &";
        string rm_ampersand = "";

        for (char c : params_to_rc[1]) {
            if (c != '&') {
                rm_ampersand.push_back(c);
            }
        }
        rm_ampersand.erase(std::find_if(rm_ampersand.rbegin(), rm_ampersand.rend(), std::bind1st(std::not_equal_to<char>(), ' ')).base(), rm_ampersand.end());
        params_to_rc[1] = rm_ampersand;
    }
    string file_name = params_to_rc[1];
    Command *command = smash.CreateCommand(params_to_rc[0].c_str());
    int ans_dup = dup(1);

    if (ans_dup == -1) {
        smashError("dup failed");
        delete command;
        return;
    }

    if (close(1) == -1) {
        smashError("close failed");
        delete command;
        return;
    }

    int ans;
    if (this->override) {
        ans = open(file_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);

    }
    else {
        ans = open(file_name.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
    }
    if (ans == -1) {
        smashError("open failed");
        dup2(ans_dup, 1);
        delete command;
        return;
    }

    smash.executeCommand(command->getCommandLine());
    if (close(1) == -1) {
        smashError("close failed");
        dup2(ans_dup, 1);
        delete command;
        return;
    }

    if (dup(ans_dup) == -1) {
        smashError("dup failed");
    }

    if (close(ans_dup) == -1) {
        smashError("close failed");
    }
    delete command;
}




///
/// special commands end
///








///
/// timeout section start
///


///
/// timeout command
///

TimeoutCommand::TimeoutCommand(const char *cmd_line, bool isBackground_flag) : Command(cmd_line) {
    this->background = isBackground_flag;
    this->external = true;
}

void TimeoutCommand::execute() {
    vector<string> params_to_timeout_command = splitStringToWords(this->commandLine);
//    this->params = params_to_timeout_command;
    if (this->params.empty() || this->params.size() < 2) { // todo: check where this.params gets values
        smashError("invalid arguments");
        return;
    }
    int duration = 0;
    if (checkIfInt(this->params[0]) == 0) {
        smashError("invalid arguments");
        return;
    }
    else {
        duration = std::atoi(this->params[0].c_str());
        if (duration < 0) {
            smashError("invalid arguments");
            return;
        }
    }

    string command_after_duration = "";
    unsigned int length = this->params.size();
    for (unsigned int i = 1; i < length; ++i) {
        if (!command_after_duration.empty()) {

            command_after_duration.append(" " + this->params[i]);
        }
        else {
            command_after_duration.append(this->params[i]);
        }
    }

    int pid = fork();
    if (pid == -1) {
        smashError("fork failed");
        return;
    }
    else if (pid == 0) {
        setpgrp();
        char params_to_time_cmd[200] = {0};
        strcpy(params_to_time_cmd, command_after_duration.c_str());
        _trim(params_to_time_cmd);
        _removeBackgroundSign(params_to_time_cmd);

        char* first_param = (char *) "/bin/bash";
        char* second_param = (char *) "-c";
        char *const params_to_exec[] = {first_param, second_param, params_to_time_cmd, nullptr};
        int answer = execv("/bin/bash", params_to_exec);

        if (answer == -1) {
            smashError("execv failed");
            return;
        }
    }
    else {
        JobsList* jobs_list = smash.get_ptr_to_jobslist();
        jobs_list->removeFinishedJobs();
        int new_job_id = jobs_list->addJob(pid, this, false);
        int command_length = strlen(this->commandLine) + 1;
        char *command_line = (char *) malloc(command_length);


        strcpy(command_line, this->commandLine);
        TimeList* time_list = smash.get_ptr_to_Timelist();
        int new_time_id = time_list->addTime(new_job_id, pid, duration, command_line);
        time_t time_now = time(nullptr);
        time_list->What_is_the_Next_Timeout(time_now);

        if (!(this->if_is_background())) {
            smash.set_fg_process(pid);
            waitpid(pid, nullptr, WUNTRACED);
            JobsList jl = smash.getJobsList();
            if (!jl.get_map().find(new_job_id)->second.if_is_stopped()) {
                jobs_list->removeJobById(new_job_id);
                time_list->removeTimeById(new_time_id);
            }
            jobs_list->change_last_stopped_job_id();
            smash.set_fg_process(0);
        }
    }
}


/////timeList section


int TimeList::getMaxId() {
    return this->maxTimeId;
}


void TimeList::setMaxTimeId(int max_time_entry_id) {
    this->maxTimeId = max_time_entry_id;
}


int TimeList::addTime(int job_id, int pid, int timeOfDur, char *command) {
    int new_time_id = getMaxId();
    new_time_id += 1;

    TimeEntry newTimeEntery(new_time_id, job_id, pid, timeOfDur, command);
    this->timeMap.insert(std::pair<int, TimeEntry>(new_time_id, newTimeEntery));

    setMaxTimeId(new_time_id);
    return new_time_id;
}


void TimeList::removeTimeById(int time_entry_id) {
    this->timeMap.erase(time_entry_id);

    int max_time_id = getMaxKeyInMap();

    setMaxTimeId(max_time_id);
}
//int TimeList::get_TimeId_Of_finished_Timeout(time_t now) {
//
//    for (auto &pair: this->timeMap) {
//        int until_finish = pair.second.getTimeOfDur() - difftime(now, pair.second.getTimeOfCommandCame());
//        if (until_finish <= 0) {
//            return pair.first;
//        }
//    }
//    return -1;
//}
//
//int TimeList::get_JobId_Of_finished_timeout(time_t now) {
//
//    for (auto &pair: this->timeMap) {
//        int until_finished = pair.second.getTimeOfDur() - difftime(now, pair.second.getTimeOfCommandCame());
//        if (until_finished <= 0) {
//            return pair.second.getJobId();
//        }
//    }
//    return -1;
//}
//
int TimeList::getMaxKeyInMap() {

    if (this->timeMap.size() == 0) {
        return 0;
    }
    int max_Time_ID = 0;

    for (const auto &pair : this->timeMap) {
        if (pair.first > max_Time_ID) {
            max_Time_ID = pair.first;
        }
    }
    return max_Time_ID;
}
//
//void TimeList::change_Max_TimeId() {
//    if (this->timeMap.size() == 0) {
//        this->maxTimeId = 0;
//    }
//
//    int max_time_Id = 0;
//
//    for (auto pair : this->timeMap) {
//        if (pair.first > max_time_Id) {
//            max_time_Id = pair.first;
//        }
//    }
//    this->maxTimeId = max_time_Id;
//}
void TimeList::What_is_the_Next_Timeout(time_t time_now) {

    if (this->timeMap.empty()) {
        return;
    }
    int next_timeout_cmd = -1;

    for (auto &pair : this->timeMap) {
        int diff = pair.second.getTimeOfDur() - difftime(time_now, pair.second.getTimeOfCommandCame());
        if (diff < next_timeout_cmd || next_timeout_cmd == -1) {
            next_timeout_cmd = diff;
        }
    }
    alarm(next_timeout_cmd);
}
//
//const map<int, TimeList::TimeEntry> &TimeList::getTimeMap() const {
//    return this->timeMap;
//}



/////timeEntry section

TimeList::TimeEntry::TimeEntry(int id, int job_id, int pid, int timeOfDur, char *command) : id(id), job_id(job_id),pid(pid),timeOfDur(timeOfDur),command(command) {
    this->timeOfCommandCame = time(nullptr);
    if (this->timeOfCommandCame == -1) {
        smashError("time failed");
    }
}
//int TimeList::TimeEntry::getJobId() const {
//    return this->job_id;
//}
//
//int TimeList::TimeEntry::getPid() const {
//    return this->pid;
//}


int TimeList::TimeEntry::getTimeOfDur() const {
    return this->timeOfDur;
}


//char *TimeList::TimeEntry::getCommand() const {
//    return this->command;
//}


time_t TimeList::TimeEntry::getTimeOfCommandCame() const {
    return this->timeOfCommandCame;
}











///
/// timeout section end
///






///
/// smash section
///


///
/// #smash
///

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}
// asaf
Command * SmallShell::CreateCommand(const char* cmd_line) {
    string command_line = string(cmd_line);

    bool background = _isBackgroundComamnd(cmd_line);

    bool ans_to_pipe1 = is_out(command_line);
    bool ans_to_pipe_err1 = is_err(command_line);

    bool ans_to_override1 = is_ovveride(command_line);
    bool ans_to_append1 = is_append(command_line);

    if (ans_to_pipe1 || ans_to_pipe_err1) {
        bool isout = true;
        if(ans_to_pipe_err1){
            isout= false;
        }
        return new PipeCommand(cmd_line,isout);
    }
    else if (ans_to_append1 || ans_to_override1) {
        bool override = true;
        if (ans_to_append1) {
            override = false;
        }
        return new RedirectionCommand(cmd_line,background,override);
    }
    else
    if (isStringCommand(command_line, "chprompt")) {
        return new ChpromptCommand(cmd_line);
    }
    else if (isStringCommand(command_line, "showpid")) {
        return new ShowPidCommand(cmd_line);
    }
    else if (isStringCommand(command_line, "pwd")) {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (isStringCommand(command_line, "cd")) {
        return new ChangeDirCommand(cmd_line);
    }
    else if (isStringCommand(command_line, "kill")) {
        return new KillCommand(cmd_line, smash.get_ptr_to_jobslist());
    }
    else if (isStringCommand(command_line, "jobs")) {
        return new JobsCommand(cmd_line, smash.get_ptr_to_jobslist());
    }
    else if (isStringCommand(command_line, "fg")) {
        return new ForegroundCommand(cmd_line,smash.get_ptr_to_jobslist());
    }
    else if (isStringCommand(command_line, "bg")) {
        return new BackgroundCommand(cmd_line,smash.get_ptr_to_jobslist());
    }
    else if (isStringCommand(command_line, "quit")) {
        return new QuitCommand(cmd_line,smash.get_ptr_to_jobslist());
    }
    else {
        if (command_line.empty()) {
            return nullptr;
        }
        return new ExternalCommand(cmd_line, background);
    }
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    Command *command = CreateCommand(cmd_line);
    if (command) {
        if (command->isExternal()) {
            command->execute();
        } else {
        command->execute();
        delete command;
        }
    }
}

string SmallShell::getPrompt() {
    return this->prompt;
}

void SmallShell::setPrompt(string newPromptName) {
    this->prompt = newPromptName;
}

int SmallShell::getPid() {
    return this->pid;
}

const string &SmallShell::getCurrDir() const {
    return this->curr_dir;
}

void SmallShell::setCurrDir(string currDir) {
    this->curr_dir = currDir;
}

const string &SmallShell::getLastDir() const {
    return this->last_dir;
}

void SmallShell::setLastDir(string lastDir) {
    this->last_dir = lastDir;
}

const JobsList &SmallShell::getJobsList() const {
    return this->jobs;
}

JobsList *SmallShell::get_ptr_to_jobslist() {
    return &(this->jobs);
}

int SmallShell::get_fg_process() const {
    return this->fgprocess;
}

void SmallShell::set_fg_process(int process_of_fg)  {
    this->fgprocess = process_of_fg;
}

void SmallShell::bringJobToForeGround(JobsList::JobEntry& jobEntry){
    this->fgprocess = jobEntry.getPid();
    jobEntry.setBackground(false);
    jobEntry.setStopped(false);
}

void SmallShell::sendJobToBackground(JobsList::JobEntry& jobEntry) {
    jobEntry.setStopped(false);
//            this->jobs_list->change_last_stopped_job_id(); todo
}

TimeList *SmallShell::get_ptr_to_Timelist() {
    return &(this->times);
}