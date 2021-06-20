#ifndef EXECUTOR_CLASS_ALREADY_SEEN
#define EXECUTOR_CLASS_ALREADY_SEEN

class Executor {
    public:
        virtual ~Executor();
        virtual void openProgram();
        virtual void execCommand();
};

#endif