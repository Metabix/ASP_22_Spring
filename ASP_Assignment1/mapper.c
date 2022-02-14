=========================================================================
=																		=
=				Name: Mohit Sathyaseelan								=
=				contact: Mohitsathyaseelan@gmail.com					=
=																		=
=========================================================================
# include <stdio.h>
# include <string.h>
#include <stdlib.h>


int main()
{
	FILE *filePointer;
	FILE *fp;
	
	char datarb[100];
	
	filePointer =fopen("input.txt", "r");
	fp = fopen("Mapper_Output.txt", "w");
	
	while( fgets (datarb, 26, filePointer) != NULL)

	{
		
		char * userID = strtok(datarb, ",");		//saving using tokens
		char* action = strtok(NULL, ",");
		char* topic = strtok(NULL, ")");
		int score=0;
		char opt = *action;
		
		switch(opt)
		{
		
		case 'P':
			printf("%s,%s,%d)\n",userID, topic, 50);
			fprintf(fp,"%s,%s,%d)\n",userID, topic, 50);
			break;
		case 'L':
			printf("%s,%s,%d)\n",userID, topic, 20);
			fprintf(fp,"%s,%s,%d)\n",userID, topic, 20);
			break;
		case 'D':
			printf("%s,%s,%d)\n",userID, topic, -10);
			fprintf(fp,"%s,%s,%d)\n",userID, topic, -10);
			break;
		case 'C':
			printf("%s,%s,%d)\n",userID, topic, 30);
			fprintf(fp,"%s,%s,%d)\n",userID, topic, 30);
			break;
		case 'S':
			printf("%s,%s,%d)\n",userID, topic, 40);
			fprintf(fp,"%s,%s,%d)\n",userID, topic, 40);
			break;
		}
	}
	printf("\n\nMapper done\n\n");
	fclose(filePointer);
	fclose(fp);
    
    return 0;       
}
