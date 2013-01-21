/* load.c
 *
 * This file is part of kbd project.
 * Copyright (C) 2012  Alexey Gladkov <gladkov.alexey@gmail.com>
 *
 * This file is covered by the GNU General Public License,
 * which should be included with kbd as the file COPYING.
 */
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>

#include "nls.h"
#include "keymap.h"
#include "keymapP.h"

int
get_keys(struct keymap *kmap, int fd)
{
	int i, t;
	struct kbentry ke;

	for (t = 0; t < MAX_NR_KEYMAPS; t++) {
		for (i = 0; i < NR_KEYS; i++) {
			ke.kb_table = t;
			ke.kb_index = i;
			ke.kb_value = 0;

			if (ioctl(fd, KDGKBENT, (unsigned long) &ke)) {
				log_error(kmap, _("KDGKBENT: %s: error at index %d in table %d"),
					strerror(errno), i, t);
				return -1;
			}

			if (addkey(kmap, i, t, ke.kb_value) < 0)
				return -1;
		}
	}

	if (do_constant(kmap) < 0)
		return -1;

	return 0;
}

int
get_funcs(struct keymap *kmap, int fd)
{
	int i;
	struct kbsentry kbs;

	for (i = 0; i < MAX_NR_FUNC; i++) {
		kbs.kb_func = i;

		if (ioctl(fd, KDGKBSENT, (unsigned long) &kbs)) {
			log_error(kmap, _("KDGKBSENT: %s: Unable to get function key string"),
				strerror(errno));
			return -1;
		}

		if (!strlen(kbs.kb_string))
			continue;

		if (addfunc(kmap, kbs) < 0)
			return -1;
	}

	return 0;
}

int
get_diacrs(struct keymap *kmap, int fd)
{
#ifdef KDGKBDIACRUC
	int request = KDGKBDIACRUC;
	struct kbdiacrsuc kd;
	struct kbdiacruc *ar = kd.kbdiacruc;
#else
	int request = KDGKBDIACR;
	struct kbdiacrs kd;
	struct kbdiacr *ar = kd.kbdiacr;
#endif
	int i;

	if (ioctl(fd, request, (unsigned long) &kd)) {
		log_error(kmap, _("KDGKBDIACR(UC): %s: Unable to get accent table"),
			strerror(errno));
		return -1;
	}

	for (i = 0; i < kd.kb_cnt; i++) {
		if (compose(kmap, (ar+i)->diacr, (ar+i)->base, (ar+i)->result) < 0)
			return -1;
	}

	return 0;
}

int
get_keymap(struct keymap *kmap, int fd)
{
	if (get_keys(kmap, fd)   < 0 ||
	    get_funcs(kmap, fd)  < 0 ||
	    get_diacrs(kmap, fd) < 0)
		return -1;
	return 0;
}
