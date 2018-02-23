struct duprec {
	struct duprec *next;
	struct duprec *prev;
	int level;
	bool isdup;
	char str[DECLSIZ];
};

struct duprec *duplist = NULL;

struct duprec *new_duprec(int level)
{
	struct duprec *dup = (struct duprec *)malloc(sizeof(struct duprec));
	struct duprec *tmp = duplist;

	if (duplist) {

		while (tmp->next != duplist)
			tmp = tmp->next;

		tmp->next = dup;
		dup->prev = tmp;
		dup->next = duplist;
		duplist->prev = dup;
	} else {
		duplist = dup;
		dup->next = dup;
		dup->prev = dup;
	}
	dup->isdup = false;
	dup->level = level;
	return dup;
}

struct duprec *find_duprec(int level)
{
	struct duprec *tmp = duplist;

	do {
		if ((tmp)->level == level)
			return tmp;
		tmp = tmp->next;
	} while (tmp != duplist);

	return NULL;
}

static inline void delete_duprec(struct duprec *dup)
{
	free(dup);
}

void delete_duplist(struct duprec *duplist)
{
	struct duprec *tmp = duplist->prev;
	do {
		tmp = tmp->prev;
		delete_duprec(tmp->next);
	} while(tmp != duplist);
	delete_duprec(duplist);
	duplist = NULL;
}

