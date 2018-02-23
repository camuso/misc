static bool is_recursive_def(struct knode *kn)
{
	char s1[STRBUFSIZ];
	char s2[STRBUFSIZ];
	int count = 0;

	extract_type(kn, s1);

	while (kn->level != 0) {
		kn = kn->parent;
		extract_type(kn, s2);

		if (count >= 1)
			return true;

		if (!strncmp(s1, s2, STRBUFSIZ))
			++count;
	}
	return false;
}

