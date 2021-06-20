class Executor {
    public:
        // virtual ~Executor();
        virtual bool openProgram(char* fileName);
        virtual void execCommand();
};