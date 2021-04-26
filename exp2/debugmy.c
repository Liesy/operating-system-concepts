#include<stdio.h>
void myputc(char* cptr)
{
	*cptr='a';
	printf("myputc=%c\n",*cptr);
}
int main(void)
{
	char c;
	char *cptr;
	c='A';
	myputc(cptr);
	return 0;
}
