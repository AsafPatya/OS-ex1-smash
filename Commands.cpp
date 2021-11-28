#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <unistd.h>

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

void smashError(string errMsg){
    cerr << "smash error: " << errMsg << endl;
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
        string msg="smash error: kill: job-id ";
        msg+=to_string(job_id);
        msg+=" does not exist";
        cerr << msg<< endl;

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
    map<int, JobsList::JobEntry> map=this->jobs_list->get_map();

    if (this->params.size() > 1) {
        smashError("fg: invalid arguments");
        return;
    }

    if (this->params.empty()) {
        if (map.size() == 0) {
            smashError("fg: jobs list is empty");
            return;
        }
        //TODO: do we need to remove finished jobs before
        job_id = this->jobs_list->return_max_job_id_in_Map();
    }
    else {
        if (!checkIfInt(this->params[0])) {
            smashError("fg: invalid arguments");
            return;
        }
        job_id = stoi(this->params[0]);
        this->jobs_list->removeFinishedJobs();

        if (map.find(job_id) == map.end()) {
            smashError("fg: job-id " + this->params[0] + " does not exist");
            return;
        }
    }

    JobsList::JobEntry currentJob = map.find(job_id)->second;
    int pid_of_job = currentJob.getPid();
    string command_line = currentJob.getCommand();

    cout << command_line << " : " << pid_of_job << endl;
//    currentJob.setBackground(false);
//    smash.set_fg_process(pid_of_job);

//    if (killpg(pid_of_job, SIGCONT) == -1) {
//        perror("smash error: kill failed");
//        return;
//    }
//    map.find(job_id)->second.setStopped(false);
//    waitpid(pid_of_job, nullptr, WUNTRACED);
//
//    if (!map.find(job_id)->second.if_is_stopped()) {
//        this->jobs_list->removeJobById(job_id);
//    }
//
//    this->jobs_list->change_last_stopped_job_id();
//    smash.set_fg_process(0);
}

///
/// #BackgroundCommand
/// \param cmd_line
/// \param jobs
BackgroundCommand::BackgroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line),jobs_list(jobs) {}
void BackgroundCommand::execute() {
    map<int, JobsList::JobEntry> map=this->jobs_list->get_map();
    int job_id = 0;

    if (this->params.size() > 1) {
        smashError("bg: invalid arguments");
        return;
    }

    if(this->params.size() == 0){
//        job_id = this->jobs_list->get_max_from_stopped_jobs_id();//todo

        if (job_id == 0) {
            smashError("bg: there is no stopped jobs to resume");
            return;
        }
    }
    else{
        if (!checkIfInt(this->params[0])) {
            smashError("bg: invalid arguments");
            return;
        }
        else {
            job_id = stoi(this->params[0]);
        }
        if (map.find(job_id) == map.end()) {
            smashError("bg: job-id " + this->params[0] + " does not exist");
            return;
        }

        JobsList::JobEntry jobEntry = map.find(job_id)->second;

        if (jobEntry.if_is_background() && !jobEntry.if_is_stopped()) {
            smashError("bg: job-id " + this->params[0] + " is already running in the background");
            return;
        }
        else if (jobEntry.if_is_background() && jobEntry.if_is_stopped()) {
            int pid_of_job = jobEntry.getPid();
            string command_line = jobEntry.getCommand();

            cout << command_line << " : " << pid_of_job << endl;

            if (killpg(pid_of_job, SIGCONT) == -1) {
                // syscall failed
                smashError(" kill failed");
                return;
            }

            map.at(job_id).setStopped(false);
//            this->jobs_list->change_last_stopped_job_id(); todo
            return;
        }
        else{
            int pid_of_job = jobEntry.getPid();
            string command_line = jobEntry.getCommand();
            cout << command_line << " : " << pid_of_job << endl;

            if (kill(pid_of_job, SIGCONT) == -1) {
                smashError(" kill failed");
                return;
            }
            map.at(job_id).setStopped(false);
//            this->jobs_list->change_last_stopped_job_id(); todo
        }
    }
}

///
/// #jobs section starts
///

///
/// #jobs list start
///

void JobsList::printJobsList() {
    for (auto &job : this->map_of_smash_jobs) {
        time_t now = time(nullptr);
        if (now == -1) {
            perror("smash error: time failed");
            return;
        }
        cout << "[" << job.second.getJobId() << "] " << job.second.getCommand() << " : "
             << job.second.getPid() << " "
             << difftime(now, job.second.get_time_of_command()) << " secs";
        if (job.second.if_is_stopped()) {
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


///
/// #job list end
///


///
/// #job entry begin
///

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

///
/// #job entry end
///

///
/// #jobs section ends
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

Command * SmallShell::CreateCommand(const char* cmd_line) {
//    cout << "\n\ncreating command...\n\n"<< endl;
    string command_line = string(cmd_line);

//    bool background = _isBackgroundComamnd(cmd_line);
//
//    bool ans_to_pipe1 = is_out(command_line);
//    bool ans_to_pipe_err1 = is_err(command_line);
//
//    bool ans_to_override1 = is_ovveride(command_line);
//    bool ans_to_append1 = is_append(command_line);
//
//    if (ans_to_pipe1 || ans_to_pipe_err1) {
//        bool isout = true;
//        if(ans_to_pipe_err1){
//            isout= false;
//        }
//        return new PipeCommand(cmd_line,isout);
//    }
//    else if (ans_to_append1 || ans_to_override1) {
//        bool override = true;
//        if (ans_to_append1) {
//            override = false;
//        }
//        return new RedirectionCommand(cmd_line,background,override);
//    }
//    else
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


    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    Command *command = CreateCommand(cmd_line);
    if (command) {
//        if (command->isExternal()) {
//            command->execute();
//        } else {
            command->execute();
            delete command;
//        }
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