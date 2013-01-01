#include <eve.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include "examples/tester/test01.eve.h"
void File_Open (__pFile self, str path, str mode)
{
(( self  ->  file ) = fopen( path ,  mode ));
(( self  ->  path ) =  path );
(( self  ->  Opened ) =  true );
}
void File_Write (__pFile self, str text)
{
fprintf(( self  ->  file ),  text );
}
void File_Close (__pFile self)
{
fclose(( self  ->  file ));
(( self  ->  path ) =  NULL );
(( self  ->  Opened ) =  false );
}
void Test01_test01 (__pTest01 self, int x)
{
printf( "%d, %d.\n" , (( self  ->  val ) +  x ),  x );
}
int Test01_test02 (__pTest01 self, int x)
{
printf( "%d, %d.\n" , ( self  ->  val ),  x );
return  x ;
}
void Test01_test03 (__pTest01 self, str t)
{
printf( "%s\n" ,  t );
}
str Test01_test04 (__pTest01 self, str t)
{
return  t ;
}
void test ()
{
Test01 T;
(( T  .  val ) =  3 );
(Test01_test01 (&T,  4 ));
}
int main ()
{
ATEST x;
Test01 t;
Test01 t2;
test();
(( t  .  val ) =  30 );
(( t2  .  val ) =  23 );
(( x  [  0  ] ) =  t );
(Test01_test01 (&t2,  0 ));
(( x  [  0  ] ) =  x );
((( x  [  0  ] ) .  val ) =  400 );
(Test01_test01 ( & ( x  [  0  ] ),  10 ));
return  0 ;
}
