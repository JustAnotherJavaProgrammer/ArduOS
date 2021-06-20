#ifndef EXECUTOR_CLASS_ALREADY_SEEN
#define EXECUTOR_CLASS_ALREADY_SEEN

class Executor {
    public:
        // virtual ~Executor();
        virtual bool openProgram(char* fileName);
        virtual void execCommand();
};

#endif