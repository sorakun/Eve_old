typedef char  * str;
typedef int  * pint;
typedef float  * pfloat;
typedef char  * pchar;
typedef FILE  * pFILE;
struct File;
typedef struct File * __pFile;
typedef struct File{
str path;
pFILE file;
bool Opened;

}File;
void File_Open (__pFile self, str path, str mode);
void File_Write (__pFile self, str text);
void File_Close (__pFile self);
struct Test01;
typedef struct Test01 * __pTest01;
typedef struct Test01{
int val;

}Test01;
void Test01_test01 (__pTest01 self, int x);
int Test01_test02 (__pTest01 self, int x);
void Test01_test03 (__pTest01 self, str t);
str Test01_test04 (__pTest01 self, str t);
typedef Test01  * ATEST;
void test ();
int main ();
