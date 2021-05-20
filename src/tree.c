#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "tree.h"

char* combineStr(char* s1, char* s2) {
	if (s1 == NULL || s2 == NULL) {
		panic("argument is NULL");
	}
	
	char* s3 = malloc(strlen(s1) + strlen(s2) + 1);
	strcpy(s3, s1);
	strcat(s3, s2);
	
	free(s1);
	free(s2);
	
	return s3;
}

struct node newTextNode(char* text) {
	return (struct node) {
		.type = TEXT_NODE,
		.value.text = text
	};
}

struct node newStatementNode(char* stm, struct tree tree) {

	struct tree* _tree = malloc(sizeof(struct tree));
	if (_tree == NULL) {
		panic("malloc");
	}
	
	memcpy(_tree, &tree, sizeof(struct tree));

	return (struct node) {
		.type = STATEMENT_NODE,
		.statement = stm,
		.value.tree = _tree
	};
}

struct node newOutputNode(char* text) {
	return (struct node) {
		.type = OUTPUT_NODE,
		.value.text = text
	};
}

struct tree newTree() {
	return (struct tree) {
		.kids = NULL,
		.kidsno = 0
	};
}

void addNode(struct tree* tree, struct node node) {
	void* tmp = realloc(tree->kids, (sizeof (struct node)) * (tree->kidsno + 1));
	if (tmp == NULL) {
		panic("realloc");
	}
	
	tree->kids = tmp;
	tree->kids[tree->kidsno++] = node;
}

struct tree combineTree(struct tree t1, struct tree t2) {
	struct tree t = {
		.kidsno = t1.kidsno + t2.kidsno
	};
	
	t.kids = malloc(sizeof(struct node) * t.kidsno);
	if (t.kids == NULL) {
		panic("malloc");
	}
	
	if (t1.kids != NULL) {
		memcpy(t.kids, t1.kids, t1.kidsno * sizeof(struct node));
		free(t1.kids);
	}
	if (t2.kids != NULL) {
		memcpy(t.kids + t1.kidsno, t2.kids, t2.kidsno * sizeof(struct node));
		free(t2.kids);	
	}
	
	return t;
}

struct params newParams() {
	return (struct params) {
		.types = NULL,
		.names = NULL,
		.no = 0
	};
}

void addParam(struct params* params, char* type, char* name) {
	void* tmp;
	tmp = realloc(params->types, sizeof(char*) * (params->no + 1));
	if (tmp == NULL) {
		panic("realloc");
	}
	params->types = tmp;
	tmp = realloc(params->names, sizeof(char*) * (params->no + 1));
	if (tmp == NULL) {
		panic("realloc");
	}
	params->names = tmp;
	
	params->types[params->no] = type;
	params->names[params->no] = name;
	params->no++;
}

struct params combineParams(struct params p1, struct params p2) {
	struct params p = {
		.no = p1.no + p2.no
	};
	
	p.types = malloc(sizeof(char*) * p.no);
	if (p.types == NULL) {
		panic("malloc");
	}
	p.names = malloc(sizeof(char*) * p.no);
	if (p.names == NULL) {
		panic("malloc");
	}
	
	memcpy(p.types, p1.types, p1.no * sizeof(char*));
	memcpy(p.types + p1.no, p2.types, p2.no * sizeof(char*));
	memcpy(p.names, p1.names, p1.no * sizeof(char*));
	memcpy(p.names + p1.no, p2.names, p2.no * sizeof(char*));
	
	return p;
}

struct stats newStats() {
	return (struct stats) {
		.texts = NULL,
		.no = 0
	};
}

void addStat(struct stats* stats, char* text) {
	void* tmp = realloc(stats->texts, sizeof(char*) * (stats->no + 1));
	if (tmp == NULL) {
		panic("realloc");
	}
	
	stats->texts = tmp;
	
	stats->texts[stats->no++] = text;
}

struct template newTemplate() {
	return (struct template) {
		.params = newParams(),
		.stats = newStats(),
		.tree = newTree()
	};
}
