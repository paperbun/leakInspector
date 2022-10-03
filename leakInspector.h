// When malloc is called, last 5 symbols in the stack trace is store here
struct TraceInfo{

    // This is used to free the no of symbols at later point.
    int noOfsymbols;
    char **symbols;
};

struct AllocInfo {
    void* address;
    int size;
    struct TraceInfo* traceInfo;

};

struct Node{
    struct AllocInfo *alloc;
    struct Node *next;
};

struct TraceInfo* getBackTraceInfo();
void insertNode(void *address, int size, struct TraceInfo* traceInfo);
void deleteNode(void* address);
void dumpReport(void);

