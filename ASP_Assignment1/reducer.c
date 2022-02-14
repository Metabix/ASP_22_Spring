=========================================================================
=																		                  =
=				Name: Mohit Sathyaseelan								            =
=				contact: Mohitsathyaseelan@gmail.com					         =
=																		                  =
=========================================================================

#include <stdio.h>
#include <string.h>

struct sample{
     char usid[5];
     char action;
     char topic[16];
     int score;
};

int main()
{
     int a = 0;
  
     int data = 3;
     char c;
     struct sample new[100];
     while(1)
     {
         data = scanf("(%[^,],%[^,],%d)%c", new[a].usid, new[a].topic, &new[a].score,&c);
         if(data != 4)
         {
               break;
         }
         a++;
     }
     
     int d = 0;
     int i = 0;
     int k = 0;
     int f = 0;
    
     int b = 0;
     FILE *fp = fopen("output.txt", "w");
     struct sample updated[100];
     for(b = 0; b < a; b++)
     {
         if(b == 0)  					//initial value stored into the structure array
         {
            strcpy(updated[d].usid, new[b].usid);
            updated[d].score = new[b].score;
            strcpy(updated[d].topic, new[b].topic);
         }
         else if(strcmp(new[b].usid, new[b-1].usid) == 0) // first check userid
         {
            for(i = k; i <= d; i++)
            {
               if(strcmp(new[b].topic,updated[i].topic) == 0) //check topic
               {
                  updated[i].score += new[b].score;		// add scores
                  f = 1;
                  
               }
            }
            if(f == 0)						//save userid and topic
            {
               d++;
               strcpy(updated[d].usid, new[b].usid);
               updated[d].score = new[b].score;
               strcpy(updated[d].topic, new[b].topic);
            }
            f = 0;
            if(b == a-1)					//compar with prev
            {
               for(i = k; i <= d; i++)			//print the struct array(for same topic)
               {
                  printf("(%s,%s,%d)\n", updated[i].usid, updated[i].topic, updated[i].score);
                  fprintf(fp,"%s,%s,%d)\n",updated[i].usid, updated[i].topic,updated[i].score);
               }
            }           
         }
         
         
         
         
         else if(strcmp(new[b].usid, new[b-1].usid) != 0) 	//different userid's
         {
            for(i = k; i<=d; i++)
            {
               printf("(%s,%s,%d)\n", updated[i].usid, updated[i].topic, updated[i].score);
               fprintf(fp,"%s,%s,%d)\n",updated[i].usid, updated[i].topic,updated[i].score);
            }
            
            
            
            d++;k = d;
            strcpy(updated[d].usid, new[b].usid);
            updated[d].score = new[b].score;
            strcpy(updated[d].topic, new[b].topic);
            if (b == a - 1)					//compar with prev
	    {
	       printf("(%s,%s,%d)\n", updated[d].usid, updated[d].topic, updated[d].score);
	       fprintf(fp,"%s,%s,%d)\n",updated[d].usid, updated[d].topic,updated[d].score);
	    }
         }
     }
     printf("\n\nReducer done\n\n");
}
