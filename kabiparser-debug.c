static int counter = 0;
static void dot(int count) {
	while (count--)
		putchar('.');
}

static void dump_pointers(struct knodelist *klist)
{
	struct knode *kn;

	printf("entry knodes: %d\n", knode_list_size(klist));

	FOR_EACH_PTR(klist, kn) {
		char *name = (kn->name) ? kn->name : "noname";
		printf("knum:%03d size:%2d k:%p p:%p name: %s\n",
				kn->number, knode_list_size(kn->children),
				kn, kn->parent, name);
		if (kn->children) {
			puts("recursive call:");
			dump_pointers(kn->children);
			printf("knum:%03d size:%2d k:%p p:%p\n",
					kn->number, knode_list_size(kn->children),
					kn, kn->parent);
			puts("return from recursive call");
			if(kn == last_knode(klist)) {
				puts("last node");
			}
		}

		printf("nomore children\n");
		printf("knum:%03d size:%2d k:%p p:%p\n",
				kn->number, knode_list_size(kn->children), kn, kn->parent);

	} END_FOR_EACH_PTR(kn);
out:
	printf("exit\n");
	printf("knum:%03d size:%2d k:%p p:%p\n",
			kn->number, knode_list_size(kn->children), kn, kn->parent);
	counter--;
}

