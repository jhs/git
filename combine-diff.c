#include "userdiff.h"
static char *grab_blob(const unsigned char *sha1, unsigned int mode,
		       unsigned long *size, struct userdiff_driver *textconv,
		       const char *path)
	} else if (textconv) {
		struct diff_filespec *df = alloc_filespec(path);
		fill_filespec(df, sha1, mode);
		*size = fill_textconv(textconv, df, &blob);
		free_filespec(df);
			 int num_parent, int result_deleted,
			 struct userdiff_driver *textconv,
			 const char *path)
	parent_file.ptr = grab_blob(parent, mode, &sz, textconv, path);
static void show_combined_header(struct combine_diff_path *elem,
				 int num_parent,
				 int dense,
				 struct rev_info *rev,
				 int mode_differs,
				 int show_file_header)
{
	struct diff_options *opt = &rev->diffopt;
	int abbrev = DIFF_OPT_TST(opt, FULL_INDEX) ? 40 : DEFAULT_ABBREV;
	const char *a_prefix = opt->a_prefix ? opt->a_prefix : "a/";
	const char *b_prefix = opt->b_prefix ? opt->b_prefix : "b/";
	int use_color = DIFF_OPT_TST(opt, COLOR_DIFF);
	const char *c_meta = diff_get_color(use_color, DIFF_METAINFO);
	const char *c_reset = diff_get_color(use_color, DIFF_RESET);
	const char *abb;
	int added = 0;
	int deleted = 0;
	int i;

	if (rev->loginfo && !rev->no_commit_id)
		show_log(rev);

	dump_quoted_path(dense ? "diff --cc " : "diff --combined ",
			 "", elem->path, c_meta, c_reset);
	printf("%sindex ", c_meta);
	for (i = 0; i < num_parent; i++) {
		abb = find_unique_abbrev(elem->parent[i].sha1,
					 abbrev);
		printf("%s%s", i ? "," : "", abb);
	}
	abb = find_unique_abbrev(elem->sha1, abbrev);
	printf("..%s%s\n", abb, c_reset);

	if (mode_differs) {
		deleted = !elem->mode;

		/* We say it was added if nobody had it */
		added = !deleted;
		for (i = 0; added && i < num_parent; i++)
			if (elem->parent[i].status !=
			    DIFF_STATUS_ADDED)
				added = 0;
		if (added)
			printf("%snew file mode %06o",
			       c_meta, elem->mode);
		else {
			if (deleted)
				printf("%sdeleted file ", c_meta);
			printf("mode ");
			for (i = 0; i < num_parent; i++) {
				printf("%s%06o", i ? "," : "",
				       elem->parent[i].mode);
			}
			if (elem->mode)
				printf("..%06o", elem->mode);
		}
		printf("%s\n", c_reset);
	}

	if (!show_file_header)
		return;

	if (added)
		dump_quoted_path("--- ", "", "/dev/null",
				 c_meta, c_reset);
	else
		dump_quoted_path("--- ", a_prefix, elem->path,
				 c_meta, c_reset);
	if (deleted)
		dump_quoted_path("+++ ", "", "/dev/null",
				 c_meta, c_reset);
	else
		dump_quoted_path("+++ ", b_prefix, elem->path,
				 c_meta, c_reset);
}

			    int dense, int working_tree_file,
			    struct rev_info *rev)
	struct userdiff_driver *userdiff;
	struct userdiff_driver *textconv = NULL;
	int is_binary;
	userdiff = userdiff_find_by_path(elem->path);
	if (!userdiff)
		userdiff = userdiff_find_by_name("default");
	if (DIFF_OPT_TST(opt, ALLOW_TEXTCONV))
		textconv = userdiff_get_textconv(userdiff);
		result = grab_blob(elem->sha1, elem->mode, &result_size,
				   textconv, elem->path);
				result = grab_blob(elem->sha1, elem->mode,
						   &result_size, NULL, NULL);
				result = grab_blob(sha1, elem->mode,
						   &result_size, NULL, NULL);
		} else if (textconv) {
			struct diff_filespec *df = alloc_filespec(elem->path);
			fill_filespec(df, null_sha1, st.st_mode);
			result_size = fill_textconv(textconv, df, &result);
			free_filespec(df);
	for (i = 0; i < num_parent; i++) {
		if (elem->parent[i].mode != elem->mode) {
			mode_differs = 1;
			break;
		}
	}

	if (textconv)
		is_binary = 0;
	else if (userdiff->binary != -1)
		is_binary = userdiff->binary;
	else {
		is_binary = buffer_is_binary(result, result_size);
		for (i = 0; !is_binary && i < num_parent; i++) {
			char *buf;
			unsigned long size;
			buf = grab_blob(elem->parent[i].sha1,
					elem->parent[i].mode,
					&size, NULL, NULL);
			if (buffer_is_binary(buf, size))
				is_binary = 1;
			free(buf);
		}
	}
	if (is_binary) {
		show_combined_header(elem, num_parent, dense, rev,
				     mode_differs, 0);
		printf("Binary files differ\n");
		free(result);
		return;
	}

				     cnt, i, num_parent, result_deleted,
				     textconv, elem->path);
		show_combined_header(elem, num_parent, dense, rev,
				     mode_differs, 1);
/*
 * The result (p->elem) is from the working tree and their
 * parents are typically from multiple stages during a merge
 * (i.e. diff-files) or the state in HEAD and in the index
 * (i.e. diff-index).
 */
		show_patch_diff(p, num_parent, dense, 1, rev);
}

static void free_combined_pair(struct diff_filepair *pair)
{
	free(pair->two);
	free(pair);
}

/*
 * A combine_diff_path expresses N parents on the LHS against 1 merge
 * result. Synthesize a diff_filepair that has N entries on the "one"
 * side and 1 entry on the "two" side.
 *
 * In the future, we might want to add more data to combine_diff_path
 * so that we can fill fields we are ignoring (most notably, size) here,
 * but currently nobody uses it, so this should suffice for now.
 */
static struct diff_filepair *combined_pair(struct combine_diff_path *p,
					   int num_parent)
{
	int i;
	struct diff_filepair *pair;
	struct diff_filespec *pool;

	pair = xmalloc(sizeof(*pair));
	pool = xcalloc(num_parent + 1, sizeof(struct diff_filespec));
	pair->one = pool + 1;
	pair->two = pool;

	for (i = 0; i < num_parent; i++) {
		pair->one[i].path = p->path;
		pair->one[i].mode = p->parent[i].mode;
		hashcpy(pair->one[i].sha1, p->parent[i].sha1);
		pair->one[i].sha1_valid = !is_null_sha1(p->parent[i].sha1);
		pair->one[i].has_more_entries = 1;
	}
	pair->one[num_parent - 1].has_more_entries = 0;

	pair->two->path = p->path;
	pair->two->mode = p->mode;
	hashcpy(pair->two->sha1, p->sha1);
	pair->two->sha1_valid = !is_null_sha1(p->sha1);
	return pair;
}

static void handle_combined_callback(struct diff_options *opt,
				     struct combine_diff_path *paths,
				     int num_parent,
				     int num_paths)
{
	struct combine_diff_path *p;
	struct diff_queue_struct q;
	int i;

	q.queue = xcalloc(num_paths, sizeof(struct diff_filepair *));
	q.alloc = num_paths;
	q.nr = num_paths;
	for (i = 0, p = paths; p; p = p->next) {
		if (!p->len)
			continue;
		q.queue[i++] = combined_pair(p, num_parent);
	}
	opt->format_callback(&q, opt, opt->format_callback_data);
	for (i = 0; i < num_paths; i++)
		free_combined_pair(q.queue[i]);
	free(q.queue);
		else if (opt->output_format & DIFF_FORMAT_CALLBACK)
			handle_combined_callback(opt, paths, num_parent, num_paths);

							0, rev);