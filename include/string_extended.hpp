#pragma once
#include <cstdlib>
#include <cstring>
static inline int strchrset(char* str, char target, char replacement) {
	char *target_ptr = strchr(str, target);
	if (!target_ptr) {
		return EXIT_FAILURE;
	}
	*target_ptr = replacement;
	return EXIT_SUCCESS;
}
