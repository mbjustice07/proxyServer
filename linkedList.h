//Alejandro Nicolette and Courtney Akin

#include <stdlib.h>

typedef struct listNode {
	void *item;
	struct listNode *next;
} listNode;

/* Utility Functions */
extern listNode*	initNode(int itemSize);
extern void		freeNodes(listNode *front);
extern listNode*	addToFront(listNode *front, listNode *newNode);
extern listNode*	addToEnd(listNode *front, listNode *newNode);
extern listNode*	deleteNode(listNode *front, void *toDelete, int(*comparator)(void *, void *));
extern void*		contains(listNode *front, void *toFind, int(*comparator)(void *, void *));
extern int		checkNode(listNode *node);

/* Comparator Functions */
extern int 		compareInts(void *numOne, void *numTwo);
