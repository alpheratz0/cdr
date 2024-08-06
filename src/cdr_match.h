#pragma once

#include "cdr_config.h"
#include "cdr_ref.h"

typedef struct {
    int start;
    int end;
} cdr_range;

typedef struct {
    int current;
    int next_match;
    cdr_range matches[2];
} cdr_next_data;

int
cdr_next_verse(const cdr_ref *ref, const cdr_config *config, cdr_next_data *next);
