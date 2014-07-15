
class A
{
public:
    A();
    ~A();


    void function1();
    virtual void function2(void *param);
    static void function3(void *param);
    virtual void function5();
};

/*#ifndef ACTIVITIES_H
#define ACTIVITIES_H

#include "string.h"
#include "json.h"

#define CUT_TIMEOUT            (600)
#define MAX_THREADS_AND_CASCADES_NUM        (1)


#pragma pack(push, 1)



#pragma pack(pop)

void recognizeFromModel(void *pContext);
void generateAndTrainBase(void *pContext);
void getFacesFromPhoto(void *pContext);
void saveFaceFromPhoto(void *pContext);

#endif // ACTIVITIES_H
*/

