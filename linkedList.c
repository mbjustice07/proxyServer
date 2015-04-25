//Alejandro Nicolette and Courtney Akin

#include "linkedList.h"

/*
	Initializes a new node for a linked list. The caller must check to make sure the value is not null. This allows the caller to handle errors however they wish.
*/
listNode* initNode(int itemSize){
	listNode *empty = NULL;
	empty = (listNode *)malloc(sizeof(listNode));
	if(empty != NULL){
		empty->item = malloc(itemSize);
	}
	empty->next = NULL;
	return empty;
}

/*
	Frees all nodes.
*/
void freeNodes(listNode *front){
	listNode *temp = front->next;
	if (temp == NULL){//The list is empty
		free(front);
		return;
	}
	else{
		while(temp != NULL){
			if(front->item != NULL){
				free(front->item);
			}
			free(front);
			front = temp;
			temp = temp->next;
		}
		if(front->item != NULL){
			free(front->item);
		}
		free(front);
	}

}

/*
	Adds a new node to the front while maintaining the empty node at the end. Returns the front of the list.
*/
listNode* addToFront(listNode *front, listNode *newNode){
	newNode->next = front;
	return newNode;
}

/*
	Adds a new node to the end while maintaining the empty node at the end. Returns the front of the list.	
*/
listNode* addToEnd(listNode *front, listNode *newNode){
	if(front->next == NULL){//The list is empty
		return addToFront(front, newNode); 
	}
	else{
		listNode *temp = front;
		while(temp->next->next != NULL){
			temp = temp->next;	
		}
		//temp should now be right before the empty node
		newNode->next = temp->next;
		temp->next = newNode;
		return front;
	}
}

/*
	Uses the comparator function to find a node that has an item matching the one to delete, then removes it. Returns the front of the list.
*/
listNode* deleteNode(listNode *front, void *toDelete, int(*comparator)(void *, void *)){
	listNode *temp = front;
	while(temp->next != NULL){
		if(comparator(temp->item, toDelete) == 0){//Found it!
			break;
		}
		temp = temp->next;
	}
	if(temp->next == NULL){//The element wasn't actually in the list, do nothing
		return front;
	}
	else{
		listNode *placeHolder = temp->next;
		if(temp->item != NULL){
			free(temp->item);
		}
		temp->item = temp->next->item;
		temp->next = temp->next->next;
		free(placeHolder);
		return front;
	}
}

/*
	Searches the list for the given element using the comparator function.
	Returns the element if it is found, and NULL otherwise.
*/
void* contains(listNode *front, void *toFind, int(*comparator)(void *, void *)){
	if(front->next == NULL){//List is empty
		return 0;
	}
	else{
		while(front->next != NULL){
			if(comparator(front->item, toFind) == 0){
				return front->item;
			}
			front = front->next;
		}
		return NULL;
	}
}

/*
	Assumes the given pointers point to ints and compares them.
	If numOne is greater, returns 1
	If they are equal, returns 0
	if numTwo is greater, returns -1
*/

int compareInts(void *numOne, void *numTwo){
	int one = *(int *)numOne;
	int two = *(int *)numTwo;

	if (one > two){
		return 1;
	}	
	else if (one == two){
		return 0;
	}
	else{
		return -1;
	}
}

/*
	Checks if a node was properly malloc'd. Returns 1 if all is well and 0 otherwise.
*/
int checkNode(listNode *node){
	return (node != NULL) && (node->item != NULL);
}

