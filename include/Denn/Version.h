#pragma once
#define VALUE_TO_STR(s) #s
#define DEFINE_TO_STR(s) VALUE_TO_STR(s)

#define DENN_VER_MAJOR   2
#define DENN_VER_MINOR   2
#define DENN_VER_PATCH   0

#define DENN_VER_STR \
		DEFINE_TO_STR(DENN_VER_MAJOR) "." \
		DEFINE_TO_STR(DENN_VER_MINOR) "." \
		DEFINE_TO_STR(DENN_VER_PATCH)

