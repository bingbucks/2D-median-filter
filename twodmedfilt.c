/*Two Dimensional Median Filter with Linked Lists*/ 
/*C Program written by Christopher I. Tassone '96*/
/*Senior Design Electrical Engineering Project*/
/*Bucknell University*/
/*College of Engineering*/
/*Lewisburg, PA 17837 USA*/
/*Spring Semester 1996*/
/*My report is at http://www.eg.bucknell.edu/~tassone*/

/*This is not a very high-level looking program. I have never had any formal C training, I learned from PASCAL and C reference books. As I get older and wiser, so will my code*/


#include <stdio.h>
#include <iostream.h>
#include <math.h>



/*Defines the maximum array dimensions M_S x M_S and defines the window size W_S*/

#define M_S 800
#define W_S 25



/*Window is a N x N window, where W_S = N x N*/

int N = sqrt(W_S);



/*Define a structure onedTYPE that allows elements of a oneDarray to have 3 fields*/

struct onedTYPE
{
    int up, down;
    float value;
};
typedef onedTYPE oneDarray[W_S];



/*Function prototypes*/

void Pixel_Assign(char *matrix_file, float image_matrix[M_S][M_S], int num_row, int num_col);
void GetWin(oneDarray X);
void Where(oneDarray Q, int curXrowptr, int curXcolptr, int OldMedianPos, float OldMedianVal, int *UpOldMedian, int *DownOldMedian, int *OutBelow, int *OutAbove, int *OutAtMedian);
void AboveMedian(oneDarray Q, int curXrowptr, int curXcolptr, int *UpOldMedian, int DownOldMedian, int OutAtMedian, int OldMedianPos, float *OldMedianVal, int *AtOldMedianPos, int *InAbove);
void BelowMedian(oneDarray Q, int curXrowptr, int curXcolptr, int UpOldMedian, int *DownOldMedian, int OutAtMedian, int OldMedianPos, float *OldMedianVal, int *AtOldMedianPos, int *InBelow);
void NewMedianIs(oneDarray Q, int OutBelow, int OutAbove, int InBelow, int InAbove, int OutAtMedian, int AtOldMedianPos, float OldMedianVal, int UpOldMedian, int DownOldMedian, int OldMedianPos, float *NewMedianVal, int *UpNewMedian, int *DownNewMedian, int *NewMedianPos);
void Insert(float image_matrix[M_S][M_S], int num_col, int num_row);




/*The first thing that the program does is to open the file fp open count through the file to see how many rows and columns there are in the file.
  Once the number of rows and columns are know, we can then read them into variable pixels. By counting prior to reading the data in, this adds a 
  little file I/O delay, but it makes things much easier if you know how big your image is before read the data values into memory*/
/*IT IS IMPORTANT THAT THE INPUT TEXT FILE HAVE AN EOF WITH NO LAST CARRIAGE RETURN CHARACTER, OTHERWISE THE NUMBER OF ROWS COUNTED WILL BE SHORT*/
/*WHEN RUNNING THE PROGRAM, RUN IT AS - a.out filename.*/

main(int word_num, char *current_word[])						
{
    int i, j, c, first, row, column;
    float pixels[M_S][M_S];
    FILE *fp;
    
    if (word_num > 1) 
    { 						
	if ((fp=fopen(*++current_word, "r")) == NULL) 
	{
	    fprintf(stderr, "Can't open %s\n", *current_word);
	    return 1;
	}
	else
	{
	    row = 0;
	    column = 0;
	    first = 1;
	    while ((c = getc(fp)) != EOF) 
	    {
		if (c == '\n') 
		    row++;
		if (c == ' ' || c == '\n' || c == '\t') 
		{
		    if (first) 
		    {
			++column;
			first = 0;
		    }
		}
		else
		{
		    first = 1;
		}
	    }
	    column = column/row; /*Every time row gets incremented, column gets incremented. This ensures the correct numbers*/
	    fclose(fp);
	}
    }
    Pixel_Assign(*current_word, pixels, row, column); /*This function assigns the data from current_word to the variable pixels*/
    Insert(pixels, column, row); /*This is the heart to the median filter. Jump down for specific information*/
}



/********************************************************************************************************************************************/



/*This function assigns the data from current_word to the variable pixels*/

void Pixel_Assign(char *matrix_file, float image_matrix[M_S][M_S], int num_row, int num_col)
{
    int i, j;
    FILE *fp;
    fp=fopen(matrix_file, "r");
    for (i=0 ; i<num_row ; i++)
    {
	for (j=0 ; j<num_col ; j++ )
	{
	    fscanf(fp, "%f", &image_matrix[i][j]);
	}
    }
    fclose(fp);
    
}



/********************************************************************************************************************************************/



/*This function creates the initial window of size W_S with inital zero values in each element, and it also predefines the links*/

void GetWin(oneDarray X)
{
    int i;
    X[0].up = 1;
    X[0].down = 0;
    for(i=1 ; i<(W_S-1) ; i++)
    {
	X[i].down = i-1;
	X[i].up = i+1;
    }
    X[W_S-1].down = W_S-2;
    X[W_S-1].up = W_S-1;
    for(i=0 ; i<W_S ; i++)
    {
	X[i].value = 0;
	
    }
}


/********************************************************************************************************************************************/



/*This is the heart to the median filter. Jump down for specific information*/

void Insert(float image_matrix[M_S][M_S], int num_col, int num_row)
{
    int i, j, ALTj, curXrow, curXcol, curmatrow, curmatcol, ptdown, ptup, PosofOldMedian, next, OldMedianUp, OldMedianDown, NewMedianUp, NewMedianDown, PosofNewMedian, RemoveAbove, InsertAbove, RemoveBelow, InsertBelow, RemoveMedian, AtPosofOldMedian, rowflag;
    
    float OldMedian, NewMedian;
    
    oneDarray X;
    FILE *fp;
    curmatrow = 0;				/*Current row in the image*/
    curmatcol = 0;				/*Current column in the image*/
    rowflag = 0;				/*Variable in order to increment curmatrow through image*/
    ALTj = 0;
    fp = fopen("mine", "w"); 			/*Output file*/
    
    for(i=1 ; i <= num_row - (N-1) ; i++) 	/*Continue until we come to the bottom of the image*/
    {
	GetWin(X); 				/*Get the initial window*/
	OldMedianUp = (W_S+1)/2;		/*Predefine the element above the OldMedian*/
    	OldMedianDown = ((W_S-1)/2)-1;		/*Predefine the element below the OldMedian*/
 	PosofOldMedian = (W_S-1)/2;		/*Predefine the OldMedian position*/
    	OldMedian = 0.0;			/*Predefine the OldMedian value*/
    	RemoveBelow = 0;			/*If 1, we removed an element sorted below the OldMedian. Else 0*/
    	RemoveAbove = 0;			/*If 1, we removed an element sorted above the OldMedian. Else 0*/
    	RemoveMedian = 0;			/*If 1, we removed the element containing the OldMedian. Else 0*/
    	InsertAbove = 0;			/*If 1, we inserted an element above the OldMedian. Else 0*/
    	InsertBelow = 0;			/*If 1, we inserted an element below the OldMedian. Else 0*/
    	NewMedian = 0;				/*Variable initialization*/
    	NewMedianUp = 0;
    	NewMedianDown = 0;
   	PosofNewMedian = 0;
    	AtPosofOldMedian = 0;
	curXrow = 0;				/*Current row in psuedo-window. Check report*/
	curXcol = 0;				/*Current row in psuedo-window. Check report*/
	
	
	for(j=1 ; j <= (num_col*N) ; j++)	/*Continue until we come to the right edge of the image*/
	{
	    
	    /*Where figures out the position, above or below, relative to the OldMedian, of the element about to be removed from the linked list*/
	    
	    Where(X, curXrow, curXcol, PosofOldMedian, OldMedian, &OldMedianUp, &OldMedianDown, &RemoveBelow, &RemoveAbove, &RemoveMedian);
	    
	    
	    
	    
	    
	    /*Assign a new value from the image to that window element*/
	    
	    X[(N*curXrow)+curXcol].value = image_matrix[curmatrow][curmatcol];
	    
	    
	    
	    
	    
	    /*If the new element is greater then or equal to the OldMedian then goto function AboveMedian*/
	    
	    if (X[(N*curXrow)+curXcol].value >= OldMedian)
		AboveMedian(X, curXrow, curXcol, &OldMedianUp, OldMedianDown, RemoveMedian, PosofOldMedian, &OldMedian, &AtPosofOldMedian, &InsertAbove);
	    
	    
	    
	    
	    
	    /*If the new element is less than the OldMedian then goto function BelowMedian*/
	    
	    else if (X[(N*curXrow)+curXcol].value < OldMedian)
		BelowMedian(X, curXrow, curXcol, OldMedianUp, &OldMedianDown, RemoveMedian, PosofOldMedian, &OldMedian, &AtPosofOldMedian, &InsertBelow);
	    
	    
	    
	    
	    
	    /*function NewMedianIs returns the value of the NewMedian*/
	    
	    NewMedianIs(X, RemoveBelow, RemoveAbove, InsertBelow, InsertAbove, RemoveMedian, AtPosofOldMedian, OldMedian, OldMedianUp, OldMedianDown, PosofOldMedian, &NewMedian, &NewMedianUp, &NewMedianDown, &PosofNewMedian);
	    
	    
	    
	    
	    
	    /*ALTj remembers the last value of j. It's relavant when you reach the right edge of the image*/
	    
	    ALTj = j;
	    
	    
	    
	    
	    
	    /*These if-thens ensure that only every SQRT(W_S)th median get written to the output file. Since we remove each column of the psuedo-window one row at a time, we don't want EVERY median value, we just want the ones that are relavant to the Median Filtered image. Refer to report.*/
	    
	    
	    /*If we are not at the edges of the image, write the NewMedian to the file*/
	    
	    if ((ALTj % N == 0) && (curmatcol < (num_col-1)) && (curmatrow < num_row))		
		fprintf(fp, "%5.4f\t", NewMedian);
	    
	    
	    
	    
	    /*If we are at the bottom right hand corner of the image, then we are done, so close the output file*/
	    
	    else if ((ALTj % N == 0) && (curmatcol == (num_col-1)) && (curmatrow == (num_row-1)))
	    {
		fprintf(fp, "%5.4f", NewMedian);
		fclose(fp);
	    }
	    
	    
	    
	    
	    /*If we are at the right hand edge of the image, then right the NewMedian and also write a carriage return. NOTICE: that if we are at the bottom right corner of the image, this loop will NOT be entered*/
	    
	    else if ((ALTj % N == 0) && (curmatcol == (num_col-1)))
	    {
		fprintf(fp, "%5.4f\n", NewMedian);
	    }
	    
	    
	    
	    
	    
	    /*Increment the psuedo-window, so that when we remove the next element, it is the correct one. Refer to report for specifics and graphics*/ 
	    
	    if (curXrow == (N-1))
	    {
		curXrow = 0;
		if (curXcol < (N-1))
		    curXcol++;
		else
		    curXcol = 0;
	    }
	    else
		curXrow++;
	    
	    
	    
	    
	    /*Increment the pointer to the image matrix, so that when we read in the next element, it is the correct one. Refer to report for specifics and graphics*/ 
	    /*rowflag is just the current row that the top of the psuedo-window is at. This ensures that we are moving down in the image*/
	    
	    if (curmatrow == rowflag + (N-1))
	    {
		if (curmatcol < (num_col-1))
		{	
		    curmatcol++;
		    curmatrow = rowflag;
		}
		else
		{
		    j = (num_col*N);
		    curmatcol = 0;
		    curmatrow = ++rowflag;
		}
	    }
	    else
		curmatrow ++;
	    
	    
	    
	    RemoveAbove = 0;			/*Reset and initialize values*/
	    InsertAbove = 0;
	    RemoveBelow = 0;
	    InsertBelow = 0;
	    RemoveMedian = 0;
	    AtPosofOldMedian = 0;
	    OldMedian = NewMedian;
	    OldMedianUp = NewMedianUp;
	    OldMedianDown = NewMedianDown;
	    PosofOldMedian = PosofNewMedian;
	    
	}
    }
}



/********************************************************************************************************************************************/



/*Where figures out the position, above or below, relative to the OldMedian, of the element about to be removed from the linked list*/

void Where(oneDarray Q, int curXrowptr, int curXcolptr, int OldMedianPos, float OldMedianVal, int *UpOldMedian, int *DownOldMedian, int *OutBelow, int *OutAbove, int *OutAtMedian)
{
    int next;
    
    
    
    
    /*If we are removing the element at the bottom of the array, then reassign links, and assert OutBelow*/
    /*Remember that elements at the bottom point down to themselves*/
    
    if (Q[((N*curXrowptr)+curXcolptr)].down == ((N*curXrowptr)+curXcolptr))
    {
	Q[Q[((N*curXrowptr)+curXcolptr)].up].down=Q[((N*curXrowptr)+curXcolptr)].up;
	*OutBelow=1;
    }
    
    
    
    
    /*If we are removing the element at the top of the array, then reassign links, and assert OutAbove*/
    /*Remember that elements at the top point up to themselves*/
    
    else if (Q[((N*curXrowptr)+curXcolptr)].up == ((N*curXrowptr)+curXcolptr))
    {
	Q[Q[((N*curXrowptr)+curXcolptr)].down].up = Q[((N*curXrowptr)+curXcolptr)].down;
	*OutAbove = 1;
    }
    




    /*Else if we are removing elements from within the array, we have to figure out if they're are above or below the position of the Old Median*/
    /*There are a few ways to do it. If the old value in the element is less than the Old Median, then obviously it is below the Old Median position*/
    /*If the old value is greater than the Old Median, then obviously it is above the Old Median position*/
    /*If we are removing the Old Median itself, then assert OutAtMedian*/
    /*If we are removing a value that is EQUAL to the Old Median, but it is NOT the old median, then we must increment through the array until we pass the position of the Old Median. If we pass the position of the Old Median while incrementing UP through the array, then we were BELOW the position of the Old Median, therefore assert OutBelow. Vice Versa for the other direction.*/
    
    else
    {
	if (Q[((N*curXrowptr)+curXcolptr)].value < OldMedianVal)
	    *OutBelow = 1;
	else if (Q[((N*curXrowptr)+curXcolptr)].value > OldMedianVal)
	    *OutAbove = 1;
	else if ((Q[((N*curXrowptr)+curXcolptr)].value == OldMedianVal) && (((N*curXrowptr)+curXcolptr) == OldMedianPos))
	    *OutAtMedian =1;
	else if ((Q[((N*curXrowptr)+curXcolptr)].value == OldMedianVal) && (((N*curXrowptr)+curXcolptr) != OldMedianPos))
	{
	    next = Q[((N*curXrowptr)+curXcolptr)].up;
	    while ((next != OldMedianPos) && (Q[next].up != next))
	    {
		next = Q[next].up;
	    }
	    if (next == OldMedianPos)
		*OutBelow = 1;
	    else
		next = Q[((N*curXrowptr)+curXcolptr)].down;
	    while ((next != OldMedianPos) && (Q[next].down != next))
	    {
		next = Q[next].down;
	    }
	    if ((next == OldMedianPos) && (*OutBelow != 1))
		*OutAbove =1;
	}
	
	
	
	/*Reassign the rest of the links*/
	
	Q[Q[((N*curXrowptr)+curXcolptr)].down].up = Q[((N*curXrowptr)+curXcolptr)].up;
	Q[Q[((N*curXrowptr)+curXcolptr)].up].down = Q[((N*curXrowptr)+curXcolptr)].down;
	
	
	
	
	/*In case we are removing the values in the elements immediately above or below the position of the Old Median, we need to ensure that the UpOldMedian and DownOldMedian variables contain the correct element*/
	
	if (((N*curXrowptr)+curXcolptr) == *UpOldMedian)
	    *UpOldMedian = Q[((N*curXrowptr)+curXcolptr)].up;
	else if (((N*curXrowptr)+curXcolptr) == *DownOldMedian)
	    *DownOldMedian = Q[((N*curXrowptr)+curXcolptr)].down;
    }
}



/********************************************************************************************************************************************/



/*If the new element is greater then or equal to the OldMedian then goto function AboveMedian*/

void AboveMedian(oneDarray Q, int curXrowptr, int curXcolptr, int *UpOldMedian, int DownOldMedian, int OutAtMedian, int OldMedianPos, float *OldMedianVal, int *AtOldMedianPos, int *InAbove)
{
    int next, ptdown;
    next = 0;
    ptdown = 0;
    next = *UpOldMedian;
    
    
    
    
    /*If the NEW value is greater than the Old Median, but less than the value right above the Old Median, AND we DIDN't remove the Old Median, then insert it here*/
    
    if ((Q[((N*curXrowptr)+curXcolptr)].value < Q[next].value) && (!(OutAtMedian)))
    {
	ptdown = OldMedianPos;
	*UpOldMedian = ((N*curXrowptr)+curXcolptr);
    }
    
    
    
    /*If the NEW value is greater than the Old Median, but less than the value right above the Old Median, AND we DID remove the Old Median, then insert it here, and the New Median is this current value.*/
    
    else if ((Q[((N*curXrowptr)+curXcolptr)].value < Q[next].value) && (OutAtMedian))
    {
	ptdown = DownOldMedian;
	*AtOldMedianPos = 1;
	*OldMedianVal = Q[((N*curXrowptr)+curXcolptr)].value;
    }

    
    
    
    /*Else increment through the array to see where the NEW value is to be inserted, then reconnected the links properly*/
    /*Increment until there is a value in the array bigger than the NEW value, or until we reach the top of the list*/
    
    while ((Q[((N*curXrowptr)+curXcolptr)].value >= Q[next].value) && (!(Q[next].up == next)))
    {
	ptdown = next;
	next = Q[next].up;
    }
    if ((Q[next].up == next) && (Q[((N*curXrowptr)+curXcolptr)].value >= Q[next].value))
    {
	Q[((N*curXrowptr)+curXcolptr)].down = next;
	Q[((N*curXrowptr)+curXcolptr)].up = ((N*curXrowptr)+curXcolptr);
	Q[next].up = ((N*curXrowptr)+curXcolptr);
    }
    else if (Q[((N*curXrowptr)+curXcolptr)].value < Q[next].value)
    {
	Q[((N*curXrowptr)+curXcolptr)].up = next;
	Q[next].down = ((N*curXrowptr)+curXcolptr);
	Q[((N*curXrowptr)+curXcolptr)].down = ptdown;
	Q[ptdown].up = ((N*curXrowptr)+curXcolptr);
    }
    
    
    /*Since we are inserting a value above the position of the Old Median, assert InAbove*/
    
    *InAbove = 1;
}


/********************************************************************************************************************************************/



/*If the new element is less than the OldMedian then goto function BelowMedian*/

void BelowMedian(oneDarray Q, int curXrowptr, int curXcolptr, int UpOldMedian, int *DownOldMedian, int OutAtMedian, int OldMedianPos, float *OldMedianVal, int *AtOldMedianPos, int *InBelow)
{
    int next, ptup;
    next = 0;
    ptup = 0;
    
    next = *DownOldMedian;
    
    
    
    /*If the NEW value is less than the Old Median, but greater than the value right below the Old Median, AND we DIDN't remove the Old Median, then insert it here*/
    
    if ((Q[((N*curXrowptr)+curXcolptr)].value > Q[next].value) && (!(OutAtMedian)))
    {
	ptup = OldMedianPos;
	*DownOldMedian = ((N*curXrowptr)+curXcolptr);
    }
    
    
    
    
    /*If the NEW value is less than the Old Median, but greater than the value right below the Old Median, AND we DID remove the Old Median, then insert it here, and the New Median is this current value.*/
    
    else if ((Q[((N*curXrowptr)+curXcolptr)].value > Q[next].value) && (OutAtMedian))
    {
	ptup = UpOldMedian;
	*AtOldMedianPos = 1;
	*OldMedianVal = Q[((N*curXrowptr)+curXcolptr)].value;
    }
    
    
    
    
    
    /*Else increment through the array to see where the NEW value is to be inserted, then reconnected the links properly*/
    /*Increment until there is a value in the array smaller than the NEW value, or until we reach the bottom of the list*/
    
    while ((Q[((N*curXrowptr)+curXcolptr)].value <= Q[next].value) && (!(Q[next].down == next)))
    {
	ptup = next;
	next = Q[next].down;
    }
    if ((Q[next].down == next) && (Q[((N*curXrowptr)+curXcolptr)].value <= Q[next].value))
    {
	Q[((N*curXrowptr)+curXcolptr)].up = next;
	Q[((N*curXrowptr)+curXcolptr)].down = ((N*curXrowptr)+curXcolptr);
	Q[next].down = ((N*curXrowptr)+curXcolptr);
    }
    else if (Q[((N*curXrowptr)+curXcolptr)].value > Q[next].value)
    {
	Q[((N*curXrowptr)+curXcolptr)].down = next;
	Q[next].up = ((N*curXrowptr)+curXcolptr);
	Q[((N*curXrowptr)+curXcolptr)].up = ptup;
	Q[ptup].down = ((N*curXrowptr)+curXcolptr);
    }


    /*Since we are inserting a value below the position of the Old Median, assert InBelow*/
    
    *InBelow = 1;
}





/********************************************************************************************************************************************/




/*function NewMedianIs returns the value of the NewMedian*/
/*If we remove a value from below(above) the position of the Old Median, and we sort in a value below(above) the position of the Old Median, then New Median = Old Median.*/
/*If we remove a value from below(above) the position of the Old Median, and we sort in a value above(below) the position of the Old Median, then the New Median = The value of the element that the Old Median was pointing up to (The value of the element that the Old Median was pointing down to).*/

void NewMedianIs(oneDarray Q, int OutBelow, int OutAbove, int InBelow, int InAbove, int OutAtMedian, int AtOldMedianPos, float OldMedianVal, int UpOldMedian, int DownOldMedian, int OldMedianPos, float *NewMedianVal, int *UpNewMedian, int *DownNewMedian, int *NewMedianPos)
{
    if ((OutBelow && InBelow) || (OutAbove && InAbove) || (OutAtMedian && AtOldMedianPos))
    {
	*NewMedianVal = OldMedianVal;
	*UpNewMedian = UpOldMedian;
	*DownNewMedian = DownOldMedian;
	*NewMedianPos = OldMedianPos;
    }
    else if ((OutBelow && InAbove) || (OutAtMedian && InAbove && (!AtOldMedianPos)))
    {
	*NewMedianVal = Q[UpOldMedian].value;
	*UpNewMedian = Q[UpOldMedian].up;
	*DownNewMedian = Q[UpOldMedian].down;
	*NewMedianPos = UpOldMedian;
    }
    else if ((OutAbove && InBelow) || (OutAtMedian && InBelow && (!AtOldMedianPos)))
    {
	*NewMedianVal = Q[DownOldMedian].value;
	*UpNewMedian = Q[DownOldMedian].up;
	*DownNewMedian = Q[DownOldMedian].down;
	*NewMedianPos = DownOldMedian;
    }
}

























