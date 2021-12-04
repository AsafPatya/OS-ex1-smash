#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

#include <vector>
#include <string>
#include <time.h>
#include <map>

using std::vector;
using std::string;

///
/// #Command
///

class Command{
protected:
    char *commandLine;
    vector<string> params;
    bool external = false;
    bool stopped = false;
    bool background = false;
    bool foreground= false;

public:
  Command(const char* cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  bool if_is_stopped() const;
  void setStopped(bool stopped);
  bool if_is_background() const;
  void setBackground(bool background);
  bool isExternal() const;
  const char *getCommandLine() const;
//    virtual void prepare();
//    virtual void cleanup();
//    const vector<string> &getParams() const;
//    const string &getStartCommand() const;
};


///
/// #BuiltInCommand
///

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {}
};

class ChpromptCommand : public BuiltInCommand {
public:
    explicit ChpromptCommand(const char *cmd_line);
    virtual ~ChpromptCommand() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char* cmd_line);
    virtual ~ShowPidCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char* cmd_line);
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
public:
    ChangeDirCommand(const char* cmd_line);
    virtual ~ChangeDirCommand() {}
    void execute() override;
};


///
/// #JobsList
///

class JobsList;

class JobsList{
 public:
  class JobEntry{
      private:
          int pid;
          int jobID = 0;
          time_t time_of_command;
          Command *command;
    //      bool stopped = false;
      public:
          JobEntry(int jobId, int pid, Command *cmd);
          int getPid() const;
    //      void setPid(int pid);
          int getJobId() const;
    //      void setJobId(int jobId);
          time_t get_time_of_command() const;
          void set_time_of_command(time_t time);
          const char *getCommand() const;
          void deleteCommand();
          bool if_is_stopped() const;
          void setStopped(bool stopped) const;
          bool if_is_background() const;
          void setBackground(bool mode) const;
          string toString() const;
          ~JobEntry(){};
      };
private:
    int max_from_jobs_id = 0;
    int max_from_stopped_jobs_id = 0;
    std::map<int, JobEntry> map_of_smash_jobs;
public:
    JobsList()=default;
    ~JobsList(){};
  int addJob(int pid, Command* cmd, bool isStopped = false, int jobId = -1);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify existing ones as needed
//   int addJob(int pid, int job_id, Command *cmd, bool stopped = false);
//   int addJob(int pid, Command *cmd, bool stopped = false);
//   int get_max_from_jobs_id() const;
   void set_max_from_jobs_id(int max_job_id);
   int get_max_from_stopped_jobs_id() const;
//   void set_max_from_stopped_jobs_id(int max_stopped_job_id);
   int return_max_job_id_in_Map();
   int get_job_id_by_pid(int pid);
   void change_last_stopped_job_id();
    const std::map<int, JobEntry> &get_map() const;
    int return_max_stopped_jobs_id();
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members

private:
    JobsList *jobs_list;
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
    JobsList *jobs_list;
  KillCommand(const char* cmd_line, JobsList* jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
private:
    JobsList *jobs_list;
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
private:
    JobsList *jobs_list;
 public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
private:
    JobsList* jobs_list;
public:
    QuitCommand(const char* cmd_line, JobsList* jobs);
    virtual ~QuitCommand() {}
    void execute() override;
};

class HeadCommand : public BuiltInCommand {
 public:
  HeadCommand(const char* cmd_line);
  virtual ~HeadCommand() {}
  void execute() override;
};


///
/// #ExternalCommand
///

class ExternalCommand : public Command {
public:
    ExternalCommand(const char* cmd_line,bool is_bg);
    virtual ~ExternalCommand() {}
    void execute() override;
};


///
/// #SpecialCommands
///

class PipeCommand : public Command {
    // TODO: Add your data members
private:
    bool ifout;
public:
    PipeCommand(const char* cmd_line, bool out1);
    virtual ~PipeCommand() {}
    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
private:
    bool override;
public:
    explicit RedirectionCommand(const char* cmd_line,bool background_flag,bool override_flag);
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};


///
/// #TimeoutCommand
///


class TimeList {
public:
    class TimeEntry {
    private:
        int id = 0;
        int job_id = 0;
        int pid = 0;
        int timeOfDur = 0;
        char *command;
        time_t timeOfCommandCame;
    public:
        TimeEntry(int id, int job_id, int pid, int timeOfDur, char *command);
        int getJobId() const;
        int getPid() const;
        int getTimeOfDur() const;
        char *getCommand() const;
        time_t getTimeOfCommandCame() const;
        ~TimeEntry() {};
    };

private:
    int maxTimeId = 0;
    std::map<int, TimeEntry> timeMap;

public:
    TimeList()=default;
    int getMaxId();
    int getMaxKeyInMap();
    void setMaxTimeId(int maxTimeEntryId);
    int addTime(int job_id, int pid, int timeOfDur, char *command);
    void removeTimeById(int time_entry_Id);
    int get_TimeId_Of_finished_Timeout(time_t time_now);
//    int get_JobId_Of_finished_timeout(time_t now);
    void change_Max_TimeId();
    void What_is_the_Next_Timeout(time_t now);
    const std::map<int, TimeEntry> &getTimeMap() const;

    ~TimeList() {};

};

class TimeoutCommand : public Command {
public:
    TimeoutCommand(const char *cmd_line, bool isBackground_flag);

    virtual ~TimeoutCommand() {}

    void execute() override;
};


///
/// #Smash
///

class SmallShell {
 private:
  // TODO: Add your data members

  string prompt = "smash> ";
  int pid;
  string last_dir = "";
  string curr_dir = "";
  JobsList jobs;
  TimeList times;
  int fgprocess;

  SmallShell();
 public:
    bool isquit= false;

    Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed

    string getPrompt();
    void setPrompt(string prompt);
    int getPid();
    const string &getCurrDir() const;
    void setCurrDir(string currDir);
    const string &getLastDir() const;
    void setLastDir(string lastDir);
    const JobsList &getJobsList() const;
    JobsList *get_ptr_to_jobslist();
    int get_fg_process() const;
    void set_fg_process(int process_of_fg);
    const TimeList &getTimeList() const;
    TimeList *get_ptr_to_Timelist();

    void bringJobToForeGround(JobsList::JobEntry& jobEntry);
    void sendJobToBackground(JobsList::JobEntry& jobEntry);
};

#endif //SMASH_