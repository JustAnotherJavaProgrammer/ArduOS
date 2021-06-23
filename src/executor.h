#ifndef EXECUTOR_CLASS_ALREADY_SEEN
#define EXECUTOR_CLASS_ALREADY_SEEN

class Executor {
    public:
        virtual bool openProgram(const char* fileName) = 0;
        virtual void execCommand() = 0;
};

#endif