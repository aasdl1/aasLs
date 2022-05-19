/**
 * implement ls by c by aas
 * function:    基本实现ls功能，按对文件名进行字典序排序后输出条目，对不同的目录的输出具有自适应宽度   
 * method:      获取文件/目录路径，若为目录，则迭代其中内容添加到自定义的struct中，
 *              并添加到一个struct lst中进行保存，
 *              输出时进行按文件名进行排序，同时计算自适应的item间宽度后输出
 * implement:   主要是用lstat函数进行获取文件信息
 * feature: 自适应宽度，文件名
 * drawback: 暂不知总用量是如何实现的，（尝试了\Sigma st_block/2，但不对...)
 */

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <dirent.h>     // for dir's
#include <stdlib.h>

#include <pwd.h>
#include <grp.h>

#include <time.h>
#include <unistd.h>

#include <string.h>

#define BUFFLEN 255     // 缓冲最大数


typedef struct aas_File_Info
{
    char fileT;
    char fileAuth[10];
    unsigned long int st_nlink;
    char userName[BUFFLEN];
    char groupName[BUFFLEN];
    unsigned long int st_size;
    char timeBuf[BUFFLEN];
    char fileName[BUFFLEN];
} aasFileInfo;

char * get_link_path(char * linkpath, char * buf, int count);
void fileAuthJudge(mode_t st_mode, char *fileAuth);
char fileType(mode_t st_mode);
char * getFileName(char * filePath, char * fileName, int buffSize);
void saveSingleFileInfo(char *, aasFileInfo *);
unsigned long int printSingleFileInfo(char *);
void printAllResults(aasFileInfo filesInfos[], int size, int , int , int, int);
int intLength(unsigned long int);

int main(int argc, char *argv[]){

    // process with the input params
    char * path;        // 文件路径
    
    for (int i = 0; (i < argc)||(argc==1); i++){
        if (argc == 1){     //指定当前目录
            path = ".";
        }
        else{
            path = argv[i+1];
        }

        // 打印目录或文件
        if (i == 0){
            printf("%s:\n",path);
        }
        else{
            printf("\n%s:\n", path);
        }

        // max 255个
        aasFileInfo filesInfos[BUFFLEN];
        

        struct stat buff;
        stat(path, &buff);
        if (S_ISDIR(buff.st_mode)){
            // 目录
            // printf("[%s] is a dir\n", path);
            struct dirent *filename;
            DIR *dp = opendir(path);

            // 数据块个数
            unsigned long int totalBlocks = 0;
            unsigned long int tmpBlock = 0;

            // 组名,Size,和Username的最大长度
            int maxGroupLength = 0, maxSizeLength = 0, maxUserNameSize = 0, maxNlinkLength = 0;
            unsigned long int maxNlink = 0, maxFileSize = 0;
            
            // 目录下文件个数
            int fileLengths = 0;

            // 切换目录到path中
            int in = chdir(path);
            // printf("\nin success:%d\n",in);

            // 读完整个目录文件，readdir返回NULL
            while (filename = readdir(dp)){
                // 不判断隐藏目录
                if ((strcmp(".", filename->d_name)==0)||(strcmp("..", filename->d_name)==0)){
                    // printf("now filename:%s\n", filename->d_name);
                    continue;

                }


                // tmpBlock = printSingleFileInfo(filename->d_name);
                saveSingleFileInfo(filename->d_name, &filesInfos[fileLengths]);

                maxGroupLength = (maxGroupLength > strlen(filesInfos[fileLengths].groupName))?maxGroupLength:strlen(filesInfos[fileLengths].groupName);
                maxUserNameSize = (maxUserNameSize > strlen(filesInfos[fileLengths].userName))?maxUserNameSize:strlen(filesInfos[fileLengths].userName);
                maxFileSize = (maxFileSize > filesInfos[fileLengths].st_size)?maxFileSize:filesInfos[fileLengths].st_size;
                maxNlink = (maxNlink > filesInfos[fileLengths].st_nlink)?maxNlink:filesInfos[fileLengths].st_nlink;

                fileLengths += 1;
                // totalBlocks += tmpBlock;
            }
            maxNlinkLength = intLength(maxNlink);
            maxSizeLength = intLength(maxFileSize);


            printAllResults(filesInfos, fileLengths, maxGroupLength, maxSizeLength, maxUserNameSize, maxNlinkLength);
      
            // printf("usernSize:%d, intLength:%d, nlinkSize:%d\n", maxUserNameSize, maxSizeLength, maxNlinkLength);
            // printf("总用量 %lu\n", totalBlocks);
        }
        else{
            // 非目录
            // unsigned long int totalBlocks = 0;
            // totalBlocks = printSingleFileInfo(path);
            printSingleFileInfo(path);
            // printf("总用量 %lu\n", totalBlocks);
        }

        if (argc == 1){
            return 0;
        }
    }



    /*
    char * filename = argv[1];
    
    printSingleFileInfo(filename);
    */
    /*  完整打开文件（对于link 则是指向内容的信息）
    int fd = open(filename, O_RDONLY);
    fstat(fd, &buff);
    */
    
    return 0;
}


int intLength(unsigned long int num){

    
    char tmp[BUFFLEN];
    sprintf(tmp, "%lu", num);
    return strlen(tmp);
    // return sizeof(num)/sizeof(char);
}


/*定义排序函数*/
int aasFileCmp(const void *a,const void *b){
    return strcmp(
        ((aasFileInfo*)a)->fileName,
        ((aasFileInfo*)b)->fileName
    );
}



int cmp(aasFileInfo a, aasFileInfo b){
    return strcmp(a.fileName, b.fileName);
}

void printAllResults(aasFileInfo filesInfos[], int size, int maxGroupLength, int maxSizeLength, int maxUserNameSize, int maxNlinkLength)
{
    // sort for filesInfos
    qsort(filesInfos, size, sizeof(filesInfos[0]), aasFileCmp);
    for (int i = 0; i < size; i++){
        aasFileInfo * fileInfo = &filesInfos[i];
        printf("%c%s%*lu %s%*s%s %*ld %s %s\n",
                fileInfo->fileT, fileInfo->fileAuth, 
                // st_nlink之前的空格数
                maxNlinkLength+1,
                fileInfo->st_nlink,
                // 在userName和GroupName之间 添加 maxUserNameSize+1个空格
                fileInfo->userName,
                maxUserNameSize-strlen(fileInfo->userName)+1, "",
                fileInfo->groupName,
                // maxGroupLength+maxSizeLength,
                maxGroupLength+maxSizeLength - strlen(fileInfo->groupName),         // 固定间隔
                fileInfo->st_size, fileInfo->timeBuf, fileInfo->fileName);
    }
}



void saveSingleFileInfo(char * filePath, aasFileInfo * fileInfo){
    // 打印单个文件的ls -l信息
    
    struct stat buff;
    
    //int fd = open(filePath, O_RDONLY);
    //fstat(fd, &buff);
    
    lstat(filePath, &buff);
    
    // get user name
    struct passwd * _passwd;
    _passwd = getpwuid(buff.st_uid);
    char * userName = _passwd->pw_name;

    // get group name
    struct group * _group;
    _group = getgrgid(buff.st_gid);
    char * groupName = _group->gr_name;


    // format time
    #define BUFFLEN 255
    char timeBuf[BUFFLEN];
    strftime(timeBuf, BUFFLEN, "%2m月  %d %H:%M", localtime(&buff.st_mtime));

    // file authorization
    char fileAuth[10] = "---------\0";
    fileAuthJudge(buff.st_mode, fileAuth);

    // file Name
    char fileT = fileType(buff.st_mode);
    char fileName[BUFFLEN];
    
    if (fileT == 'l'){
        char linkedPath[BUFFLEN];
        get_link_path(filePath, linkedPath, BUFFLEN);
        sprintf(fileName, "%s -> %s", filePath, linkedPath);
    }
    else{
        strcpy(fileName, filePath);
    }

    // 构造fileInfo
    fileInfo -> fileT = fileT;
    strcpy(fileInfo -> fileAuth, fileAuth);
    fileInfo -> st_nlink = buff.st_nlink;
    strcpy(fileInfo -> userName, userName);
    strcpy(fileInfo -> groupName, groupName);
    fileInfo -> st_size = buff.st_size;
    strcpy(fileInfo -> timeBuf, timeBuf);
    strcpy(fileInfo -> fileName, fileName);

}


unsigned long int printSingleFileInfo(char * filePath){

    // 打印单个文件的ls -l信息
    
    struct stat buff;
    
    //int fd = open(filePath, O_RDONLY);
    //fstat(fd, &buff);
    
    lstat(filePath, &buff);
    
    // get user name
    struct passwd * _passwd;
    _passwd = getpwuid(buff.st_uid);
    char * userName = _passwd->pw_name;

    // get group name
    struct group * _group;
    _group = getgrgid(buff.st_gid);
    char * groupName = _group->gr_name;

    // format time
    #define BUFFLEN 255
    char timeBuf[BUFFLEN];
    strftime(timeBuf, BUFFLEN, "%2m月%d %H:%M", localtime(&buff.st_mtime));

    // file authorization
    char fileAuth[10] = "---------\0";
    fileAuthJudge(buff.st_mode, fileAuth);

    // 格式化打印内容
    // printf("总用量 %lu\n", 0);
    char fileT = fileType(buff.st_mode);

    // file Name
    char fileName[BUFFLEN];
    
    if (fileT == 'l'){
        char linkedPath[BUFFLEN];
        get_link_path(filePath, linkedPath, BUFFLEN);
        sprintf(fileName, "%s -> %s", filePath, linkedPath);
    }
    else{
        strcpy(fileName, filePath);
    }

    // printf("%s name: %s\n", filePath, fileName);
    printf("%c%s%3lu %s %s %3ld %s %s\n",
            fileT, fileAuth, buff.st_nlink,
            userName, groupName, buff.st_size, timeBuf, fileName);


    /*
    printf("mode type:%c\n", fileType(buff.st_mode));
    printf("mode:%o\n", buff.st_mode); 
    printf("ino:%d\n", buff.st_ino);
    printf("dev:%d\n", buff.st_dev);
    printf("rdev:%d\n", buff.st_rdev);
    printf("nlink:%d\n", buff.st_nlink);
    printf("uid:%d\n", buff.st_uid);
    printf("gid:%d\n", buff.st_gid);
    printf("size:%d\n", buff.st_size);
    printf("atime:%d\n", buff.st_atime);
    printf("mtime:%d\n", buff.st_mtime);
    printf("blksize:%d\n", buff.st_blksize);
    printf("blocks:%d\n", buff.st_blocks);
    */

    // 总用量中的size貌似是buff中除以2的
    unsigned long int blocks = buff.st_blocks / 2;
    return blocks;
}

char * get_link_path(char * linkpath, char * buf, int count)
{
    int i;
    int rslt = readlink(linkpath, buf, count - 1);
    if (rslt < 0 || (rslt >= count - 1))
    {
        return NULL;
    }
    buf[rslt] = '\0';
    return buf;
}
 


void fileAuthJudge(mode_t st_mode, char *fileAuth){
    char authTmp[7] = "000000\0";
    sprintf(authTmp, "%o", st_mode);
    
    int authTmpLength = strlen(authTmp);
    // 4r,2w,1x, 1,2,3,4,5,6,7
    // printf("authTmp:%s\n", authTmp);
    char auth;
    int readInx, writeInx, execInx;
    for (int i = 0; i < 3; i++){
        auth = authTmp[i+authTmpLength-3];   
        readInx = 3*i+0;
        writeInx = 3*i+1;
        execInx = 3*i+2;
        if (auth == '0'){
            // do not change anything;
        }
        else if (auth == '1'){
            fileAuth[execInx]='x';
        }
        else if (auth == '2'){
            fileAuth[writeInx] = 'w';
        }
        else if (auth == '3'){
            fileAuth[execInx] = 'x';
            fileAuth[writeInx] = 'w';
        }
        else if (auth == '4'){
            fileAuth[readInx] = 'r';
        }
        else if (auth == '5'){
            fileAuth[execInx] = 'x';
            fileAuth[readInx] = 'r';
        }
        else if (auth == '6'){
            fileAuth[writeInx] = 'w';
            fileAuth[readInx] = 'r';
        }
        else if (auth == '7'){
            fileAuth[readInx] = 'r';
            fileAuth[writeInx] = 'w';
            fileAuth[execInx] = 'x';
        }
    }
    
    // printf("Auth:%s, sizeof Auth:%d\n", authTmp, strlen(authTmp));
}

char fileType(mode_t st_mode){
    // 根据st_mode返回文件类型
    char tmp = '0';
    if (S_ISREG(st_mode))
        tmp = '-';
    else if (S_ISDIR(st_mode))
        tmp = 'd';
    else if (S_ISBLK(st_mode))
        tmp = 'b';
    else if (S_ISCHR(st_mode))
        tmp = 'c';
    else if (S_ISSOCK(st_mode))
        tmp = 's';
    else if (S_ISLNK(st_mode))
        tmp = 'l';
    else if (S_ISFIFO(st_mode))
        tmp = 'p';

    return tmp;
}

char * getFileName(char * filePath, char * fileName, int buffSize){
    // 将filePath的fileName解析到fileName中去，buffsize为fileName和filePath的大小
    int k, lengths;
    for (int i = 0; i < buffSize; i++){
        if (filePath[i] == '/'){
            k = i;
        }
        else if (filePath[i] == '\0'){
            lengths = i;
            break;
        }
    }
    // from [k:lengths]
    for (int i = 0; i < lengths - k; i++){
        fileName[i] = filePath[k+1+i];
    }
    fileName[lengths] = '\0';
    return fileName;
}













