#ifndef TREE_H_
#define TREE_H_

#include <stdbool.h>

char* combineStr(char*, char*);

#define TEXT_NODE      (0)
#define STATEMENT_NODE (1) 
#define OUTPUT_NODE    (2)

struct node {
	int type;
	char* statement;
	union {
		struct tree* tree;
		char* text;
	} value;
};

struct tree {
	struct node* kids;
	size_t kidsno;
};


struct node newTextNode(char*);
struct node newStatementNode(char*, struct tree);
struct node newOutputNode(char*);

struct tree newTree();
void addNode(struct tree*, struct node);
struct tree combineTree(struct tree, struct tree);

struct params {
	char** types;
	char** names;
	size_t no;
};

struct params newParams();
void addParam(struct params*, char*, char*);
struct params combineParams(struct params, struct params);

struct stats {
	char** texts;
	size_t no;
};

struct stats newStats();
void addStat(struct stats*, char*);

struct template {
	struct params params;
	struct tree tree;
	struct stats stats;
};

struct template newTemplate();

bool checkCharset(const char*, const char*);

#endif
