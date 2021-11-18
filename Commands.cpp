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
/// smash helper functions
///

bool xxx(string command, string s){
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