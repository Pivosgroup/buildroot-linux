/*
 * Copyright (C) 2002-2005 Roman Zippel <zippel@linux-m68k.org>
 * Copyright (C) 2002-2005 Sam Ravnborg <sam@ravnborg.org>
 *
 * Released under the terms of the GNU GPL v2.0.
 */

#include <string.h>
#include "lkc.h"

/* file already present in list? If not add it */
struct file *file_lookup(const char *name)
{
	struct file *file;
	const char *file_name = sym_expand_string_value(name);

	for (file = file_list; file; file = file->next) {
		if (!strcmp(name, file->name)) {
			free((void *)file_name);
			return file;
		}
	}

	file = malloc(sizeof(*file));
	memset(file, 0, sizeof(*file));
	file->name = file_name;
	file->next = file_list;
	file_list = file;
	return file;
}

static char* br2_symbol_printer(const char * const in)
{
	ssize_t i, j, len = strlen(in);
	char *ret;
	if (len < 1)
		return NULL;
	ret = malloc(len+1);
	if (!ret) {
		printf("Out of memory!");
		exit(1);
	}
	memset(ret, 0, len+1);
	i = j = 0;
	if (strncmp("BR2_", in, 4) == 0)
		i += 4;
	if (strncmp("PACKAGE_", in + i, 8) == 0)
		i += 8;
	else if (strncmp("TARGET_", in + i, 7) == 0)
		i += 7;
	while (i <= len)
		ret[j++] = tolower(in[i++]);
	return ret;
}

/* write dependencies of the individual config-symbols */
static int write_make_deps(const char *name)
{
	char *str;
	char dir[PATH_MAX+1], buf[PATH_MAX+1], buf2[PATH_MAX+1];
	struct menu *menu;
	struct symbol *sym;
	struct property *prop, *p;
	unsigned done;
	const char * const name_tmp = "..make.deps.tmp";
	FILE *out;
	if (!name)
		name = ".auto.deps";

	strcpy(dir, conf_get_configname());
	str = strrchr(dir, '/');
	if (str)
		str[1] = 0;
	else
		dir[0] = 0;

	sprintf(buf, "%s%s", dir, name_tmp);
	out = fopen(buf, "w");
	if (!out)
		return 1;
	fprintf(out, "# ATTENTION! This does not handle 'depends', just 'select'! \n"
		"# See support/kconfig/util.c write_make_deps()\n#\n");
	menu = &rootmenu;//rootmenu.list;
	while (menu) {
		sym = menu->sym;
		if (!sym) {
			if (!menu_is_visible(menu))
				goto next;
		} else if (!(sym->flags & SYMBOL_CHOICE)) {
			sym_calc_value(sym);
			if (sym->type == S_BOOLEAN
			    && sym_get_tristate_value(sym) != no) {
			    done = 0;
			    for_all_prompts(sym, prop) {
			        struct expr *e;
//printf("\nname=%s\n", sym->name);
			        for_all_properties(sym, p, P_SELECT) {
				    e = p->expr;
				    if (e && e->left.sym->name) {
				        if (!done) {
					    fprintf(out, "%s: $(BASE_TARGETS)", br2_symbol_printer(sym->name));
					    done = 1;
					}
//printf("SELECTS %s\n",e->left.sym->name);
					fprintf(out, " %s",br2_symbol_printer(e->left.sym->name));
				    }
				}
				if (done)
				    fprintf(out, "\n");
#if 0
				e = sym->rev_dep.expr;
				if (e && e->type == E_SYMBOL
					&& e->left.sym->name) {
				    fprintf(out, "%s: %s", br2_symbol_printer(e->left.sym->name),
						br2_symbol_printer(sym->name));
printf("%s is Selected BY: %s", sym->name, e->left.sym->name);
				}
#endif
			    }
			}
		}
next:
		if (menu->list) {
			menu = menu->list;
			continue;
		}
		if (menu->next)
			menu = menu->next;
		else while ((menu = menu->parent)) {
			if (menu->next) {
				menu = menu->next;
				break;
			}
		}
	}
	fclose(out);
	sprintf(buf2, "%s%s", dir, name);
	rename(buf, buf2);
	printf(_("#\n"
		 "# make dependencies written to %s\n"
		 "# ATTENTION buildroot devels!\n"
		 "# See top of this file before playing with this auto-preprequisites!\n"
		 "#\n"), name);
	return 0;
}

/* write a dependency file as used by kbuild to track dependencies */
int file_write_dep(const char *name)
{
	char *str;
	char buf[PATH_MAX+1], buf2[PATH_MAX+1], dir[PATH_MAX+1];
	struct symbol *sym, *env_sym;
	struct expr *e;
	struct file *file;
	FILE *out;

	if (!name)
		name = ".kconfig.d";

	strcpy(dir, conf_get_configname());
	str = strrchr(dir, '/');
	if (str)
		str[1] = 0;
	else
		dir[0] = 0;

	sprintf(buf, "%s..config.tmp", dir);
	out = fopen(buf, "w");
	if (!out)
		return 1;
	fprintf(out, "deps_config := \\\n");
	for (file = file_list; file; file = file->next) {
		if (file->next)
			fprintf(out, "\t%s \\\n", file->name);
		else
			fprintf(out, "\t%s\n", file->name);
	}
	fprintf(out, "\n%s: \\\n"
		     "\t$(deps_config)\n\n", conf_get_autoconfig_name());

	expr_list_for_each_sym(sym_env_list, e, sym) {
		struct property *prop;
		const char *value;

		prop = sym_get_env_prop(sym);
		env_sym = prop_get_symbol(prop);
		if (!env_sym)
			continue;
		value = getenv(env_sym->name);
		if (!value)
			value = "";
		fprintf(out, "ifneq \"$(%s)\" \"%s\"\n", env_sym->name, value);
		fprintf(out, "%s: FORCE\n", conf_get_autoconfig_name());
		fprintf(out, "endif\n");
	}

	fprintf(out, "\n$(deps_config): ;\n");
	fclose(out);
	sprintf(buf2, "%s%s", dir, name);
	rename(buf, buf2);
	return write_make_deps(NULL);
}


/* Allocate initial growable string */
struct gstr str_new(void)
{
	struct gstr gs;
	gs.s = malloc(sizeof(char) * 64);
	gs.len = 64;
	gs.max_width = 0;
	strcpy(gs.s, "\0");
	return gs;
}

/* Allocate and assign growable string */
struct gstr str_assign(const char *s)
{
	struct gstr gs;
	gs.s = strdup(s);
	gs.len = strlen(s) + 1;
	gs.max_width = 0;
	return gs;
}

/* Free storage for growable string */
void str_free(struct gstr *gs)
{
	if (gs->s)
		free(gs->s);
	gs->s = NULL;
	gs->len = 0;
}

/* Append to growable string */
void str_append(struct gstr *gs, const char *s)
{
	size_t l;
	if (s) {
		l = strlen(gs->s) + strlen(s) + 1;
		if (l > gs->len) {
			gs->s   = realloc(gs->s, l);
			gs->len = l;
		}
		strcat(gs->s, s);
	}
}

/* Append printf formatted string to growable string */
void str_printf(struct gstr *gs, const char *fmt, ...)
{
	va_list ap;
	char s[10000]; /* big enough... */
	va_start(ap, fmt);
	vsnprintf(s, sizeof(s), fmt, ap);
	str_append(gs, s);
	va_end(ap);
}

/* Retrieve value of growable string */
const char *str_get(struct gstr *gs)
{
	return gs->s;
}

