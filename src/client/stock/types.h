namespace stock
{
    typedef enum { qfalse, qtrue } qboolean;
    typedef void (*xcommand_t)(void);
    
    typedef struct cvar_s
    {
        char* name;
        char* string;
        char* resetString;
        char* latchedString;
        int flags;
        qboolean modified;
        int modificationCount;
        float value;
        int integer;
        // ...
    } cvar_t;
}