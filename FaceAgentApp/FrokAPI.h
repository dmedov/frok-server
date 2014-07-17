#ifndef FROKAPI_H
#define FROKAPI_H

#include <vector>
#include <string>

enum FrokResult
{
    FROK_RESULT_SUCCESS,
    FROK_RESULT_CASCADE_ERROR,
    FROK_RESULT_UNSPECIFIED_ERROR,
    // etc
};

typedef FrokResult (*FrokAPIFunction) (void *params);


typedef struct FrokAPI
{
    // Pointer to function
    FrokAPIFunction     function;
    // Function description
    const char         *functionDescription;
    // Vector of mandatory parameters for the function
    //std::vector< std::string > jsonParams;
    // Function params description
    const char         *jsonParamsDescription;
    // Estimated timeout in seconds
    unsigned long int   timeout;
} FrokAPI;

FrokResult Recognize(void *param);
FrokResult TrainUserModel(void *param);
FrokResult GetFacesFromPhoto(void *param);
FrokResult AddFaceFromPhoto(void *param);

const FrokAPI FROK_API_FUNCTIONS[] =
{
    {Recognize,             "Recognize user on target photo. To add new user to database use \"TrainUserModel\" function.", /*{"user_id"},*/"\"user_id\" - string, add some description \n etc\n", 300}
    /*{TrainUserModel,        "description",  "params description", {"user_id"}, 3000},
    {GetFacesFromPhoto,     "description",  "params description", {"user_id"}, 30},
    {AddFaceFromPhoto,      "description",  "params description", {"user_id"}, 30},*/
};
#endif // FROKAPI_H
