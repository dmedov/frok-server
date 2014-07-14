#ifndef IO_H
#define IO_H

// Include dependencies
#include <vector>
#include <string>

//externals
extern char *log_file;

// don't forget '/' in the end of dir name.
// Example: good = "/home/workspace/" bad: "/home/workspace"
int getFilesFromDir(const char *dir, std::vector<std::string> &files);
int getSubdirsFromDir(const char *dir, std::vector<std::string> &subdirs);

void FilePrintMessage(char* file, const char* expr...);

#endif // IO_H
