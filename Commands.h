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
  bool is_stopped() const;
  void set_stopped(bool stopped);
  bool is_background() const;
  void set_background(bool background);
  bool is_external() const;
  const char *get_command_line() const;
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
          int jobId = 0;
          time_t command_arrival_time;
          Command *command;
    //      bool stopped = false;
      public:
          JobEntry(int jobId, int pid, Command *cmd);
          int getPid() const;
    //      void setPid(int pid);
          int getJobId() const;
    //      void setJobId(int jobId);
          time_t get_command_arrival_time() const;
          void set_time_of_command_arrival_time(time_t time);
          const char *get_command() const;
          void delete_command();
          bool is_stopped() const;
          void set_stopped(bool stopped) const;
          bool is_background() const;
          void set_background(bool mode) const;
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
//   void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
//   JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify existing ones as needed
//   int addJob(int pid, int job_id, Command *cmd, bool stopped = false);
//   int addJob(int pid, Command *cmd, bool stopped = false);
//   int get_max_from_jobs_id() const;
   void set_max_from_jobs_id(int max_job_id);
   int get_max_from_stopped_jobs_id() const;
//   void set_max_from_stopped_jobs_id(int max_stopped_job_id);
   int get_max_job_id_in_map();
   int get_job_id_by_pid(int pid);
   void change_last_stopped_job_id();
    const std::map<int, JobEntry> &get_map() const;
    int get_max_stopped_jobs_id_in_map();
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
    bool out;
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
        int get_job_id() const;
        int get_pid() const;
        int get_duration() const;
        char *get_command() const;
        time_t get_command_arrival_time() const;
        ~TimeEntry() {};
    };

private:
    int max_arrival_time_id = 0;
    std::map<int, TimeEntry> timeMap;

public:
    TimeList()=default;
    int get_max_id();
    int get_max_key();
    void set_max_time_id(int maxTimeEntryId);
    int add_time(int job_id, int pid, int timeOfDur, char *command);
    void remove_by_id(int time_entry_Id);
    int get_ids_of_finished_timeouts(time_t time_now);
//    int get_JobId_Of_finished_timeout(time_t now);
    void refresh_time_list();
    void get_next_time_out(time_t now);
    const std::map<int, TimeEntry> &get_map() const;

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
  string last_dirrectory = "";
  string current_directory = "";
  JobsList jobs;
  TimeList times;
  int foreground_proccess_id;

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

    string get_prompt();
    void set_prompt(string prompt);
    int get_pid();
    const string &get_current_directory() const;
    void set_current_directory(string currDir);
    const string &get_last_dirrectory() const;
    void set_last_dirrectory(string lastDir);
    const JobsList &get_jobs() const;
    JobsList *get_jobs_list_pointer();
    int get_foreground_proccess_id() const;
    void set_foreground_proccess_id(int process_of_fg);
    const TimeList &get_time_list() const;
    TimeList *get_time_list_pointer();

    void bringJobToForeGround(JobsList::JobEntry& jobEntry);
    void sendJobToBackground(JobsList::JobEntry& jobEntry);
};

#endif //SMASH_